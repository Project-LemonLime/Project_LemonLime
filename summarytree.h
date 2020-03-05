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
 * summarytree.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef SUMMARYTREE_H
#define SUMMARYTREE_H

#include <QtCore>
#include <QtGui>
#include <QTreeWidget>
#include <QAction>
#include <QMessageBox>
#include <QMenu>
#include <QApplication>

class Settings;
class Contest;

class SummaryTree : public QTreeWidget
{
	Q_OBJECT
public:
	explicit SummaryTree(QWidget *parent = nullptr);
	void changeEvent(QEvent *) override;
	void setContest(Contest *);
	void setSettings(Settings *);
	void contextMenuEvent(QContextMenuEvent *) override;

private:
	int addCount;
	Contest *curContest;
	Settings *settings{};
	QAction *addTaskAction;
	QAction *addTestCaseAction;
	QAction *addTestCasesAction;
	QAction *addTaskKeyAction;
	QAction *addTestCaseKeyAction;
	QAction *deleteTaskAction;
	QAction *deleteTestCaseAction;
	QAction *deleteTaskKeyAction;
	QAction *deleteTestCaseKeyAction;

private slots:
	void addTask();
	void addTestCase();
	void addTestCases();
	void deleteTask();
	void deleteTestCase();
	void selectionChanged();
	void itemChanged(QTreeWidgetItem *);
	void titleChanged(const QString &);

signals:
	void taskChanged();
};

#endif // SUMMARYTREE_H
