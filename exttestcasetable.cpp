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

#include <qtablewidget.h>
#include <qheaderview.h>
#include "exttestcasetable.h"
#include "task.h"
#include "testcase.h"

ExtTestCaseTable::ExtTestCaseTable(QWidget *parent)
{
	clear();

	horizontalHeader()->setMinimumHeight(25);
	horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

	verticalHeader()->hide();

	setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(this, &QTableWidget::itemSelectionChanged, this, &ExtTestCaseTable::whenItemSelectionChanged);
	connect(this, &QTableWidget::itemClicked, this, [this](QTableWidgetItem * item)
	{
		if (!item->column())
		{
			int p = item->row();
			QTableWidgetSelectionRange range(p, 0, p + rowSpan(p, 0) - 1, this->columnCount() - 1);
			this->setRangeSelected(range, true);
		}
	});

	isCanModify = 0;
	isCanAddSub = 1;
	isCanAddCase = 0;
	isCanRemove = 0;
	isCanUp = 0;
	isCanDown = 0;
	isCanMerge = 0;
	isCanSplit = 0;
	noDfs = 0;

	clear();
	setRowCount(0);
	setColumnCount(3);
	setHorizontalHeaderLabels(
	{
		tr("Informations"),
		tr("Input"),
		tr("Output")
	});

}

int ExtTestCaseTable::canModify()
{
	return isCanModify;
}

int ExtTestCaseTable::canAddSub()
{
	return isCanAddSub;
}

int ExtTestCaseTable::canAddCase()
{
	return isCanAddCase;
}

int ExtTestCaseTable::canRemove()
{
	return isCanRemove;
}
int ExtTestCaseTable::canUp()
{
	return isCanUp;
}
int ExtTestCaseTable::canDown()
{
	return isCanDown;
}
int ExtTestCaseTable::canMerge()
{
	return isCanMerge;
}
int ExtTestCaseTable::canSplit()
{
	return isCanSplit;
}

QList<int> ExtTestCaseTable::getSelectedHaveSub()
{
	return haveSub;
}

QList<QPair<int, QPair<int, int>>> ExtTestCaseTable::getSelectedResSub()
{
	return resSub;
}

QPair<int, int> ExtTestCaseTable::getSelectRange()
{
	return qMakePair(selectMi, selectMx);
}

void ExtTestCaseTable::addItem(int row, int column, const QString &text)
{
	QTableWidgetItem *item = new QTableWidgetItem(text);
	item->setTextAlignment(Qt::AlignCenter);
	item->setToolTip(text);
	this->setItem(row, column, item);
}

void ExtTestCaseTable::addItem(int row, int column, const QString &text, const QString &tipText)
{
	QTableWidgetItem *item = new QTableWidgetItem(text);
	item->setTextAlignment(Qt::AlignCenter);
	item->setToolTip(tipText);
	this->setItem(row, column, item);
}

void ExtTestCaseTable::refreshTask(Task *nowTask)
{
	editTask = nowTask;

	if (editTask == nullptr) return;

	setRowCount(0);

	auto testCases = editTask->getTestCaseList();

	int nowrow = 0;

	for (int i = 0; i < testCases.size(); i++)
	{
		auto nowsub = editTask->getTestCase(i);

		int score = nowsub->getFullScore();
		auto inputs = nowsub->getInputFiles();
		auto outputs = nowsub->getOutputFiles();
		auto times = nowsub->getTimeLimit();
		auto mems = nowsub->getMemoryLimit();
		auto depends = nowsub->getDependenceSubtask();
		int xlen = nowsub->getInputFiles().size();

		for (int j = 0; j < xlen; j++)
		{
			insertRow(nowrow);

			addItem(nowrow, 1, inputs[j]);
			addItem(nowrow, 2, outputs[j]);

			QString depStr = "", tipStr;

			if (editTask->getTaskType() == Task::AnswersOnly)
				tipStr = QString(tr("Test Case #%1:\n%2 Pt")).arg(i + 1).arg(score);
			else
				tipStr = QString(tr("Test Case #%1:\n%2 Pt\nTime Limit: %3 ms\nMemory Limit: %4 MB")).arg(i + 1).arg(score).arg(times).arg(mems);

			if (depends.empty())depStr = "(-)", tipStr = tipStr + tr("\nNo Depends");
			else
			{
				depStr = QString("(%1)").arg(depends.size());
				tipStr = tipStr + QString(tr("\nDepends: "));

				for (auto k : depends)tipStr = tipStr + QString::number(k) + ", ";
			}

			if (editTask->getTaskType() == Task::AnswersOnly)
				addItem(nowrow, 0, QString(tr("#%1 :: %2 pt, %3")).arg(i + 1).arg(score).arg(depStr), tipStr);
			else
				addItem(nowrow, 0, QString(tr("#%1 :: %2 pt, TL %3 ms, ML %4 MB, %5")).arg(i + 1).arg(score).arg(times).arg(mems).arg(depStr), tipStr);

			nowrow++;
		}

		if (xlen > 1)setSpan(nowrow - xlen, 0, xlen, 1);
	}

	noDfs = 0;
}

