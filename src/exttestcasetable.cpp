/*
 * SPDX-FileCopyrightText: 2020-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "exttestcasetable.h"
//
#include "core/task.h"
#include "core/testcase.h"
//
#include <QHeaderView>
#include <limits>

ExtTestCaseTable::ExtTestCaseTable(QWidget * /*parent*/) {
	clear();

	horizontalHeader()->setMinimumHeight(25);
	horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

	verticalHeader()->hide();

	setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(this, &QTableWidget::itemSelectionChanged, this, &ExtTestCaseTable::whenItemSelectionChanged);
	connect(this, &QTableWidget::itemClicked, this, [this](QTableWidgetItem *item) {
		if (! item->column()) {
			int p = item->row();
			QTableWidgetSelectionRange range(p, 0, p + rowSpan(p, 0) - 1, this->columnCount() - 1);
			this->setRangeSelected(range, true);
		}
	});

	isCanModify = false;
	isCanAddSub = true;
	isCanAddCase = false;
	isCanRemove = false;
	isCanUp = false;
	isCanDown = false;
	isCanMerge = false;
	isCanSplit = false;
	noDfs = false;

	clear();
	setRowCount(0);
	setColumnCount(3);
	setHorizontalHeaderLabels({tr("Informations"), tr("Input"), tr("Output")});
}

auto ExtTestCaseTable::canModify() const -> bool { return isCanModify; }

auto ExtTestCaseTable::canAddSub() const -> bool { return isCanAddSub; }

auto ExtTestCaseTable::canAddCase() const -> bool { return isCanAddCase; }

auto ExtTestCaseTable::canRemove() const -> bool { return isCanRemove; }
auto ExtTestCaseTable::canUp() const -> bool { return isCanUp; }
auto ExtTestCaseTable::canDown() const -> bool { return isCanDown; }
auto ExtTestCaseTable::canMerge() const -> bool { return isCanMerge; }
auto ExtTestCaseTable::canSplit() const -> bool { return isCanSplit; }

auto ExtTestCaseTable::getSelectedHaveSub() const -> QList<int> { return haveSub; }

auto ExtTestCaseTable::getSelectedResSub() const -> QList<std::pair<int, std::pair<int, int>>> {
	return resSub;
}

auto ExtTestCaseTable::getSelectRange() const -> std::pair<int, int> {
	return std::make_pair(selectMi, selectMx);
}

void ExtTestCaseTable::addItem(int row, int column, const QString &text) {
	auto *item = new QTableWidgetItem(text);
	item->setTextAlignment(Qt::AlignCenter);
	item->setToolTip(text);
	this->setItem(row, column, item);
}

void ExtTestCaseTable::addItem(int row, int column, const QString &text, const QString &tipText) {
	auto *item = new QTableWidgetItem(text);
	item->setTextAlignment(Qt::AlignCenter);
	item->setToolTip(tipText);
	this->setItem(row, column, item);
}

void ExtTestCaseTable::refreshTask(Task *nowTask) {
	editTask = nowTask;

	if (editTask == nullptr)
		return;

	setRowCount(0);

	auto testCases = editTask->getTestCaseList();

	int nowrow = 0;

	for (int i = 0; i < testCases.size(); i++) {
		auto *nowsub = editTask->getTestCase(i);

		int score = nowsub->getFullScore();
		auto inputs = nowsub->getInputFiles();
		auto outputs = nowsub->getOutputFiles();
		auto times = nowsub->getTimeLimit();
		auto mems = nowsub->getMemoryLimit();
		auto depends = nowsub->getDependenceSubtask();
		int fileCount = nowsub->getInputFiles().size();

		for (int j = 0; j < fileCount; j++) {
			insertRow(nowrow);

			addItem(nowrow, 1, inputs[j]);
			addItem(nowrow, 2, outputs[j]);

			QString depStr = "", tipStr;

			if (editTask->getTaskType() == Task::AnswersOnly)
				tipStr = QString(tr("Test Case #%1:\n%2 Pt")).arg(i + 1).arg(score);
			else
				tipStr = QString(tr("Test Case #%1:\n%2 Pt\nTime Limit: %3 ms\nMemory Limit: %4 MiB"))
				             .arg(i + 1)
				             .arg(score)
				             .arg(times)
				             .arg(mems);

			if (depends.empty())
				depStr = "(-)", tipStr = tipStr + tr("\nNo Depends");
			else {
				depStr = QString("(%1)").arg(depends.size());
				tipStr = tipStr + QString(tr("\nDepends: "));

				for (auto k : depends)
					tipStr = tipStr + QString::number(k) + ", ";
			}

			if (editTask->getTaskType() == Task::AnswersOnly)
				addItem(nowrow, 0, QString(tr("#%1 :: %2 pt, %3")).arg(i + 1).arg(score).arg(depStr), tipStr);
			else
				addItem(nowrow, 0,
				        QString(tr("#%1 :: %2 pt, TL %3 ms, ML %4 MiB, %5"))
				            .arg(i + 1)
				            .arg(score)
				            .arg(times)
				            .arg(mems)
				            .arg(depStr),
				        tipStr);

			nowrow++;
		}

		if (fileCount > 1)
			setSpan(nowrow - fileCount, 0, fileCount, 1);
	}

	noDfs = false;
}

