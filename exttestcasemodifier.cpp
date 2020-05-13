#include "exttestcasemodifier.h"
#include "exttestcaseupdater.h"
#include "ui_exttestcasemodifier.h"

extTestCaseModifier::extTestCaseModifier(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::extTestCaseModifier)
{
	ui->setupUi(this);

	connect(ui->testCaseTable, &extTestCaseTable::testCaseSelectionChanged, this, &extTestCaseModifier::whenTestCaseSelectionChanged);

	connect(ui->buttonModify, SIGNAL(clicked()),
	        this, SLOT(modifySelected()));
	connect(ui->buttonUp, SIGNAL(clicked()),
	        this, SLOT(moveUpSelected()));
	connect(ui->buttonDown, SIGNAL(clicked()),
	        this, SLOT(moveDownSelected()));
	connect(ui->buttonRemove, SIGNAL(clicked()),
	        this, SLOT(removeSelected()));
	connect(ui->buttonMerge, SIGNAL(clicked()),
	        this, SLOT(mergeSelected()));
	connect(ui->buttonSplit, SIGNAL(clicked()),
	        this, SLOT(splitSelected()));
	connect(ui->buttonNewSub, SIGNAL(clicked()),
	        this, SLOT(appendNewSub()));
	connect(ui->buttonNewCase, SIGNAL(clicked()),
	        this, SLOT(appendNewCase()));
}

extTestCaseModifier::~extTestCaseModifier()
{
	delete ui;
}

void extTestCaseModifier::refresh()
{
	ui->testCaseTable->refreshTask(editTask);
	whenTestCaseSelectionChanged();
}

void extTestCaseModifier::init(Task *theTask, const Settings *theSettings)
{
	editTask = theTask;
	editSettings = theSettings;

	refresh();

	whenTestCaseSelectionChanged();
}

void extTestCaseModifier::whenTestCaseSelectionChanged()
{
	ui->buttonModify->setEnabled(ui->testCaseTable->canModify());
	ui->buttonNewSub->setEnabled(ui->testCaseTable->canAddSub());
	ui->buttonNewCase->setEnabled(ui->testCaseTable->canAddCase());
	ui->buttonRemove->setEnabled(ui->testCaseTable->canRemove());
	ui->buttonUp->setEnabled(ui->testCaseTable->canUp());
	ui->buttonDown->setEnabled(ui->testCaseTable->canDown());
	ui->buttonMerge->setEnabled(ui->testCaseTable->canMerge());
	ui->buttonSplit->setEnabled(ui->testCaseTable->canSplit());
}

void extTestCaseModifier::modifySelected()
{
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	if (res.empty())
	{
		int l = hav.front(), r = hav.back();

		if (l != r)
		{
			auto *dialog = new extTestCaseUpdater(this, editTask, editSettings, -1, -2, -2, -1, -1, -2);

			if (dialog->exec() == QDialog::Accepted)
			{
				for (int i = l; i <= r; i++)
				{
					editTask->getTestCase(i)->setTimeLimit(dialog->getTimeLimit());
					editTask->getTestCase(i)->setMemoryLimit(dialog->getMemoryLimit());
				}
			}

			delete dialog;
		}
		else
		{
			auto *dialog = new extTestCaseUpdater(this, editTask, editSettings, l + 1, editTask->getTestCase(l)->getFullScore(), -2, editTask->getTestCase(l)->getTimeLimit(), editTask->getTestCase(l)->getMemoryLimit(), 1, editTask->getTestCase(l)->getDependenceSubtask());

			if (dialog->exec() == QDialog::Accepted)
			{
				editTask->getTestCase(l)->setFullScore(dialog->getScore());
				editTask->getTestCase(l)->setTimeLimit(dialog->getTimeLimit());
				editTask->getTestCase(l)->setMemoryLimit(dialog->getMemoryLimit());
				editTask->getTestCase(l)->setDependenceSubtask(dialog->getDepends());
			}

			delete dialog;
		}
	}
	else
	{
		int who = res.front().first, loc = res.front().second.first;

		auto *dialog = new extTestCaseUpdater(this, editTask, editSettings, -2, -2, 1, -2, -2, -2);

		if (dialog->exec() == QDialog::Accepted)
		{
			editTask->getTestCase(who)->setInputFiles(loc, dialog->getInput());
			editTask->getTestCase(who)->setOutputFiles(loc, dialog->getOutput());
		}
	}

	refresh();
}

