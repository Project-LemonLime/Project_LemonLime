/*
 * SPDX-FileCopyrightText: 2011-2019 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <algorithm>
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <string>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int pid;

void cleanUp(int dummy) {
	kill(pid, SIGKILL);
	exit(0);
}

std::string getCpuBrandString() {
	char buf[1024];
	size_t buflen = 1024;
	sysctlbyname("machdep.cpu.brand_string", &buf, &buflen, NULL, 0);
	return std::string(buf, buflen);
}

int calculateStaticMemoryUsage(const char* executableName) {
    char command[256];
    snprintf(command, sizeof(command), "size %s", executableName);

    FILE* output = popen(command, "r");
    if (output == NULL)
        return -1;

    char buffer[256];
    size_t staticMemoryUsage = 0;

    while (fgets(buffer, sizeof(buffer), output) != NULL) {
        // Find the line containing the relevant sections
        if (std::isdigit(buffer[0])) {
            // Parse the sizes from the line
            size_t size__TEXT, size__DATA;
            sscanf(buffer, "%zu%zu", &size__TEXT, &size__DATA);
            staticMemoryUsage += size__TEXT + size__DATA;
        }
    }

    pclose(output);

    return staticMemoryUsage;
}

int main(int argc, char *argv[]) {
	int isAppleSilicon = getCpuBrandString().find("Apple") != std::string::npos;

	int timeLimit, memoryLimit;
	sscanf(argv[5], "%d", &timeLimit);
	timeLimit = (timeLimit - 1) / 1000 + 1;
	sscanf(argv[6], "%d", &memoryLimit);
	memoryLimit *= 1024 * (isAppleSilicon ? 4 : 1);
	int staticMemoryUsage = calculateStaticMemoryUsage(argv[1]);
	if (staticMemoryUsage != -1 && staticMemoryUsage > memoryLimit) {
		printf("0\n");
		printf("%d\n", staticMemoryUsage);
		return 0;
	}
	pid = fork();

	if (pid > 0) {
		signal(SIGINT, cleanUp);
		signal(SIGABRT, cleanUp);
		signal(SIGTERM, cleanUp);
		struct rusage usage;
		int status;

		if (wait4(pid, &status, 0, &usage) == -1)
			return 1;

		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 1)
				return 1;

			printf("%d\n", (int)(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000));
			printf("%d\n", (int)(usage.ru_maxrss) / (isAppleSilicon ? 4 : 1));

			if (WEXITSTATUS(status) != 0)
				return 2;

			return 0;
		}

		if (WIFSIGNALED(status)) {
			printf("%d\n", (int)(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000));
			printf("%d\n", (int)(usage.ru_maxrss) / (isAppleSilicon ? 4 : 1));

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

		rlimit memlim, stalim, timlim;

		if (memoryLimit > 0) {
			memlim = (rlimit){(rlim_t)memoryLimit, (rlim_t)memoryLimit};
			stalim = (rlimit){(rlim_t)memoryLimit, (rlim_t)memoryLimit};
		} else {
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
