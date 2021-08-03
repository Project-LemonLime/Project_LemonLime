/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "contest.h"
//
#include "base/LemonUtils.hpp"
#include "base/compiler.h"
#include "base/settings.h"
#include "core/assignmentthread.h"
#include "core/contestant.h"
#include "core/judgingthread.h"
#include "core/task.h"
#include "core/testcase.h"
//
#include <QMessageBox>
#include <algorithm>
#include <utility>

Contest::Contest(QObject *parent) : QObject(parent) {}

void Contest::setSettings(Settings *_settings) { settings = _settings; }

void Contest::copySettings(Settings &_settings) { _settings.copyFrom(settings); }

void Contest::setContestTitle(const QString &title) { contestTitle = title; }

auto Contest::getContestTitle() const -> const QString & { return contestTitle; }

auto Contest::getTask(int index) const -> Task * {
	if (0 <= index && index < taskList.size()) {
		return taskList[index];
	}

	return nullptr;
}

auto Contest::getTaskList() const -> const QList<Task *> & { return taskList; }

void Contest::swapTask(int a, int b) {
	if (0 <= a && a < taskList.size()) {
		if (0 <= b && b < taskList.size()) {
			taskList.swapItemsAt(a, b);
		}
	}

	for (auto &i : contestantList)
		i->swapTask(a, b);
}

auto Contest::getContestant(const QString &name) const -> Contestant * {
	if (contestantList.contains(name)) {
		return contestantList.value(name);
	}

	return nullptr;
}

auto Contest::getContestantList() const -> QList<Contestant *> { return contestantList.values(); }

auto Contest::getTotalTimeLimit() const -> int {
	int total = 0;

	for (auto *i : taskList) {
		QList<TestCase *> testCaseList = i->getTestCaseList();

		for (auto &j : testCaseList) {
			total += j->getTimeLimit() * j->getInputFiles().size();
		}
	}

	return total;
}

auto Contest::getTotalScore() const -> int {
	int total = 0;

	for (auto *i : taskList) {
		total += i->getTotalScore();
	}

	return total;
}

void Contest::addTask(Task *task) {
	task->setParent(this);
	taskList.append(task);
	connect(task, &Task::problemTitleChanged, this, &Contest::problemTitleChanged);
	emit taskAddedForContestant();
	emit taskAddedForViewer();
}

void Contest::deleteTask(int index) {
	if (0 <= index && index < taskList.size()) {
		delete taskList[index];
		taskList.removeAt(index);
	}

	emit taskDeletedForContestant(index);
	emit taskDeletedForViewer(index);
}

void Contest::refreshContestantList() {
	QStringList nameList =
	    QDir(Settings::sourcePath()).entryList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
	QStringList curNameList = contestantList.keys();

	for (int i = 0; i < curNameList.size(); i++) {
		if (! nameList.contains(curNameList[i])) {
			delete contestantList[curNameList[i]];
			contestantList.remove(curNameList[i]);
		}
	}

	for (int i = 0; i < nameList.size(); i++) {
		if (! contestantList.contains(nameList[i])) {
			auto *newContestant = new Contestant(this);
			newContestant->setContestantName(nameList[i]);

			for (int j = 0; j < taskList.size(); j++) {
				newContestant->addTask();
			}

			contestantList.insert(nameList[i], newContestant);
			connect(this, &Contest::taskAddedForContestant, newContestant, &Contestant::addTask);
			connect(this, &Contest::taskDeletedForContestant, newContestant, &Contestant::deleteTask);
		}
	}
}

void Contest::deleteContestant(const QString &name) {
	if (! contestantList.contains(name))
		return;

	delete contestantList[name];
	contestantList.remove(name);
}

void Contest::clearPath(const QString &curDir) {
	QDir dir(curDir);
	QStringList fileList = dir.entryList(QDir::Files);

	for (int i = 0; i < fileList.size(); i++) {
		if (! dir.remove(fileList[i])) {
#ifdef Q_OS_WIN32
			QProcess::execute(QString("attrib"), QStringList("-R") + QStringList(curDir + fileList[i]));
#else
			QProcess::execute(QString("chmod"), QStringList("+w") + QStringList(curDir + fileList[i]));
#endif
			dir.remove(fileList[i]);
		}
	}

	QStringList dirList = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (int i = 0; i < dirList.size(); i++) {
		clearPath(curDir + dirList[i] + QDir::separator());
		dir.rmdir(dirList[i]);
	}
}

