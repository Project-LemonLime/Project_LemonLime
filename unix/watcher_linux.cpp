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

void initWatcher() {
	return;
}

ssize_t calculateStaticMemoryUsage(const std::string& fileName)
{
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

ssize_t getMemoryRLimit(ssize_t memoryLimitInMB) {
	return memoryLimitInMB * 1024 * 1024;
}

size_t getMaxRSSInByte(long ru_maxrss) {
	return ru_maxrss * 1024;
}
