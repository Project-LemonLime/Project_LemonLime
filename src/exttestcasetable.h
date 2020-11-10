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

class ExtTestCaseTable : public QTableWidget {
	Q_OBJECT

  public:
	ExtTestCaseTable(QWidget *parent);

	void refreshTask(Task *);

	bool canModify() const;
	bool canAddSub() const;
	bool canAddCase() const;
	bool canRemove() const;
	bool canUp() const;
	bool canDown() const;
	bool canMerge() const;
	bool canSplit() const;

	QList<int> getSelectedHaveSub() const;
	QList<QPair<int, QPair<int, int>>> getSelectedResSub() const;
	QPair<int, int> getSelectRange() const;
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
	int selectMi{};
	int selectMx{};
	bool noDfs;

  private slots:
	void whenItemSelectionChanged();

  signals:
	void testCaseSelectionChanged();
};
