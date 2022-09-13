/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "judgingthread.h"
#include "base/settings.h"
#include "core/judgesharedvariables.h"
#include "core/task.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QTime>
#include <QUuid>
#include <QtMath>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef Q_OS_WIN32

#include <Psapi.h>
#include <UserEnv.h>
#include <VersionHelpers.h>
#include <accctrl.h>
#include <aclapi.h>
#include <windows.h>

#endif

JudgingThread::JudgingThread(QObject *parent) : QThread(parent) {
	moveToThread(this);
	// checkRejudgeMode = false;
	needRejudge = false;
	stopJudging = false;
	timeUsed = -1;
	memoryUsed = -1;
	judgedTimes = 0;
	// QTime   t =  QTime::currentTime();
	// qsrand(static_cast<unsigned int>(t.msec() + t.second() * 1000));
}

/*void JudgingThread::setCheckRejudgeMode(bool check)
{
    checkRejudgeMode = check;
}*/

void JudgingThread::setExtraTimeRatio(double ratio) { extraTimeRatio = ratio; }

void JudgingThread::setEnvironment(const QProcessEnvironment &env) { environment = env; }

void JudgingThread::setWorkingDirectory(const QString &directory) { workingDirectory = directory; }

void JudgingThread::setSpecialJudgeTimeLimit(int limit) { specialJudgeTimeLimit = limit; }

void JudgingThread::setExecutableFile(const QString &fileName) { executableFile = fileName; }

void JudgingThread::setArguments(const QString &argumentsList) { arguments = argumentsList; }

void JudgingThread::setAnswerFile(const QString &fileName) { answerFile = fileName; }

void JudgingThread::setInputFile(const QString &fileName) { inputFile = fileName; }

void JudgingThread::setOutputFile(const QString &fileName) { outputFile = fileName; }

void JudgingThread::setDiffPath(const QString &path) { diffPath = path; }

void JudgingThread::setTask(Task *_task) { task = _task; }

void JudgingThread::setFullScore(int score) { fullScore = score; }

void JudgingThread::setTimeLimit(int limit) { timeLimit = limit; }

void JudgingThread::setMemoryLimit(int limit) { memoryLimit = limit; }

auto JudgingThread::getTimeUsed() const -> int { return timeUsed; }

auto JudgingThread::getMemoryUsed() const -> int { return memoryUsed; }

auto JudgingThread::getScore() const -> int { return score; }

auto JudgingThread::getFullScore() const -> int { return fullScore; }

auto JudgingThread::getJudgeTimes() const -> int { return judgedTimes; }

auto JudgingThread::getResult() const -> ResultState { return result; }

auto JudgingThread::getMessage() const -> const QString & { return message; }

auto JudgingThread::getNeedRejudge() const -> bool { return needRejudge; }

void JudgingThread::stopJudgingSlot() { stopJudging = true; }

class BufferedStreamReader {

  public:
	static const int BUFFER_SIZE = 1 << 18; // 128 KiB
	explicit BufferedStreamReader(const char *filename) : file(fopen(filename, "r")) {
		result.reserve(32);
		bufPos = buffer + BUFFER_SIZE;
		bufEnd = buffer + BUFFER_SIZE;
		lineNumber = 1;
		isEof = 0;
	}

	BufferedStreamReader(const BufferedStreamReader &) = delete;
	BufferedStreamReader &operator=(const BufferedStreamReader &) = delete;

	~BufferedStreamReader() {
		if (file)
			fclose(file);
	}

  private:
	FILE *file;
	char buffer[BUFFER_SIZE], *bufEnd, *bufPos;
	bool isEof;
	int lineNumber;
	std::string result; // Fixed Length String,Optimize for speed
	auto peekChar() -> char;
	void tryNextLine();

  public:
	auto valid() const -> bool { return file != nullptr; }
	auto eof() const -> bool { return isEof; }
	auto line() const -> int { return lineNumber; }
	auto nextUntilNewLine() -> std::string_view;
	auto nextUntilSpace() -> std::string_view;
};

