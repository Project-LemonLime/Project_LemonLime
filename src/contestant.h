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
 * contestant.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef CONTESTANT_H
#define CONTESTANT_H

#include "base/LemonType.hpp"
#include <QObject>
#include <QtCore>

class Contestant : public QObject
{
	Q_OBJECT
  public:
	explicit Contestant(QObject *parent = nullptr);

	const QString &getContestantName() const;
	bool getCheckJudged(int) const;
	CompileState getCompileState(int) const;
	const QString &getSourceFile(int) const;
	const QString &getCompileMessage(int) const;
	const QList<QStringList> &getInputFiles(int) const;
	const QList<QList<ResultState>> &getResult(int) const;
	const QList<QStringList> &getMessage(int) const;
	const QList<QList<int>> &getSocre(int) const;
	const QList<QList<int>> &getTimeUsed(int) const;
	const QList<QList<int>> &getMemoryUsed(int) const;
	QDateTime getJudingTime() const;
	int getTaskScore(int) const;
	int getTotalScore() const;
	int getTotalUsedTime() const;

	void setContestantName(const QString &);
	void setCheckJudged(int, bool);
	void setCompileState(int, CompileState);
	void setSourceFile(int, const QString &);
	void setCompileMessage(int, const QString &);
	void setInputFiles(int, const QList<QStringList> &);
	void setResult(int, const QList<QList<ResultState>> &);
	void setMessage(int, const QList<QStringList> &);
	void setScore(int, const QList<QList<int>> &);
	void setTimeUsed(int, const QList<QList<int>> &);
	void setMemoryUsed(int, const QList<QList<int>> &);
	void setJudgingTime(QDateTime);

	void writeToStream(QDataStream &);
	void readFromStream(QDataStream &);

  private:
	QString contestantName;
	QList<bool> checkJudged;
	QList<CompileState> compileState;
	QStringList sourceFile;
	QStringList compileMesaage;
	QList<QList<QStringList>> inputFiles;
	QList<QList<QList<ResultState>>> result;
	QList<QList<QStringList>> message;
	QList<QList<QList<int>>> score;
	QList<QList<QList<int>>> timeUsed;
	QList<QList<QList<int>>> memoryUsed;
	QDateTime judgingTime;

  signals:

  public slots:
	void addTask();
	void deleteTask(int);
	void swapTask(int, int);
};

#endif // CONTESTANT_H
