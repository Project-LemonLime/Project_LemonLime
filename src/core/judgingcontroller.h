/*
 * SPDX-FileCopyrightText: 2021-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#pragma once

#include "base/LemonType.hpp"
#include "base/settings.h"
#include "taskjudger.h"

#include <QMap>
#include <QMutex>
#include <QObject>
#include <QQueue>
#include <QThread>

class JudgingController : public QObject {
	Q_OBJECT

  public:
	explicit JudgingController(Settings *settings, QObject *parent = nullptr);
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
