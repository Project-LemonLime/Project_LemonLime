/*
 * SPDX-FileCopyrightText: 2021-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "taskresult.h"

#include <QFutureWatcher>
#include <QMap>
#include <QObject>
#include <QQueue>
#include <QThreadPool>
#include <memory>

class Contestant;
class Settings;
class SleepInhibitor;
class TaskJudger;

class JudgingController : public QObject {
	Q_OBJECT

  public:
	explicit JudgingController(const Settings *settings, QObject *parent = nullptr);
	~JudgingController() override;
	void addTask(TaskJudger *judger, Contestant *contestant, int taskId);

  public slots:
	void start();
	void stop();

  signals:
	void taskJudged();
	void judgeFinished();

  private:
	struct PendingTask {
		TaskJudger *judger;
		Contestant *contestant;
		int taskId;
	};

	void assign();
	void finishTask(QFutureWatcher<TaskResult> *watcher);

	QThreadPool threadPool;
	QQueue<PendingTask> queuingTasks;
	QMap<QFutureWatcher<TaskResult> *, PendingTask> runningTasks;
	bool isJudging = false;
	bool preventSleepWhileJudging;
	std::unique_ptr<SleepInhibitor> sleepInhibitor;
};
