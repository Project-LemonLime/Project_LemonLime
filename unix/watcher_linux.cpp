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
#include <linux/elf.h>
#include <string>
#include <sys/fcntl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

ssize_t calculateStaticMemoryUsage(const std::string &fileName) {
	char e_ident[EI_NIDENT];
	ssize_t staticMemoryUsage = 0;
	int fd = open(fileName.c_str(), O_RDONLY);
	if (fd < 0) {
		return -1;
	}

	if (read_elf_ident(fd, e_ident) == false) {
		return -1;
	}
	if (e_ident[EI_CLASS] == ELFCLASS32) {
		staticMemoryUsage = calculateStaticMemoryUsage<Elf32_Ehdr, Elf32_Phdr>(fd);
	} else if (e_ident[EI_CLASS] == ELFCLASS64) {
		staticMemoryUsage = calculateStaticMemoryUsage<Elf64_Ehdr, Elf64_Phdr>(fd);
	} else {
		staticMemoryUsage = -1;
	}
	close(fd);

	return staticMemoryUsage;
}

ssize_t getResidentMemoryInByte(pid_t child) {
	char path[64];
	snprintf(path, sizeof(path), "/proc/%d/statm", static_cast<int>(child));
	const int fd = open(path, O_RDONLY);
	if (fd == -1)
		return errno == ENOENT || errno == ESRCH ? 0 : -1;
	char data[256];
	const auto size = read(fd, data, sizeof(data) - 1);
	const int error = errno;
	close(fd);
	if (size < 0) {
		errno = error;
		return -1;
	}
	data[size] = '\0';
	unsigned long virtualPages = 0, residentPages = 0;
	if (sscanf(data, "%lu %lu", &virtualPages, &residentPages) != 2) {
		errno = EIO;
		return -1;
	}
	static const long pageSize = sysconf(_SC_PAGESIZE);
	if (pageSize <= 0) {
		errno = EINVAL;
		return -1;
	}
	return static_cast<ssize_t>(residentPages * pageSize);
}

size_t getMaxRSSInByte(long ru_maxrss) { return ru_maxrss * 1024; }
