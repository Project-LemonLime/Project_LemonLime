/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QTableWidget>

class Task;
class TestCase;

class ExtTestCaseTable : public QTableWidget
{
	Q_OBJECT

  public:
	ExtTestCaseTable(QWidget *parent);

	void refreshTask(Task *);

	int canModify();
	int canAddSub();
	int canAddCase();
	int canRemove();
	int canUp();
	int canDown();
	int canMerge();
	int canSplit();

	QList<int> getSelectedHaveSub();
	QList<QPair<int, QPair<int, int>>> getSelectedResSub();
	QPair<int, int> getSelectRange();
	void modifySelected(int, int);

  private:
	Task *editTask{};

	void addItem(int row, int column, const QString &text);
	void addItem(int row, int column, const QString &text, const QString &tipText);

	QList<int> haveSub;
	QList<QPair<int, QPair<int, int>>> resSub;

	int isCanModify;
	int isCanAddSub;
	int isCanAddCase;
	int isCanRemove;
	int isCanUp;
	int isCanDown;
	int isCanMerge;
	int isCanSplit;
	int selectMi{};
	int selectMx{};
	int noDfs;

  private slots:
	void whenItemSelectionChanged();

  signals:
	void testCaseSelectionChanged();
};