void ExtTestCaseTable::whenItemSelectionChanged()
{
	if (noDfs) return;

	noDfs = 1;

	isCanModify = 0;
	isCanAddSub = 1;
	isCanAddCase = 0;
	isCanRemove = 0;
	isCanUp = 0;
	isCanDown = 0;
	isCanMerge = 0;
	isCanSplit = 0;

	haveSub.clear();
	resSub.clear();

	auto setLists = selectedRanges();

	int mi = 1e9, mx = -1;

	for (const auto &i : qAsConst(setLists))
	{
		mi = qMin(mi, i.topRow());
		mx = qMax(mx, i.bottomRow());
	}

	selectMi = mi;
	selectMx = mx;

	if (mi > mx)
	{
		noDfs = 0;
		return;
	}

	{
		QTableWidgetSelectionRange range(mi, 1, mx, columnCount() - 1);
		setRangeSelected(range, true);
	}

	int subCnt = 0;

	QList<int> subSize;
	haveSub.clear();
	resSub.clear();

	for (int i = 0, j = 0; i < rowCount(); i = j + 1, subCnt++)
	{
		j = i + rowSpan(i, 0) - 1;
		subSize.append(j - i + 1);

		if (mi <= i && j <= mx)
		{
			QTableWidgetSelectionRange range(i, 0, j, columnCount() - 1);
			setRangeSelected(range, true);
			haveSub.append(subCnt);
		}
		else if (j < mi || i > mx)continue;
		else
		{
			haveSub.append(subCnt);
			resSub.append(qMakePair(subCnt, qMakePair(qMax(0, mi - i), qMin(j - i, mx - i))));
		}
	}

	if ((haveSub.size() == 1 && !resSub.empty() && resSub.front().second.first == resSub.front().second.second) || resSub.empty())isCanModify = 1;

	if (haveSub.size() == 1 && resSub.empty())isCanAddCase = 1;

	if (!haveSub.empty()) isCanRemove = 1;

	if (haveSub.size() == 1 && resSub.size() == 1 && resSub[0].second.first > 0)isCanUp = 1;

	if (resSub.empty() && haveSub.first() != 0)isCanUp = 1;

	if (haveSub.size() == 1 && resSub.size() == 1 && resSub[0].second.second < subSize[resSub[0].first] - 1)isCanDown = 1;

	if (resSub.empty() && haveSub.last() != subCnt - 1)isCanDown = 1;

	if (haveSub.size() >= 2 && resSub.empty())isCanMerge = 1;

	if (haveSub.size() >= 1 && resSub.empty())isCanSplit = 1;

	emit testCaseSelectionChanged();

	noDfs = 0;
}

void ExtTestCaseTable::modifySelected(int mi, int mx)
{
	QTableWidgetSelectionRange range(mi, 1, mx, columnCount() - 1);
	setRangeSelected(range, true);
	whenItemSelectionChanged();
}
