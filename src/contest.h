/***************************************************************************
    This file is part of Project Lemon
    Copyright (C) 2011 Zhipeng Jia

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/
/**
 * contest.h @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * contest.h @Project Lemon+
 * Update 2019 iotang
 **/

#ifndef CONTEST_H
#define CONTEST_H

#include "globaltype.h"
#include <QObject>
#include <QtCore>
#define MagicNumber 0x20111127

class Task;
class Settings;
class Contestant;

class Contest : public QObject
{
	Q_OBJECT
public:
	explicit Contest(QObject *parent = nullptr);
	void setSettings(Settings *);
	void copySettings(Settings &);
	void setContestTitle(const QString &);
	const QString &getContestTitle() const;
	Task *getTask(int) const;
	void swapTask(int, int);
	const QList<Task *> &getTaskList() const;
	Contestant *getContestant(const QString &) const;
	QList<Contestant *> getContestantList() const;
	int getTotalTimeLimit() const;
	int getTotalScore() const;
	void addTask(Task *);
	void deleteTask(int);
	void refreshContestantList();
	void deleteContestant(const QString &);
	void writeToStream(QDataStream &);
	void readFromStream(QDataStream &);

private:
	QString contestTitle;
	Settings *settings{};
	QList<Task *> taskList;
	QMap<QString, Contestant *> contestantList;
	bool stopJudging{};
	void judge(Contestant *);
	void judge(Contestant *, int);
	void judge(Contestant *, const QSet<int> &);
	void clearPath(const QString &);

public slots:
	void judge(const QString &);
	void judge(const QString &, int);
	void judge(const QString &, const QSet<int> &);
	void judgeAll();
	void stopJudgingSlot();

signals:
	void taskAddedForContestant();
	void taskDeletedForContestant(int);
	void taskAddedForViewer();
	void taskDeletedForViewer(int);
	void problemTitleChanged();
	void dialogAlert(QString);
	void singleCaseFinished(int, int, int, int, int, int, int);
	void singleSubtaskDependenceFinished(int, int, int);
	void taskJudgingStarted(QString);
	void taskJudgingFinished();
	void taskJudgedDisplay(const QString &, const QList<QList<int>> &, const int);
	void contestantJudgingStart(QString);
	void contestantJudgingFinished();
	void contestantJudgedDisplay(const QString &, const int, const int);
	void compileError(int, int);
	void stopJudgingSignal();
};

#endif // CONTEST_H