void extTestCaseModifier::moveUpSelected()
{
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	auto temp = ui->testCaseTable->getSelectRange();
	int dlt = 0;

	if (!res.empty())
	{
		int l = res[0].second.first, r = res[0].second.second;
		dlt = -1;

		for (int i = l; i <= r; i++)
			editTask->getTestCaseList()[res[0].first]->swapFiles(i - 1, i);
	}
	else
	{
		int l = hav.front(), r = hav.back();
		dlt = -editTask->getTestCase(l - 1)->getInputFiles().size();

		for (int i = l; i <= r; i++)
			editTask->swapTestCase(i - 1, i);

		for (int i = l - 1; i <= r - 1; i++)
		{
			auto nowd = editTask->getTestCase(i)->getDependenceSubtask();
			QSet<int> havd;

			for (auto a_ : nowd)
			{
				int a = a_ - 1;

				if (a == l - 1)continue;

				if (l <= a && a <= r)a--;

				havd.insert(a + 1);
			}

			editTask->getTestCase(i)->setDependenceSubtask(havd);
		}

		for (int i = r + 1, ii = editTask->getTestCaseList().size(); i < ii; i++)
		{
			auto nowd = editTask->getTestCase(i)->getDependenceSubtask();
			QSet<int> havd;

			for (auto a_ : nowd)
			{
				int a = a_ - 1;

				if (l <= a && a <= r)a--;
				else if (a == l - 1)a = r;

				havd.insert(a + 1);
			}

			editTask->getTestCase(i)->setDependenceSubtask(havd);
		}
	}

	refresh();

	ui->testCaseTable->modifySelected(temp.first + dlt, temp.second + dlt);
}

void extTestCaseModifier::moveDownSelected()
{
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	auto temp = ui->testCaseTable->getSelectRange();
	int dlt = 0;

	if (!res.empty())
	{
		int l = res[0].second.first, r = res[0].second.second;
		dlt = 1;

		for (int i = r; i >= l; i--)
			editTask->getTestCaseList()[res[0].first]->swapFiles(i, i + 1);
	}
	else
	{
		int l = hav.front(), r = hav.back();
		dlt = editTask->getTestCase(r + 1)->getInputFiles().size();

		for (int i = r; i >= l; i--)
			editTask->swapTestCase(i, i + 1);

		{
			auto nowd = editTask->getTestCase(l)->getDependenceSubtask();
			QSet<int> havd;

			for (auto a_ : nowd)
			{
				int a = a_ - 1;

				if (l <= a && a <= r)continue;

				havd.insert(a + 1);
			}

			editTask->getTestCase(l)->setDependenceSubtask(havd);
		}

		for (int i = l + 1; i <= r + 1; i++)
		{
			auto nowd = editTask->getTestCase(i)->getDependenceSubtask();
			QSet<int> havd;

			for (auto a_ : nowd)
			{
				int a = a_ - 1;

				if (l <= a && a <= r)a++;

				havd.insert(a + 1);
			}

			editTask->getTestCase(i)->setDependenceSubtask(havd);
		}

		for (int i = r + 1, ii = editTask->getTestCaseList().size(); i < ii; i++)
		{
			auto nowd = editTask->getTestCase(i)->getDependenceSubtask();
			QSet<int> havd;

			for (auto a_ : nowd)
			{
				int a = a_ - 1;

				if (l <= a && a <= r)a++;
				else if (a == r + 1)a = l;

				havd.insert(a + 1);
			}

			editTask->getTestCase(i)->setDependenceSubtask(havd);
		}
	}

	refresh();

	ui->testCaseTable->modifySelected(temp.first + dlt, temp.second + dlt);
}

