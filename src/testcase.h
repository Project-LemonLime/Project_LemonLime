/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef TESTCASE_H
#define TESTCASE_H

#include <QObject>
#include <QtCore>

class TestCase : public QObject
{
	Q_OBJECT
  public:
	explicit TestCase(QObject *parent = nullptr);
	void copyTo(TestCase *);
	int getFullScore() const;
	int getTimeLimit() const;
	int getMemoryLimit() const;
	const QStringList &getInputFiles() const;
	const QStringList &getOutputFiles() const;
	const QList<int> &getDependenceSubtask() const;
	void setIndex(int);
	void setFullScore(int);
	void setTimeLimit(int);
	void setMemoryLimit(int);
	void setInputFiles(int, const QString &);
	void setOutputFiles(int, const QString &);
	void setDependenceSubtask(const QStringList &);
	void setDependenceSubtask(const QList<int> &);
	void setDependenceSubtask(const QSet<int> &);
	bool checkDependenceSubtask(const QStringList &);
	void addSingleCase(const QString &, const QString &);
	void deleteSingleCase(int);
	void writeToStream(QDataStream &);
	void readFromStream(QDataStream &);
	void clearDependenceSubtask();
	void swapFiles(int, int);

  private:
	QStringList inputFiles;
	QStringList outputFiles;
	QList<int> dependenceSubtask;
	int index{};
	int fullScore{};
	int timeLimit{};
	int memoryLimit{};
};

#endif // TESTCASE_H
