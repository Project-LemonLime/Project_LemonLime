/*
 * SPDX-FileCopyrightText: 2020-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "exttestcasemodifier.h"
#include "ui_exttestcasemodifier.h"
//
#include "core/task.h"
#include "core/testcase.h"
#include "exttestcaseupdaterdialog.h"

ExtTestCaseModifier::ExtTestCaseModifier(QWidget *parent) : QWidget(parent), ui(new Ui::ExtTestCaseModifier) {
	ui->setupUi(this);

	connect(ui->testCaseTable, &ExtTestCaseTable::testCaseSelectionChanged, this,
	        &ExtTestCaseModifier::whenTestCaseSelectionChanged);

	connect(ui->buttonModify, &QPushButton::clicked, this, &ExtTestCaseModifier::modifySelected);
	connect(ui->buttonUp, &QPushButton::clicked, this, &ExtTestCaseModifier::moveUpSelected);
	connect(ui->buttonDown, &QPushButton::clicked, this, &ExtTestCaseModifier::moveDownSelected);
	connect(ui->buttonRemove, &QPushButton::clicked, this, &ExtTestCaseModifier::removeSelected);
	connect(ui->buttonMerge, &QPushButton::clicked, this, &ExtTestCaseModifier::mergeSelected);
	connect(ui->buttonSplit, &QPushButton::clicked, this, &ExtTestCaseModifier::splitSelected);
	connect(ui->buttonNewSub, &QPushButton::clicked, this, &ExtTestCaseModifier::appendNewSub);
	connect(ui->buttonNewCase, &QPushButton::clicked, this, &ExtTestCaseModifier::appendNewCase);
}

ExtTestCaseModifier::~ExtTestCaseModifier() { delete ui; }

void ExtTestCaseModifier::refresh() {
	ui->testCaseTable->refreshTask(editTask);
	whenTestCaseSelectionChanged();
}

void ExtTestCaseModifier::init(Task *theTask, const Settings *theSettings) {
	editTask = theTask;
	editSettings = theSettings;

	refresh();

	whenTestCaseSelectionChanged();
}

void ExtTestCaseModifier::whenTestCaseSelectionChanged() {
	ui->buttonModify->setEnabled(ui->testCaseTable->canModify());
	ui->buttonNewSub->setEnabled(ui->testCaseTable->canAddSub());
	ui->buttonNewCase->setEnabled(ui->testCaseTable->canAddCase());
	ui->buttonRemove->setEnabled(ui->testCaseTable->canRemove());
	ui->buttonUp->setEnabled(ui->testCaseTable->canUp());
	ui->buttonDown->setEnabled(ui->testCaseTable->canDown());
	ui->buttonMerge->setEnabled(ui->testCaseTable->canMerge());
	ui->buttonSplit->setEnabled(ui->testCaseTable->canSplit());
}

void ExtTestCaseModifier::modifySelected() {
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	if (res.empty()) {
		int l = hav.front(), r = hav.back();

		if (l != r) {
			auto *dialog = new ExtTestCaseUpdaterDialog(this, editTask, editSettings, -1, MAY_EDIT, NO_EDIT,
			                                            MAY_EDIT, MAY_EDIT, NO_EDIT);

			if (dialog->exec() == QDialog::Accepted) {
				auto sc = dialog->getScore();
				auto tl = dialog->getTimeLimit();
				auto ml = dialog->getMemoryLimit();

				for (int i = l; i <= r; i++) {
					if (sc != MAY_EDIT)
						editTask->getTestCase(i)->setFullScore(sc);

					if (tl != MAY_EDIT)
						editTask->getTestCase(i)->setTimeLimit(tl);

					if (ml != MAY_EDIT)
						editTask->getTestCase(i)->setMemoryLimit(ml);
				}
			}

			delete dialog;
		} else {
			auto *dialog = new ExtTestCaseUpdaterDialog(
			    this, editTask, editSettings, l + 1, editTask->getTestCase(l)->getFullScore(), NO_EDIT,
			    editTask->getTestCase(l)->getTimeLimit(), editTask->getTestCase(l)->getMemoryLimit(), 1,
			    editTask->getTestCase(l)->getDependenceSubtask());

			if (dialog->exec() == QDialog::Accepted) {
				editTask->getTestCase(l)->setFullScore(dialog->getScore());
				editTask->getTestCase(l)->setTimeLimit(dialog->getTimeLimit());
				editTask->getTestCase(l)->setMemoryLimit(dialog->getMemoryLimit());
				editTask->getTestCase(l)->setDependenceSubtask(dialog->getDepends());
			}

			delete dialog;
		}
	} else {
		int who = res.front().first, loc = res.front().second.first;

		auto *dialog = new ExtTestCaseUpdaterDialog(this, editTask, editSettings, -1, NO_EDIT, 1, NO_EDIT,
		                                            NO_EDIT, NO_EDIT);

		if (dialog->exec() == QDialog::Accepted) {
			editTask->getTestCase(who)->setInputFiles(loc, dialog->getInput());
			editTask->getTestCase(who)->setOutputFiles(loc, dialog->getOutput());
		}
	}

	refresh();
}

void ExtTestCaseModifier::moveUpSelected() {
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	auto temp = ui->testCaseTable->getSelectRange();
	int dlt = 0;

	if (! res.empty()) {
		int l = res[0].second.first, r = res[0].second.second;
		dlt = -1;

		for (int i = l; i <= r; i++)
			editTask->getTestCaseList()[res[0].first]->swapFiles(i - 1, i);
	} else {
		int l = hav.front(), r = hav.back();
		dlt = -editTask->getTestCase(l - 1)->getInputFiles().size();

		for (int i = l; i <= r; i++)
			editTask->swapTestCase(i - 1, i);

		for (int i = l - 1; i <= r - 1; i++) {
			auto nowd = editTask->getTestCase(i)->getDependenceSubtask();
			QSet<int> havd;

			for (auto a_ : nowd) {
				int a = a_ - 1;

				if (a == l - 1)
					continue;

				if (l <= a && a <= r)
					a--;

				havd.insert(a + 1);
			}

			editTask->getTestCase(i)->setDependenceSubtask(havd);
		}

		for (int i = r + 1, ii = editTask->getTestCaseList().size(); i < ii; i++) {
			auto nowd = editTask->getTestCase(i)->getDependenceSubtask();
			QSet<int> havd;

			for (auto a_ : nowd) {
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

void ExtTestCaseModifier::moveDownSelected() {
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	auto temp = ui->testCaseTable->getSelectRange();
	int dlt = 0;

	if (! res.empty()) {
		int l = res[0].second.first, r = res[0].second.second;
		dlt = 1;

		for (int i = r; i >= l; i--)
			editTask->getTestCaseList()[res[0].first]->swapFiles(i, i + 1);
	} else {
		int l = hav.front(), r = hav.back();
		dlt = editTask->getTestCase(r + 1)->getInputFiles().size();

		for (int i = r; i >= l; i--)
			editTask->swapTestCase(i, i + 1);

		{
			auto nowd = editTask->getTestCase(l)->getDependenceSubtask();
			QSet<int> havd;

			for (auto a_ : nowd) {
				int a = a_ - 1;

				if (l <= a && a <= r)
					continue;

				havd.insert(a + 1);
			}

			editTask->getTestCase(l)->setDependenceSubtask(havd);
		}

		for (int i = l + 1; i <= r + 1; i++) {
			auto nowd = editTask->getTestCase(i)->getDependenceSubtask();
			QSet<int> havd;

			for (auto a_ : nowd) {
				int a = a_ - 1;

				if (l <= a && a <= r)
					a++;

				havd.insert(a + 1);
			}

			editTask->getTestCase(i)->setDependenceSubtask(havd);
		}

		for (int i = r + 1, ii = editTask->getTestCaseList().size(); i < ii; i++) {
			auto nowd = editTask->getTestCase(i)->getDependenceSubtask();
			QSet<int> havd;

			for (auto a_ : nowd) {
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

void ExtTestCaseModifier::removeSelected() {
	auto hav = ui->testCaseTable->getSelectedHaveSub();
	auto res = ui->testCaseTable->getSelectedResSub();

	int ban1 = -1, ban2 = -1;

	for (auto i : qAsConst(res)) {
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

	for (int i = l + 1, ii = editTask->getTestCaseList().size(); i < ii; i++) {
		auto nowd = editTask->getTestCase(i)->getDependenceSubtask();
		QSet<int> havd;

		for (auto a_ : nowd) {
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

void ExtTestCaseModifier::mergeSelected() {
	auto hav = ui->testCaseTable->getSelectedHaveSub();

	auto temp = ui->testCaseTable->getSelectRange();

	int l = hav.front(), r = hav.back();

	auto ans = *editTask->getTestCase(l);

	for (int i = l + 1; i <= r; i++) {
		ans.setFullScore(ans.getFullScore() + editTask->getTestCase(i)->getFullScore());
		auto in = editTask->getTestCase(i)->getInputFiles();
		auto out = editTask->getTestCase(i)->getOutputFiles();

		for (int j = 0; j < in.size(); j++)
			ans.addSingleCase(in[j], out[j]);
	}

	for (int i = l + 1; i <= r; i++)
		editTask->deleteTestCase(l + 1);

	*editTask->getTestCase(l) = ans;

	refresh();

	ui->testCaseTable->modifySelected(temp.first, temp.second);
}

void ExtTestCaseModifier::splitSelected() {
	auto hav = ui->testCaseTable->getSelectedHaveSub();

	auto temp = ui->testCaseTable->getSelectRange();

	QList<TestCase *> ans;

	int l = hav.front(), r = hav.back();

	for (int i = l; i <= r; i++) {
		TestCase *now = editTask->getTestCase(i);
		auto in = now->getInputFiles();
		auto out = now->getOutputFiles();
		int allScore = now->getFullScore(), gar = 0;

		if (! in.empty())
			gar = in.size() - allScore % in.size();
		else
			gar = 0;

		for (int j = 0; j < in.size(); j++) {
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

void ExtTestCaseModifier::appendNewSub() {
	auto *dialog =
	    new ExtTestCaseUpdaterDialog(this, editTask, editSettings, editTask->getTestCaseList().size() + 1,
	                                 EDIT_WITH_DEFAULT, 1, EDIT_WITH_DEFAULT, EDIT_WITH_DEFAULT, 1);

	if (dialog->exec() == QDialog::Accepted) {
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

void ExtTestCaseModifier::appendNewCase() {
	int who = ui->testCaseTable->getSelectedHaveSub().constLast();

	auto *dialog = new ExtTestCaseUpdaterDialog(this, editTask, editSettings, who + 1, NO_EDIT, 1, NO_EDIT,
	                                            NO_EDIT, NO_EDIT);

	if (dialog->exec() == QDialog::Accepted) {
		editTask->getTestCase(who)->addSingleCase(dialog->getInput(), dialog->getOutput());
	}

	delete dialog;

	refresh();
}

#include "moc_exttestcasemodifier.cpp"