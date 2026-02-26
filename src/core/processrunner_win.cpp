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

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QtMath>

#include <random>

#include <windows.h>
// Don't change the include order
// psapi need type BOOL, which is included in windef.h
#include <Psapi.h>
#include <UserEnv.h>
#include <VersionHelpers.h>
#include <accctrl.h>
#include <aclapi.h>

#define LEMON_MODULE_NAME "ProcessRunner"

namespace {

	QString getRandomString(int length) {
		static const QString possibleCharacters(
		    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
		std::mt19937 gen(std::random_device{}());

		std::uniform_int_distribution<int> rand(0, possibleCharacters.length());
		QString randomString;
		for (int i = 0; i < length; ++i) {
			int index = rand(gen) % possibleCharacters.length();
			QChar nextChar = possibleCharacters.at(index);
			randomString.append(nextChar);
		}
		return randomString;
	}

	bool grantFileAccessPermissions(PSID appContainerSid, LPWSTR name, ACCESS_MASK accessMask) {
		PACL oldAcl, newAcl = nullptr;
		DWORD status;
		EXPLICIT_ACCESSW ea;
		do {
			ea.grfAccessMode = GRANT_ACCESS;
			ea.grfAccessPermissions = accessMask;
			ea.grfInheritance = OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE;
			ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
			ea.Trustee.pMultipleTrustee = nullptr;
			ea.Trustee.ptstrName = (PWSTR)appContainerSid;
			ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
			ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
			status = GetNamedSecurityInfoW(name, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr,
			                               &oldAcl, nullptr, nullptr);
			if (status != ERROR_SUCCESS) {
				return false;
			}
			if (SetEntriesInAclW(1, &ea, oldAcl, &newAcl) != ERROR_SUCCESS) {
				return false;
			}
			status = SetNamedSecurityInfoW(name, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr,
			                               newAcl, nullptr);
			if (status != ERROR_SUCCESS) {
				break;
			}
		} while (false);
		if (newAcl) {
			LocalFree(newAcl);
		}
		return status == ERROR_SUCCESS;
	}

} // namespace

ProcessRunnerResult WinProcessRunner::run() {
	ProcessRunnerResult res;
	res.result = CorrectAnswer;
	int extraTime = qCeil(qMax(2000, config.timeLimit * 2) * config.extraTimeRatio);

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

	PSID appContainerSID{nullptr};
	QString appContainerName = "Lemonlime" + getRandomString(10);

#ifdef WINDOWS_APPCONTAINER_SUPPORT
	//  Create Window App Container (Windows 8+)

	auto hResult = CreateAppContainerProfile((const WCHAR *)(appContainerName.utf16()),
	                                         (const WCHAR *)(appContainerName.utf16()),
	                                         (const WCHAR *)(appContainerName.utf16()), nullptr, 0,
	                                         &appContainerSID); // Without any Permissions

	if (hResult != S_OK) {
		res.score = 0;
		res.result = CannotStartProgram;
		res.message = "Failed to create app container";
		WARN("Failed to create app container");
		switch (hResult) {
			case E_ACCESSDENIED:
				WARN("Possible error: Access Denied");
				break;
			case HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS):
				WARN("Possible error:", appContainerName, "Already Exists");
				break;
			case E_INVALIDARG:
				WARN("Possible error: Invalid Args, Please Report Bugs");
				break;
			default:
				WARN("Unknown error");
		}
		return res;
	}

	auto cleanupContainer = qScopeGuard([&] {
		FreeSid(appContainerSID);
		DeleteAppContainerProfile((const WCHAR *)(appContainerName.utf16()));
	});

	SECURITY_CAPABILITIES sc;
	ZeroMemory(&sc, sizeof(sc));

	sc.AppContainerSid = appContainerSID;

	SIZE_T attributesSize;

	InitializeProcThreadAttributeList(nullptr, 3, 0, &attributesSize);

