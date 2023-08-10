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

/**
 * argv[1]: QString("\"%1\" %2").arg(executableFile, arguments) in `judgingthread.cpp` 执行命令
 * argv[2]: 重定向输入文件（如果有）
 * argv[3]: 重定向输出文件（如果有）
 * argv[4]: 重定向错误流文件（如果有）
 * argv[5]: 时间限制（毫秒）
 * argv[6]: 空间限制（MB），若为负数表示无限制
 */
auto main(int /*argc*/, char *argv[]) -> int {
	initWatcher();

	int timeLimit = 0;
	ssize_t memoryLimit = 0;
	sscanf(argv[5], "%d", &timeLimit);
	timeLimit = (timeLimit - 1) / 1000 + 1;
	sscanf(argv[6], "%zd", &memoryLimit);

	/* check static memory usage */
	// 匹配 "" 来解析出文件名
	std::string fileName(argv[1]);
	fileName = fileName.substr(1);
	fileName = fileName.substr(0, fileName.find("\""));

	if (memoryLimit > 0) {
		ssize_t staticMemoryUsage = calculateStaticMemoryUsage(fileName);
		if (staticMemoryUsage == -1) {
			return 1;
		}
		if (staticMemoryUsage > memoryLimit * 1024 * 1024) {
			printf("0\n%zd\n", staticMemoryUsage);
			return 0;
		}
	}

	memoryLimit = getMemoryRLimit(memoryLimit);

	pid = fork();

	if (pid > 0) {
		signal(SIGINT, cleanUp);
		signal(SIGABRT, cleanUp);
		signal(SIGTERM, cleanUp);
		struct rusage usage {};
		int status = 0;

		if (wait4(pid, &status, 0, &usage) == -1)
			return 1;

		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 1)
				return 1;

			printf("%d\n", static_cast<int>(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000));
			printf("%zu\n", getMaxRSSInByte(usage.ru_maxrss));

			if (WEXITSTATUS(status) != 0)
				return 2;

			return 0;
		}

		if (WIFSIGNALED(status)) {
			printf("%d\n", static_cast<int>(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000));
			printf("%zu\n", getMaxRSSInByte(usage.ru_maxrss));

			if (WTERMSIG(status) == SIGXCPU)
				return 3;

			if (WTERMSIG(status) == SIGKILL)
				return 4;

			if (WTERMSIG(status) == SIGABRT)
				return 4;

			return 2;
		}
	} else {
		if (strlen(argv[2]) > 0)
			freopen(argv[2], "r", stdin);

		if (strlen(argv[3]) > 0)
			freopen(argv[3], "w", stdout);

		if (strlen(argv[4]) > 0)
			freopen(argv[4], "w", stderr);

		rlimit memlim{}, stalim{}, timlim{};

		if (memoryLimit > 0) {
			memlim = (rlimit){(rlim_t)memoryLimit, (rlim_t)memoryLimit};
			stalim = (rlimit){(rlim_t)memoryLimit, (rlim_t)memoryLimit};
		} else {
			memlim = (rlimit){RLIM_INFINITY, RLIM_INFINITY};
			stalim = (rlimit){(rlim_t)2147483647LL, (rlim_t)2147483647LL};
		}

		timlim = (rlimit){(rlim_t)timeLimit, (rlim_t)(timeLimit + 1)};
		setrlimit(RLIMIT_AS, &memlim);
		setrlimit(RLIMIT_STACK, &stalim);
		setrlimit(RLIMIT_CPU, &timlim);

		if (execlp("bash", "bash", "-c", argv[1], NULL) == -1)
			return 1;
	}

	return 0;
}
