/*
 * SPDX-FileCopyrightText: 2011-2019 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2019-2023 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <cassert>
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
 */
auto main(int argc, char *argv[]) -> int {
	if (argc != 12) {
		printf("-1\n-1\n");
		fprintf(stderr, "Expected 11 arguments, found %d\n", argc);
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

	pid = fork();

	if (pid > 0) {
		// Parent process
		signal(SIGINT, cleanUp);
		signal(SIGABRT, cleanUp);
		signal(SIGTERM, cleanUp);
		struct rusage usage{};
		int status = 0;

		if (wait4(pid, &status, 0, &usage) == -1) {
			printf("-1\n-1\n");
			perror("wait4");
			return RS_FAIL;
		}

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

		rlimit memlim{}, stalim{}, timlim{};

		if (memoryLimitMib > 0) {
			memlim = (rlimit){(rlim_t)actualMemoryRLimit, (rlim_t)actualMemoryRLimit};
			stalim = (rlimit){(rlim_t)actualMemoryRLimit, (rlim_t)actualMemoryRLimit};
		} else {
			// No memory limit specified, set to infinity
			memlim = (rlimit){RLIM_INFINITY, RLIM_INFINITY};
			stalim = (rlimit){(rlim_t)2147483647LL, (rlim_t)2147483647LL};
		}

		// Calculate time limit in seconds, rounding up
		rlim_t soft_time_limit_sec = (timeLimitMs + 999) / 1000;
		timlim = (rlimit){soft_time_limit_sec, soft_time_limit_sec + 1}; // Soft limit + 1 for hard limit

		setrlimit(RLIMIT_AS, &memlim);
		setrlimit(RLIMIT_STACK, &stalim);
		setrlimit(RLIMIT_CPU, &timlim);

		if (execlp("bash", "bash", "-c", runCmd.c_str(), NULL) == -1) {
			perror("execlp");
			exit(RS_FAIL);
		}
	}

	return 0;
}
