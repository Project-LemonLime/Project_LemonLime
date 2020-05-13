#include <qtablewidget.h>
#include <qheaderview.h>
#include "exttestcasetable.h"

extTestCaseTable::extTestCaseTable(QWidget *parent)
{
	clear();

	horizontalHeader()->setMinimumHeight(25);
	horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader()->setDefaultAlignment(Qt::AlignRight | Qt::AlignVCenter);

	setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(this, &QTableWidget::itemSelectionChanged, this, &extTestCaseTable::whenItemSelectionChanged);
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

void extTestCaseTable::refreshTask(Task *nowTask)
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

void extTestCaseTable::whenItemSelectionChanged()
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

void extTestCaseTable::modifySelected(int mi, int mx)
{
	QTableWidgetSelectionRange range(mi, 1, mx, columnCount() - 1);
	setRangeSelected(range, true);
	whenItemSelectionChanged();
}
