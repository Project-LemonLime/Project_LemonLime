// SPDX-License-Identifier: GPL-3.0-or-later
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <sys/mman.h>
#include <thread>
#ifdef __APPLE__
#include <mach/mach.h>
#else
#include <fstream>
#include <csignal>
#include <unistd.h>
#endif
int main(int argc, char **argv) {
	const bool accuracy = argc == 3 && ! strcmp(argv[1], "accuracy");
	if (argc != 2 && ! accuracy)
		return 1;
	if (accuracy) {
		const auto mib = std::strtoul(argv[2], nullptr, 10);
		if (mib != 16 && mib != 64)
			return 20;
		const size_t bytes = mib * 1024 * 1024;
		void *mapping = mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
		if (mapping == MAP_FAILED)
			return 21;
#ifdef __APPLE__
		auto *pages = static_cast<volatile char *>(mapping);
		// macOS uses RSS, so fault in every page. Linux leaves the mapping untouched
		// to ensure that an accidental RSS measurement cannot pass the VSS comparison.
		for (size_t offset = 0; offset < bytes; offset += 4096)
			pages[offset] = 1;
#endif
		unsigned long long reference = 0;
#ifdef __APPLE__
		// Independent of the watcher's proc_pidinfo implementation.
		mach_task_basic_info_data_t info{};
		mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
		if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, reinterpret_cast<task_info_t>(&info), &count) !=
		    KERN_SUCCESS)
			return 22;
		reference = info.resident_size;
#else
		// statm reports current VSS in pages; the watcher reads VmPeak from status.
		std::ifstream statm("/proc/self/statm");
		const auto pageSize = sysconf(_SC_PAGESIZE);
		if (! (statm >> reference) || pageSize <= 0)
			return 22;
		reference *= static_cast<unsigned long long>(pageSize);
#endif
		printf("reference-bytes=%llu allocation-bytes=%llu\n", reference,
		       static_cast<unsigned long long>(bytes));
		fflush(stdout);
		munmap(mapping, bytes);
		return 0;
	}
#ifdef __linux__
	if (! strcmp(argv[1], "stop-continue")) {
		printf("pid=%ld\n", static_cast<long>(getpid()));
		fflush(stdout);
		raise(SIGSTOP);
		puts("resumed");
		return 0;
	}
#endif
	if (! strcmp(argv[1], "abort"))
		std::abort();
	if (! strcmp(argv[1], "caught") || ! strcmp(argv[1], "uncaught")) {
		try {
			auto *p = new char[1024ULL * 1024 * 1024 * 1024];
			// Keep the allocation observable even with optimisation enabled.
			printf("%p\n", static_cast<void *>(p));
			delete[] p;
		} catch (const std::bad_alloc &) {
			if (! strcmp(argv[1], "uncaught"))
				throw;
			puts("caught");
		}
		return 0;
	}
	if (! strcmp(argv[1], "resident")) {
		auto *p = static_cast<volatile char *>(malloc(96 * 1024 * 1024));
		if (! p)
			return 7;
		for (int i = 0; i < 96 * 1024 * 1024; i += 4096)
			p[i] = 1;
		return 0;
	}
	if (! strcmp(argv[1], "reserve")) {
		void *p = mmap(nullptr, 128 * 1024 * 1024, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
		puts(p == MAP_FAILED ? "denied" : "reserved");
		return 0;
	}
	if (! strcmp(argv[1], "sleep")) {
		std::this_thread::sleep_for(std::chrono::seconds(20));
		return 0;
	}
	return 2;
}