auto BufferedStreamReader::peekChar() -> char {
	if (isEof) {
		return '\0';
	}
	if (bufPos == bufEnd) {
		bufPos = buffer;
		bufEnd = buffer + fread(buffer, 1, BUFFER_SIZE, file);
		if (bufPos == bufEnd) {
			isEof = true;
			return '\0';
		}
	}
	return *bufPos;
}

void BufferedStreamReader::tryNextLine() {
	char c = peekChar();
	if (c != '\r' && c != '\n')
		return;
	if (c == '\r')
		bufPos++;
	c = peekChar();
	if (c == '\n')
		bufPos++;
	lineNumber++;
}

auto BufferedStreamReader::nextUntilNewLine() -> std::string_view {
	result.clear();
	tryNextLine();
	for (int i = 0; i < 32; i++) {
		char c = peekChar();
		if (c == '\n' || c == '\r' || eof())
			break;
		result.push_back(c);
		bufPos++;
	}
	return result;
}

auto BufferedStreamReader::nextUntilSpace() -> std::string_view {
	result.clear();
	for (char c = peekChar(); std::isspace(c) && ! eof(); c = peekChar())
		if (c == '\r' || c == '\n')
			tryNextLine();
		else
			bufPos++;
	if (eof())
		return result;
	for (int i = 0; i < 32; i++) {
		char c = peekChar();
		if (std::isspace(c) || eof())
			break;
		result.push_back(c);
		bufPos++;
	}
	return result;
}

void JudgingThread::compareLineByLine(const QString &contestantOutput) {
	BufferedStreamReader contestantReader(contestantOutput.toLocal8Bit().data());
	BufferedStreamReader standardOutputReader(outputFile.toLocal8Bit().data());

	if (! contestantReader.valid()) {
		score = 0;
		result = FileError;
		message = tr(R"(Cannot open contestant's output file)");
		return;
	}

	if (! standardOutputReader.valid()) {
		score = 0;
		result = FileError;
		message = tr(R"(Cannot open standard output file)");
		return;
	}

	while (true) {
		auto contestantLine = contestantReader.nextUntilNewLine();
		auto standardOutputLine = standardOutputReader.nextUntilNewLine();

		if (contestantReader.eof() && ! standardOutputReader.eof()) {
			score = 0;
			result = WrongAnswer;
			message =
			    tr(R"(On line %1, Contestant's output has less contents)").arg(standardOutputReader.line());
			return;
		}

		if (! standardOutputReader.eof() && contestantReader.eof()) {
			score = 0;
			result = OutputLimitExceeded;
			message =
			    tr(R"(On line %1, Contestant's output has too much contents)").arg(contestantReader.line());
			return;
		}

		if (contestantLine != standardOutputLine) {
			score = 0;
			result = WrongAnswer;
			message = tr(R"(On line %3, Read "%1" but expect "%2")")
			              .arg(contestantLine.data())
			              .arg(standardOutputLine.data())
			              .arg(contestantReader.line());
			return;
		}

		if (contestantReader.eof() && standardOutputReader.eof())
			break;
	}

	score = fullScore;
	result = CorrectAnswer;
}

void JudgingThread::compareIgnoreSpaces(const QString &contestantOutput) {
	BufferedStreamReader contestantReader(contestantOutput.toLocal8Bit().data());
	BufferedStreamReader standardOutputReader(outputFile.toLocal8Bit().data());

	if (! contestantReader.valid()) {
		score = 0;
		result = FileError;
		message = tr(R"(Cannot open contestant's output file)");
		return;
	}

	if (! standardOutputReader.valid()) {
		score = 0;
		result = FileError;
		message = tr(R"(Cannot open standard output file)");
		return;
	}

	while (true) {
		auto contestantLine = contestantReader.nextUntilSpace();
		auto standardOutputLine = standardOutputReader.nextUntilSpace();

		if (contestantLine == standardOutputLine) {
			if (contestantReader.eof() && standardOutputReader.eof())
				break;

			if (contestantReader.line() != standardOutputReader.line()) {
				score = 0;
				result = PresentationError;
				message = tr("Presentation error on line %1").arg(contestantReader.line());
				return;
			}
			continue;
		} else {
			if (contestantLine.size() == 0 && contestantReader.eof() && ! standardOutputReader.eof()) {
				score = 0;
				result = WrongAnswer;
				message = tr(R"(On line %1, Contestant's output has less contents)")
				              .arg(standardOutputReader.line());
				return;
			}

			if (standardOutputLine.size() == 0 && ! standardOutputReader.eof() && contestantReader.eof()) {
				score = 0;
				result = OutputLimitExceeded;
				message = tr(R"(On line %1, Contestant's output has too much contents)")
				              .arg(contestantReader.line());
				return;
			}

			score = 0;
			result = WrongAnswer;
			message = tr(R"(On line %3, Read "%1" but expect "%2")")
			              .arg(contestantLine.data())
			              .arg(standardOutputLine.data())
			              .arg(contestantReader.line());
			return;
		}
	}

	score = fullScore;
	result = CorrectAnswer;
}

