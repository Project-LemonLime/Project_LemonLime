/*
 * SPDX-FileCopyrightText: 2021-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include "base/LemonType.hpp"
#include "base/sandboxsettings.h"
#include "core/taskresult.h"

#include <QFuture>
#include <QList>
#include <QMap>
#include <QObject>
#include <QProcessEnvironment>
#include <QPromise>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>

class Settings;
class Task;

class TaskJudger : public QObject {
	Q_OBJECT
  public:
	TaskJudger(QObject *parent = nullptr);
	// void setCheckRejudgeMode(bool);
	void setNeedRejudge(const QList<std::pair<int, int>> &);
	void setSettings(const Settings *);
	void setTask(const Task *);
	void setContestantName(QString);
	CompileState getCompileState() const;
	// const QList< std::pair<int, int> >& getNeedRejudge() const;

  private:
	// bool checkRejudgeMode;

	const QString commExecGrader = "grader";
	bool interpreterFlag{};
	const Settings *settings{};
	const Task *task{};
	QString contestantName;
	QFuture<void> cancellation;
	CompileState compileState;
	QString compileMessage;
	QString sourceFile;
	QString executableFile;
	QString arguments;
	QString diffPath;
	double compilerTimeLimitRatio{};
	double compilerMemoryLimitRatio{};
	bool disableMemoryLimitCheck{};
	bool interpreterAsWatcher{};
	QProcessEnvironment environment;
	SandboxSettings sandboxSettings;
	QString runtimeExecutable;
	QProcessEnvironment runtimeEnvironment;
	QList<int> overallStatus;
	QList<QList<int>> timeUsed;
	QList<QList<qint64>> memoryUsed;
	QList<QList<int>> score;
	QList<QList<ResultState>> result;
	QList<QStringList> message;
	QList<QStringList> inputFiles;

	QList<int> testCaseScore;
	bool traditionalTaskPrepare();
	void assign();
	void taskSkipped(const std::pair<int, int> &);
	void makeDialogAlert(QString);
	int judge();

	QTemporaryDir temporaryDir;

  public:
	TaskResult judgeIt(QPromise<TaskResult> &promise);
  signals:
	void judgingStarted(QString);
	void dialogAlert(QString);
	void singleCaseFinished(QString, int, int, int, int, int, int, qint64);
	void singleSubtaskDependenceFinished(int, int, int);
	void compileError(int, int);
};
