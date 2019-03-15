/***************************************************************************
    This file is part of Project Lemon
    Copyright (C) 2011 Zhipeng Jia

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/
/**
 * judgingthread.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef JUDGINGTHREAD_H
#define JUDGINGTHREAD_H

#include <QtCore>
#include <QThread>
#include "globaltype.h"

class Task;

class JudgingThread : public QThread
{
	Q_OBJECT
public:
	explicit JudgingThread(QObject *parent = 0);
	//void setCheckRejudgeMode(bool);
	void setExtraTimeRatio(double);
	void setEnvironment(const QProcessEnvironment&);
	void setWorkingDirectory(const QString&);
	void setSpecialJudgeTimeLimit(int);
	void setExecutableFile(const QString&);
	void setArguments(const QString&);
	void setAnswerFile(const QString&);
	void setInputFile(const QString&);
	void setOutputFile(const QString&);
	void setDiffPath(const QString&);
	void setTask(Task*);
	void setFullScore(int);
	void setTimeLimit(int);
	void setMemoryLimit(int);
	int getTimeUsed() const;
	int getMemoryUsed() const;
	int getScore() const;
	int getJudgeTimes() const;
	ResultState getResult() const;
	const QString& getMessage() const;
	bool getNeedRejudge() const;
	void run();

private:
	//bool checkRejudgeMode;
	bool needRejudge;
	double extraTimeRatio;
	QProcessEnvironment environment;
	QString workingDirectory;
	QString executableFile;
	QString arguments;
	QString answerFile;
	QString inputFile;
	QString outputFile;
	QString diffPath;
	Task *task;
	int specialJudgeTimeLimit;
	int fullScore;
	int timeLimit;
	int memoryLimit;
	int timeUsed;
	int memoryUsed;
	int score;
	int judgedTimes;
	ResultState result;
	QString message;
	bool stopJudging;
	void compareLineByLine(const QString&);
	void compareIgnoreSpaces(const QString&);
	void compareWithDiff(const QString&);
	void compareRealNumbers(const QString&);
	void specialJudge(const QString&);
	void runProgram();
	void judgeOutput();
	void judgeTraditionalTask();
	void judgeAnswersOnlyTask();
	//void judgeInteractionTask();

public slots:
	void stopJudgingSlot();
};

#endif // JUDGINGTHREAD_H
