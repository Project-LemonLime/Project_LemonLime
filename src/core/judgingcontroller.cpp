/*
 * SPDX-FileCopyrightText: 2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "judgingcontroller.h"
#include "core/contestant.h"

#include <QtMath>

#define LEMON_MODULE_NAME "JudgingController"

JudgingController::JudgingController(Settings *settings, QObject *parent) : QObject(parent) {
	isJudging = false;
	maxThreads = qMax(1, settings->getMaxJudgingThreads());
}

void JudgingController::assign() {
	if (! isJudging) {
		return;
	}
	if (queuingTasks.empty())
		return;
	QThread *thread = new QThread;
	auto *taskJudger = queuingTasks.front();
	queuingTasks.pop_front();
	taskJudger->moveToThread(thread);
	connect(taskJudger, &TaskJudger::judgeFinished, this, &JudgingController::taskFinished);
	runningTasks[taskJudger] = thread;
	thread->start();
	QMetaObject::invokeMethod(taskJudger, &TaskJudger::judgeIt);
}

void JudgingController::taskFinished() {
	auto *taskJudger = qobject_cast<TaskJudger *>(sender());
	if (taskJudger == nullptr) {
		return;
	}
	if (runningTasks.count(taskJudger)) {
		auto *thread = runningTasks[taskJudger];
		thread->quit();
		thread->wait();
		delete thread;
		runningTasks.remove(taskJudger);
		delete taskJudger;
	}
	assign();
	if (runningTasks.empty()) {
		isJudging = false;
		emit judgeFinished();
	}
}
void JudgingController::start() {
	if (queuingTasks.size() == 0) {
		emit judgeFinished();
		return;
	}
	isJudging = true;
	while (! queuingTasks.empty() && runningTasks.size() < maxThreads) {
		assign();
	}
}
void JudgingController::stop() {
	if (! isJudging)
		return;
	isJudging = false;
	for (auto [taskJudger, thread] : runningTasks.toStdMap()) {
		QMetaObject::invokeMethod(taskJudger, &TaskJudger::stop);
	}
	// emit judgeFinished();
}
void JudgingController::addTask(TaskJudger *taskJudger) { queuingTasks.push_back(taskJudger); }
