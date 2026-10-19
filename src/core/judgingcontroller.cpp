/*
 * SPDX-FileCopyrightText: 2021-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "judgingcontroller.h"
#include "base/settings.h"
#include "core/contestant.h"
#include "core/taskjudger.h"
#include "sleepinhibitor.h"

#include <QtConcurrent/QtConcurrentRun>

JudgingController::JudgingController(const Settings *settings, QObject *parent)
    : QObject(parent), preventSleepWhileJudging(settings->getPreventSleepWhileJudging()) {
	threadPool.setMaxThreadCount(qMax(1, settings->getMaxJudgingThreads()));
}

JudgingController::~JudgingController() {
	stop();
	threadPool.waitForDone();
}

void JudgingController::addTask(TaskJudger *judger, Contestant *contestant, int taskId) {
	judger->setParent(this);
	queuingTasks.enqueue({judger, contestant, taskId});
}

void JudgingController::assign() {
	while (isJudging && ! queuingTasks.isEmpty() && runningTasks.size() < threadPool.maxThreadCount()) {
		const auto task = queuingTasks.dequeue();
		auto *watcher = new QFutureWatcher<TaskResult>(this);
		connect(watcher, &QFutureWatcher<TaskResult>::finished, this,
		        [this, watcher] { finishTask(watcher); });
		runningTasks.insert(watcher, task);
		watcher->setFuture(
		    QtConcurrent::run(&threadPool, [judger = task.judger](QPromise<TaskResult> &promise) {
			    promise.addResult(judger->judgeIt(promise));
		    }));
	}
}

void JudgingController::finishTask(QFutureWatcher<TaskResult> *watcher) {
	const auto task = runningTasks.take(watcher);
	if (watcher->isCanceled()) {
		task.contestant->setCheckJudged(task.taskId, false);
	} else {
		task.contestant->applyResult(task.taskId, watcher->result());
	}
	delete task.judger;
	watcher->deleteLater();
	emit taskJudged();
	assign();
	if (runningTasks.isEmpty() && queuingTasks.isEmpty()) {
		isJudging = false;
		sleepInhibitor.reset();
		emit judgeFinished();
	}
}

void JudgingController::start() {
	if (queuingTasks.isEmpty()) {
		emit judgeFinished();
		return;
	}
	if (preventSleepWhileJudging)
		sleepInhibitor = std::make_unique<SleepInhibitor>(tr("Judging submissions"));
	isJudging = true;
	assign();
}

void JudgingController::stop() {
	isJudging = false;
	while (! queuingTasks.isEmpty())
		delete queuingTasks.dequeue().judger;
	for (auto *watcher : runningTasks.keys()) {
		// Preserve results that finished before the stop request reached the GUI.
		if (! watcher->isFinished())
			watcher->cancel();
	}
}