void JudgingThread::compareWithDiff(const QString &contestantOutput) {
	QString cmd = diffPath;
	QStringList cmdArgs =
	    (QStringList(task->getDiffArguments())
	     << QFileInfo(outputFile).absoluteFilePath().replace('/', QDir::separator()) << contestantOutput);

	if (QProcess::execute(cmd, cmdArgs) != 0) {
		score = 0;
		result = WrongAnswer;
	} else {
		score = fullScore;
		result = CorrectAnswer;
	}
}

void JudgingThread::compareRealNumbers(const QString &contestantOutput) {
	FILE *contestantOutputFile = fopen(contestantOutput.toLocal8Bit().data(), "r");

	if (contestantOutputFile == nullptr) {
		score = 0;
		result = FileError;
		message = tr(R"(Cannot open contestant's output file)");
		return;
	}

	FILE *standardOutputFile = fopen(outputFile.toLocal8Bit().data(), "r");

	if (standardOutputFile == nullptr) {
		score = 0;
		result = FileError;
		message = tr(R"(Cannot open standard output file)");
		fclose(contestantOutputFile);
		return;
	}

	long double eps = 1;

	for (int i = 0; i < task->getRealPrecision(); i++)
		eps *= 0.1;

	long double a = NAN;
	long double b = NAN;
	int nowRow = 1;

	while (true) {
		int cnt1 = fscanf(contestantOutputFile, "%Lf", &a);
		int cnt2 = fscanf(standardOutputFile, "%Lf", &b);

		char temps = fgetc(standardOutputFile);

		if (cnt1 == 0) {
			score = 0;
			result = WrongAnswer;
			message = tr(R"(On line %1, Invalid characters in contestant's output file)").arg(nowRow);
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}

		if (cnt2 == 0) {
			score = 0;
			result = FileError;
			message = tr(R"(On line %1, Invalid characters in standard output file)").arg(nowRow);
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}

		if (cnt1 == EOF && cnt2 == EOF)
			break;

		if (cnt1 == EOF && cnt2 == 1) {
			score = 0;
			result = WrongAnswer;
			message = tr(R"(On line %1, Contestant's Output has less contents)").arg(nowRow);
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}

		if (cnt1 == 1 && cnt2 == EOF) {
			score = 0;
			result = OutputLimitExceeded;
			message = tr(R"(On line %1, Contestant's Output has too much contents)").arg(nowRow);
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}

		if (std::isnan(a) != std::isnan(b) || std::isinf(a) != std::isinf(b) ||
		    (std::abs(a - b) > eps && std::abs(a - b) > eps * std::abs(b))) {
			score = 0;
			result = WrongAnswer;
			message = tr(R"(On line %3, Read "%1" but expect "%2")")
			              .arg(a, 0, 'g', 18)
			              .arg(b, 0, 'g', 18)
			              .arg(nowRow);
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}

		QCoreApplication::processEvents();

		if (stopJudging) {
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}

		if (temps == '\r' || temps == '\n')
			nowRow++;
	}

	score = fullScore;
	result = CorrectAnswer;
	fclose(contestantOutputFile);
	fclose(standardOutputFile);
}

