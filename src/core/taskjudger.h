/*
 * SPDX-FileCopyrightText: 2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include "base/LemonType.hpp"
#include "base/settings.h"
#include "core/judgingthread.h"
#include "core/task.h"
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>

class TaskJudger : public QObject {
	Q_OBJECT
  public:
	TaskJudger(QObject *parent = nullptr);
	// void setCheckRejudgeMode(bool);
	void setNeedRejudge(const QList<std::pair<int, int>> &);
	void setSettings(Settings *);
	void setTask(Task *);
	void setContestantName(const QString &);
	CompileState getCompileState() const;
	// const QList< std::pair<int, int> >& getNeedRejudge() const;

  private:
	// bool checkRejudgeMode;

	const QString commExecGrader = "grader";
	bool interpreterFlag{};
	Settings *settings{};
	Task *task{};
	QString contestantName;
	CompileState compileState;
	QString compileMessage;
	QString sourceFile;
	QString executableFile;
	QString arguments;
	QString diffPath;
	double compilerTimeLimitRatio{};
	double compilerMemoryLimitRatio{};
	bool disableMemoryLimitCheck{};
	QProcessEnvironment environment;
	QList<int> overallStatus;
	QList<QList<int>> timeUsed;
	QList<QList<int>> memoryUsed;
	QList<QList<int>> score;
	QList<QList<ResultState>> result;
	QList<QStringList> message;
	QList<QStringList> inputFiles;

	QList<int> testCaseScore;
	bool stopJudging;
	bool traditionalTaskPrepare();
	void assign();
	void taskSkipped(const std::pair<int, int> &);
	void makeDialogAlert(QString);
	QTemporaryDir temporaryDir;

  public:
	TaskResult judge();
  signals:
	void dialogAlert(QString);
	void singleCaseFinished(int, int, int, int, int, int, int);
	void singleSubtaskDependenceFinished(int, int, int);
	void compileError(int, int);
	void stopJudgingSignal();
	void judgeFinished();
};