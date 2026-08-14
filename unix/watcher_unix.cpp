/*
 * SPDX-FileCopyrightText: 2011-2019 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2019-2023 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <cassert>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <sys/fcntl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#if defined(__linux__)
#include <linux/sched.h>
#include <poll.h>
#include <sys/syscall.h>
#include <sys/timerfd.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||          \
    defined(__DragonFly__)
#include <sys/event.h>
#endif
#include <unistd.h>

int pid;

void cleanUp(int /*dummy*/) {
	kill(pid, SIGKILL);
	exit(0);
}

extern void initWatcher();
extern ssize_t calculateStaticMemoryUsage(const std::string &);
extern ssize_t getMemoryRLimit(ssize_t memoryLimitInMB);
extern size_t getMaxRSSInByte(long ru_maxrss);

enum : int {
	RS_AC = 0,
	RS_FAIL = 1,
	RS_RE = 2,
	RS_TLE = 3,
	RS_MLE = 4,
};

/**
 * argv[1]: executable file path
 * argv[2]: 执行选手程序时，传递的命令行参数，但不包含 argv0
 * argv[3]: 重定向输入文件（如果有）
 * argv[4]: 重定向输出文件（如果有）
 * argv[5]: 重定向错误流文件（如果有）
 * argv[6]: 时间限制（毫秒）
 * argv[7]: 空间限制（MiB），若为负数表示无限制
 * argv[8]: 原始（未经语言设置缩放的）时间限制（毫秒）
 * argv[9]: 原始（未经语言设置缩放的）空间限制（MiB）
 * argv[10]: 选手程序只读的文件
 * argv[11]: 选手程序只写的文件
 * argv[12]: wall clock 额外超时时间（毫秒）
 */
