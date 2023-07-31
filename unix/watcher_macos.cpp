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
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/utils.h>
#include <string>
#include <sys/fcntl.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int pid;

template <typename header> static ssize_t calculateStaticMemoryUsage(int fd, int start_offset = 0) {
	ssize_t res = 0;

	header hdr;
	if (lseek(fd, start_offset, SEEK_SET) < 0) {
		return -1;
	}
	if (read(fd, &hdr, sizeof(header)) != sizeof(header)) {
		return -1;
	}
	if (hdr.filetype != MH_EXECUTE) {
		return -1;
	}

	uint32_t ncmds = hdr.ncmds;
	while (ncmds--) {
		struct load_command command;
		struct segment_command seg;
		struct segment_command_64 seg64;

		if (read(fd, &command, sizeof(struct load_command)) != sizeof(struct load_command)) {
			return -1;
		}
		if (lseek(fd, -sizeof(struct load_command), SEEK_CUR) < 0) {
			return -1;
		}

		switch (command.cmd) {
			case LC_SEGMENT:
				if (read(fd, &seg, sizeof(struct segment_command)) != sizeof(struct segment_command)) {
					return -1;
				}
				if (strcmp(seg.segname, SEG_DATA) == 0 || strcmp(seg.segname, SEG_TEXT) == 0) {
					res += seg.vmsize;
				}
				if (sizeof(struct segment_command) != command.cmdsize) {
					if (sizeof(struct segment_command) > command.cmdsize ||
					    lseek(fd, command.cmdsize - sizeof(struct segment_command), SEEK_CUR) < 0) {
						return -1;
					}
				}
				break;
			case LC_SEGMENT_64:
				if (read(fd, &seg64, sizeof(struct segment_command_64)) !=
				    sizeof(struct segment_command_64)) {
					return -1;
				}
				if (strcmp(seg64.segname, SEG_DATA) == 0 || strcmp(seg64.segname, SEG_TEXT) == 0) {
					res += seg64.vmsize;
				}
				if (sizeof(struct segment_command_64) != command.cmdsize) {
					if (sizeof(struct segment_command_64) > command.cmdsize ||
					    lseek(fd, command.cmdsize - sizeof(struct segment_command_64), SEEK_CUR) < 0) {
						return -1;
					}
				}
				break;
			default:
				if (lseek(fd, command.cmdsize, SEEK_CUR) < 0) {
					return -1;
				}
		}
	}

	return res;
}

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

int main(int argc, char *argv[]) {
	int isAppleSilicon = getCpuBrandString().find("Apple") != std::string::npos;

	int timeLimit, memoryLimit;
	sscanf(argv[5], "%d", &timeLimit);
	timeLimit = (timeLimit - 1) / 1000 + 1;
	sscanf(argv[6], "%d", &memoryLimit);
	memoryLimit *= 1024 * (isAppleSilicon ? 4 : 1);

	/* check static memory usage */
	uint32_t magic;
	__block ssize_t staticMemoryUsage = 0;
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		return 1;
	}

	int rc =
	    macho_best_slice_in_fd(fd, ^(const mach_header *slice, uint64_t sliceFileOffset, size_t sliceSize) {
		  if (slice->magic == MH_MAGIC) {
			  staticMemoryUsage = calculateStaticMemoryUsage<mach_header>(fd, sliceFileOffset);
		  } else if (slice->magic == MH_MAGIC_64) {
			  staticMemoryUsage = calculateStaticMemoryUsage<mach_header_64>(fd, sliceFileOffset);
		  } else {
			  staticMemoryUsage = -1;
		  }
	    });

	if (rc != 0) {
		// not a fat file, consider a mach-o file
		if (lseek(fd, 0, SEEK_SET) < 0) {
			return 1;
		}
		if (read(fd, &magic, sizeof(magic)) != sizeof(magic)) {
			return 1;
		}
		if (magic == MH_MAGIC) {
			staticMemoryUsage = calculateStaticMemoryUsage<mach_header>(fd);
		} else if (magic == MH_MAGIC_64) {
			staticMemoryUsage = calculateStaticMemoryUsage<mach_header_64>(fd);
		} else {
			staticMemoryUsage = -1;
		}
	}
	close(fd);

	if (staticMemoryUsage == -1) {
		return 1;
	}
	if (staticMemoryUsage > memoryLimit) {
		printf("0\n%zd\n", staticMemoryUsage);
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
