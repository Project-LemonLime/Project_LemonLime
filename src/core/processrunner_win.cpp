/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef Q_OS_WIN32

#include "processrunner_win.h"
#include "base/LemonLog.hpp"
#include "memoryunits.h"
#include "windowsprocessutils.h"
#include "windowssandbox.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <QtMath>

#include <windows.h>
// Don't change the include order
// psapi need type BOOL, which is included in windef.h
#include <Psapi.h>

#define LEMON_MODULE_NAME "ProcessRunner"

using namespace Lemon::Windows;

namespace {
	class ExecutionJob {
		Handle handle{CreateJobObjectW(nullptr, nullptr)};

	  public:
		HANDLE get() const { return handle.get(); }
		bool apply(Lemon::Execution::Bytes limit) {
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
	Lemon::Execution::Bytes sampleMemory(HANDLE process) {
		PROCESS_MEMORY_COUNTERS_EX info{};
		info.cb = sizeof(info);
		if (! GetProcessMemoryInfo(process, reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&info), sizeof(info)))
			return -1;
		// PeakPagefileUsage is the process lifetime peak of private committed memory.
		return static_cast<Lemon::Execution::Bytes>(info.PeakPagefileUsage);
	}

	bool killAndReap(HANDLE process, HANDLE job) {
		if (! TerminateJobObject(job, 1) && WaitForSingleObject(process, 0) != WAIT_OBJECT_0)
			return false;
		return WaitForSingleObject(process, 2000) == WAIT_OBJECT_0;
	}
} // namespace

ProcessRunnerResult WinProcessRunner::run() {
	ProcessRunnerResult res;
	res.result = CorrectAnswer;
	int extraTime = qCeil(qMax(2000, config.timeLimit * 2) * config.extraTimeRatio);

	std::unique_ptr<WindowsSandbox> sandbox;
	if (config.sandboxSettings.enabled) {
		sandbox = std::make_unique<WindowsSandbox>(config);
		QElapsedTimer preparation;
		preparation.start();
		QString error;
		const bool prepared = sandbox->prepare(error);
		res.preparationTime = preparation.elapsed();
		res.runtimeAclUpdates = sandbox->aclUpdates();
		res.runtimeCacheHit = sandbox->cacheHit();
		if (! prepared) {
			res.result = CannotStartProgram;
			res.message = error;
			WARN(error);
			return res;
		}
	}

	SetErrorMode(SEM_NOGPFAULTERRORBOX);
	STARTUPINFOEX siex;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&siex, sizeof(siex));
	siex.StartupInfo.cb = sizeof(siex);
	siex.StartupInfo.dwFlags = STARTF_USESTDHANDLES;
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = TRUE;
	if (sandbox)
		sa.nLength = sizeof(sa);

