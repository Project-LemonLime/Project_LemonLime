/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include "base/LemonType.hpp"
#include "processrunner.h"
#include <QCoreApplication>
#include <QFuture>
#include <QProcessEnvironment>

class Task;

class CaseJudger {
	// Keep the existing translation context after removing QThread.
	Q_DECLARE_TR_FUNCTIONS(JudgingThread)
  public:
	CaseJudger();
	void setCancellation(QFuture<void>);
	void setExtraTimeRatio(double);
	void setEnvironment(const QProcessEnvironment &);
	void setWorkingDirectory(const QString &);
	void setSpecialJudgeTimeLimit(int);
	void setExecutableFile(const QString &);
	void setArguments(const QString &);
	void setAnswerFile(const QString &);
	void setInputFile(const QString &);
	void setOutputFile(const QString &);
	void setDiffPath(const QString &);
	void setTask(const Task *);
	void setFullScore(int);
	void setTimeLimit(int);
	void setRawTimeLimit(int);
	void setMemoryLimit(int);
	void setRawMemoryLimit(int);
	void setInterpreterAsWatcher(bool);
	void setSandboxConfiguration(const SandboxSettings &, const QString &, const QProcessEnvironment &,
	                             const std::shared_ptr<WindowsSandboxSession> &);
	int getTimeUsed() const;
	qint64 getMemoryUsed() const;
	int getScore() const;
	int getFullScore() const;
	int getJudgeTimes() const;
	ResultState getResult() const;
	const QString &getMessage() const;
	bool getNeedRejudge() const;
	void run();

  private:
	bool needRejudge;
	// Control some extra time program used, like kernel time, judge system fluctuation
	double extraTimeRatio{};
	QProcessEnvironment environment;
	QString workingDirectory;
	QString executableFile;
	QString arguments;
	QString answerFile;
	QString inputFile;
	QString outputFile;
	QString diffPath;
	const Task *task{};
	int specialJudgeTimeLimit{};
	int fullScore{};
	int timeLimit{};
	int rawTimeLimit{};
	int memoryLimit{};
	int rawMemoryLimit{};
	int timeUsed;
	qint64 memoryUsed;
	int score{};
	int judgedTimes;
	ResultState result;
	QString message;
	QFuture<void> cancellation;
	bool interpreterAsWatcher{};
	SandboxSettings sandboxSettings;
	QString runtimeExecutable;
	QProcessEnvironment runtimeEnvironment;
	std::shared_ptr<WindowsSandboxSession> sandboxSession;
	void compareLineByLine(const QString &);
	void compareIgnoreSpaces(const QString &);
	void compareWithDiff(const QString &);
	void compareRealNumbers(const QString &);
	void lemonSpecialJudge(const QString &);
	void testlibSpecialJudge(const QString &);

	void judgeOutput();
	void judgeTraditionalTask();
	void judgeAnswersOnlyTask();
};
