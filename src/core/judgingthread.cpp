/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
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
#include <QtMath>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef Q_OS_WIN32
// #pragma comment(lib, "Psapi.lib")
#include <windows.h>

#include <Psapi.h>
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

void JudgingThread::compareLineByLine(const QString &contestantOutput) {
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
		message = tr("Cannot open standard output file");
		fclose(contestantOutputFile);
		return;
	}

	char str1[23];
	char str2[23];
	char ch = 0;
	bool chk1 = false;
	bool chk2 = false;
	bool chkEof1 = false;
	bool chkEof2 = false;
	short len1 = 0;
	short len2 = 0;
	int nowRow = 1;

	while (true) {
		len1 = 0;

		while (len1 < 20) {
			if (feof(contestantOutputFile))
				break;

			ch = static_cast<char>(fgetc(contestantOutputFile));

			if (ch == EOF)
				break;

			if (! chk1 && ch == '\n')
				break;

			if (chk1 && ch == '\n') {
				chk1 = false;
				continue;
			}

			if (ch == '\r') {
				chk1 = true;
				break;
			}

			if (chk1)
				chk1 = false;

			str1[len1++] = ch;
		}

		str1[len1++] = '\0';
		chkEof1 = ch == EOF;
		len2 = 0;

		bool isNewRowStarted = false;

		while (len2 < 20) {
			if (feof(standardOutputFile))
				break;

			ch = static_cast<char>(fgetc(standardOutputFile));

			if (ch == EOF)
				break;

			if (! chk2 && ch == '\n') {
				isNewRowStarted = true;
				break;
			}

			if (chk2 && ch == '\n') {
				chk2 = false;
				continue;
			}

			if (ch == '\r') {
				isNewRowStarted = true;
				chk2 = true;
				break;
			}

			if (chk2)
				chk2 = false;

			str2[len2++] = ch;
		}

		str2[len2++] = '\0';
		chkEof2 = ch == EOF;

		if (len1 != len2 || strcmp(str1, str2) != 0) {
			score = 0;
			result = WrongAnswer;
			message = tr(R"(On line %3, Read "%1" but expect "%2")").arg(str1).arg(str2).arg(nowRow);
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}

		if (chkEof1 && ! chkEof2) {
			score = 0;
			result = WrongAnswer;
			message = tr(R"(On line %1, Contestant's output has less contents)").arg(nowRow);
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}

		if (! chkEof1 && chkEof2) {
			score = 0;
			result = OutputLimitExceeded;
			message = tr(R"(On line %1, Contestant's output has too much contents)").arg(nowRow);
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}

		if (chkEof1 && chkEof2)
			break;

		QCoreApplication::processEvents();

		if (stopJudging) {
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}

		if (isNewRowStarted)
			nowRow++;
	}

	score = fullScore;
	result = CorrectAnswer;
	fclose(contestantOutputFile);
	fclose(standardOutputFile);
}

