// SPDX-License-Identifier: GPL-3.0-or-later
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <sys/mman.h>
#include <unistd.h>
int main(int argc, char **argv) {
	if (argc != 2)
		return 1;
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
		usleep(300000);
		return 0;
	}
	if (! strcmp(argv[1], "reserve")) {
		void *p = mmap(nullptr, 128 * 1024 * 1024, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
		puts(p == MAP_FAILED ? "denied" : "reserved");
		usleep(100000);
		return 0;
	}
	if (! strcmp(argv[1], "sleep")) {
		sleep(20);
		return 0;
	}
	return 2;
}
