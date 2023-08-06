/*
 * SPDX-FileCopyrightText: 2011-2019 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <linux/elf.h>
#include <sys/fcntl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int pid;

static auto read_elf_ident(int fd, char *e_ident) -> bool {
	if (read(fd, e_ident, EI_NIDENT) != EI_NIDENT) {
		return false;
	}
	if (strncmp(e_ident, "\177ELF", 4) != 0) {
		return false;
	}
	return true;
}

template <typename Ehdr, typename Phdr> static auto calculateStaticMemoryUsage(int fd) -> ssize_t {
	ssize_t res = 0;

	Ehdr elf_header;
	if (lseek64(fd, 0, SEEK_SET) < 0) {
		return -1;
	}
	if (read(fd, &elf_header, sizeof(Ehdr)) != sizeof(Ehdr)) {
		return -1;
	}
	// gcc with PIE will set executable as ET_DYN
	// https://stackoverflow.com/questions/34519521
	if (elf_header.e_type != ET_EXEC && elf_header.e_type != ET_DYN) {
		return -1;
	}
	if (elf_header.e_phoff == 0) {
		return -1; // no program header, not executable
	}
	if (elf_header.e_phentsize != sizeof(Phdr)) {
		return -1;
	}

	if (lseek64(fd, elf_header.e_phoff, SEEK_SET) < 0) {
		return -1;
	}
	Phdr program_header;
	for (int i = 0; i < elf_header.e_phnum; ++i) {
		if (read(fd, &program_header, sizeof(Phdr)) != sizeof(Phdr)) {
			return -1;
		}
		if (program_header.p_type == PT_LOAD) {
			res += program_header.p_memsz;
		}
	}

	return res;
}

void cleanUp(int /*dummy*/) {
	kill(pid, SIGKILL);
	exit(0);
}

auto main(int /*argc*/, char *argv[]) -> int {
	int timeLimit = 0, memoryLimit = 0;
	sscanf(argv[5], "%d", &timeLimit);
	timeLimit = (timeLimit - 1) / 1000 + 1;
	sscanf(argv[6], "%d", &memoryLimit);
	memoryLimit *= 1024 * 1024;

	/* check static memory usage */
	char e_ident[EI_NIDENT];
	ssize_t staticMemoryUsage = 0;
	int fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		return 1;
	}

	if (read_elf_ident(fd, e_ident) == false) {
		return 1;
	}
	if (e_ident[EI_CLASS] == ELFCLASS32) {
		staticMemoryUsage = calculateStaticMemoryUsage<Elf32_Ehdr, Elf32_Phdr>(fd);
	} else if (e_ident[EI_CLASS] == ELFCLASS64) {
		staticMemoryUsage = calculateStaticMemoryUsage<Elf64_Ehdr, Elf64_Phdr>(fd);
	} else {
		staticMemoryUsage = -1;
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
		struct rusage usage {};
		int status = 0;

		if (wait4(pid, &status, 0, &usage) == -1)
			return 1;

		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 1)
				return 1;

			printf("%d\n", static_cast<int>(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000));
			printf("%d\n", static_cast<int>(usage.ru_maxrss) * 1024);

			if (WEXITSTATUS(status) != 0)
				return 2;

			return 0;
		}

		if (WIFSIGNALED(status)) {
			printf("%d\n", static_cast<int>(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000));
			printf("%d\n", static_cast<int>(usage.ru_maxrss) * 1024);

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
