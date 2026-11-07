// SPDX-License-Identifier: GPL-3.0-or-later
#include "memoryusage.h"
#include <cstdio>
#include <cstring>
#include <string>
using namespace Lemon::Execution;
MemorySample sampleMemory(pid_t pid) {
	MemorySample result;
	auto path = "/proc/" + std::to_string(pid) + "/status";
	FILE *file = fopen(path.c_str(), "r");
	if (! file)
		return result;
	char line[512];
	while (fgets(line, sizeof(line), file)) {
		long long kib = 0;
		if (sscanf(line, "VmSize: %lld kB", &kib) == 1 || sscanf(line, "VmPeak: %lld kB", &kib) == 1)
			result.declared = std::max(result.declared, Bytes(kib) * 1024);
		if (sscanf(line, "VmRSS: %lld kB", &kib) == 1 || sscanf(line, "VmHWM: %lld kB", &kib) == 1)
			result.actual = std::max(result.actual, Bytes(kib) * 1024);
	}
	fclose(file);
	return result;
}
Bytes maxRSSBytes(long value) { return Bytes(value) * 1024; }
