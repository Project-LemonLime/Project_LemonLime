// SPDX-License-Identifier: GPL-3.0-or-later
#include "memoryusage.h"
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <string>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef __linux__
#include <poll.h>
#include <sys/prctl.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>
#elif defined(__APPLE__)
#include <sys/event.h>
#endif
using namespace Lemon::Execution;
namespace {
	enum { RS_AC, RS_FAIL, RS_RE, RS_TLE, RS_MLE };
	class Events {
		int signals[4] = {SIGCHLD, SIGTERM, SIGINT, SIGABRT};
		sigset_t previous{};
		bool masked = false;
#ifdef __linux__
		int descriptors[3] = {-1, -1, -1};
#else
		int queue = -1;
		struct sigaction actions[4]{};
		int installed = 0;
		static void ignore(int) {}
#endif
	  public:
		enum { Sample = 1, Deadline = 2, Child = 4, Stop = 8 };
		Events() = default;
		Events(const Events &) = delete;
		Events &operator=(const Events &) = delete;
		~Events() { restore(); }
		void restore() {
#ifdef __linux__
			for (int &fd : descriptors) {
				if (fd >= 0)
					close(fd);
				fd = -1;
			}
#else
			if (queue >= 0)
				close(queue);
			queue = -1;
			while (installed) {
				--installed;
				sigaction(signals[installed], &actions[installed], nullptr);
			}
#endif
			if (masked)
				sigprocmask(SIG_SETMASK, &previous, nullptr);
			masked = false;
		}
		bool initialize() {
			sigset_t mask;
			sigemptyset(&mask);
			for (int signal : signals)
				sigaddset(&mask, signal);
			if (sigprocmask(SIG_BLOCK, &mask, &previous) != 0)
				return false;
			masked = true;
#ifdef __linux__
			descriptors[0] = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
			descriptors[1] = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
			descriptors[2] = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
			return descriptors[0] >= 0 && descriptors[1] >= 0 && descriptors[2] >= 0;
#else
			queue = kqueue();
			if (queue < 0 || fcntl(queue, F_SETFD, FD_CLOEXEC) < 0)
				return false;
			for (int signal : signals) {
				struct sigaction action{};
				action.sa_handler = ignore;
				sigemptyset(&action.sa_mask);
				if (sigaction(signal, &action, &actions[installed]) != 0)
					return false;
				++installed;
				struct kevent event{};
				EV_SET(&event, signal, EVFILT_SIGNAL, EV_ADD, 0, 0, nullptr);
				if (kevent(queue, &event, 1, nullptr, 0, nullptr) < 0)
					return false;
			}
			return sigprocmask(SIG_SETMASK, &previous, nullptr) == 0;
#endif
		}
		bool arm(long long milliseconds, bool sampling) {
#ifdef __linux__
			for (int i = 0; i < 2; ++i) {
				long long delay = i == 0 ? milliseconds : (sampling ? 10 : 0);
				itimerspec spec{};
				spec.it_value = {delay / 1000, (delay % 1000) * 1000000};
				if (i == 1)
					spec.it_interval = spec.it_value;
				if (timerfd_settime(descriptors[i], 0, &spec, nullptr) < 0)
					return false;
			}
			return true;
#else
			struct kevent changes[2]{};
			EV_SET(&changes[0], Deadline, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, milliseconds, nullptr);
			EV_SET(&changes[1], Sample, EVFILT_TIMER, EV_ADD | (sampling ? EV_ENABLE : EV_DISABLE), 0, 10,
			       nullptr);
			return kevent(queue, changes, 2, nullptr, 0, nullptr) == 0;
#endif
		}
		int wait() {
			int events = 0;
#ifdef __linux__
			pollfd ready[3] = {
			    {descriptors[0], POLLIN, 0}, {descriptors[1], POLLIN, 0}, {descriptors[2], POLLIN, 0}};
			if (::poll(ready, 3, -1) < 0)
				return errno == EINTR ? 0 : -1;
			for (int i = 0; i < 3; ++i) {
				if (ready[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
					errno = EIO;
					return -1;
				}
				if (! (ready[i].revents & POLLIN))
					continue;
				if (i < 2) {
					uint64_t count;
					if (read(descriptors[i], &count, sizeof(count)) != sizeof(count))
						return -1;
					events |= i == 0 ? Deadline : Sample;
				} else {
					signalfd_siginfo info;
					while (read(descriptors[2], &info, sizeof(info)) == sizeof(info))
						events |= info.ssi_signo == SIGCHLD ? Child : Stop;
					if (errno != EAGAIN && errno != EINTR)
						return -1;
				}
			}
#else
			struct kevent ready[6]{};
			int count = kevent(queue, nullptr, 0, ready, 6, nullptr);
			if (count < 0)
				return errno == EINTR ? 0 : -1;
			for (int i = 0; i < count; ++i) {
				if (ready[i].flags & EV_ERROR) {
					errno = ready[i].data;
					return -1;
				}
				events |= ready[i].filter == EVFILT_TIMER ? ready[i].ident
				                                          : (ready[i].ident == SIGCHLD ? Child : Stop);
			}
#endif
			return events;
		}
	};
	class UnixProcess {
	  public:
		pid_t pid;
		int status = 0;
		rusage usage{};
		bool reaped = false;
		bool cleaned = false;
		Events &events;
		explicit UnixProcess(pid_t pid, Events &events) : pid(pid), events(events) {}
		~UnixProcess() {
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
		bool killAndReap() {
			if (cleaned)
				return reaped;
			cleaned = true;
			kill(-pid, SIGKILL);
			if (! reaped)
				kill(pid, SIGKILL);
			if (reaped)
				return true;
			if (! events.arm(2000, false)) {
				perror("reaping events");
				return false;
			}
			for (;;) {
				if (poll() < 0)
					return false;
				if (reaped)
					break;
				int event = events.wait();
				if (event < 0) {
					perror("reaping wait");
					return false;
				}
				if (event & Events::Deadline) {
					if (poll() < 0)
						return false;
					break;
				}
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
		Events events;
		if (! events.initialize()) {
			perror("events");
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
			events.restore();
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
			if (! applyMemoryLimit(limit))
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
		UnixProcess process(pid, events);
		Limiter limiter(limit);
		bool ready = events.arm(std::max(2000LL, 3LL * timeLimit), true);
		if (! ready)
			perror("events arm");
		int event = Events::Sample;
		int verdict = RS_AC;
		int missing = 0;
		ChildError childError{};
		bool setupFailed = false;
		for (;;) {
			if (! ready) {
				verdict = RS_FAIL;
				break;
			}
			auto sample = event & Events::Sample ? sampleMemory(pid) : Bytes(-1);
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
			if (event & Events::Deadline) {
				verdict = RS_TLE;
				break;
			}
			if (event & Events::Stop) {
				verdict = RS_FAIL;
				break;
			}
			if (event & Events::Sample)
				missing = sample < 0 ? missing + 1 : 0;
			if (missing >= 10) {
				fprintf(stderr, "Memory statistics unavailable\n");
				verdict = RS_FAIL;
				break;
			}
			event = events.wait();
			if (event < 0) {
				perror("events wait");
				verdict = RS_FAIL;
				break;
			}
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
#ifdef __APPLE__
			// Only macOS defines DeclaredUsage as RSS. Linux must not use RSS as a VSS fallback.
			limiter.observe(static_cast<Bytes>(process.usage.ru_maxrss));
#endif
			if (verdict == RS_AC) {
				if (limiter.isExceeded())
					verdict = RS_MLE;
				else if (WIFSIGNALED(process.status))
					verdict = WTERMSIG(process.status) == SIGXCPU ? RS_TLE : RS_RE;
				else if (! WIFEXITED(process.status) || WEXITSTATUS(process.status) != 0)
					verdict = RS_RE;
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