	auto attributesListBuffer = std::make_unique<std::byte[]>(attributesSize);
	siex.lpAttributeList = reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attributesListBuffer.get());

	if (! InitializeProcThreadAttributeList(siex.lpAttributeList, 3, 0, &attributesSize)) {
		res.score = 0;
		res.result = CannotStartProgram;
		res.message = "Internal error (See log for further information)";
		WARN("Failed to InitializeProcThreadAttributeList()");
		WARN("Last Error code:", GetLastError());
		return res;
	}

	// Make App Run in App Container
	if (! UpdateProcThreadAttribute(siex.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES, &sc,
	                                sizeof(sc), nullptr, nullptr)) {
		res.score = 0;
		res.result = CannotStartProgram;
		res.message = "Internal error (See log for further information)";
		WARN("Failed to UpdateProcThreadAttribute()");
		WARN("Unable to make app run in app container");
		WARN("Last Error code:", GetLastError());
		return res;
	}

	// Ban Child Processs

	DWORD childProcessAttribute = PROCESS_CREATION_CHILD_PROCESS_RESTRICTED;

	if (! UpdateProcThreadAttribute(siex.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_CHILD_PROCESS_POLICY,
	                                &childProcessAttribute, sizeof(childProcessAttribute), nullptr,
	                                nullptr)) {
		res.score = 0;
		res.result = CannotStartProgram;
		res.message = "Internal error (See log for further information)";
		WARN("Failed to UpdateProcThreadAttribute()");
		WARN("Unable to limit fork");
		WARN("Last Error code:", GetLastError());
		return res;
	}

	// Load dlls
	// Todo: If there's too many files under the path it'll be very slow
	for (auto f : config.environment.value("PATH").split(';')) {
		grantFileAccessPermissions(appContainerSID, (WCHAR *)f.utf16(),
		                           FILE_GENERIC_EXECUTE | FILE_GENERIC_READ);
	}

	if (! config.standardInputCheck) {
		grantFileAccessPermissions(appContainerSID,
		                           (WCHAR *)(config.workingDirectory + config.inputFileName).utf16(),
		                           FILE_GENERIC_READ);
	}

	if (! config.standardOutputCheck) {
		grantFileAccessPermissions(appContainerSID, (WCHAR *)(config.workingDirectory).utf16(),
		                           FILE_GENERIC_READ | FILE_ADD_FILE);
		grantFileAccessPermissions(appContainerSID,
		                           (WCHAR *)(config.workingDirectory + config.outputFileName).utf16(),
		                           FILE_GENERIC_READ | FILE_GENERIC_WRITE);
	}

#endif

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

	// Need 4 \0 to end the environment string, see CreateProcessW() documentation
	QString environmentValues =
	    config.environment.toStringList().join(QChar('\0')) + '\0' + '\0' + '\0' + '\0';

	QString commandLine = QString(R"("%1" %2)").arg(config.executableFile).arg(config.arguments);

	if (! CreateProcessW(nullptr, (WCHAR *)(commandLine).utf16(), nullptr, &sa, TRUE,
	                     HIGH_PRIORITY_CLASS | EXTENDED_STARTUPINFO_PRESENT | DETACHED_PROCESS |
	                         CREATE_UNICODE_ENVIRONMENT,
	                     (LPVOID)(environmentValues.utf16()),
	                     (const WCHAR *)(config.workingDirectory.utf16()), (STARTUPINFO *)(&siex), &pi)) {
		res.score = 0;
		res.result = CannotStartProgram;
		res.message = "Failed to create process";
		WARN(config.executableFile, "Failed to be started");
		WARN("Last Error code:", GetLastError());
		return res;
	}

	auto closeProcessHandle = qScopeGuard([&] {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	});

	PROCESS_MEMORY_COUNTERS_EX memoryInfo;
	ZeroMemory(&memoryInfo, sizeof(memoryInfo));
	memoryInfo.cb = sizeof(memoryInfo);

	if (config.memoryLimit != -1) {
		GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS *)&memoryInfo, sizeof(memoryInfo));

		if (qMax(memoryInfo.PrivateUsage, memoryInfo.PeakWorkingSetSize) > config.memoryLimit * 1024 * 1024) {
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
			    config.memoryLimit * 1024U * 1024) {
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
	LOG(config.executableFile, "Successfully exited.");
	LOG("User Time Used:", res.timeUsed, "ms, Kernel Time Used:", kernelTimeUsed,
	    "ms, User+Kernel Time Used:", res.timeUsed + kernelTimeUsed, "ms, Memory Used:", res.memoryUsed,
	    "bytes");

	return res;
}

#endif