void JudgingThread::compareIgnoreSpaces(const QString &contestantOutput) {
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
		message = tr("Cannot open standard output file");
		fclose(contestantOutputFile);
		return;
	}

	char ch1 = ' ';
	char ch2 = ' ';
	char str1[23];
	char str2[23];
	int flag1 = 0;
	int flag2 = 0;
	int nowRow = 1;

	while (true) {
		if (ch1 == '\n' || ch1 == '\r' || ch1 == EOF) {
			if (ch1 == '\r') {
				ch1 = static_cast<char>(fgetc(contestantOutputFile));

				if (ch1 == '\n')
					ch1 = static_cast<char>(fgetc(contestantOutputFile));
			} else {
				ch1 = static_cast<char>(fgetc(contestantOutputFile));
			}

			while (ch1 == ' ' || ch1 == '\t') {
				ch1 = static_cast<char>(fgetc(contestantOutputFile));
			}

			flag1 = 2;
		} else {
			if (ch1 == ' ' || ch1 == '\t') {
				while (ch1 == ' ' || ch1 == '\t') {
					ch1 = static_cast<char>(fgetc(contestantOutputFile));
				}

				if (ch1 == '\n' || ch1 == '\r' || ch1 == EOF) {
					if (ch1 == '\r') {
						ch1 = static_cast<char>(fgetc(contestantOutputFile));

						if (ch1 == '\n')
							ch1 = static_cast<char>(fgetc(contestantOutputFile));
					} else {
						ch1 = static_cast<char>(fgetc(contestantOutputFile));
					}

					while (ch1 == ' ' || ch1 == '\t') {
						ch1 = static_cast<char>(fgetc(contestantOutputFile));
					}

					flag1 = 2;
				} else {
					flag1 = 1;
				}
			} else {
				flag1 = 0;
			}
		}

		bool isNewRowStarted = false;

		if (ch2 == '\n' || ch2 == '\r' || ch2 == EOF) {
			if (ch2 == '\r') {
				ch2 = static_cast<char>(fgetc(standardOutputFile));

				if (ch2 == '\n')
					ch2 = static_cast<char>(fgetc(standardOutputFile));
			} else {
				ch2 = static_cast<char>(fgetc(standardOutputFile));
			}

			isNewRowStarted = true;

			while (ch2 == ' ' || ch2 == '\t')
				ch2 = static_cast<char>(fgetc(standardOutputFile));

			flag2 = 2;
		} else {
			if (ch2 == ' ' || ch2 == '\t') {
				while (ch2 == ' ' || ch2 == '\t') {
					ch2 = static_cast<char>(fgetc(standardOutputFile));
				}

				if (ch2 == '\n' || ch2 == '\r' || ch2 == EOF) {
					if (ch2 == '\r') {
						ch2 = static_cast<char>(fgetc(standardOutputFile));

						if (ch2 == '\n')
							ch2 = static_cast<char>(fgetc(standardOutputFile));
					} else {
						ch2 = static_cast<char>(fgetc(standardOutputFile));
					}

					isNewRowStarted = true;

					while (ch2 == ' ' || ch2 == '\t') {
						ch2 = static_cast<char>(fgetc(standardOutputFile));
					}

					flag2 = 2;
				} else {
					flag2 = 1;
				}
			} else {
				flag2 = 0;
			}
		}

		if (flag1 != flag2) {
			if (ch1 == EOF && ch2 != EOF) {
				score = 0;
				result = WrongAnswer;
				message = tr(R"(On line %1, Contestant's output has less contents)").arg(nowRow);
				fclose(contestantOutputFile);
				fclose(standardOutputFile);
				return;
			}

			if (ch1 != EOF && ch2 == EOF) {
				score = 0;
				result = OutputLimitExceeded;
				message = tr(R"(On line %1, Contestant's output has too much contents)").arg(nowRow);
				fclose(contestantOutputFile);
				fclose(standardOutputFile);
				return;
			}

			score = 0;
			result = PresentationError;
			message = tr("Presentation error on line %1").arg(nowRow);
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}

		if (isNewRowStarted)
			nowRow++;

		int len1 = 0;

		while (len1 < 20) {
			if (ch1 != ' ' && ch1 != '\t' && ch1 != '\n' && ch1 != '\r' && ch1 != EOF) {
				str1[len1++] = ch1;
			} else {
				break;
			}

			ch1 = static_cast<char>(fgetc(contestantOutputFile));
		}

		str1[len1] = '\0';
		int len2 = 0;

		while (len2 < 20) {
			if (ch2 != ' ' && ch2 != '\t' && ch2 != '\n' && ch2 != '\r' && ch2 != EOF) {
				str2[len2++] = ch2;
			} else {
				break;
			}

			ch2 = static_cast<char>(fgetc(standardOutputFile));
		}

		str2[len2] = '\0';

		if (len1 != len2 || strcmp(str1, str2) != 0) {
			if (len1 <= 0) {
				score = 0;
				result = WrongAnswer;
				message = tr(R"(On line %1, Contestant's output has less contents)").arg(nowRow);
				fclose(contestantOutputFile);
				fclose(standardOutputFile);
				return;
			}

			if (len2 <= 0) {
				score = 0;
				result = OutputLimitExceeded;
				message = tr(R"(On line %1, Contestant's output has too much contents)").arg(nowRow);
				fclose(contestantOutputFile);
				fclose(standardOutputFile);
				return;
			}

			score = 0;
			result = WrongAnswer;
			message = tr(R"(On line %3, Read "%1" but expect "%2")").arg(str1).arg(str2).arg(nowRow);
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}

		if (ch1 == EOF && ch2 == EOF)
			break;

		QCoreApplication::processEvents();

		if (stopJudging) {
			fclose(contestantOutputFile);
			fclose(standardOutputFile);
			return;
		}
	}

	score = fullScore;
	result = CorrectAnswer;
	fclose(contestantOutputFile);
	fclose(standardOutputFile);
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

	auto *judge = new QProcess(this);
	QStringList arguments;
	arguments << inputFile << fileName << outputFile << QString("%1").arg(fullScore);
	arguments << workingDirectory + "_score";
	arguments << workingDirectory + "_message";
	judge->start(Settings::dataPath() + task->getSpecialJudge(), arguments);

	if (! judge->waitForStarted(-1)) {
		score = 0;
		result = InvalidSpecialJudge;
		delete judge;
		return;
	}

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
			delete judge;
			return;
		}

		msleep(10);
	}

	if (! flag) {
		judge->kill();
		score = 0;
		result = SpecialJudgeTimeLimitExceeded;
		delete judge;
		return;
	}

	if (judge->exitCode() != 0) {
		score = 0;
		result = SpecialJudgeRunTimeError;
		delete judge;
		return;
	}

	delete judge;
	QFile scoreFile(workingDirectory + "_score");

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

	QFile messageFile(workingDirectory + "_message");

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

	scoreFile.remove();
	messageFile.remove();
}

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
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = TRUE;

	if (task->getStandardInputCheck()) {
		si.hStdInput = CreateFileW((const WCHAR *)(inputFile.utf16()), GENERIC_READ,
		                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, OPEN_ALWAYS,
		                           FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if (task->getStandardOutputCheck()) {
		si.hStdOutput = CreateFileW((const WCHAR *)((workingDirectory + "_tmpout").utf16()), GENERIC_WRITE,
		                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa,
		                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	si.hStdError = CreateFileW((const WCHAR *)((workingDirectory + "_tmperr").utf16()), GENERIC_WRITE,
	                           FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, CREATE_ALWAYS,
	                           FILE_ATTRIBUTE_NORMAL, NULL);
	QString values = environment.toStringList().join(QChar('\0')) + '\0';

	if (! CreateProcessW(NULL, (WCHAR *)(QString("\"%1\" %2").arg(executableFile, arguments).utf16()), NULL,
	                     &sa, TRUE, HIGH_PRIORITY_CLASS | CREATE_NO_WINDOW,
	                     (LPVOID)(values.toLocal8Bit().data()), (const WCHAR *)(workingDirectory.utf16()),
	                     &si, &pi)) {
		if (task->getStandardInputCheck())
			CloseHandle(si.hStdInput);

		if (task->getStandardOutputCheck())
			CloseHandle(si.hStdOutput);

		CloseHandle(si.hStdError);
		score = 0;
		result = CannotStartProgram;
		return;
	}

	PROCESS_MEMORY_COUNTERS_EX info;
	ZeroMemory(&info, sizeof(info));
	info.cb = sizeof(info);

	if (memoryLimit != -1) {
		GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS *)&info, sizeof(info));

		if (qMax(info.PrivateUsage, info.PeakWorkingSetSize) > memoryLimit * 1024 * 1024) {
			TerminateProcess(pi.hProcess, 0);

			if (task->getStandardInputCheck())
				CloseHandle(si.hStdInput);

			if (task->getStandardOutputCheck())
				CloseHandle(si.hStdOutput);

			CloseHandle(si.hStdError);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
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
			GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS *)&info, sizeof(info));

			if (qMax(info.PrivateUsage, info.PeakWorkingSetSize) > memoryLimit * 1024U * 1024) {
				TerminateProcess(pi.hProcess, 0);

				if (task->getStandardInputCheck())
					CloseHandle(si.hStdInput);

				if (task->getStandardOutputCheck())
					CloseHandle(si.hStdOutput);

				CloseHandle(si.hStdError);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				score = 0;
				result = MemoryLimitExceeded;
				memoryUsed = timeUsed = -1;
				return;
			}
		}

		QCoreApplication::processEvents();

		if (stopJudging || skipEnabled) {
			TerminateProcess(pi.hProcess, 0);

			if (task->getStandardInputCheck())
				CloseHandle(si.hStdInput);

			if (task->getStandardOutputCheck())
				CloseHandle(si.hStdOutput);

			CloseHandle(si.hStdError);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);

			if (skipEnabled) {
				score = 0;
				result = TimeLimitExceeded;
				timeUsed = memoryUsed = -1;
			}

			return;
		}

		Sleep(10);
	}

	if (! isProgramFinishedInExtraTimeLimit) {
		TerminateProcess(pi.hProcess, 0);

		if (task->getStandardInputCheck())
			CloseHandle(si.hStdInput);

		if (task->getStandardOutputCheck())
			CloseHandle(si.hStdOutput);

		CloseHandle(si.hStdError);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		score = 0;
		result = TimeLimitExceeded;
		timeUsed = -1;
		return;
	}

	unsigned long exitCode;
	GetExitCodeProcess(pi.hProcess, &exitCode);

	if (exitCode != 0) {
		if (task->getStandardInputCheck())
			CloseHandle(si.hStdInput);

		if (task->getStandardOutputCheck())
			CloseHandle(si.hStdOutput);

		CloseHandle(si.hStdError);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
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
	qDebug() << "User Time Used" << timeUsed;
	qDebug() << "Kernel Time Used" << kernelTimeUsed;
	GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS *)&info, sizeof(info));
	memoryUsed = info.PeakWorkingSetSize;

	if (task->getStandardInputCheck())
		CloseHandle(si.hStdInput);

	if (task->getStandardOutputCheck())
		CloseHandle(si.hStdOutput);

	CloseHandle(si.hStdError);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
