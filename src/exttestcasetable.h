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

	bool canModify();
	bool canAddSub();
	bool canAddCase();
	bool canRemove();
	bool canUp();
	bool canDown();
	bool canMerge();
	bool canSplit();

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

	bool isCanModify;
	bool isCanAddSub;
	bool isCanAddCase;
	bool isCanRemove;
	bool isCanUp;
	bool isCanDown;
	bool isCanMerge;
	bool isCanSplit;
	bool selectMi{};
	bool selectMx{};
	bool noDfs;

  private slots:
	void whenItemSelectionChanged();

  signals:
	void testCaseSelectionChanged();
};
