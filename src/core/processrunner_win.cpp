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
#include "windowsprocessutils.h"
#include "windowssandbox.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QtMath>

#include <windows.h>
// Don't change the include order
// psapi need type BOOL, which is included in windef.h
#include <Psapi.h>

#define LEMON_MODULE_NAME "ProcessRunner"

using namespace Lemon::Windows;

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

	if (sandbox && ! sandbox->prepareProcess(siex, res.message)) {
		res.result = CannotStartProgram;
		return res;
	}

	// Need 4 \0 to end the environment string, see CreateProcessW() documentation
	const auto &environment = sandbox ? sandbox->environment() : config.environment;
	QString environmentValues =
	    environment.toStringList().join(QChar('\0')) + QChar('\0') + QChar('\0') + QChar('\0') + QChar('\0');

	QString commandLine = QString(R"("%1" %2)").arg(config.executableFile).arg(config.arguments);
	if (! CreateProcessW(nullptr, reinterpret_cast<WCHAR *>(commandLine.data()), nullptr, &sa, TRUE,
	                     HIGH_PRIORITY_CLASS | EXTENDED_STARTUPINFO_PRESENT | DETACHED_PROCESS |
	                         CREATE_UNICODE_ENVIRONMENT,
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
		if (sandbox)
			WaitForSingleObject(pi.hProcess, 5000);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	});

	PROCESS_MEMORY_COUNTERS_EX memoryInfo;
	ZeroMemory(&memoryInfo, sizeof(memoryInfo));
	memoryInfo.cb = sizeof(memoryInfo);

	if (config.memoryLimit != -1) {
		GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS *)&memoryInfo, sizeof(memoryInfo));

		if (qMax(memoryInfo.PrivateUsage, memoryInfo.PeakWorkingSetSize) >
		    1ll * config.memoryLimit * 1024 * 1024) {
			TerminateProcess(pi.hProcess, 0);

			res.score = 0;
			res.result = MemoryLimitExceeded;
			res.memoryUsed = res.timeUsed = -1;
			return res;
		}
	}

	bool isProgramFinishedInExtraTimeLimit = false;
	QElapsedTimer timer;
	timer.start();

	while (timer.elapsed() <= config.timeLimit + extraTime) {
		if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) {
			isProgramFinishedInExtraTimeLimit = true;
			break;
		}

		if (config.memoryLimit != -1) {
			GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS *)&memoryInfo, sizeof(memoryInfo));

			if (qMax(memoryInfo.PrivateUsage, memoryInfo.PeakWorkingSetSize) >
			    1ll * config.memoryLimit * 1024 * 1024) {
				TerminateProcess(pi.hProcess, 0);
				res.score = 0;
				res.result = MemoryLimitExceeded;
				res.memoryUsed = res.timeUsed = -1;
				return res;
			}
		}

		QCoreApplication::processEvents();

		if (stopFlag) {
			TerminateProcess(pi.hProcess, 0);

			return res;
		}

		QThread::msleep(10);
	}

	if (! isProgramFinishedInExtraTimeLimit) {
		TerminateProcess(pi.hProcess, 0);

		res.score = 0;
		res.result = TimeLimitExceeded;
		res.timeUsed = -1;
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

		res.memoryUsed = res.timeUsed = -1;
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
	GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS *)&memoryInfo, sizeof(memoryInfo));
	res.memoryUsed = memoryInfo.PeakWorkingSetSize;
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
