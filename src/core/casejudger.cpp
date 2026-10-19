/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "casejudger.h"
#include "LemonType.hpp"
#include "base/LemonLog.hpp"
#include "base/settings.h"
#include "core/task.h"

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QTextStream>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#define LEMON_MODULE_NAME "CaseJudger"

CaseJudger::CaseJudger() {
	needRejudge = false;
	timeUsed = -1;
	memoryUsed = -1;
	judgedTimes = 0;
}

void CaseJudger::setCancellation(QFuture<void> future) { cancellation = std::move(future); }

void CaseJudger::setExtraTimeRatio(double ratio) { extraTimeRatio = ratio; }

void CaseJudger::setEnvironment(const QProcessEnvironment &env) { environment = env; }

void CaseJudger::setWorkingDirectory(const QString &directory) { workingDirectory = directory; }

void CaseJudger::setSpecialJudgeTimeLimit(int limit) { specialJudgeTimeLimit = limit; }

void CaseJudger::setExecutableFile(const QString &fileName) { executableFile = fileName; }

void CaseJudger::setArguments(const QString &argumentsList) { arguments = argumentsList; }

void CaseJudger::setAnswerFile(const QString &fileName) { answerFile = fileName; }

void CaseJudger::setInputFile(const QString &fileName) { inputFile = fileName; }

void CaseJudger::setOutputFile(const QString &fileName) { outputFile = fileName; }

void CaseJudger::setDiffPath(const QString &path) { diffPath = path; }

void CaseJudger::setTask(const Task *_task) { task = _task; }

void CaseJudger::setFullScore(int score) { fullScore = score; }

void CaseJudger::setTimeLimit(int limit) { timeLimit = limit; }

void CaseJudger::setRawTimeLimit(int limit) { rawTimeLimit = limit; }

void CaseJudger::setMemoryLimit(int limit) { memoryLimit = limit; }

void CaseJudger::setRawMemoryLimit(int limit) { rawMemoryLimit = limit; }

void CaseJudger::setInterpreterAsWatcher(bool use) { interpreterAsWatcher = use; }

void CaseJudger::setSandboxConfiguration(const SandboxSettings &settings, const QString &executable,
                                         const QProcessEnvironment &environment,
                                         const std::shared_ptr<WindowsSandboxSession> &session) {
	sandboxSettings = settings;
	runtimeExecutable = executable;
	runtimeEnvironment = environment;
	sandboxSession = session;
}

auto CaseJudger::getTimeUsed() const -> int { return timeUsed; }

auto CaseJudger::getMemoryUsed() const -> qint64 { return memoryUsed; }

auto CaseJudger::getScore() const -> int { return score; }

auto CaseJudger::getFullScore() const -> int { return fullScore; }

auto CaseJudger::getJudgeTimes() const -> int { return judgedTimes; }

auto CaseJudger::getResult() const -> ResultState { return result; }

auto CaseJudger::getMessage() const -> const QString & { return message; }

auto CaseJudger::getNeedRejudge() const -> bool { return needRejudge; }

// Chunked file reader used by the line/space comparators below.
//
// nextUntilNewLine() / nextUntilSpace() return at most 32 chars per call --
// they do NOT truncate a long line; callers must loop until eof() to consume
// the whole line/token. The 32-byte cap keeps `result` (a fixed-capacity
// std::string reserved at 32) from reallocating, which is the hot path of
// per-test-case comparison.
class BufferedStreamReader {

  public:
	static const int BUFFER_SIZE = 1 << 18; // 128 KiB
	explicit BufferedStreamReader(const char *filename) : file(fopen(filename, "r")) {
		result.reserve(32);
		// Start with bufPos == bufEnd so the first peekChar() triggers an
		// fread; avoids a separate "primed?" flag.
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
		// Buffer drained -- refill. fread returning 0 means real EOF; latch it
		// so further peekChar() calls short-circuit without touching the FILE*.
		bufPos = buffer;
		bufEnd = buffer + fread(buffer, 1, BUFFER_SIZE, file);
		if (bufPos == bufEnd) {
			isEof = true;
			return '\0';
		}
	}
	return *bufPos;
}

// Consume one line terminator (\n, \r, or \r\n) if the cursor sits on one,
// and bump lineNumber. Idempotent when not on a terminator -- safe to call
// before each token read.
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

// Read up to 32 chars from the current position, stopping at any line
// terminator or EOF. Does NOT consume the terminator -- the next call to
// tryNextLine() (invoked at the top here) will. Returning a chunk shorter
// than 32 chars is the signal that the caller has reached end-of-line/EOF;
// otherwise the caller must call again to read the rest of a long line.
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

// Skip leading whitespace (advancing lineNumber across newlines), then read
// up to 32 non-space chars. Same chunked-read contract as nextUntilNewLine:
// short return means token end; long tokens must be read across multiple
// calls. Returns an empty view at EOF.
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

