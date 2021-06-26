/*
 * SPDX-FileCopyrightText: 2011-2019 Project Lemon, Zhipeng Jia
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sysctl.h>
#include <unistd.h>

int pid;

void cleanUp(int dummy)
{
	kill(pid, SIGKILL);
	exit(0);
}

int processIsTranslated() {
	int ret = 0;
	size_t size = sizeof(ret);
	if (sysctlbyname("sysctl.proc_translated", &ret, &size, NULL, 0) == -1) 
	{
		if (errno == ENOENT)
			return 0;
		return -1;
	}
	return ret;
}

int main(int argc, char *argv[]) {
	int isARM = processIsTranslated();
	if (isARM == -1)
		return 1;

	int timeLimit, memoryLimit;
	sscanf(argv[5], "%d", &timeLimit);
	timeLimit = (timeLimit - 1) / 1000 + 1;
	sscanf(argv[6], "%d", &memoryLimit);
	memoryLimit *= 1024 * (isARM ? 4 : 1);
	pid = fork();

	if (pid > 0)
	{
		signal(SIGINT, cleanUp);
		signal(SIGABRT, cleanUp);
		signal(SIGTERM, cleanUp);
		struct rusage usage;
		int status;

		if (wait4(pid, &status, 0, &usage) == -1)
			return 1;

		if (WIFEXITED(status))
		{
			if (WEXITSTATUS(status) == 1)
				return 1;

			printf("%d\n", (int)(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000));
			printf("%d\n", (int)(usage.ru_maxrss) / (isARM ? 4 : 1));

			if (WEXITSTATUS(status) != 0)
				return 2;

			return 0;
		}

		if (WIFSIGNALED(status))
		{
			printf("%d\n", (int)(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000));
			printf("%d\n", (int)(usage.ru_maxrss) / (isARM ? 4 : 1));

			if (WTERMSIG(status) == SIGXCPU)
				return 3;

			if (WTERMSIG(status) == SIGKILL)
				return 4;

			if (WTERMSIG(status) == SIGABRT)
				return 4;

			return 2;
		}
	}
	else
	{
		if (strlen(argv[2]) > 0)
			assert(freopen(argv[2], "r", stdin));

		if (strlen(argv[3]) > 0)
			assert(freopen(argv[3], "w", stdout));

		if (strlen(argv[4]) > 0)
			assert(freopen(argv[4], "w", stderr));

		rlimit memlim, stalim, timlim;

		if (memoryLimit > 0)
		{
			memlim = (rlimit){(rlim_t)memoryLimit, (rlim_t)memoryLimit};
			stalim = (rlimit){(rlim_t)memoryLimit, (rlim_t)memoryLimit};
		}
		else
		{
			memlim = (rlimit){RLIM_INFINITY, RLIM_INFINITY};
			stalim = (rlimit){(rlim_t)2147483647ll, (rlim_t)2147483647ll};
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