void Contest::judge(Contestant *contestant, const QVector<int> &indexes) {
	stopJudging = false;
	emit contestantJudgingStart(contestant->getContestantName());
	for (auto i : indexes) {
		emit taskJudgingStarted(taskList[i]->getProblemTitle());
		auto *thread = new AssignmentThread();
		connect(thread, &AssignmentThread::dialogAlert, this, &Contest::dialogAlert);
		connect(thread, &AssignmentThread::singleCaseFinished, this, &Contest::singleCaseFinished);
		connect(thread, &AssignmentThread::singleSubtaskDependenceFinished, this,
		        &Contest::singleSubtaskDependenceFinished);
		connect(thread, &AssignmentThread::compileError, this, &Contest::compileError);
		connect(this, &Contest::stopJudgingSignal, thread, &AssignmentThread::stopJudgingSlot);
		thread->setSettings(settings);
		thread->setTask(taskList[i]);
		thread->setContestantName(contestant->getContestantName());
		auto *eventLoop = new QEventLoop(this);
		connect(thread, &AssignmentThread::finished, eventLoop, &QEventLoop::quit);
		thread->start();
		eventLoop->exec();
		delete eventLoop;

		if (stopJudging) {
			delete thread;
			return;
		}

		contestant->setCompileState(i, thread->getCompileState());
		contestant->setCompileMessage(i, thread->getCompileMessage());
		contestant->setSourceFile(i, thread->getSourceFile());
		contestant->setInputFiles(i, thread->getInputFiles());
		contestant->setResult(i, thread->getResult());
		contestant->setMessage(i, thread->getMessage());
		contestant->setScore(i, thread->getScore());
		contestant->setTimeUsed(i, thread->getTimeUsed());
		contestant->setMemoryUsed(i, thread->getMemoryUsed());
		contestant->setCheckJudged(i, true);
		emit taskJudgedDisplay(taskList[i]->getProblemTitle(), thread->getScore(),
		                       taskList[i]->getTotalScore());
		emit taskJudgingFinished();
		delete thread;
	}

	contestant->setJudgingTime(QDateTime::currentDateTime());

	emit contestantJudgedDisplay(contestant->getContestantName(), contestant->getTotalScore(),
	                             getTotalScore());
	emit contestantJudgingFinished();
}

void Contest::judge(Contestant *contestant) {
	QVector<int> indexes;
	indexes.resize(taskList.size());
	std::iota(indexes.begin(), indexes.end(), 0);
	judge(contestant, indexes);
}

void Contest::judge(const QString &name) { judge(contestantList.value(name)); }

void Contest::judge(const QString &name, const QSet<int> &indexes) {
	judge(contestantList.value(name), QVector<int>(indexes.begin(), indexes.end()));
}

void Contest::judge(const QString &name, int index) { judge(contestantList.value(name), {index}); }

void Contest::judgeAll() {
	QList<Contestant *> contestants = contestantList.values();

	for (auto &contestant : contestants) {
		judge(contestant);

		if (stopJudging)
			break;
	}
}

void Contest::stopJudgingSlot() {
	stopJudging = true;
	emit stopJudgingSignal();
}

void Contest::writeToStream(QDataStream &out) {
	out << contestTitle;
	out << taskList.size();

	for (auto &i : taskList) {
		i->writeToStream(out);
	}

	out << contestantList.size();
	QList<Contestant *> list = contestantList.values();

	for (auto &i : list) {
		i->writeToStream(out);
	}
}
void Contest::writeToJson(QJsonObject &out) {
	WRITE_JSON(out, contestTitle);

	QJsonArray tasks;

	for (const auto &i : taskList) {
		QJsonObject obj;
		i->writeToJson(obj);
		tasks.append(obj);
	}

	WRITE_JSON(out, tasks);

	QJsonArray contestants;

	for (const auto &i : contestantList) {
		QJsonObject obj;
		i->writeToJson(out);
		contestants.append(obj);
	}

	WRITE_JSON(out, contestants);
}
int Contest::readFromJson(const QJsonObject &in) {
	READ_JSON(in, contestTitle);

	QJsonArray tasks;
	READ_JSON(in, tasks);

	taskList.clear();
	for (const auto &task : tasks) {
		Task *newTask = new Task(this);
		if (newTask->readFromJson(task.toObject()) == -1)
			return -1;
		taskList.append(newTask);
	}

	QJsonArray contestants;
	READ_JSON(in, contestants);

	contestantList.clear();
	for (const auto &contestant : contestants) {
		auto *newContestant = new Contestant(this);
		if (newContestant->readFromJson(contestant.toObject()) == -1)
			return -1;
		connect(this, &Contest::taskAddedForContestant, newContestant, &Contestant::addTask);
		connect(this, &Contest::taskDeletedForContestant, newContestant, &Contestant::deleteTask);
		contestantList.insert(newContestant->getContestantName(), newContestant);
	}
	return 0;
}
void Contest::readFromStream(QDataStream &in) {
	int count = 0;
	in >> contestTitle;
	in >> count;

	for (int i = 0; i < count; i++) {
		Task *newTask = new Task(this);
		newTask->readFromStream(in);
		newTask->refreshCompilerConfiguration(settings);
		taskList.append(newTask);
	}

	in >> count;

	for (int i = 0; i < count; i++) {
		auto *newContestant = new Contestant(this);
		newContestant->readFromStream(in);
		connect(this, &Contest::taskAddedForContestant, newContestant, &Contestant::addTask);
		connect(this, &Contest::taskDeletedForContestant, newContestant, &Contestant::deleteTask);
		contestantList.insert(newContestant->getContestantName(), newContestant);
	}
}