#else
	// TODO: rewrite with cgroup
	QFile::copy(":/watcher/watcher_unix", workingDirectory + "watcher");
	QProcess::execute(QString("chmod"), (QStringList("+wx") << QString(workingDirectory + "watcher")));
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
	runner->start(workingDirectory + "watcher", argumentsList);

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

	runProgram();

	if (stopJudging)
		return;

	if (result != CorrectAnswer) {
		if (! task->getStandardInputCheck()) {
			QFile::remove(workingDirectory + task->getInputFileName());
		}

		if (! task->getStandardOutputCheck()) {
			QFile::remove(workingDirectory + task->getOutputFileName());
		} else {
			QFile::remove(workingDirectory + "_tmpout");
		}

		return;
	}

	judgeOutput();

	if (stopJudging)
		return;

	if (timeUsed > timeLimit) {
		if (score > 0 &&
		    (timeUsed <= timeLimit * (1 + extraTimeRatio) || timeUsed <= timeLimit + 1000 * extraTimeRatio)) {
			needRejudge = true;
		}
		score = 0;
		result = TimeLimitExceeded;
		message = "";
	}

	if (! task->getStandardInputCheck()) {
		QFile::remove(workingDirectory + task->getInputFileName());
	}

	if (! task->getStandardOutputCheck()) {
		QFile::remove(workingDirectory + task->getOutputFileName());
	} else {
		QFile::remove(workingDirectory + "_tmpout");
	}
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
/*
void JudgingThread::judgeInteractionTask()
{
    if (! QFileInfo(inputFile).exists()) {
        score = 0;
        result = FileError;
        message = tr("Cannot find input file");
        return;
    }
    if (! task->getStandardInputCheck()) {
        if (! QFile::copy(inputFile, workingDirectory + task->getInputFileName())) {
            score = 0;
            result = FileError;
            message = tr("Cannot copy input file");
            return;
        }
    }

    arguments.clear();
    QStringList argumentsList;
    QString score_log = "_score_";
    for(int i = 0; i != 5; ++i)
        score_log.push_back(QRandomGenerator::global()->bounded(0, 25) + 'a');
    argumentsList << QString("%1").arg(fullScore);
    argumentsList << workingDirectory + score_log;
    argumentsList << workingDirectory + "_message";
    arguments = argumentsList.join(' ');

    runProgram();
    if (stopJudging) return;

    if (result != CorrectAnswer) {
        if (! task->getStandardInputCheck()) {
            QFile::remove(workingDirectory + task->getInputFileName());
        }
        if (! task->getStandardOutputCheck()) {
            QFile::remove(workingDirectory + task->getOutputFileName());
        } else {
            QFile::remove(workingDirectory + "_tmpout");
        }
        return;
    }

    QFile scoreFile(workingDirectory + score_log);
    if (! scoreFile.open(QFile::ReadOnly)) {
        score = 0;
        result = InteractorError;
        message = tr("Cannot read score log");
        return;
    }

    QTextStream scoreStream(&scoreFile);
    scoreStream >> score;
    if (scoreStream.status() == QTextStream::ReadCorruptData) {
        score = 0;
        result = InteractorError;
        message = tr("Cannot read score log");
        return;
    }
    scoreFile.close();

    if (score < 0) {
        score = 0;
        result = InteractorError;
        message = tr("Score is below zero");
        return;
    }

    QFile messageFile(workingDirectory + "_message");
    if (messageFile.open(QFile::ReadOnly)) {
        QTextStream messageStream(&messageFile);
        message = messageStream.readAll();
        messageFile.close();
    }

    if (score == 0) result = WrongAnswer;
    if (0 < score && score < fullScore) result = PartlyCorrect;
    if (score >= fullScore) result = CorrectAnswer;

    scoreFile.remove();
    messageFile.remove();
    if (stopJudging) return;

    if (timeUsed > timeLimit) {
        if (score > 0 && (timeUsed <= timeLimit * (1 + extraTimeRatio)
                                                || timeUsed <= timeLimit + 1000 * extraTimeRatio)) {
            needRejudge = true;
        }
        score = 0;
        result = TimeLimitExceeded;
        message = "";
    }

    if (! task->getStandardInputCheck()) {
        QFile::remove(workingDirectory + task->getInputFileName());
    }
    if (! task->getStandardOutputCheck()) {
        QFile::remove(workingDirectory + task->getOutputFileName());
    } else {
        QFile::remove(workingDirectory + "_tmpout");
    }
}
*/
// rebuild interaction task, built in tradtional task

void JudgingThread::run() {
	++judgedTimes;
	needRejudge = false;

	switch (task->getTaskType()) {
		case Task::Interaction:
		case Task::Traditional:
		case Task::Communication:
		case Task::CommunicationExec:
			judgeTraditionalTask();
			break;

		case Task::AnswersOnly:
			judgeAnswersOnlyTask();
			break;
	}
}
