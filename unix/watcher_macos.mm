// SPDX-License-Identifier: GPL-3.0-or-later
#include "memoryusage.h"
#include <libproc.h>
#include <sys/proc_info.h>
using namespace Lemon::Execution;
Bytes sampleMemory(pid_t pid) {
	proc_taskinfo info{};
	if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &info, sizeof(info)) != sizeof(info))
		return -1;
	Bytes rss = static_cast<Bytes>(info.pti_resident_size);
	return rss;
}
Bytes maxRSSBytes(long value) { return value; }