auto main(int argc, char *argv[]) -> int {
	if (argc != 13) {
		printf("-1\n-1\n");
		fprintf(stderr, "Expected 12 arguments, found %d\n", argc - 1);
		return RS_FAIL;
	}
	std::string fileName = argv[1];
	std::string runArgs = argv[2];
	std::string stdinRedirect = argv[3];
	std::string stdoutRedirect = argv[4];
	std::string stderrRedirect = argv[5];
	long long timeLimitMs = std::stoll(argv[6]);
	long long memoryLimitMib = std::stoll(argv[7]);
	[[maybe_unused]] long long rawTimeLimitMs = std::stoll(argv[8]);
	[[maybe_unused]] long long rawMemoryLimitMib = std::stoll(argv[9]);
	[[maybe_unused]] std::string readableFile = argv[10];
	[[maybe_unused]] std::string writableFile = argv[11];
	long long extraTimeMs = std::stoll(argv[12]);

	initWatcher();

	std::ostringstream ss;
	ss << '"';
	ss << fileName;
	ss << "\" ";
	ss << runArgs;
	std::string runCmd = ss.str();

	if (memoryLimitMib > 0) {
		ssize_t staticMemoryUsageByte = calculateStaticMemoryUsage(fileName);
		if (staticMemoryUsageByte == -1) {
			printf("-1\n-1\n");
			fprintf(stderr, "Error in calculating static memory usage\n");
			return RS_FAIL;
		}
		if (staticMemoryUsageByte > memoryLimitMib * 1024 * 1024) {
			// If static memory usage exceeds the limit, it's an MLE.
			printf("0\n%zd\n", staticMemoryUsageByte);
			fprintf(stderr, "Static memory usage exceeds the limit\n");
			return RS_MLE;
		}
	}

	ssize_t actualMemoryRLimit = getMemoryRLimit(memoryLimitMib);

#if defined(__linux__)
	int childPfd = -1;
	struct clone_args args{};
	args.flags = CLONE_PIDFD;
	args.pidfd = (unsigned long long)&childPfd;
	args.exit_signal = SIGCHLD;
	pid = syscall(SYS_clone3, &args, sizeof(args));
#else
	pid = fork();
#endif

	if (pid < 0) {
		perror("fork");
		printf("-1\n-1\n");
		return RS_FAIL;
	}

	if (pid > 0) {
		// Parent process
		signal(SIGINT, cleanUp);
		signal(SIGABRT, cleanUp);
		signal(SIGTERM, cleanUp);
		struct rusage usage{};
		int status = 0;

		// Time limit is enforced by a wall-clock timer instead of `setrlimit(RLIMIT_CPU)`:
		// RLIMIT_CPU counts CPU time while the judge measures user time, and a program that
		// merely sleeps never hits it.
		long long wallClockMs = timeLimitMs + extraTimeMs;
#if defined(__linux__)
		int timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
		if (timerFd < 0) {
			perror("timerfd_create");
			printf("-1\n-1\n");
			return RS_FAIL;
		}

		struct itimerspec ts{};
		ts.it_value.tv_sec = wallClockMs / 1000;
		ts.it_value.tv_nsec = (wallClockMs % 1000) * 1000000;
		if (timerfd_settime(timerFd, 0, &ts, nullptr) < 0) {
			perror("timerfd_settime");
			printf("-1\n-1\n");
			return RS_FAIL;
		}

		struct pollfd pfds[2]{};
		pfds[0].fd = childPfd;
		pfds[0].events = POLLIN;
		pfds[1].fd = timerFd;
		pfds[1].events = POLLIN;

		int pollResult;
		do {
			pollResult = poll(pfds, 2, -1);
		} while (pollResult < 0 && errno == EINTR);

		if (pollResult < 0) {
			perror("poll");
			printf("-1\n-1\n");
			return RS_FAIL;
		}

		bool childExited = pfds[0].revents & POLLIN;
		bool timedOut = pfds[1].revents & POLLIN;

		if (! childExited)
			kill(pid, SIGKILL);

		wait4(pid, &status, 0, &usage);

		close(childPfd);
		close(timerFd);

		if (timedOut) {
			printf("-1\n-1\n");
			return RS_TLE;
		}
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||          \
    defined(__DragonFly__)
		int kq = kqueue();
		if (kq < 0) {
			perror("kqueue");
			printf("-1\n-1\n");
			return RS_FAIL;
		}

		struct kevent changes[2];
		EV_SET(&changes[0], pid, EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, NULL);
		EV_SET(&changes[1], 0, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, wallClockMs, NULL);

		struct kevent events[2];
		int n;
		do {
			n = kevent(kq, changes, 2, events, 2, NULL);
		} while (n < 0 && errno == EINTR);

		if (n < 0) {
			perror("kevent");
			printf("-1\n-1\n");
			return RS_FAIL;
		}

		bool childExited = false;
		bool timedOut = false;
		for (int i = 0; i < n; i++) {
			if (events[i].filter == EVFILT_PROC)
				childExited = true;
			else if (events[i].filter == EVFILT_TIMER)
				timedOut = true;
		}

		if (timedOut && ! childExited)
			kill(pid, SIGKILL);

		wait4(pid, &status, 0, &usage);

		close(kq);

		if (timedOut) {
			printf("-1\n-1\n");
			return RS_TLE;
		}
#else
		if (wait4(pid, &status, 0, &usage) == -1) {
			printf("-1\n-1\n");
			perror("wait4");
			return RS_FAIL;
		}
#endif

		if (WIFEXITED(status)) {
			long long timeUsedMs =
			    static_cast<long long>(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000);
			size_t memoryUsed = getMaxRSSInByte(usage.ru_maxrss);
			printf("%lld\n%zu\n", timeUsedMs, memoryUsed);
			if (WEXITSTATUS(status) != 0) {
				// Any non-zero exit status indicates a runtime error.
				return RS_RE;
			}
			if (timeUsedMs > timeLimitMs) {
				return RS_TLE;
			}
			if (memoryUsed > memoryLimitMib * 1024 * 1024) {
				return RS_MLE;
			}
			return RS_AC;
		}

		if (WIFSIGNALED(status)) {
			long long timeUsedMs =
			    static_cast<long long>(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000);
			printf("%lld\n-1\n", timeUsedMs);
			if (WTERMSIG(status) == SIGXCPU) {
				return RS_TLE;
			}
			if (WTERMSIG(status) == SIGKILL || WTERMSIG(status) == SIGABRT) {
				return RS_MLE;
			}
			return RS_RE;
		}
	} else {
		// Child process
		std::string finalStdinRedirect = stdinRedirect.empty() ? "/dev/null" : stdinRedirect;
		if (freopen(finalStdinRedirect.c_str(), "r", stdin) == NULL) {
			perror("freopen stdin");
			exit(RS_FAIL);
		}
		std::string finalStdoutRedirect = stdoutRedirect.empty() ? "/dev/null" : stdoutRedirect;
		if (freopen(finalStdoutRedirect.c_str(), "w", stdout) == NULL) {
			perror("freopen stdout");
			exit(RS_FAIL);
		}
		std::string finalStderrRedirect = stderrRedirect.empty() ? "/dev/null" : stderrRedirect;
		if (freopen(finalStderrRedirect.c_str(), "w", stderr) == NULL) {
			perror("freopen stderr");
			exit(RS_FAIL);
		}

		rlimit memlim{}, stalim{};

		if (memoryLimitMib > 0) {
			memlim = (rlimit){(rlim_t)actualMemoryRLimit, (rlim_t)actualMemoryRLimit};
			stalim = (rlimit){(rlim_t)actualMemoryRLimit, (rlim_t)actualMemoryRLimit};
		} else {
			// No memory limit specified, set to infinity
			memlim = (rlimit){RLIM_INFINITY, RLIM_INFINITY};
			stalim = (rlimit){(rlim_t)2147483647LL, (rlim_t)2147483647LL};
		}

		setrlimit(RLIMIT_AS, &memlim);
		setrlimit(RLIMIT_STACK, &stalim);

		if (execlp("bash", "bash", "-c", runCmd.c_str(), NULL) == -1) {
			perror("execlp");
			exit(RS_FAIL);
		}
	}

	return 0;
}