void JudgingThread::specialJudge(const QString &fileName) {
	if (! QFileInfo::exists(inputFile)) {
		score = 0;
		result = FileError;
		message = tr("Cannot find standard input file");
		return;
	}

	if (! QFileInfo::exists(fileName)) {
		score = 0;
		result = FileError;
		message = tr(R"(Cannot find contestant's output file)");
		return;
	}

	if (! QFileInfo::exists(outputFile)) {
		score = 0;
		result = FileError;
		message = tr("Cannot find standard output file");
		return;
	}

	auto judge = std::make_unique<QProcess>(this);
	QStringList arguments;
	arguments << inputFile << fileName << outputFile << QString("%1").arg(fullScore);
	arguments << workingDirectory + "_score";
	arguments << workingDirectory + "_message";
	judge->start(Settings::dataPath() + task->getSpecialJudge(), arguments);

	if (! judge->waitForStarted(-1)) {
		score = 0;
		result = InvalidSpecialJudge;
		return;
	}

	QFile scoreFile(workingDirectory + "_score");
	QFile messageFile(workingDirectory + "_message");

	auto removeTempFiles = qScopeGuard([&] {
		scoreFile.remove();
		messageFile.remove();
	});

	QElapsedTimer timer;
	timer.start();
	bool flag = false;

	while (timer.elapsed() < specialJudgeTimeLimit) {
		if (judge->state() != QProcess::Running) {
			flag = true;
			break;
		}

		QCoreApplication::processEvents();

		if (stopJudging) {
			judge->kill();
			return;
		}

		msleep(10);
	}

	if (! flag) {
		judge->kill();
		score = 0;
		result = SpecialJudgeTimeLimitExceeded;
		return;
	}

	if (judge->exitCode() != 0) {
		score = 0;
		result = SpecialJudgeRunTimeError;
		return;
	}

	if (! scoreFile.open(QFile::ReadOnly)) {
		score = 0;
		result = InvalidSpecialJudge;
		return;
	}

	QTextStream scoreStream(&scoreFile);
	scoreStream >> score;

	if (scoreStream.status() == QTextStream::ReadCorruptData) {
		score = 0;
		result = InvalidSpecialJudge;
		return;
	}

	scoreFile.close();

	if (score < 0) {
		score = 0;
		result = InvalidSpecialJudge;
		return;
	}

	if (messageFile.open(QFile::ReadOnly)) {
		QTextStream messageStream(&messageFile);
		message = messageStream.readAll();
		messageFile.close();
	}

	if (score == 0)
		result = WrongAnswer;

	if (0 < score && score < fullScore)
		result = PartlyCorrect;

	if (score >= fullScore)
		result = CorrectAnswer;
}

#ifdef Q_OS_WIN

