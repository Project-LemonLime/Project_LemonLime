/*
 * SPDX-FileCopyrightText: 2011-2019 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2019-2023 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <sys/fcntl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int pid;

void cleanUp(int /*dummy*/) {
	kill(pid, SIGKILL);
	exit(0);
}

extern ssize_t calculateStaticMemoryUsage(const std::string &);
extern ssize_t getResidentMemoryInByte(pid_t);
extern size_t getMaxRSSInByte(long ru_maxrss);

enum : int {
	RS_AC = 0,
	RS_FAIL = 1,
	RS_RE = 2,
	RS_TLE = 3,
	RS_MLE = 4,
};

struct ChildError {
	int error;
	char operation[32];
};

[[noreturn]] static void failChild(int errorPipe, const char *operation) {
	ChildError error{};
	error.error = errno;
	snprintf(error.operation, sizeof(error.operation), "%s", operation);
	const auto ignored = write(errorPipe, &error, sizeof(error));
	(void)ignored;
	_exit(RS_FAIL);
}

static void setLimit(int errorPipe, int resource, rlim_t soft, rlim_t hard, const char *operation) {
	rlimit inherited{};
	if (getrlimit(resource, &inherited) == -1)
		failChild(errorPipe, operation);
	// A child cannot raise a hard limit inherited from its launcher.
	rlimit limit{std::min(soft, inherited.rlim_max), std::min(hard, inherited.rlim_max)};
	if (setrlimit(resource, &limit) == -1)
		failChild(errorPipe, operation);
}

/**
 * argv[1]: executable file path
 * argv[2]: 执行选手程序时，传递的命令行参数，但不包含 argv0
 * argv[3]: 重定向输入文件（如果有）
 * argv[4]: 重定向输出文件（如果有）
 * argv[5]: 重定向错误流文件（如果有）
 * argv[6]: 时间限制（毫秒）
 * argv[7]: 空间限制（MiB），若为负数表示无限制
 * argv[8]: 原始（未经语言设置缩放的）时间限制（毫秒）
 * argv[9]: 原始（未经语言设置缩放的）空间限制（MiB）
 * argv[10]: 选手程序只读的文件
 * argv[11]: 选手程序只写的文件
 */
