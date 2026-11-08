// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "memorylimiter.h"
#include "windowsprocessutils.h"
#ifdef Q_OS_WIN
#include <psapi.h>
namespace Lemon::Windows {
	class ExecutionJob {
		Handle handle{CreateJobObjectW(nullptr, nullptr)};

	  public:
		HANDLE get() const { return handle.get(); }
		bool apply(Execution::Bytes limit) {
			if (! handle)
				return false;
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION info{};
			info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
			if (limit >= 0) {
				if (static_cast<unsigned long long>(limit) > SIZE_MAX) {
					SetLastError(ERROR_INVALID_PARAMETER);
					return false;
				}
				info.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;
				info.ProcessMemoryLimit = static_cast<SIZE_T>(limit);
			}
			return SetInformationJobObject(get(), JobObjectExtendedLimitInformation, &info, sizeof(info));
		}
		bool restrictToOneProcess() {
			JOBOBJECT_EXTENDED_LIMIT_INFORMATION info{};
			if (! QueryInformationJobObject(get(), JobObjectExtendedLimitInformation, &info, sizeof(info),
			                                nullptr))
				return false;
			info.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
			info.BasicLimitInformation.ActiveProcessLimit = 1;
			return SetInformationJobObject(get(), JobObjectExtendedLimitInformation, &info, sizeof(info));
		}
	};
	// Platform operations stay separate from the shared memory-limit policy.
	inline Execution::Bytes sampleMemory(HANDLE process) {
		PROCESS_MEMORY_COUNTERS_EX info{};
		info.cb = sizeof(info);
		if (! GetProcessMemoryInfo(process, reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&info), sizeof(info)))
			return -1;
		// PeakPagefileUsage is the process lifetime peak of private committed memory.
		return static_cast<Execution::Bytes>(info.PeakPagefileUsage);
	}

	inline bool killAndReap(HANDLE process, HANDLE job) {
		if (! TerminateJobObject(job, 1) && WaitForSingleObject(process, 0) != WAIT_OBJECT_0)
			return false;
		return WaitForSingleObject(process, 2000) == WAIT_OBJECT_0;
	}
} // namespace Lemon::Windows
#endif
