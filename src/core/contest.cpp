/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "contest.h"
//
#include "core/assignmentthread.h"
#include "core/compiler.h"
#include "core/contestant.h"
#include "core/judgingthread.h"
#include "core/settings.h"
#include "core/task.h"
#include "core/testcase.h"
//
#include <QMessageBox>
#include <utility>

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#define swapItemsAt swap
#endif

Contest::Contest(QObject *parent) : QObject(parent) {}

void Contest::setSettings(Settings *_settings) { settings = _settings; }

void Contest::copySettings(Settings &_settings) { _settings.copyFrom(settings); }

void Contest::setContestTitle(const QString &title) { contestTitle = title; }

auto Contest::getContestTitle() const -> const QString & { return contestTitle; }

auto Contest::getTask(int index) const -> Task *
{
	if (0 <= index && index < taskList.size())
	{
		return taskList[index];
	}

	return nullptr;
}

auto Contest::getTaskList() const -> const QList<Task *> & { return taskList; }

void Contest::swapTask(int a, int b)
{
	if (0 <= a && a < taskList.size())
	{
		if (0 <= b && b < taskList.size())
		{
			taskList.swapItemsAt(a, b);
		}
	}

	for (auto &i : contestantList)
		i->swapTask(a, b);
}

auto Contest::getContestant(const QString &name) const -> Contestant *
{
	if (contestantList.contains(name))
	{
		return contestantList.value(name);
	}

	return nullptr;
}

auto Contest::getContestantList() const -> QList<Contestant *> { return contestantList.values(); }

auto Contest::getTotalTimeLimit() const -> int
{
	int total = 0;

	for (auto i : taskList)
	{
		QList<TestCase *> testCaseList = i->getTestCaseList();

		for (auto &j : testCaseList)
		{
			total += j->getTimeLimit() * j->getInputFiles().size();
		}
	}

	return total;
}

auto Contest::getTotalScore() const -> int
{
	int total = 0;

	for (auto i : taskList)
	{
		total += i->getTotalScore();
	}

	return total;
}

void Contest::addTask(Task *task)
{
	task->setParent(this);
	taskList.append(task);
	connect(task, SIGNAL(problemTitleChanged(QString)), this, SIGNAL(problemTitleChanged()));
	emit taskAddedForContestant();
	emit taskAddedForViewer();
}

void Contest::deleteTask(int index)
{
	if (0 <= index && index < taskList.size())
	{
		delete taskList[index];
		taskList.removeAt(index);
	}

	emit taskDeletedForContestant(index);
	emit taskDeletedForViewer(index);
}

void Contest::refreshContestantList()
{
	QStringList nameList =
	    QDir(Settings::sourcePath()).entryList(QStringList(), QDir::Dirs | QDir::NoDotAndDotDot);
	QStringList curNameList = contestantList.keys();

	for (int i = 0; i < curNameList.size(); i++)
	{
		if (! nameList.contains(curNameList[i]))
		{
			delete contestantList[curNameList[i]];
			contestantList.remove(curNameList[i]);
		}
	}

	for (int i = 0; i < nameList.size(); i++)
	{
		if (! contestantList.contains(nameList[i]))
		{
			auto *newContestant = new Contestant(this);
			newContestant->setContestantName(nameList[i]);

			for (int j = 0; j < taskList.size(); j++)
			{
				newContestant->addTask();
			}

			contestantList.insert(nameList[i], newContestant);
			connect(this, SIGNAL(taskAddedForContestant()), newContestant, SLOT(addTask()));
			connect(this, SIGNAL(taskDeletedForContestant(int)), newContestant, SLOT(deleteTask(int)));
		}
	}
}

void Contest::deleteContestant(const QString &name)
{
	if (! contestantList.contains(name))
		return;

	delete contestantList[name];
	contestantList.remove(name);
}

void Contest::clearPath(const QString &curDir)
{
	QDir dir(curDir);
	QStringList fileList = dir.entryList(QDir::Files);

	for (int i = 0; i < fileList.size(); i++)
	{
		if (! dir.remove(fileList[i]))
		{
#ifdef Q_OS_WIN32
			QProcess::execute(QString("attrib"), QStringList("-R") + QStringList(curDir + fileList[i]));
#else
			QProcess::execute(QString("chmod"), QStringList("+w") + QStringList(curDir + fileList[i]));
#endif
			dir.remove(fileList[i]);
		}
	}

	QStringList dirList = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (int i = 0; i < dirList.size(); i++)
	{
		clearPath(curDir + dirList[i] + QDir::separator());
		dir.rmdir(dirList[i]);
	}
}