void ExtTestCaseTable::whenItemSelectionChanged() {
	if (noDfs)
		return;

	noDfs = true;

	isCanModify = false;
	isCanAddSub = true;
	isCanAddCase = false;
	isCanRemove = false;
	isCanUp = false;
	isCanDown = false;
	isCanMerge = false;
	isCanSplit = false;

	haveSub.clear();
	resSub.clear();

	auto setLists = selectedRanges();

	int minRow = std::numeric_limits<int>::max(), maxRow = -1;

	for (const auto &i : std::as_const(setLists)) {
		minRow = qMin(minRow, i.topRow());
		maxRow = qMax(maxRow, i.bottomRow());
	}

	selectMi = minRow;
	selectMx = maxRow;

	if (minRow > maxRow) {
		noDfs = false;
		return;
	}

	{
		QTableWidgetSelectionRange range(minRow, 1, maxRow, columnCount() - 1);
		setRangeSelected(range, true);
	}

	int subCnt = 0;

	QList<int> subSize;
	haveSub.clear();
	resSub.clear();

	for (int i = 0, j = 0; i < rowCount(); i = j + 1, subCnt++) {
		j = i + rowSpan(i, 0) - 1;
		subSize.append(j - i + 1);

		if (minRow <= i && j <= maxRow) {
			QTableWidgetSelectionRange range(i, 0, j, columnCount() - 1);
			setRangeSelected(range, true);
			haveSub.append(subCnt);
		} else if (j < minRow || i > maxRow)
			continue;
		else {
			haveSub.append(subCnt);
			resSub.append(
			    std::make_pair(subCnt, std::make_pair(qMax(0, minRow - i), qMin(j - i, maxRow - i))));
		}
	}

	if ((haveSub.size() == 1 && ! resSub.empty() &&
	     resSub.front().second.first == resSub.front().second.second) ||
	    resSub.empty())
		isCanModify = true;

	if (haveSub.size() == 1 && resSub.empty())
		isCanAddCase = true;

	if (! haveSub.empty())
		isCanRemove = true;

	if (haveSub.size() == 1 && resSub.size() == 1 && resSub[0].second.first > 0)
		isCanUp = true;

	if (resSub.empty() && haveSub.first() != 0)
		isCanUp = true;

	if (haveSub.size() == 1 && resSub.size() == 1 && resSub[0].second.second < subSize[resSub[0].first] - 1)
		isCanDown = true;

	if (resSub.empty() && haveSub.last() != subCnt - 1)
		isCanDown = true;

	if (haveSub.size() >= 2 && resSub.empty())
		isCanMerge = true;

	if (haveSub.size() >= 1 && resSub.empty())
		isCanSplit = true;

	emit testCaseSelectionChanged();

	noDfs = false;
}

void ExtTestCaseTable::modifySelected(int mi, int mx) {
	QTableWidgetSelectionRange range(mi, 1, mx, columnCount() - 1);
	setRangeSelected(range, true);
	whenItemSelectionChanged();
}
