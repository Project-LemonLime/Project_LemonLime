/*
 * SPDX-FileCopyrightText: 2011-2019 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2019-2023 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "memoryusage.h"
#include <algorithm>
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
#include <map>
#include <cstddef>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <sys/ptrace.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/timerfd.h>
#elif defined(__APPLE__)
#include <sys/event.h>
#endif
using namespace Lemon::Execution;
namespace {
#ifdef __linux__
	bool installExecTrace() {
		sock_filter filter[] = {
		    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(seccomp_data, arch)),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_X86_64, 6 - 1 - 1, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_I386, 17 - 2 - 1, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_AARCH64, 22 - 3 - 1, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, AUDIT_ARCH_ARM, 27 - 4 - 1, 0),
#if defined(__x86_64__) || defined(__i386__) || defined(__aarch64__) || defined(__arm__)
		    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
#else
		    // Other builds retain native SYS numbers, not guessed compat ABIs.
		    BPF_STMT(BPF_JMP | BPF_JA, 32 - 5 - 1),
#endif
		    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(seccomp_data, nr)),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 59, 8, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 322, 7, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0x40000208, 6, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0x40000221, 5, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 520, 4, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 545, 3, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0x4000003b, 2, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 0x40000142, 1, 0),
		    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
		    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRACE),
		    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(seccomp_data, nr)),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 11, 2, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 358, 1, 0),
		    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
		    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRACE),
		    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(seccomp_data, nr)),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 221, 2, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 281, 1, 0),
		    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
		    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRACE),
		    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(seccomp_data, nr)),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 11, 2, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, 387, 1, 0),
		    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
		    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRACE),
#if !defined(__x86_64__) && !defined(__i386__) && !defined(__aarch64__) && !defined(__arm__)
		    BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(seccomp_data, nr)),
#ifdef SYS_execveat
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_execve, 2, 0),
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_execveat, 1, 0),
#else
		    BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SYS_execve, 1, 0),
#endif
		    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
		    BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRACE),
#endif
		};
		sock_fprog program{static_cast<unsigned short>(sizeof(filter) / sizeof(filter[0])), filter};
		// Inherited NNP forbids exec privilege gains; only exec syscalls notify the tracer.
		return prctl(PR_SET_NO_NEW_PRIVS, 1L, 0L, 0L, 0L) == 0 &&
		       prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &program, 0L, 0L) == 0;
	}
#endif
	enum { RS_AC, RS_FAIL, RS_RE, RS_TLE, RS_MLE };
	class Events {
		int signals[4] = {SIGCHLD, SIGTERM, SIGINT, SIGABRT};
		sigset_t previous{};
		bool masked = false;
#ifdef __linux__
		int descriptors[2] = {-1, -1};
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
			descriptors[1] = signalfd(-1, &mask, SFD_CLOEXEC | SFD_NONBLOCK);
			return descriptors[0] >= 0 && descriptors[1] >= 0;
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
			(void)sampling;
			itimerspec spec{};
			spec.it_value = {milliseconds / 1000, (milliseconds % 1000) * 1000000};
			return timerfd_settime(descriptors[0], 0, &spec, nullptr) == 0;
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
			pollfd ready[2] = {{descriptors[0], POLLIN, 0}, {descriptors[1], POLLIN, 0}};
			if (::poll(ready, 2, -1) < 0)
				return errno == EINTR ? 0 : -1;
			for (int i = 0; i < 2; ++i) {
				if (ready[i].revents & (POLLERR | POLLHUP | POLLNVAL)) {
					errno = EIO;
					return -1;
				}
				if (! (ready[i].revents & POLLIN))
					continue;
				if (i == 0) {
					uint64_t count;
					if (read(descriptors[i], &count, sizeof(count)) != sizeof(count))
						return -1;
					events |= Deadline;
				} else {
					signalfd_siginfo info;
					while (read(descriptors[1], &info, sizeof(info)) == sizeof(info))
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
		Bytes peak = -1;
#ifdef __linux__
		std::map<pid_t, pid_t> traced; // tid -> tgid, zero until identified.
		bool drained = false;
#endif
		explicit UnixProcess(pid_t pid, Events &events) : pid(pid), events(events) {
#ifdef __linux__
			traced.emplace(pid, pid);
#endif
		}
		~UnixProcess() { killAndReap(); }
		bool finished() const { return reaped; }
		bool allReaped() const {
#ifdef __linux__
			return drained && traced.empty() && reaped;
#else
			return reaped;
#endif
		}
#ifdef __linux__
		void terminateTracee(pid_t tid, pid_t group) {
			if (group)
				(void)syscall(SYS_tgkill, group, tid, SIGKILL);
			else
				(void)syscall(SYS_tkill, tid, SIGKILL);
		}
#endif
		int poll(bool cleanup = false) {
			if (! cleanup && finished())
				return 1;
#ifdef __linux__
			for (;;) {
				int state = 0;
				rusage sample{};
				const pid_t tid = wait4(-1, &state, WNOHANG | __WALL, &sample);
				if (tid < 0) {
					if (errno == EINTR)
						continue;
					if (errno == ECHILD && traced.empty() && reaped) {
						drained = true;
						return 1;
					}
					return -1;
				}
				if (! tid)
					return ! cleanup && finished() ? 1 : 0;
				if (WIFEXITED(state) || WIFSIGNALED(state)) {
					traced.erase(tid);
					if (tid == pid) {
						status = state;
						usage = sample;
						reaped = true;
						if (! cleanup)
							return 1;
					}
					continue;
				}
				if (! WIFSTOPPED(state))
					continue;
				auto [it, newborn] = traced.try_emplace(tid, 0);
				unsigned event = static_cast<unsigned>(state) >> 16;
				int signal = WSTOPSIG(state);
				if (! it->second) {
					int group = 0;
					const auto path = "/proc/" + std::to_string(tid) + "/status";
					FILE *file = fopen(path.c_str(), "r");
					if (file) {
						char line[512];
						while (fgets(line, sizeof(line), file))
							if (sscanf(line, "Tgid: %d", &group) == 1)
								break;
						fclose(file);
					}
					if (group <= 0 && ! cleanup)
						return -1;
					it->second = group > 0 ? group : 0;
				}
				if ((event == PTRACE_EVENT_SECCOMP || event == PTRACE_EVENT_EXIT) && it->second == pid) {
					const auto memory = sampleMemory(tid);
					if (memory < 0 && ! cleanup)
						return -1;
					peak = std::max(peak, memory);
				}
				if (event == PTRACE_EVENT_EXEC) {
					unsigned long oldTid = 0;
					if (ptrace(PTRACE_GETEVENTMSG, tid, nullptr, &oldTid) < 0 && ! cleanup)
						return -1;
					// Exec replaces this thread group, including its former leader.
					for (auto entry = traced.begin(); entry != traced.end();) {
						if (entry->second == tid || entry->first == static_cast<pid_t>(oldTid))
							entry = traced.erase(entry);
						else
							++entry;
					}
					traced[tid] = tid;
				}
				if (cleanup) {
					terminateTracee(tid, traced.at(tid));
					signal = 0;
				} else if (event == PTRACE_EVENT_STOP && ! newborn &&
				           (signal == SIGSTOP || signal == SIGTSTP || signal == SIGTTIN || signal == SIGTTOU)) {
					if (ptrace(PTRACE_LISTEN, tid, nullptr, nullptr) < 0 && errno != ESRCH)
						return -1;
					continue;
				} else if (event)
					signal = 0;
				if (ptrace(PTRACE_CONT, tid, nullptr, signal) < 0 && errno != ESRCH)
					return -1;
			}
#else
			pid_t result = wait4(pid, &status, WNOHANG, &usage);
			if (result == pid) {
				reaped = true;
				return 1;
			}
			if (result < 0 && errno != EINTR)
				return -1;
			return 0;
#endif
		}
		bool killAndReap() {
			if (cleaned)
				return allReaped();
			cleaned = true;
			kill(-pid, SIGKILL);
			if (! reaped)
				kill(pid, SIGKILL);
#ifdef __linux__
			for (const auto &[tid, group] : traced)
				terminateTracee(tid, group);
#endif
			if (allReaped())
				return true;
			if (! events.arm(2000, false)) {
				perror("reaping events");
				return false;
			}
			for (;;) {
				if (poll(true) < 0)
					return false;
				if (allReaped())
					break;
				int event = events.wait();
				if (event < 0) {
					perror("reaping wait");
					return false;
				}
				if (event & Events::Deadline) {
					if (poll(true) < 0)
						return false;
					break;
				}
			}
			return allReaped();
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
#ifdef __linux__
		int startup[2];
		if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, startup) != 0) {
			perror("socketpair");
			close(errors[0]);
			close(errors[1]);
			printf("-1\n-1\n");
			return RS_FAIL;
		}
#endif
		pid_t pid = fork();
		if (pid < 0) {
			perror("fork");
#ifdef __linux__
			close(startup[0]);
			close(startup[1]);
#endif
			close(errors[0]);
			close(errors[1]);
			printf("-1\n-1\n");
			return RS_FAIL;
		}
		if (pid == 0) {
#ifdef __linux__
			close(startup[0]);
#endif
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
			char token = 0;
			ssize_t received;
			do {
				received = recv(startup[1], &token, 1, 0);
			} while (received < 0 && errno == EINTR);
			if (received != 1 || token != 1) {
				if (received >= 0)
					errno = EPROTO;
				fail(8);
			}
			close(startup[1]);
			if (! installExecTrace())
				fail(9);
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
#ifdef __linux__
		close(startup[1]);
#endif
		(void)setpgid(pid, pid);
		UnixProcess process(pid, events);
		Bytes &peak = process.peak;
		bool ready = events.arm(std::max(2000LL, 3LL * timeLimit), true);
		if (! ready)
			perror("events arm");
#ifdef __linux__
		if (ready && ptrace(PTRACE_SEIZE, pid, nullptr,
		                    PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK | PTRACE_O_TRACECLONE |
		                        PTRACE_O_TRACEEXEC | PTRACE_O_TRACEEXIT | PTRACE_O_TRACESECCOMP |
		                        PTRACE_O_EXITKILL) < 0) {
			perror("ptrace seize");
			ready = false;
		}
		if (ready) {
			const char token = 1;
			ssize_t sent;
			do {
				sent = send(startup[0], &token, 1, MSG_NOSIGNAL);
			} while (sent < 0 && errno == EINTR);
			if (sent != 1) {
				perror("startup send");
				ready = false;
			}
		}
		close(startup[0]);
#endif
		int event = 0;
		int verdict = RS_AC;

#ifdef __APPLE__
		event = Events::Sample;
		int missing = 0;
#endif
		ChildError childError{};
		bool setupFailed = false;
		for (;;) {
			if (! ready) {
				verdict = RS_FAIL;
				break;
			}
#ifdef __APPLE__
			auto sample = event & Events::Sample ? sampleMemory(pid) : Bytes(-1);
			peak = std::max(peak, sample);
#endif
			/* Flawfinder: ignore */
			if (read(errors[0], &childError, sizeof(childError)) == sizeof(childError))
				setupFailed = true;
			int finished = process.poll();
			if (setupFailed || finished < 0) {
				verdict = RS_FAIL;
				break;
			}
			if (limit >= 0 && peak > limit) {
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
#ifdef __APPLE__
			if (event & Events::Sample)
				missing = sample < 0 ? missing + 1 : 0;
			if (missing >= 10) {
				fprintf(stderr, "Memory statistics unavailable\n");
				verdict = RS_FAIL;
				break;
			}
#endif
			event = events.wait();
			if (event < 0) {
				perror("events wait");
				verdict = RS_FAIL;
				break;
			}
		}
		// Catch setup errors delivered concurrently with the final wait.
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
			peak = std::max(peak, static_cast<Bytes>(process.usage.ru_maxrss));
#endif
			if (verdict == RS_AC) {
				if (limit >= 0 && peak > limit)
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
		printf("%lld\n%lld\n", elapsed, static_cast<long long>(peak));
		return verdict;
	} catch (...) {
		printf("-1\n-1\n");
		fprintf(stderr, "Invalid watcher arguments\n");
		return RS_FAIL;
	}
}
