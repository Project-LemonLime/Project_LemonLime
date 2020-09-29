/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef JUDGINGTHREAD_H
#define JUDGINGTHREAD_H

#include "base/LemonType.hpp"
#include <QThread>
#include <QtCore>

class Task;

class JudgingThread : public QThread
{
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
	void setMemoryLimit(int);
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
	int memoryLimit{};
	int timeUsed;
	int memoryUsed;
	int score{};
	int judgedTimes;
	ResultState result;
	QString message;
	bool stopJudging;
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

#endif // JUDGINGTHREAD_H
