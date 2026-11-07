// SPDX-License-Identifier: GPL-3.0-or-later
#include "memoryusage.h"
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <string>
#include <sys/resource.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#ifdef __linux__
#include <sys/prctl.h>
#endif
using namespace Lemon::Execution;
namespace {
	enum { RS_AC, RS_FAIL, RS_RE, RS_TLE, RS_MLE };
	volatile sig_atomic_t stopped = 0;
	void stop(int) { stopped = 1; }
	class UnixPrelimit : public Prelimit {
	  public:
		bool apply(Bytes limit) override {
#ifdef __linux__
			if (limit >= 0) {
				rlimit memory{static_cast<rlim_t>(limit), static_cast<rlim_t>(limit)};
				if (setrlimit(RLIMIT_AS, &memory) != 0)
					return false;
				rlimit stack{};
				if (getrlimit(RLIMIT_STACK, &stack) != 0)
					return false;
				stack.rlim_cur = std::min(stack.rlim_max, static_cast<rlim_t>(limit));
				if (setrlimit(RLIMIT_STACK, &stack) != 0)
					return false;
			}
#else
			(void)limit; // macOS intentionally has no memory prelimit.
#endif
			return true;
		}
	};
	class UnixUsage : public UsageProvider {
		pid_t pid;

	  public:
		explicit UnixUsage(pid_t pid) : pid(pid) {}
		MemorySample sample() override { return sampleMemory(pid); }
	};
	class UnixProcess : public Killer {
	  public:
		pid_t pid;
		int status = 0;
		rusage usage{};
		bool reaped = false;
		bool cleaned = false;
		explicit UnixProcess(pid_t pid) : pid(pid) {}
		~UnixProcess() override {
			if (! cleaned)
				killAndReap();
		}
		// Do not destroy the exit information until all final samples are consumed.
		int poll() {
			if (reaped)
				return 1;
			pid_t result = wait4(pid, &status, WNOHANG, &usage);
			if (result == pid) {
				reaped = true;
				return 1;
			}
			if (result < 0 && errno != EINTR)
				return -1;
			return 0;
		}
		bool killAndReap() override {
			if (cleaned)
				return reaped;
			cleaned = true;
			kill(-pid, SIGKILL);
			if (! reaped)
				kill(pid, SIGKILL);
			auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
			while (! reaped && std::chrono::steady_clock::now() < deadline) {
				if (poll() < 0)
					return false;
				if (! reaped)
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			return reaped;
		}
	};
	struct ChildError {
		int stage;
		int error;
	};
} // namespace
int main(int argc, char **argv) {
	if (argc != 12) {
		printf("-1\n-1\n");
		return RS_FAIL;
	}
	try {
		long long timeLimit = std::stoll(argv[6]);
		Bytes limit;
		if (! memoryBytes(std::stoll(argv[7]), limit) || timeLimit < 0 || timeLimit > 86400000LL) {
			printf("-1\n-1\n");
			return RS_FAIL;
		}
		int errors[2];
		if (pipe(errors) != 0) {
			perror("pipe");
			printf("-1\n-1\n");
			return RS_FAIL;
		}
		if (fcntl(errors[1], F_SETFD, FD_CLOEXEC) < 0 || fcntl(errors[0], F_SETFL, O_NONBLOCK) < 0) {
			perror("fcntl");
			close(errors[0]);
			close(errors[1]);
			printf("-1\n-1\n");
			return RS_FAIL;
		}
		pid_t pid = fork();
		if (pid < 0) {
			perror("fork");
			close(errors[0]);
			close(errors[1]);
			printf("-1\n-1\n");
			return RS_FAIL;
		}
		if (pid == 0) {
			close(errors[0]);
			auto fail = [&](int stage) {
				ChildError error{stage, errno};
				(void)write(errors[1], &error, sizeof(error));
				_exit(127);
			};
			if (setpgid(0, 0) != 0)
				fail(1);
			const char *modes[] = {"r", "w", "w"};
			FILE *streams[] = {stdin, stdout, stderr};
			for (int i = 0; i < 3; ++i)
				if (! freopen(*argv[3 + i] ? argv[3 + i] : "/dev/null", modes[i], streams[i]))
					fail(2);
#ifdef __linux__
			// Disable dump generation, including piped core handlers.
			if (prctl(PR_SET_DUMPABLE, 0) != 0)
				fail(6);
#endif
			rlimit core{0, 0};
			if (setrlimit(RLIMIT_CORE, &core) != 0)
				fail(7);
			UnixPrelimit prelimit;
			if (! prelimit.apply(limit))
				fail(3);
			rlim_t seconds = static_cast<rlim_t>((timeLimit + 999) / 1000);
			rlimit cpu{seconds, seconds + 1};
			if (setrlimit(RLIMIT_CPU, &cpu) != 0)
				fail(4);
			// The shell replaces itself, so pid continues to identify the submission.
			std::string command = "exec \"" + std::string(argv[1]) + "\" " + argv[2];
			execlp("bash", "bash", "-c", command.c_str(), static_cast<char *>(nullptr));
			fail(5);
		}
		close(errors[1]);
		(void)setpgid(pid, pid);
		signal(SIGTERM, stop);
		signal(SIGINT, stop);
		signal(SIGABRT, stop);
		UnixProcess process(pid);
		UnixUsage provider(pid);
		Limiter limiter(limit);
		auto deadline =
		    std::chrono::steady_clock::now() + std::chrono::milliseconds(std::max(2000LL, 3LL * timeLimit));
		int verdict = RS_AC;
		int missing = 0;
		ChildError childError{};
		bool setupFailed = false;
		for (;;) {
			auto sample = provider.sample();
			limiter.observe(sample);
			// The fixed-size read is bounded by the destination object on every iteration.
			/* Flawfinder: ignore */
			if (read(errors[0], &childError, sizeof(childError)) == sizeof(childError))
				setupFailed = true;
			int finished = process.poll();
			if (setupFailed || finished < 0) {
				verdict = RS_FAIL;
				break;
			}
			if (limiter.isExceeded()) {
				verdict = RS_MLE;
				break;
			}
			if (finished)
				break;
			if (std::chrono::steady_clock::now() >= deadline) {
				verdict = RS_TLE;
				break;
			}
			if (stopped) {
				verdict = RS_FAIL;
				break;
			}
			missing = sample.declared < 0 || sample.actual < 0 ? missing + 1 : 0;
			if (missing >= 10) {
				fprintf(stderr, "Memory statistics unavailable\n");
				verdict = RS_FAIL;
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		// Catch setup errors delivered concurrently with the final wait.
		// The final read uses the same destination-size bound.
		/* Flawfinder: ignore */
		if (read(errors[0], &childError, sizeof(childError)) == sizeof(childError))
			setupFailed = true;
		close(errors[0]);
		if (! process.killAndReap())
			fprintf(stderr, "Process reaping deadline exceeded\n");
		if (setupFailed) {
			fprintf(stderr, "Child setup stage %d failed: errno %d\n", childError.stage, childError.error);
			verdict = RS_FAIL;
		}
		if (process.reaped) {
			Bytes rss = maxRSSBytes(process.usage.ru_maxrss);
#ifdef __APPLE__
			limiter.observe({rss, rss});
#else
			limiter.observe({-1, rss});
#endif
			if (verdict == RS_AC) {
				ExitKind exit = ExitKind::Normal;
				if (WIFSIGNALED(process.status))
					exit = WTERMSIG(process.status) == SIGXCPU ? ExitKind::TimeLimit : ExitKind::RuntimeError;
				else if (! WIFEXITED(process.status) || WEXITSTATUS(process.status) != 0)
					exit = ExitKind::RuntimeError;
				switch (limiter.finish(exit).reason) {
					case RunReason::MemoryLimit:
						verdict = RS_MLE;
						break;
					case RunReason::TimeLimit:
						verdict = RS_TLE;
						break;
					case RunReason::RuntimeError:
						verdict = RS_RE;
						break;
					case RunReason::InternalError:
						verdict = RS_FAIL;
						break;
					case RunReason::Accepted:
						break;
				}
			}
		}
		long long elapsed =
		    process.reaped ? process.usage.ru_utime.tv_sec * 1000LL + process.usage.ru_utime.tv_usec / 1000
		                   : -1;
		if (verdict == RS_AC && elapsed > timeLimit)
			verdict = RS_TLE;
		printf("%lld\n%lld\n", elapsed, static_cast<long long>(limiter.usage()));
		return verdict;
	} catch (...) {
		printf("-1\n-1\n");
		fprintf(stderr, "Invalid watcher arguments\n");
		return RS_FAIL;
	}
}
