/*
 * SPDX-FileCopyrightText: 2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "base/LemonType.hpp"
#include "taskjudger.h"

#include <QMutex>
#include <QObject>
#include <QThread>

class JudgingController : public QObject {
	Q_OBJECT

  public:
	explicit JudgingController(QObject *parent = nullptr);
	void addTask(TaskJudger *judger);

  private:
	QQueue<TaskJudger *> queuingTasks;
	QMap<TaskJudger *, QThread *> runningTasks;
	bool isJudging;
	int maxThreads;
  public slots:
	void stop();
	void taskFinished();
	void assign();
	void start();

  signals:
	void judgeFinished();
};