QString getRandomString(int length) {
	static const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
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
#endif
void JudgingThread::runProgram() {
	result = CorrectAnswer;
	int extraTime = qCeil(qMax(2000, timeLimit * 2) * extraTimeRatio);

	if (skipEnabled) {
		result = TimeLimitExceeded;
		score = 0;
		return;
	}

#ifdef Q_OS_WIN32
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

	//  Create Window App Container (Windows 8+)

	auto hResult = CreateAppContainerProfile((const WCHAR *)(appContainerName.utf16()),
	                                         (const WCHAR *)(appContainerName.utf16()),
	                                         (const WCHAR *)(appContainerName.utf16()), nullptr, 0,
	                                         &appContainerSID); // Without any Permissions

	if (hResult != S_OK) {
		score = 0;
		result = CannotStartProgram;
		message = "Failed to create app container";
		return;
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
		score = 0;
		result = CannotStartProgram;
		message = "Internal error (Failed to InitializeProcThreadAttributeList())";
		return;
	}

	// Make App Run in App Container
	if (! UpdateProcThreadAttribute(siex.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_SECURITY_CAPABILITIES, &sc,
	                                sizeof(sc), nullptr, nullptr)) {
		score = 0;
		result = CannotStartProgram;
		message = "Internal error (Failed to UpdateProcThreadAttribute())";
		return;
	}

	// Ban Child Processs

	DWORD childProcessAttribute = PROCESS_CREATION_CHILD_PROCESS_RESTRICTED;

	if (! UpdateProcThreadAttribute(siex.lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_CHILD_PROCESS_POLICY,
	                                &childProcessAttribute, sizeof(childProcessAttribute), nullptr,
	                                nullptr)) {
		score = 0;
		result = CannotStartProgram;
		message = "Internal error (Failed to UpdateProcThreadAttribute())";
		return;
	}

	// Load dlls
	// Todo: If there's too many files under the path it'll be very slow
	for (auto f : environment.value("PATH").split(';')) {
		grantFileAccessPermissions(appContainerSID, (WCHAR *)f.utf16(), FILE_ALL_ACCESS);
	}

	if (! task->getStandardInputCheck()) {
		grantFileAccessPermissions(appContainerSID,
		                           (WCHAR *)(workingDirectory + task->getInputFileName()).utf16(),
		                           FILE_GENERIC_READ);
	}

	if (! task->getStandardOutputCheck()) {
		grantFileAccessPermissions(appContainerSID, (WCHAR *)(workingDirectory).utf16(),
		                           FILE_GENERIC_READ | FILE_ADD_FILE);
		grantFileAccessPermissions(appContainerSID,
		                           (WCHAR *)(workingDirectory + task->getOutputFileName()).utf16(),
		                           FILE_GENERIC_READ | FILE_GENERIC_WRITE);
	}

	if (task->getStandardInputCheck()) {
		siex.StartupInfo.hStdInput = CreateFileW((const WCHAR *)(inputFile.utf16()), GENERIC_READ,
		                                         FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa,
		                                         OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if (task->getStandardOutputCheck()) {
		siex.StartupInfo.hStdOutput =
		    CreateFileW((const WCHAR *)((workingDirectory + "_tmpout").utf16()), GENERIC_WRITE,
		                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, CREATE_ALWAYS,
		                FILE_ATTRIBUTE_NORMAL, NULL);
	}

	siex.StartupInfo.hStdError =
	    CreateFileW((const WCHAR *)((workingDirectory + "_tmperr").utf16()), GENERIC_WRITE,
	                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, CREATE_ALWAYS,
	                FILE_ATTRIBUTE_NORMAL, NULL);

	auto closeStdIO = qScopeGuard([&] {
		if (task->getStandardInputCheck())
			CloseHandle(siex.StartupInfo.hStdInput);

		if (task->getStandardOutputCheck())
			CloseHandle(siex.StartupInfo.hStdOutput);

		CloseHandle(siex.StartupInfo.hStdError);
	});

	QString environmentValues = environment.toStringList().join(QChar('\0')) + '\0';

	if (! CreateProcessW((WCHAR *)executableFile.utf16(), (WCHAR *)(arguments).toStdWString().data(), NULL,
	                     &sa, TRUE, HIGH_PRIORITY_CLASS | EXTENDED_STARTUPINFO_PRESENT | DETACHED_PROCESS,
	                     (LPVOID)(environmentValues.toLocal8Bit().data()),
	                     (const WCHAR *)(workingDirectory.utf16()), (STARTUPINFO *)(&siex), &pi)) {
		score = 0;
		qDebug() << GetLastError();
		result = CannotStartProgram;
		message = "Failed to create process";
		return;
	}

	auto closeProcessHandle = qScopeGuard([&] {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	});

	PROCESS_MEMORY_COUNTERS_EX memoryInfo;
	ZeroMemory(&memoryInfo, sizeof(memoryInfo));
	memoryInfo.cb = sizeof(memoryInfo);

	if (memoryLimit != -1) {
		GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS *)&memoryInfo, sizeof(memoryInfo));

		if (qMax(memoryInfo.PrivateUsage, memoryInfo.PeakWorkingSetSize) > memoryLimit * 1024 * 1024) {
			TerminateProcess(pi.hProcess, 0);

			score = 0;
			result = MemoryLimitExceeded;
			memoryUsed = timeUsed = -1;
			return;
		}
	}

	bool isProgramFinishedInExtraTimeLimit = false;
	QElapsedTimer timer;
	timer.start();

	while (timer.elapsed() <= timeLimit + extraTime) {
		if (WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) {
			isProgramFinishedInExtraTimeLimit = true;
			break;
		}

		if (memoryLimit != -1) {
			GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS *)&memoryInfo, sizeof(memoryInfo));

			if (qMax(memoryInfo.PrivateUsage, memoryInfo.PeakWorkingSetSize) > memoryLimit * 1024U * 1024) {
				TerminateProcess(pi.hProcess, 0);
				score = 0;
				result = MemoryLimitExceeded;
				memoryUsed = timeUsed = -1;
				return;
			}
		}

		QCoreApplication::processEvents();

		if (stopJudging || skipEnabled) {
			TerminateProcess(pi.hProcess, 0);

			if (skipEnabled) {
				score = 0;
				result = TimeLimitExceeded;
				timeUsed = memoryUsed = -1;
			}
			return;
		}

		QThread::msleep(10);
	}

	if (! isProgramFinishedInExtraTimeLimit) {
		TerminateProcess(pi.hProcess, 0);

		score = 0;
		result = TimeLimitExceeded;
		timeUsed = -1;
		return;
	}

	unsigned long exitCode;
	GetExitCodeProcess(pi.hProcess, &exitCode);

	if (exitCode != 0) {

		score = 0;
		result = RunTimeError;
		QFile file(workingDirectory + "_tmperr");

		if (file.open(QFile::ReadOnly)) {
			QTextStream stream(&file);
			message = stream.readAll();
			file.close();
		}

		memoryUsed = timeUsed = -1;
		return;
	}

	FILETIME creationTime, exitTime, kernelTime, userTime;
	GetProcessTimes(pi.hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
	SYSTEMTIME realUserTime, realKernelTime;
	FileTimeToSystemTime(&userTime, &realUserTime);
	FileTimeToSystemTime(&kernelTime, &realKernelTime);
	timeUsed = realUserTime.wMilliseconds + realUserTime.wSecond * 1000 + realUserTime.wMinute * 60 * 1000 +
	           realUserTime.wHour * 60 * 60 * 1000;
	int kernelTimeUsed = realKernelTime.wMilliseconds + realKernelTime.wSecond * 1000 +
	                     realKernelTime.wMinute * 60 * 1000 + realKernelTime.wHour * 60 * 60 * 1000;
	GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS *)&memoryInfo, sizeof(memoryInfo));
	memoryUsed = memoryInfo.PeakWorkingSetSize;

