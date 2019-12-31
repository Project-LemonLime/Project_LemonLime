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
 * assignmentthread.h @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * assignmentthread.h @Project LemonLime
 * Update 2019 iotang
 **/

#ifndef ASSIGNMENTTHREAD_H
#define ASSIGNMENTTHREAD_H

#include <QtCore>
#include <QThread>
#include "globaltype.h"

class Settings;
class Task;
class JudgingThread;

class AssignmentThread : public QThread
{
	Q_OBJECT
public:
	explicit AssignmentThread(QObject *parent = nullptr);
	//void setCheckRejudgeMode(bool);
	void setNeedRejudge(const QList< QPair<int, int> > &);
	void setSettings(Settings *);
	void setTask(Task *);
	void setContestantName(const QString &);
	CompileState getCompileState() const;
	const QString &getCompileMessage() const;
	const QString &getSourceFile() const;
	const QList< QList<int> > &getScore() const;
	const QList< QList<int> > &getTimeUsed() const;
	const QList< QList<int> > &getMemoryUsed() const;
	const QList< QList<ResultState> > &getResult() const;
	const QList<QStringList> &getMessage() const;
	const QList<QStringList> &getInputFiles() const;
	//const QList< QPair<int, int> >& getNeedRejudge() const;
	void run();

private:
	//bool checkRejudgeMode;
	bool interpreterFlag;
	Settings *settings;
	Task *task;
	QString contestantName;
	CompileState compileState;
	QString compileMessage;
	QString sourceFile;
	QString executableFile;
	QString arguments;
	QString diffPath;
	double timeLimitRatio;
	double memoryLimitRatio;
	bool disableMemoryLimitCheck;
	QProcessEnvironment environment;
	QList<int> overallStatus;
	QList< QList<int> > timeUsed;
	QList< QList<int> > memoryUsed;
	QList< QList<int> > score;
	QList< QList<ResultState> > result;
	QList<QStringList> message;
	QList<QStringList> inputFiles;
	//QList< QPair<int, int> > needRejudge;
	QList<int> testCaseScore;
	int curTestCaseIndex;
	int curSingleCaseIndex;
	int countFinished;
	int totalSingleCase;
	QMap< JudgingThread *, QPair<int, int> > running;
	bool stopJudging;
	bool traditionalTaskPrepare();
	void assign();
	void taskSkipped(const QPair<int, int> &);

private slots:
	void threadFinished();

public slots:
	void stopJudgingSlot();

signals:
	void singleCaseFinished(int, int, int, int, int, int, int);
	void singleSubtaskDependenceFinished(int, int, int);
	void compileError(int, int);
	void stopJudgingSignal();
};

#endif // ASSIGNMENTTHREAD_H
