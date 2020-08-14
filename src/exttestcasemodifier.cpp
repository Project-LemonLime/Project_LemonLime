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

/**
 * exttestcasemodifier.cpp @Project LemonLime
 * Update 2020 Coelacanthus
 **/

#include "exttestcasemodifier.h"
#include "exttestcaseupdaterdialog.h"
#include "task.h"
#include "testcase.h"
#include "ui_exttestcasemodifier.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#define qAsConst
#endif

ExtTestCaseModifier::ExtTestCaseModifier(QWidget *parent) : QWidget(parent), ui(new Ui::ExtTestCaseModifier)
{
	ui->setupUi(this);

	connect(ui->testCaseTable, &ExtTestCaseTable::testCaseSelectionChanged, this,
	        &ExtTestCaseModifier::whenTestCaseSelectionChanged);

	connect(ui->buttonModify, SIGNAL(clicked()), this, SLOT(modifySelected()));
	connect(ui->buttonUp, SIGNAL(clicked()), this, SLOT(moveUpSelected()));
	connect(ui->buttonDown, SIGNAL(clicked()), this, SLOT(moveDownSelected()));
	connect(ui->buttonRemove, SIGNAL(clicked()), this, SLOT(removeSelected()));
	connect(ui->buttonMerge, SIGNAL(clicked()), this, SLOT(mergeSelected()));
	connect(ui->buttonSplit, SIGNAL(clicked()), this, SLOT(splitSelected()));
	connect(ui->buttonNewSub, SIGNAL(clicked()), this, SLOT(appendNewSub()));
	connect(ui->buttonNewCase, SIGNAL(clicked()), this, SLOT(appendNewCase()));
}

ExtTestCaseModifier::~ExtTestCaseModifier() { delete ui; }

void ExtTestCaseModifier::refresh()
{
	ui->testCaseTable->refreshTask(editTask);
	whenTestCaseSelectionChanged();
}

void ExtTestCaseModifier::init(Task *theTask, const Settings *theSettings)
{
	editTask = theTask;
	editSettings = theSettings;

	refresh();

	whenTestCaseSelectionChanged();
}

void ExtTestCaseModifier::whenTestCaseSelectionChanged()
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

void ExtTestCaseModifier::modifySelected()
{
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	if (res.empty())
	{
		int l = hav.front(), r = hav.back();

		if (l != r)
		{
			auto *dialog = new ExtTestCaseUpdaterDialog(this, editTask, editSettings, -1, MAY_EDIT, NO_EDIT,
			                                            MAY_EDIT, MAY_EDIT, NO_EDIT);

			if (dialog->exec() == QDialog::Accepted)
			{
				auto sc = dialog->getScore();
				auto tl = dialog->getTimeLimit();
				auto ml = dialog->getMemoryLimit();

				for (int i = l; i <= r; i++)
				{
					if (sc != MAY_EDIT)
						editTask->getTestCase(i)->setFullScore(sc);

					if (tl != MAY_EDIT)
						editTask->getTestCase(i)->setTimeLimit(tl);

					if (ml != MAY_EDIT)
						editTask->getTestCase(i)->setMemoryLimit(ml);
				}
			}

			delete dialog;
		}
		else
		{
			auto *dialog = new ExtTestCaseUpdaterDialog(
			    this, editTask, editSettings, l + 1, editTask->getTestCase(l)->getFullScore(), NO_EDIT,
			    editTask->getTestCase(l)->getTimeLimit(), editTask->getTestCase(l)->getMemoryLimit(), 1,
			    editTask->getTestCase(l)->getDependenceSubtask());

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

		auto *dialog = new ExtTestCaseUpdaterDialog(this, editTask, editSettings, -1, NO_EDIT, 1, NO_EDIT,
		                                            NO_EDIT, NO_EDIT);

		if (dialog->exec() == QDialog::Accepted)
		{
			editTask->getTestCase(who)->setInputFiles(loc, dialog->getInput());
			editTask->getTestCase(who)->setOutputFiles(loc, dialog->getOutput());
		}
	}

	refresh();
}

void ExtTestCaseModifier::moveUpSelected()
{
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	auto temp = ui->testCaseTable->getSelectRange();
	int dlt = 0;

	if (! res.empty())
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

				if (a == l - 1)
					continue;

				if (l <= a && a <= r)
					a--;

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

				if (l <= a && a <= r)
					a--;
				else if (a == l - 1)
					a = r;

				havd.insert(a + 1);
			}

			editTask->getTestCase(i)->setDependenceSubtask(havd);
		}
	}

	refresh();

	ui->testCaseTable->modifySelected(temp.first + dlt, temp.second + dlt);
}

