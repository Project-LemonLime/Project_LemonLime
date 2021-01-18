/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include "base/LemonType.hpp"
#include <QThread>
#include <QtCore>

class Settings;
class Task;
class JudgingThread;

class AssignmentThread : public QThread {
	Q_OBJECT
  public:
	explicit AssignmentThread(QObject *parent = nullptr);
	// void setCheckRejudgeMode(bool);
	void setNeedRejudge(const QList<std::pair<int, int>> &);
	void setSettings(Settings *);
	void setTask(Task *);
	void setContestantName(const QString &);
	CompileState getCompileState() const;
	const QString &getCompileMessage() const;
	const QString &getSourceFile() const;
	const QList<QList<int>> &getScore() const;
	const QList<QList<int>> &getTimeUsed() const;
	const QList<QList<int>> &getMemoryUsed() const;
	const QList<QList<ResultState>> &getResult() const;
	const QList<QStringList> &getMessage() const;
	const QList<QStringList> &getInputFiles() const;
	// const QList< std::pair<int, int> >& getNeedRejudge() const;
	void run();

  private:
	// bool checkRejudgeMode;

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
	// QList< std::pair<int, int> > needRejudge;

	QList<int> testCaseScore;
	int curTestCaseIndex;
	int curSingleCaseIndex;
	int countFinished;
	int totalSingleCase;
	QMap<JudgingThread *, std::pair<int, int>> running;
	bool stopJudging;
	bool traditionalTaskPrepare();
	void assign();
	void taskSkipped(const std::pair<int, int> &);
	void makeDialogAlert(QString);
	QTemporaryDir temporaryDir;

  private slots:
	void threadFinished();

  public slots:
	void stopJudgingSlot();

  signals:
	void dialogAlert(QString);
	void singleCaseFinished(int, int, int, int, int, int, int);
	void singleSubtaskDependenceFinished(int, int, int);
	void compileError(int, int);
	void stopJudgingSignal();
};
