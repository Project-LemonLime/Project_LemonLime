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
#include <csignal>
#include <fstream>
#include <unistd.h>
#endif
#ifdef __linux__
#include <cerrno>
#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <sys/wait.h>

extern char **environ;

namespace {
	char executable[4096];
	// Keep these mappings alive until exec/exit: Linux accuracy measures VSS, not RSS.
	bool mapAndReport(size_t mib, const char *label = "reference-bytes") {
		const size_t bytes = mib * 1024 * 1024;
		if (mmap(nullptr, bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0) == MAP_FAILED)
			return false;
		unsigned long long pages = 0;
		std::ifstream statm("/proc/thread-self/statm");
		const auto pageSize = sysconf(_SC_PAGESIZE);
		if (! (statm >> pages) || pageSize <= 0)
			return false;
		printf("%s=%llu\n", label, pages * static_cast<unsigned long long>(pageSize));
		return fflush(stdout) == 0;
	}

	int execSelf(const char *mode) {
		execl(executable, "memory_cases", mode, static_cast<char *>(nullptr));
		return 31;
	}

	pthread_t leader;

	void *execFromWorker(void *) {
		if (pthread_join(leader, nullptr) != 0)
			std::exit(32);
		if (! mapAndReport(64))
			std::exit(33);
		std::exit(execSelf("exec-sink"));
	}
} // namespace
#endif
int main(int argc, char **argv) {
	const bool accuracy = argc == 3 && ! strcmp(argv[1], "accuracy");
	if (argc != 2 && ! accuracy)
		return 1;
#ifdef __linux__
	// /proc/self/exe and /proc/self/statm cease to describe a dead leader.
	const auto length = readlink("/proc/self/exe", executable, sizeof(executable) - 1);
	if (length <= 0 || static_cast<size_t>(length) >= sizeof(executable) - 1)
		return 30;
	executable[length] = '\0';
#endif
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
	if (! strcmp(argv[1], "exec-sink")) {
		puts("exec-ok=1");
		return 0;
	}
	if (! strcmp(argv[1], "exec-chain") || ! strcmp(argv[1], "exec-chain-next")) {
		const bool first = ! strcmp(argv[1], "exec-chain");
		if (! mapAndReport(first ? 128 : 64))
			return 34;
		return execSelf(first ? "exec-chain-next" : "exec-sink");
	}
	if (! strcmp(argv[1], "exec-failed")) {
		if (! mapAndReport(64))
			return 35;
		execl("/proc/self/lemon-watcher-no-such-executable", "missing", static_cast<char *>(nullptr));
		const int error = errno;
		printf("failed-exec-errno=%d\n", error);
		if (error != ENOENT)
			return 36;
		puts("failed-exec-handled=1");
		fflush(stdout);
		return execSelf("exec-sink");
	}
	if (! strcmp(argv[1], "execveat-peak")) {
		if (! mapAndReport(64))
			return 37;
		const int fd = open("/proc/self/exe", O_RDONLY | O_CLOEXEC);
		if (fd < 0)
			return 38;
		char name[] = "memory_cases";
		char mode[] = "exec-sink";
		char *arguments[] = {name, mode, nullptr};
		// Deliberately no libc fallback: the execveat notification is under test.
		syscall(SYS_execveat, fd, "", arguments, environ, AT_EMPTY_PATH);
		close(fd);
		return 39;
	}
	if (! strcmp(argv[1], "nonleader-exec")) {
#ifdef __GLIBC__
		// Avoid glibc's transient arena reservation obscuring this fixture's mapping peak.
		if (! mallopt(M_ARENA_MAX, 1))
			return 40;
#endif
		leader = pthread_self();
		pthread_t worker;
		pthread_attr_t attributes;
		if (pthread_attr_init(&attributes) != 0)
			return 40;
		const int sized = pthread_attr_setstacksize(&attributes, 1024 * 1024);
		const int created =
		    sized == 0 ? pthread_create(&worker, &attributes, execFromWorker, nullptr) : sized;
		pthread_attr_destroy(&attributes);
		if (created != 0)
			return 40;
		pthread_exit(nullptr);
	}
	if (! strcmp(argv[1], "fork-exec")) {
		if (! mapAndReport(16, "root-reference-bytes"))
			return 41;
		const pid_t child = fork();
		if (child < 0)
			return 42;
		if (child == 0) {
			if (! mapAndReport(128, "child-reference-bytes"))
				_exit(43);
			_exit(execSelf("exec-sink"));
		}
		int status = 0;
		pid_t result;
		do {
			result = waitpid(child, &status, 0);
		} while (result < 0 && errno == EINTR);
		return result == child && WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : 44;
	}
	if (! strcmp(argv[1], "background-child")) {
		int ready[2];
		if (pipe(ready) != 0)
			return 45;
		const pid_t child = fork();
		if (child < 0) {
			close(ready[0]);
			close(ready[1]);
			return 46;
		}
		if (child == 0) {
			close(ready[0]);
			if (setsid() < 0)
				_exit(47);
			const char token = 1;
			ssize_t sent;
			do {
				sent = write(ready[1], &token, 1);
			} while (sent < 0 && errno == EINTR);
			close(ready[1]);
			if (sent != 1)
				_exit(48);
			for (;;)
				pause();
		}
		close(ready[1]);
		char token = 0;
		ssize_t received;
		do {
			received = read(ready[0], &token, 1);
		} while (received < 0 && errno == EINTR);
		close(ready[0]);
		if (received != 1 || token != 1) {
			kill(child, SIGKILL);
			while (waitpid(child, nullptr, 0) < 0 && errno == EINTR) {
			}
			return 49;
		}
		printf("child-pid=%ld\n", static_cast<long>(child));
		fflush(stdout);
		return 0;
	}
	if (! strcmp(argv[1], "syscall-burst")) {
		unsigned long long checksum = 0;
		const auto expected = static_cast<unsigned long long>(getpid()) * 100000;
		for (int i = 0; i < 100000; ++i) {
			const long pid = syscall(SYS_getpid);
			if (pid <= 0)
				return 50;
			checksum += static_cast<unsigned long long>(pid);
		}
		if (checksum != expected)
			return 51;
		printf("syscall-checksum=%llu\n", checksum);
		puts("syscall-burst-ok=1");
		return 0;
	}
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