auto main(int argc, char *argv[]) -> int {
	if (argc != 12) {
		printf("-1\n-1\n");
		fprintf(stderr, "Expected 11 arguments, found %d\n", argc);
		return RS_FAIL;
	}
	std::string fileName = argv[1];
	std::string runArgs = argv[2];
	std::string stdinRedirect = argv[3];
	std::string stdoutRedirect = argv[4];
	std::string stderrRedirect = argv[5];
	long long timeLimitMs = std::stoll(argv[6]);
	long long memoryLimitMib = std::stoll(argv[7]);
	[[maybe_unused]] long long rawTimeLimitMs = std::stoll(argv[8]);
	[[maybe_unused]] long long rawMemoryLimitMib = std::stoll(argv[9]);
	[[maybe_unused]] std::string readableFile = argv[10];
	[[maybe_unused]] std::string writableFile = argv[11];

	std::ostringstream ss;
	ss << "exec \"";
	ss << fileName;
	ss << "\" ";
	ss << runArgs;
	std::string runCmd = ss.str();

	if (memoryLimitMib > 0) {
		ssize_t staticMemoryUsageByte = calculateStaticMemoryUsage(fileName);
		if (staticMemoryUsageByte == -1) {
			printf("-1\n-1\n");
			fprintf(stderr, "Error in calculating static memory usage\n");
			return RS_FAIL;
		}
		if (staticMemoryUsageByte > memoryLimitMib * 1024 * 1024) {
			// If static memory usage exceeds the limit, it's an MLE.
			printf("0\n%zd\n", staticMemoryUsageByte);
			fprintf(stderr, "Static memory usage exceeds the limit\n");
			return RS_MLE;
		}
	}

	int errorPipe[2];
	if (pipe(errorPipe) == -1) {
		printf("-1\n-1\n");
		perror("pipe");
		return RS_FAIL;
	}
	if (fcntl(errorPipe[1], F_SETFD, FD_CLOEXEC) == -1) {
		printf("-1\n-1\n");
		perror("fcntl");
		close(errorPipe[0]);
		close(errorPipe[1]);
		return RS_FAIL;
	}

	pid = fork();
	if (pid == -1) {
		printf("-1\n-1\n");
		perror("fork");
		close(errorPipe[0]);
		close(errorPipe[1]);
		return RS_FAIL;
	}

	if (pid > 0) {
		// Parent process
		close(errorPipe[1]);
		signal(SIGINT, cleanUp);
		signal(SIGABRT, cleanUp);
		signal(SIGTERM, cleanUp);
		struct rusage usage{};
		int status = 0;

		bool memoryExceeded = false;
		bool monitoringFailed = false;
		size_t peakMemory = 0;
		for (;;) {
			const auto waited = wait4(pid, &status, WNOHANG, &usage);
			if (waited == pid)
				break;
			if (waited == -1) {
				if (errno == EINTR)
					continue;
				printf("-1\n-1\n");
				perror("wait4");
				close(errorPipe[0]);
				return RS_FAIL;
			}
			if (memoryLimitMib > 0 && ! memoryExceeded && ! monitoringFailed) {
				const auto residentMemory = getResidentMemoryInByte(pid);
				if (residentMemory < 0) {
					perror("reading child resident memory");
					monitoringFailed = true;
					kill(pid, SIGKILL);
				} else {
					peakMemory = std::max(peakMemory, static_cast<size_t>(residentMemory));
					if (peakMemory > static_cast<size_t>(memoryLimitMib) * 1024 * 1024) {
						memoryExceeded = true;
						kill(pid, SIGKILL);
					}
				}
			}
			usleep(1000);
		}

		ChildError childError{};
		ssize_t errorSize;
		do {
			errorSize = read(errorPipe[0], &childError, sizeof(childError));
		} while (errorSize == -1 && errno == EINTR);
		close(errorPipe[0]);
		if (errorSize != 0 || monitoringFailed) {
			printf("-1\n-1\n");
			if (errorSize == sizeof(childError))
				fprintf(stderr, "%s: %s\n", childError.operation, strerror(childError.error));
			return RS_FAIL;
		}

		peakMemory = std::max(peakMemory, getMaxRSSInByte(usage.ru_maxrss));
		if (memoryExceeded ||
		    (memoryLimitMib > 0 && peakMemory > static_cast<size_t>(memoryLimitMib) * 1024 * 1024)) {
			const auto timeUsedMs =
			    static_cast<long long>(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000);
			printf("%lld\n%zu\n", timeUsedMs, peakMemory);
			return RS_MLE;
		}

		if (WIFEXITED(status)) {
			long long timeUsedMs =
			    static_cast<long long>(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000);
			size_t memoryUsed = peakMemory;
			printf("%lld\n%zu\n", timeUsedMs, memoryUsed);
			if (WEXITSTATUS(status) != 0) {
				// Any non-zero exit status indicates a runtime error.
				return RS_RE;
			}
			if (timeUsedMs > timeLimitMs) {
				return RS_TLE;
			}
			return RS_AC;
		}

		if (WIFSIGNALED(status)) {
			long long timeUsedMs =
			    static_cast<long long>(usage.ru_utime.tv_sec * 1000 + usage.ru_utime.tv_usec / 1000);
			printf("%lld\n-1\n", timeUsedMs);
			if (WTERMSIG(status) == SIGXCPU) {
				return RS_TLE;
			}
			if (WTERMSIG(status) == SIGKILL || WTERMSIG(status) == SIGABRT) {
				return RS_MLE;
			}
			return RS_RE;
		}
	} else {
		close(errorPipe[0]);
		std::string finalStdinRedirect = stdinRedirect.empty() ? "/dev/null" : stdinRedirect;
		if (freopen(finalStdinRedirect.c_str(), "r", stdin) == NULL) {
			failChild(errorPipe[1], "freopen stdin");
		}
		std::string finalStdoutRedirect = stdoutRedirect.empty() ? "/dev/null" : stdoutRedirect;
		if (freopen(finalStdoutRedirect.c_str(), "w", stdout) == NULL) {
			failChild(errorPipe[1], "freopen stdout");
		}
		std::string finalStderrRedirect = stderrRedirect.empty() ? "/dev/null" : stderrRedirect;
		if (freopen(finalStderrRedirect.c_str(), "w", stderr) == NULL) {
			failChild(errorPipe[1], "freopen stderr");
		}

		// RLIMIT_AS measures virtual address space, not resident memory. It can reject
		// allocations well below the requested RSS limit, and Darwin's shared cache
		// can already exceed it before exec. Enforce RSS in the parent instead.
		if (memoryLimitMib > 0) {
			const auto stackLimit = static_cast<rlim_t>(memoryLimitMib) * 1024 * 1024;
			setLimit(errorPipe[1], RLIMIT_STACK, stackLimit, stackLimit, "setrlimit stack");
		}

		// CPU limits have whole-second resolution; the runner owns the wall deadline.
		const auto softTimeLimit = static_cast<rlim_t>((timeLimitMs + 999) / 1000);
		setLimit(errorPipe[1], RLIMIT_CPU, softTimeLimit, softTimeLimit + 1, "setrlimit cpu");

		if (execlp("bash", "bash", "-c", runCmd.c_str(), NULL) == -1) {
			failChild(errorPipe[1], "execlp bash");
		}
	}

	return 0;
}