	if (config.standardInputCheck) {
		siex.StartupInfo.hStdInput = CreateFileW((const WCHAR *)(config.inputFile.utf16()), GENERIC_READ,
		                                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa,
		                                         OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if (config.standardOutputCheck) {
		siex.StartupInfo.hStdOutput =
		    CreateFileW((const WCHAR *)((config.workingDirectory + "_tmpout").utf16()), GENERIC_WRITE,
		                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, CREATE_ALWAYS,
		                FILE_ATTRIBUTE_NORMAL, NULL);
	}

	siex.StartupInfo.hStdError =
	    CreateFileW((const WCHAR *)((config.workingDirectory + "_tmperr").utf16()), GENERIC_WRITE,
	                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, CREATE_ALWAYS,
	                FILE_ATTRIBUTE_NORMAL, NULL);

	auto closeStdIO = qScopeGuard([&] {
		if (config.standardInputCheck)
			CloseHandle(siex.StartupInfo.hStdInput);

		if (config.standardOutputCheck)
			CloseHandle(siex.StartupInfo.hStdOutput);

		CloseHandle(siex.StartupInfo.hStdError);
	});

	Lemon::Execution::Bytes memoryLimit;
	ExecutionJob job;
	if (! Lemon::Execution::memoryBytes(config.memoryLimit, memoryLimit) || ! job.apply(memoryLimit) ||
	    (sandbox && ! job.restrictToOneProcess())) {
		res.result = CannotStartProgram;
		res.message = "Cannot configure memory prelimit";
		return res;
	}
	if (sandbox && ! sandbox->prepareProcess(siex, job.get(), res.message)) {
		res.result = CannotStartProgram;
		return res;
	}

	// Need 4 \0 to end the environment string, see CreateProcessW() documentation
	const auto &environment = sandbox ? sandbox->environment() : config.environment;
	QString environmentValues =
	    environment.toStringList().join(QChar('\0')) + QChar('\0') + QChar('\0') + QChar('\0') + QChar('\0');

	QString commandLine = QString(R"("%1" %2)").arg(config.executableFile).arg(config.arguments);
	if (! CreateProcessW(nullptr, reinterpret_cast<WCHAR *>(commandLine.data()), nullptr, &sa, TRUE,
	                     CREATE_SUSPENDED | HIGH_PRIORITY_CLASS | EXTENDED_STARTUPINFO_PRESENT |
	                         DETACHED_PROCESS | CREATE_UNICODE_ENVIRONMENT,
	                     (LPVOID)(environmentValues.utf16()),
	                     (const WCHAR *)(config.workingDirectory.utf16()), (STARTUPINFO *)(&siex), &pi)) {
		const DWORD code = GetLastError();
		res.score = 0;
		res.result = CannotStartProgram;
		res.message = "Failed to create process";
		WARN(config.executableFile, "Failed to be started");
		WARN("Last Error code:", code);
		return res;
	}

	auto closeProcessHandle = qScopeGuard([&] {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	});

	auto reap = qScopeGuard([&] {
		if (! killAndReap(pi.hProcess, job.get()))
			WARN("Process reaping deadline exceeded");
	});
	std::stop_callback stopCallback(config.stopToken, [&] {
		TerminateJobObject(job.get(), 1);
		// Also cover cancellation before the suspended process joins the Job.
		TerminateProcess(pi.hProcess, 1);
	});
	if (config.stopToken.stop_requested() || cancellation.isCanceled())
		return res;
	if ((! sandbox && ! AssignProcessToJobObject(job.get(), pi.hProcess)) ||
	    ResumeThread(pi.hThread) == DWORD(-1)) {
		TerminateProcess(pi.hProcess, 1);
		res.result = CannotStartProgram;
		res.message = "Cannot attach or resume process";
		if (config.stopToken.stop_requested()) {
			res.result = CorrectAnswer;
			res.message.clear();
		}
		return res;
	}
	Handle deadline(CreateWaitableTimerW(nullptr, TRUE, nullptr));
	LARGE_INTEGER due{};
	// Negative values specify a relative interval in 100 ns units.
	due.QuadPart = -qMax(1LL, (static_cast<long long>(config.timeLimit) + extraTime) * 10000LL);
	if (! deadline || ! SetWaitableTimer(deadline.get(), &due, 0, nullptr, nullptr, FALSE)) {
		const DWORD code = GetLastError();
		res.result = CannotStartProgram;
		res.message = "Cannot configure execution timers";
		WARN("Last Error code:", code);
		return res;
	}
	HANDLE handles[] = {pi.hProcess, deadline.get()};
	DWORD state = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
	if (config.stopToken.stop_requested() || cancellation.isCanceled()) {
		if (! killAndReap(pi.hProcess, job.get()))
			WARN("Process reaping deadline exceeded");
		res.memoryUsed = sampleMemory(pi.hProcess);
		return res;
	}
	if (state == WAIT_FAILED) {
		WARN("Last Error code:", GetLastError());
		res.result = CannotStartProgram;
		res.message = "Cannot wait for process events";
		return res;
	}
	if (state == WAIT_OBJECT_0 + 1) {
		if (! killAndReap(pi.hProcess, job.get()))
			WARN("Process reaping deadline exceeded");
		res.result = TimeLimitExceeded;
	}
	auto sample = sampleMemory(pi.hProcess);
	res.memoryUsed = sample;
	if (res.result == TimeLimitExceeded)
		return res;
	if (sample < 0) {
		res.result = CannotStartProgram;
		res.message = "Process statistics unavailable";
		return res;
	}
	if (memoryLimit >= 0 && res.memoryUsed > memoryLimit) {
		res.result = MemoryLimitExceeded;
		return res;
	}
	unsigned long exitCode;
	GetExitCodeProcess(pi.hProcess, &exitCode);

	if (exitCode != 0) {

		res.score = 0;
		res.result = RunTimeError;
		QFile file(config.workingDirectory + "_tmperr");

		if (file.open(QFile::ReadOnly)) {
			QTextStream stream(&file);
			res.message = stream.readAll().right(1024);
			file.close();
		}

		res.timeUsed = -1;
		LOG(config.executableFile, "Unexpectedly exited.");
		LOG("Exit code", exitCode);
		return res;
	}

	FILETIME creationTime, exitTime, kernelTime, userTime;
	GetProcessTimes(pi.hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
	SYSTEMTIME realUserTime, realKernelTime;
	FileTimeToSystemTime(&userTime, &realUserTime);
	FileTimeToSystemTime(&kernelTime, &realKernelTime);
	res.timeUsed = realUserTime.wMilliseconds + realUserTime.wSecond * 1000 +
	               realUserTime.wMinute * 60 * 1000 + realUserTime.wHour * 60 * 60 * 1000;
	int kernelTimeUsed = realKernelTime.wMilliseconds + realKernelTime.wSecond * 1000 +
	                     realKernelTime.wMinute * 60 * 1000 + realKernelTime.wHour * 60 * 60 * 1000;
	if (sandbox) {
		const QDir work(config.workingDirectory);
		const auto outputPath =
		    work.filePath(config.standardOutputCheck ? QString("_tmpout") : config.outputFileName);
		Handle output(CreateFileW(wide(QDir::toNativeSeparators(outputPath)), FILE_READ_ATTRIBUTES,
		                          FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
		                          FILE_FLAG_OPEN_REPARSE_POINT, nullptr));
		if (output) {
			BY_HANDLE_FILE_INFORMATION info{};
			if (! GetFileInformationByHandle(output.get(), &info) ||
			    (info.dwFileAttributes & (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DIRECTORY)) ||
			    info.nNumberOfLinks > 1) {
				res.result = CannotStartProgram;
				res.message = QObject::tr("Internal error (See log for further information)");
				WARN("Sandbox output must be a regular private file.");
				return res;
			}
		}
		LOG("Windows sandbox preparation (ms):", res.preparationTime, "ACL updates:", res.runtimeAclUpdates,
		    "Cache hit:", res.runtimeCacheHit);
	}
	LOG(config.executableFile, "Successfully exited.");
	LOG("User Time Used:", res.timeUsed, "ms, Kernel Time Used:", kernelTimeUsed,
	    "ms, User+Kernel Time Used:", res.timeUsed + kernelTimeUsed, "ms, Memory Used:", res.memoryUsed,
	    "bytes");

	return res;
}

#endif
