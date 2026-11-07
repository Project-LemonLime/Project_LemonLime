/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {
	constexpr size_t MiB = 1024 * 1024;

	int fixture(const char *mode) {
		if (strcmp(mode, "normal") == 0)
			return 0;
		if (strcmp(mode, "nonzero") == 0)
			return 7;
		if (strcmp(mode, "abort") == 0) {
			// Do not leave core files on developer machines. Piped Linux core handlers
			// ignore this limit, so this does not bypass their abnormal-exit behavior.
			rlimit core{0, 0};
			if (setrlimit(RLIMIT_CORE, &core) == -1)
				return 8;
			std::abort();
		}
		if (strcmp(mode, "cpu") == 0) {
			volatile unsigned counter = 0;
			for (;;)
				counter = counter + 1;
		}
		const bool reservation = strcmp(mode, "virtual") == 0;
		const bool unlimited = strcmp(mode, "unlimited") == 0;
		const bool fast = strcmp(mode, "fast-resident") == 0;
		const size_t size = (reservation ? 1024 : 96) * MiB;
		void *memory = mmap(nullptr, size, reservation ? PROT_NONE : PROT_READ | PROT_WRITE,
		                    MAP_PRIVATE | MAP_ANON, -1, 0);
		if (memory == MAP_FAILED)
			return 9;
		if (! reservation) {
			auto *pages = static_cast<volatile unsigned char *>(memory);
			for (size_t offset = 0; offset < size; offset += 4096)
				pages[offset] = static_cast<unsigned char>(offset / 4096);
		}
		if (! reservation && ! unlimited && ! fast) {
			// A resident violation must be detected while the child is still alive,
			// even when it is sleeping and cannot hit RLIMIT_CPU.
			for (;;)
				pause();
		}
		if (! fast)
			usleep(30000);
		return munmap(memory, size) == 0 ? 0 : 10;
	}

	bool runCase(const char *self, const char *mode, int expected, long long memoryLimit,
	             long long minimumMemory = -1) {
		int output[2];
		if (pipe(output) == -1) {
			perror("pipe");
			return false;
		}
		const pid_t child = fork();
		if (child == -1) {
			perror("fork");
			close(output[0]);
			close(output[1]);
			return false;
		}
		if (child == 0) {
			setpgid(0, 0);
			close(output[0]);
			if (dup2(output[1], STDOUT_FILENO) == -1)
				_exit(125);
			close(output[1]);
			auto arguments = std::string("--fixture ") + mode;
			if (strcmp(mode, "quoted-arguments") == 0)
				arguments += " 'space value' '' '$literal'";
			const auto memory = std::to_string(memoryLimit);
			const char *time = strcmp(mode, "cpu") == 0 ? "100" : "3000";
			execl(WATCHER_PATH, WATCHER_PATH, self, arguments.c_str(), "", "", "", time, memory.c_str(), time,
			      memory.c_str(), "", "", static_cast<char *>(nullptr));
			_exit(125);
		}
		close(output[1]);
		const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
		int status = 0;
		bool finished = false;
		while (std::chrono::steady_clock::now() < deadline) {
			const auto waited = waitpid(child, &status, WNOHANG);
			if (waited == child) {
				finished = true;
				break;
			}
			if (waited == -1 && errno != EINTR)
				break;
			usleep(10000);
		}
		if (! finished) {
			kill(-child, SIGKILL);
			while (waitpid(child, &status, 0) == -1 && errno == EINTR) {
			}
			close(output[0]);
			fprintf(stderr, "%s: watcher did not complete within five seconds\n", mode);
			return false;
		}
		char buffer[256];
		const auto count = read(output[0], buffer, sizeof(buffer) - 1);
		close(output[0]);
		if (count < 0) {
			perror("read watcher output");
			return false;
		}
		buffer[count] = '\0';
		std::istringstream stream(buffer);
		long long timeUsed = -2, memoryUsed = -2;
		std::string extra;
		const bool protocol = bool(stream >> timeUsed >> memoryUsed) && ! (stream >> extra);
		if (! WIFEXITED(status) || WEXITSTATUS(status) != expected || ! protocol || timeUsed < 0 ||
		    memoryUsed < minimumMemory) {
			fprintf(stderr, "%s: expected exit %d and memory >= %lld; status %d, stdout: %s\n", mode,
			        expected, minimumMemory, status, buffer);
			return false;
		}
		return true;
	}
} // namespace

int main(int argc, char *argv[]) {
	if (argc >= 3 && strcmp(argv[1], "--fixture") == 0) {
		if (strcmp(argv[2], "quoted-arguments") == 0)
			return argc == 6 && strcmp(argv[3], "space value") == 0 && argv[4][0] == '\0' &&
			               strcmp(argv[5], "$literal") == 0
			           ? 0
					   : 11;
		return fixture(argv[2]);
	}
	char *self = realpath(argv[0], nullptr);
	if (! self) {
		perror("realpath");
		return 1;
	}
	bool success = true;
	success &= runCase(self, "normal", 0, 64);
	success &= runCase(self, "quoted-arguments", 0, 64);
	success &= runCase(self, "nonzero", 2, 64);
	success &= runCase(self, "virtual", 0, 64);
	success &= runCase(self, "resident", 4, 64, 64 * MiB + 1);
	success &= runCase(self, "fast-resident", 4, 64, 64 * MiB + 1);
	success &= runCase(self, "unlimited", 0, -1, 96 * MiB);
	// Preserve the watcher's existing SIGABRT and SIGXCPU result protocol.
	success &= runCase(self, "abort", 4, 64);
	success &= runCase(self, "cpu", 3, 64);
	free(self);
	return success ? 0 : 1;
}
