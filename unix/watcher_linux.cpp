/*
 * SPDX-FileCopyrightText: 2011-2019 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2019-2023 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "memoryusage.h"
#include <algorithm>
#include <cstdio>
#include <string>
#include <sys/resource.h>
using namespace Lemon::Execution;
Bytes sampleMemory(pid_t pid) {
	Bytes result = -1;
	auto path = "/proc/" + std::to_string(pid) + "/status";
	FILE *file = fopen(path.c_str(), "r");
	if (! file)
		return result;
	char line[512];
	while (fgets(line, sizeof(line), file)) {
		long long kib = 0;
		if (sscanf(line, "VmSize: %lld kB", &kib) == 1 || sscanf(line, "VmPeak: %lld kB", &kib) == 1)
			result = std::max(result, Bytes(kib) * KiB);
	}
	fclose(file);
	return result;
}

bool applyMemoryLimit(Bytes limit) {
	if (limit >= 0) {
		rlimit memory{static_cast<rlim_t>(limit), static_cast<rlim_t>(limit)};
		if (setrlimit(RLIMIT_AS, &memory) != 0)
			return false;
	}
	rlimit stack{};
	if (getrlimit(RLIMIT_STACK, &stack) != 0)
		return false;
	stack.rlim_cur = std::min(stack.rlim_max, static_cast<rlim_t>(limit < 0 ? 2147483647LL : limit));
	return setrlimit(RLIMIT_STACK, &stack) == 0;
}