void CaseJudger::compareLineByLine(const QString &contestantOutput) {
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
		if (cancellation.isCanceled())
			return;

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

void CaseJudger::compareIgnoreSpaces(const QString &contestantOutput) {
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
		if (cancellation.isCanceled())
			return;

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

void CaseJudger::compareWithDiff(const QString &contestantOutput) {
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

void CaseJudger::compareRealNumbers(const QString &contestantOutput) {
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

		if (cancellation.isCanceled()) {
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

void CaseJudger::lemonSpecialJudge(const QString &fileName) {
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

	QProcess judge;
	QStringList arguments;
	arguments << inputFile << fileName << outputFile << QString("%1").arg(fullScore);
	arguments << workingDirectory + "_score";
	arguments << workingDirectory + "_message";
	judge.start(Settings::dataPath() + task->getSpecialJudge(), arguments);

	if (! judge.waitForStarted(-1)) {
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
		if (judge.waitForFinished(10)) {
			flag = true;
			break;
		}

		if (cancellation.isCanceled()) {
			judge.kill();
			judge.waitForFinished(-1);
			return;
		}
	}

	if (! flag) {
		judge.kill();
		judge.waitForFinished(-1);
		score = 0;
		result = SpecialJudgeTimeLimitExceeded;
		return;
	}

	if (judge.exitCode() != 0) {
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

void CaseJudger::testlibSpecialJudge(const QString &fileName) {
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

	QProcess judge;
	QStringList arguments;
	arguments << inputFile << fileName << outputFile;
	judge.setStandardErrorFile(workingDirectory + "_score");
	judge.start(Settings::dataPath() + task->getSpecialJudge(), arguments);

	if (! judge.waitForStarted(-1)) {
		score = 0;
		result = InvalidSpecialJudge;
		return;
	}

	QFile scoreFile(workingDirectory + "_score");

	auto removeTempFiles = qScopeGuard([&] { scoreFile.remove(); });

	QElapsedTimer timer;
	timer.start();
	bool flag = false;

	while (timer.elapsed() < specialJudgeTimeLimit) {
		if (judge.waitForFinished(10)) {
			flag = true;
			break;
		}

		if (cancellation.isCanceled()) {
			judge.kill();
			return;
		}
	}

	if (! flag) {
		judge.kill();
		score = 0;
		result = SpecialJudgeTimeLimitExceeded;
		return;
	}

	if (! scoreFile.open(QFile::ReadOnly)) {
		score = 0;
		result = InvalidSpecialJudge;
		return;
	}

	QTextStream scoreStream(&scoreFile);
	message = scoreStream.readAll();

	score = 0;

	if (message.startsWith("ok")) {
		score = fullScore;
	}

	if (message.startsWith("FAIL")) {
		score = 0;
		result = InvalidSpecialJudge;
		return;
	}

	QRegularExpressionMatch m = QRegularExpression(R"(^partially correct \((\d+)\))").match(message);
	if (m.hasMatch()) {
		int val = m.captured(1).toInt();
		score = val * fullScore / 100;
	}

	m = QRegularExpression(R"(^points ([0-9]*\.[0-9]+|[0-9]+))").match(message);
	if (m.hasMatch()) {
		double val = m.captured(1).toDouble();
		score = val * fullScore;
	}

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

	if (score == 0)
		result = WrongAnswer;

	if (0 < score && score < fullScore)
		result = PartlyCorrect;

	if (score >= fullScore)
		result = CorrectAnswer;
}

void CaseJudger::judgeOutput() {
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

		case Task::LemonSpecialJudgeMode:
			lemonSpecialJudge(fileName);
			break;

		case Task::TestlibSpecialJudgeMode:
			testlibSpecialJudge(fileName);
			break;
	}
}

void CaseJudger::judgeTraditionalTask() {
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

	ProcessRunnerConfig cfg{
	    .executableFile = executableFile,
	    .arguments = arguments,
	    .workingDirectory = workingDirectory,
	    .inputFile = inputFile,
	    .environment = environment,
	    .timeLimit = timeLimit,
	    .rawTimeLimit = rawTimeLimit,
	    .memoryLimit = memoryLimit,
	    .rawMemoryLimit = rawMemoryLimit,
	    .extraTimeRatio = extraTimeRatio,
	    .standardInputCheck = task->getStandardInputCheck(),
	    .standardOutputCheck = task->getStandardOutputCheck(),
	    .inputFileName = task->getInputFileName(),
	    .outputFileName = task->getOutputFileName(),
	    .interpreterAsWatcher = interpreterAsWatcher,
	    .sandboxSettings = sandboxSettings,
	    .runtimeExecutable = runtimeExecutable,
	    .runtimeEnvironment = runtimeEnvironment,
	    .sandboxSession = sandboxSession,
	};

	auto processRunner = ProcessRunner::create(cfg, cancellation);
	auto runResult = processRunner->run();
	result = runResult.result;
	score = runResult.score;
	timeUsed = runResult.timeUsed;
	memoryUsed = runResult.memoryUsed;
	message = runResult.message;

	if (timeUsed > timeLimit) {
		if (score > 0 &&
		    (timeUsed <= timeLimit * (1 + extraTimeRatio) || timeUsed <= timeLimit + 1000 * extraTimeRatio)) {
			needRejudge = true;
		}
		score = 0;
		result = TimeLimitExceeded;
		message = "";
	}

	if (cancellation.isCanceled())
		return;

	if (result != CorrectAnswer)
		return;

	judgeOutput();
}

void CaseJudger::judgeAnswersOnlyTask() {
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

		case Task::LemonSpecialJudgeMode:
			lemonSpecialJudge(answerFile);
			break;

		case Task::TestlibSpecialJudgeMode:
			testlibSpecialJudge(answerFile);
			break;
	}
}

void CaseJudger::run() {
	if (cancellation.isCanceled())
		return;

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