#else
	// TODO: rewrite with cgroup
	QFile watcher(workingDirectory + QUuid::createUuid().toString());
	QFile::copy(":/watcher/watcher_unix", watcher.fileName());
	watcher.setPermissions(QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther);
	auto *runner = new QProcess(this);
	QStringList argumentsList;
	argumentsList << QString("\"%1\" %2").arg(executableFile, arguments);

	if (task->getStandardInputCheck()) {
		argumentsList << QFileInfo(inputFile).absoluteFilePath();
	} else {
		argumentsList << "";
	}

	if (task->getStandardOutputCheck()) {
		argumentsList << "_tmpout";
	} else {
		argumentsList << "";
	}

	argumentsList << "_tmperr";
	argumentsList << QString("%1").arg(timeLimit + extraTime);
	argumentsList << QString("%1").arg(memoryLimit);

	runner->setProcessEnvironment(environment);
	runner->setWorkingDirectory(workingDirectory);
	runner->start(watcher.fileName(), argumentsList);

	if (! runner->waitForStarted(-1)) {
		delete runner;
		score = 0;
		result = CannotStartProgram;
		return;
	}

	bool isProgramFinishedInExtraTimeLimit = false;
	QElapsedTimer timer;
	timer.start();

	while (timer.elapsed() <= timeLimit + extraTime) {
		if (runner->state() != QProcess::Running) {
			isProgramFinishedInExtraTimeLimit = true;
			break;
		}

		QCoreApplication::processEvents();

		if (stopJudging || skipEnabled) {
			runner->terminate();
			runner->waitForFinished(-1);
			delete runner;

			if (skipEnabled) {
				score = 0;
				result = TimeLimitExceeded;
				timeUsed = memoryUsed = -1;
			}

			return;
		}

		msleep(10);
	}

	if (! isProgramFinishedInExtraTimeLimit) {
		runner->terminate();
		runner->waitForFinished(-1);
		delete runner;
		score = 0;
		result = TimeLimitExceeded;
		timeUsed = memoryUsed = -1;
		return;
	}

	int code = runner->exitCode();

	if (code == 1) {
		delete runner;
		score = 0;
		result = CannotStartProgram;
		timeUsed = memoryUsed = -1;
		return;
	}

	if (code == 2) {
		delete runner;
		score = 0;
		result = RunTimeError;
		QFile file(workingDirectory + "_tmperr");

		if (file.open(QFile::ReadOnly)) {
			QTextStream stream(&file);
			message = stream.readAll();
			file.close();
		}

		timeUsed = memoryUsed = -1;
		return;
	}

	QString out = QString::fromLocal8Bit(runner->readAllStandardOutput().constData());
	QTextStream stream(&out, QIODevice::ReadOnly);
	stream >> timeUsed >> memoryUsed;

	if (memoryUsed <= 0)
		memoryLimit = -1;

	if (code == 3) {
		delete runner;
		score = 0;
		result = TimeLimitExceeded;
		timeUsed = -1;
		return;
	}

	if (code == 4) {
		delete runner;
		score = 0;
		result = MemoryLimitExceeded;
		memoryUsed = -1;
		return;
	}

	if (memoryUsed > memoryLimit * 1024LL * 1024) {
		delete runner;
		score = 0;
		result = MemoryLimitExceeded;
		return;
	}

	delete runner;