void Contest::judge(Contestant *contestant)
{
	emit contestantJudgingStart(contestant->getContestantName());
	QDir(QDir::current()).mkdir(Settings::temporaryPath());

	for (int i = 0; i < taskList.size(); i++)
	{
		emit taskJudgingStarted(taskList[i]->getProblemTile());
		auto *thread = new AssignmentThread();
		connect(thread, SIGNAL(dialogAlert(QString)), this, SIGNAL(dialogAlert(QString)));
		connect(thread, SIGNAL(singleCaseFinished(int, int, int, int, int, int, int)), this,
		        SIGNAL(singleCaseFinished(int, int, int, int, int, int, int)));
		connect(thread, SIGNAL(singleSubtaskDependenceFinished(int, int, int)), this,
		        SIGNAL(singleSubtaskDependenceFinished(int, int, int)));
		connect(thread, SIGNAL(compileError(int, int)), this, SIGNAL(compileError(int, int)));
		connect(this, SIGNAL(stopJudgingSignal()), thread, SLOT(stopJudgingSlot()));
		thread->setSettings(settings);
		thread->setTask(taskList[i]);
		thread->setContestantName(contestant->getContestantName());
		auto *eventLoop = new QEventLoop(this);
		connect(thread, SIGNAL(finished()), eventLoop, SLOT(quit()));
		thread->start();
		eventLoop->exec();
		delete eventLoop;

		if (stopJudging)
		{
			delete thread;
			clearPath(Settings::temporaryPath());
			QDir().rmdir(Settings::temporaryPath());
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
		emit taskJudgedDisplay(taskList[i]->getProblemTile(), thread->getScore(),
		                       taskList[i]->getTotalScore());
		emit taskJudgingFinished();
		delete thread;
		clearPath(Settings::temporaryPath());
	}

	contestant->setJudgingTime(QDateTime::currentDateTime());
	QDir().rmdir(Settings::temporaryPath());
	emit contestantJudgedDisplay(contestant->getContestantName(), contestant->getTotalScore(),
	                             getTotalScore());
	emit contestantJudgingFinished();
}

void Contest::judge(Contestant *contestant, const QSet<int> &index)
{
	emit contestantJudgingStart(contestant->getContestantName());
	QDir(QDir::current()).mkdir(Settings::temporaryPath());

	for (int i = 0; i < taskList.size(); i++)
	{
		if (! index.contains(i))
			continue;

		emit taskJudgingStarted(taskList[i]->getProblemTile());
		auto *thread = new AssignmentThread();
		connect(thread, SIGNAL(dialogAlert(QString)), this, SIGNAL(dialogAlert(QString)));
		connect(thread, SIGNAL(singleCaseFinished(int, int, int, int, int, int, int)), this,
		        SIGNAL(singleCaseFinished(int, int, int, int, int, int, int)));
		connect(thread, SIGNAL(singleSubtaskDependenceFinished(int, int, int)), this,
		        SIGNAL(singleSubtaskDependenceFinished(int, int, int)));
		connect(thread, SIGNAL(compileError(int, int)), this, SIGNAL(compileError(int, int)));
		connect(this, SIGNAL(stopJudgingSignal()), thread, SLOT(stopJudgingSlot()));
		thread->setSettings(settings);
		thread->setTask(taskList[i]);
		thread->setContestantName(contestant->getContestantName());
		auto *eventLoop = new QEventLoop(this);
		connect(thread, SIGNAL(finished()), eventLoop, SLOT(quit()));
		thread->start();
		eventLoop->exec();
		delete eventLoop;

		if (stopJudging)
		{
			delete thread;
			clearPath(Settings::temporaryPath());
			QDir().rmdir(Settings::temporaryPath());
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
		emit taskJudgedDisplay(taskList[i]->getProblemTile(), thread->getScore(),
		                       taskList[i]->getTotalScore());
		emit taskJudgingFinished();
		delete thread;
		clearPath(Settings::temporaryPath());
	}

	contestant->setJudgingTime(QDateTime::currentDateTime());
	QDir().rmdir(Settings::temporaryPath());
	emit contestantJudgedDisplay(contestant->getContestantName(), contestant->getTotalScore(),
	                             getTotalScore());
	emit contestantJudgingFinished();
}

void Contest::judge(Contestant *contestant, int index)
{
	emit contestantJudgingStart(contestant->getContestantName());
	QDir(QDir::current()).mkdir(Settings::temporaryPath());
	emit taskJudgingStarted(taskList[index]->getProblemTile());
	auto *thread = new AssignmentThread();
	connect(thread, SIGNAL(dialogAlert(QString)), this, SIGNAL(dialogAlert(QString)));
	connect(thread, SIGNAL(singleCaseFinished(int, int, int, int, int, int, int)), this,
	        SIGNAL(singleCaseFinished(int, int, int, int, int, int, int)));
	connect(thread, SIGNAL(singleSubtaskDependenceFinished(int, int, int)), this,
	        SIGNAL(singleSubtaskDependenceFinished(int, int, int)));
	connect(thread, SIGNAL(compileError(int, int)), this, SIGNAL(compileError(int, int)));
	connect(this, SIGNAL(stopJudgingSignal()), thread, SLOT(stopJudgingSlot()));
	thread->setSettings(settings);
	thread->setTask(taskList[index]);
	thread->setContestantName(contestant->getContestantName());
	auto *eventLoop = new QEventLoop(this);
	connect(thread, SIGNAL(finished()), eventLoop, SLOT(quit()));
	thread->start();
	eventLoop->exec();
	delete eventLoop;

	if (stopJudging)
	{
		delete thread;
		clearPath(Settings::temporaryPath());
		QDir().rmdir(Settings::temporaryPath());
		return;
	}

	contestant->setCompileState(index, thread->getCompileState());
	contestant->setCompileMessage(index, thread->getCompileMessage());
	contestant->setSourceFile(index, thread->getSourceFile());
	contestant->setInputFiles(index, thread->getInputFiles());
	contestant->setResult(index, thread->getResult());
	contestant->setMessage(index, thread->getMessage());
	contestant->setScore(index, thread->getScore());
	contestant->setTimeUsed(index, thread->getTimeUsed());
	contestant->setMemoryUsed(index, thread->getMemoryUsed());
	contestant->setCheckJudged(index, true);
	emit taskJudgedDisplay(taskList[index]->getProblemTile(), thread->getScore(),
	                       taskList[index]->getTotalScore());
	emit taskJudgingFinished();
	delete thread;
	clearPath(Settings::temporaryPath());
	contestant->setJudgingTime(QDateTime::currentDateTime());
	QDir().rmdir(Settings::temporaryPath());
	emit contestantJudgedDisplay(contestant->getContestantName(), contestant->getTotalScore(),
	                             getTotalScore());
	emit contestantJudgingFinished();
}

void Contest::judge(const QString &name)
{
	clearPath(Settings::temporaryPath());
	stopJudging = false;
	judge(contestantList.value(name));
}

void Contest::judge(const QString &name, const QSet<int> &index)
{
	clearPath(Settings::temporaryPath());
	stopJudging = false;
	judge(contestantList.value(name), index);
}

void Contest::judge(const QString &name, int index)
{
	clearPath(Settings::temporaryPath());
	stopJudging = false;
	judge(contestantList.value(name), index);
}

void Contest::judgeAll()
{
	clearPath(Settings::temporaryPath());
	stopJudging = false;
	QList<Contestant *> contestants = contestantList.values();

	for (auto &contestant : contestants)
	{
		judge(contestant);

		if (stopJudging)
			break;
	}
}

void Contest::stopJudgingSlot()
{
	stopJudging = true;
	emit stopJudgingSignal();
}

void Contest::writeToStream(QDataStream &out)
{
	out << contestTitle;
	out << taskList.size();

	for (auto &i : taskList)
	{
		i->writeToStream(out);
	}

	out << contestantList.size();
	QList<Contestant *> list = contestantList.values();

	for (auto &i : list)
	{
		i->writeToStream(out);
	}
}

void Contest::readFromStream(QDataStream &in)
{
	int count = 0;
	in >> contestTitle;
	in >> count;

	for (int i = 0; i < count; i++)
	{
		Task *newTask = new Task(this);
		newTask->readFromStream(in);
		newTask->refreshCompilerConfiguration(settings);
		taskList.append(newTask);
	}

	in >> count;

	for (int i = 0; i < count; i++)
	{
		auto *newContestant = new Contestant(this);
		newContestant->readFromStream(in);
		connect(this, SIGNAL(taskAddedForContestant()), newContestant, SLOT(addTask()));
		connect(this, SIGNAL(taskDeletedForContestant(int)), newContestant, SLOT(deleteTask(int)));
		contestantList.insert(newContestant->getContestantName(), newContestant);
	}
}