void extTestCaseModifier::removeSelected()
{
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	int ban1 = -1, ban2 = -1;

	for (auto i : qAsConst(res))
	{
		if (ban1 < 0)ban1 = i.first;
		else ban2 = i.first;

		int l = i.second.first, r = i.second.second;

		for (int j = l; j <= r; j++)
			editTask->getTestCaseList()[i.first]->deleteSingleCase(l);
	}

	int l = hav.front(), r = hav.back();

	while (l == ban1 || l == ban2)l++;

	while (r == ban1 || r == ban2)r--;

	for (int i = l; i <= r; i++)
		editTask->deleteTestCase(l);

	for (int i = l + 1, ii = editTask->getTestCaseList().size(); i < ii; i++)
	{
		auto nowd = editTask->getTestCase(i)->getDependenceSubtask();
		QSet<int> havd;

		for (auto a_ : nowd)
		{
			int a = a_ - 1;

			if (l <= a && a <= r)continue;
			else if (a > r)a -= r - l;

			havd.insert(a + 1);
		}

		editTask->getTestCase(i)->setDependenceSubtask(havd);
	}

	refresh();
}

void extTestCaseModifier::mergeSelected()
{
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	auto temp = ui->testCaseTable->getSelectRange();

	TestCase *ans = new TestCase;

	int l = hav.front(), r = hav.back();

	editTask->getTestCase(l)->copyTo(ans);

	for (int i = l + 1; i <= r; i++)
	{
		ans->setFullScore(ans->getFullScore() + editTask->getTestCase(i)->getFullScore());
		auto in = editTask->getTestCase(i)->getInputFiles();
		auto out = editTask->getTestCase(i)->getOutputFiles();

		for (int j = 0; j < in.size(); j++)
			ans->addSingleCase(in[j], out[j]);
	}

	for (int i = l + 1; i <= r; i++)
		editTask->deleteTestCase(l + 1);

	ans->copyTo(editTask->getTestCase(l));
	delete ans;

	refresh();

	ui->testCaseTable->modifySelected(temp.first, temp.second);
}

void extTestCaseModifier::splitSelected()
{
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	auto temp = ui->testCaseTable->getSelectRange();

	QList<TestCase *> ans;

	int xcnt = 0;

	int l = hav.front(), r = hav.back();

	for (int i = l; i <= r; i++)
	{
		TestCase *now = editTask->getTestCase(i);
		auto in = now->getInputFiles();
		auto out = now->getOutputFiles();
		int allScore = now->getFullScore(), gar;

		if (in.size())gar = in.size() - allScore % in.size();
		else gar = 0;

		xcnt += r + l - 2;

		for (int j = 0; j < in.size(); j++)
		{
			TestCase *app = new TestCase;
			app->setFullScore((allScore / in.size()) + (j >= gar));
			app->setTimeLimit(now->getTimeLimit());
			app->setMemoryLimit(now->getMemoryLimit());
			app->setDependenceSubtask(now->getDependenceSubtask());
			app->addSingleCase(in[j], out[j]);

			ans.append(app);
		}
	}

	for (int i = l ; i <= r; i++)
		editTask->deleteTestCase(l);

	for (int i = ans.size() - 1; i >= 0; i--)
		editTask->addTestCase(ans[i], l);

	refresh();

	ui->testCaseTable->modifySelected(temp.first, temp.second);
}


void extTestCaseModifier::appendNewSub()
{
	auto *dialog = new extTestCaseUpdater(this, editTask, editSettings, editTask->getTestCaseList().size() + 1, -1, 1, -1, -1, 1);

	if (dialog->exec() == QDialog::Accepted)
	{
		TestCase *now = new TestCase;
		now->setIndex(editTask->getTestCaseList().size() + 1);
		now->setFullScore(dialog->getScore());
		now->setTimeLimit(dialog->getTimeLimit());
		now->setMemoryLimit(dialog->getMemoryLimit());
		now->addSingleCase(dialog->getInput(), dialog->getOutput());
		now->setDependenceSubtask(dialog->getDepends());
		editTask->addTestCase(now);
	}

	delete dialog;

	refresh();
}

void extTestCaseModifier::appendNewCase()
{
	int who = ui->testCaseTable->getSelectedHaveSub().front();

	auto *dialog = new extTestCaseUpdater(this, editTask, editSettings, who + 1, -2, 1, -2, -2, -2);

	if (dialog->exec() == QDialog::Accepted)
	{
		editTask->getTestCase(who)->addSingleCase(dialog->getInput(), dialog->getOutput());
	}

	delete dialog;

	refresh();
}