#endif
}

void JudgingThread::judgeOutput() {
	QString fileName;

	if (task->getStandardOutputCheck()) {
		fileName = workingDirectory + "_tmpout";
	} else {
		fileName = workingDirectory + task->getOutputFileName();
	}

	switch (task->getComparisonMode()) {
		case Task::LineByLineMode:
			compareLineByLine(fileName);
			break;

		case Task::IgnoreSpacesMode:
			compareIgnoreSpaces(fileName);
			break;

		case Task::ExternalToolMode:
			compareWithDiff(fileName);
			break;

		case Task::RealNumberMode:
			compareRealNumbers(fileName);
			break;

		case Task::SpecialJudgeMode:
			specialJudge(fileName);
			break;
	}
}

void JudgingThread::judgeTraditionalTask() {
	if (! QFileInfo::exists(inputFile)) {
		score = 0;
		result = FileError;
		message = tr("Cannot find standard input file");
		return;
	}

	if (! task->getStandardInputCheck()) {
		if (! QFile::copy(inputFile, workingDirectory + task->getInputFileName())) {
			score = 0;
			result = FileError;
			message = tr("Cannot copy standard input file");
			return;
		}
	}

	auto cleanupTempFiles = qScopeGuard([&] {
		if (! task->getStandardInputCheck()) {
			QFile::remove(workingDirectory + task->getInputFileName());
		}

		if (! task->getStandardOutputCheck()) {
			QFile::remove(workingDirectory + task->getOutputFileName());
		} else {
			QFile::remove(workingDirectory + "_tmpout");
		}
		QFile::remove(workingDirectory + "_tmperr");
	});

	runProgram();

	if (timeUsed > timeLimit) {
		if (score > 0 &&
		    (timeUsed <= timeLimit * (1 + extraTimeRatio) || timeUsed <= timeLimit + 1000 * extraTimeRatio)) {
			needRejudge = true;
		}
		score = 0;
		result = TimeLimitExceeded;
		message = "";
	}

	if (stopJudging)
		return;

	if (result != CorrectAnswer)
		return;

	judgeOutput();
}

void JudgingThread::judgeAnswersOnlyTask() {
	switch (task->getComparisonMode()) {
		case Task::LineByLineMode:
			compareLineByLine(answerFile);
			break;

		case Task::IgnoreSpacesMode:
			compareIgnoreSpaces(answerFile);
			break;

		case Task::ExternalToolMode:
			compareWithDiff(answerFile);
			break;

		case Task::RealNumberMode:
			compareRealNumbers(answerFile);
			break;

		case Task::SpecialJudgeMode:
			specialJudge(answerFile);
			break;
	}
}

void JudgingThread::run() {
	++judgedTimes;
	needRejudge = false;

	switch (task->getTaskType()) {
		case Task::Interaction: // treat interaction task as traditional task
			[[fallthrough]];
		case Task::Traditional:
			[[fallthrough]];
		case Task::Communication:
			[[fallthrough]];
		case Task::CommunicationExec:
			judgeTraditionalTask();
			break;

		case Task::AnswersOnly:
			judgeAnswersOnlyTask();
			break;
	}
}
