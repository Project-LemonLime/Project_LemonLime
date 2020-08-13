/***************************************************************************
    This file is part of Project LemonLime
    Copyright (C) 2020 iotang

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

#ifndef EXTTESTCASETABLE_H
#define EXTTESTCASETABLE_H

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

#endif // EXTTESTCASETABLE_H