void ExtTestCaseModifier::moveDownSelected()
{
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	auto temp = ui->testCaseTable->getSelectRange();
	int dlt = 0;

	if (! res.empty())
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

				if (l <= a && a <= r)
					continue;

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

				if (l <= a && a <= r)
					a++;

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

				if (l <= a && a <= r)
					a++;
				else if (a == r + 1)
					a = l;

				havd.insert(a + 1);
			}

			editTask->getTestCase(i)->setDependenceSubtask(havd);
		}
	}

	refresh();

	ui->testCaseTable->modifySelected(temp.first + dlt, temp.second + dlt);
}

void ExtTestCaseModifier::removeSelected()
{
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	int ban1 = -1, ban2 = -1;

	for (auto i : qAsConst(res))
	{
		if (ban1 < 0)
			ban1 = i.first;
		else
			ban2 = i.first;

		int l = i.second.first, r = i.second.second;

		for (int j = l; j <= r; j++)
			editTask->getTestCaseList()[i.first]->deleteSingleCase(l);
	}

	int l = hav.front(), r = hav.back();

	while (l == ban1 || l == ban2)
		l++;

	while (r == ban1 || r == ban2)
		r--;

	for (int i = l; i <= r; i++)
		editTask->deleteTestCase(l);

	for (int i = l + 1, ii = editTask->getTestCaseList().size(); i < ii; i++)
	{
		auto nowd = editTask->getTestCase(i)->getDependenceSubtask();
		QSet<int> havd;

		for (auto a_ : nowd)
		{
			int a = a_ - 1;

			if (l <= a && a <= r)
				continue;
			else if (a > r)
				a -= r - l;

			havd.insert(a + 1);
		}

		editTask->getTestCase(i)->setDependenceSubtask(havd);
	}

	refresh();
}

void ExtTestCaseModifier::mergeSelected()
{
	auto hav = ui->testCaseTable->getSelectedHaveSub();

	auto temp = ui->testCaseTable->getSelectRange();

	auto *ans = new TestCase;

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

void ExtTestCaseModifier::splitSelected()
{
	auto hav = ui->testCaseTable->getSelectedHaveSub();

	auto temp = ui->testCaseTable->getSelectRange();

	QList<TestCase *> ans;

	int l = hav.front(), r = hav.back();

	for (int i = l; i <= r; i++)
	{
		TestCase *now = editTask->getTestCase(i);
		auto in = now->getInputFiles();
		auto out = now->getOutputFiles();
		int allScore = now->getFullScore(), gar = 0;

		if (in.size())
			gar = in.size() - allScore % in.size();
		else
			gar = 0;

		for (int j = 0; j < in.size(); j++)
		{
			auto *app = new TestCase;
			app->setFullScore((allScore / in.size()) + (j >= gar));
			app->setTimeLimit(now->getTimeLimit());
			app->setMemoryLimit(now->getMemoryLimit());
			app->setDependenceSubtask(now->getDependenceSubtask());
			app->addSingleCase(in[j], out[j]);

			ans.append(app);
		}
	}

	for (int i = l; i <= r; i++)
		editTask->deleteTestCase(l);

	for (int i = ans.size() - 1; i >= 0; i--)
		editTask->addTestCase(ans[i], l);

	refresh();

	ui->testCaseTable->modifySelected(temp.first, temp.second);
}

void ExtTestCaseModifier::appendNewSub()
{
	auto *dialog =
	    new ExtTestCaseUpdaterDialog(this, editTask, editSettings, editTask->getTestCaseList().size() + 1,
	                                 EDIT_WITH_DEFAULT, 1, EDIT_WITH_DEFAULT, EDIT_WITH_DEFAULT, 1);

	if (dialog->exec() == QDialog::Accepted)
	{
		auto *now = new TestCase;
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

void ExtTestCaseModifier::appendNewCase()
{
	int who = ui->testCaseTable->getSelectedHaveSub().back();

	auto *dialog = new ExtTestCaseUpdaterDialog(this, editTask, editSettings, who + 1, NO_EDIT, 1, NO_EDIT,
	                                            NO_EDIT, NO_EDIT);

	if (dialog->exec() == QDialog::Accepted)
	{
		editTask->getTestCase(who)->addSingleCase(dialog->getInput(), dialog->getOutput());
	}

	delete dialog;

	refresh();
}
