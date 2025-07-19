/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include "base/LemonType.hpp"
#include <QProcessEnvironment>
#include <QThread>

class Task;

class JudgingThread : public QThread {
	Q_OBJECT
  public:
	explicit JudgingThread(QObject *parent = nullptr);
	// void setCheckRejudgeMode(bool);
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
	void setTask(Task *);
	void setFullScore(int);
	void setTimeLimit(int);
	void setRawTimeLimit(int);
	void setMemoryLimit(int);
	void setRawMemoryLimit(int);
	void setUseRunnerWrap(bool);
	int getTimeUsed() const;
	int getMemoryUsed() const;
	int getScore() const;
	int getFullScore() const;
	int getJudgeTimes() const;
	ResultState getResult() const;
	const QString &getMessage() const;
	bool getNeedRejudge() const;
	void run();

  private:
	// bool checkRejudgeMode;

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
	Task *task{};
	int specialJudgeTimeLimit{};
	int fullScore{};
	int timeLimit{};
	int rawTimeLimit{};
	int memoryLimit{};
	int rawMemoryLimit{};
	int timeUsed;
	int memoryUsed;
	int score{};
	int judgedTimes;
	ResultState result;
	QString message;
	bool stopJudging;
	bool useRunnerWrap{};
	void compareLineByLine(const QString &);
	void compareIgnoreSpaces(const QString &);
	void compareWithDiff(const QString &);
	void compareRealNumbers(const QString &);
	void specialJudge(const QString &);
	void runProgram();
	void judgeOutput();
	void judgeTraditionalTask();
	void judgeAnswersOnlyTask();
	// void judgeInteractionTask();

  public slots:
	void stopJudgingSlot();
};
