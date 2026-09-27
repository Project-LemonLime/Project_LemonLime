/*
 * SPDX-FileCopyrightText: 2021-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "judgingcontroller.h"
#include "core/contestant.h"
#include "sleepinhibitor.h"

#include <QtMath>

#define LEMON_MODULE_NAME "JudgingController"

JudgingController::JudgingController(Settings *settings, QObject *parent) : QObject(parent) {
	isJudging = false;
	maxThreads = qMax(1, settings->getMaxJudgingThreads());
	preventSleepWhileJudging = settings->getPreventSleepWhileJudging();
}

JudgingController::~JudgingController() = default;

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
	connect(taskJudger, &TaskJudger::judgingFinished, this, &JudgingController::taskFinished);
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
		sleepInhibitor.reset();
		emit judgeFinished();
	}
}
void JudgingController::start() {
	if (queuingTasks.size() == 0) {
		emit judgeFinished();
		return;
	}
	if (preventSleepWhileJudging && ! sleepInhibitor)
		sleepInhibitor = std::make_unique<SleepInhibitor>(tr("Judging submissions"));
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
