/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QTreeWidget>

class Settings;
class Contest;

class SummaryTree : public QTreeWidget {
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
	QAction *ExtTestCaseModifierAction;

  private slots:
	void addTask();
	void addTestCase();
	void addTestCases();
	void deleteTask();
	void deleteTestCase();
	void selectionChanged();
	void itemChanged(QTreeWidgetItem *);
	void titleChanged(const QString &);
	void launchExtTestCaseModifier();

  signals:
	void taskChanged();
};
