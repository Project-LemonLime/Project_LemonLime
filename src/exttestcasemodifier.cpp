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
//
#include <functional>
#include <memory>
#include <optional>

// Remap a single test case's subtask dependencies using a mapping function.
// The mapper receives 0-indexed dependency indices and returns:
//   - a mapped 0-indexed index, or
//   - std::nullopt to remove that dependency.
static void remapTestCaseDeps(TestCase *testCase, const std::function<std::optional<int>(int)> &mapper) {
	const auto &currentDeps = testCase->getDependenceSubtask();
	QSet<int> remappedDeps;

	for (int dep : currentDeps) {
		auto mapped = mapper(dep - 1);

		if (mapped.has_value())
			remappedDeps.insert(mapped.value() + 1);
	}

	testCase->setDependenceSubtask(remappedDeps);
}

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
	auto selectedSubtasks = ui->testCaseTable->getSelectedHaveSub();
	auto partialSelections = ui->testCaseTable->getSelectedResSub();

	if (partialSelections.empty()) {
		int l = selectedSubtasks.front(), r = selectedSubtasks.back();

		if (l != r) {
			auto dialog = std::make_unique<ExtTestCaseUpdaterDialog>(
			    this, editTask, editSettings, -1, MAY_EDIT, NO_EDIT, MAY_EDIT, MAY_EDIT, NO_EDIT);

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
		} else {
			auto dialog = std::make_unique<ExtTestCaseUpdaterDialog>(
			    this, editTask, editSettings, l + 1, editTask->getTestCase(l)->getFullScore(), NO_EDIT,
			    editTask->getTestCase(l)->getTimeLimit(), editTask->getTestCase(l)->getMemoryLimit(), 1,
			    editTask->getTestCase(l)->getDependenceSubtask());

			if (dialog->exec() == QDialog::Accepted) {
				editTask->getTestCase(l)->setFullScore(dialog->getScore());
				editTask->getTestCase(l)->setTimeLimit(dialog->getTimeLimit());
				editTask->getTestCase(l)->setMemoryLimit(dialog->getMemoryLimit());
				editTask->getTestCase(l)->setDependenceSubtask(dialog->getDepends());
			}
		}
	} else {
		int who = partialSelections.front().first, loc = partialSelections.front().second.first;

		auto dialog = std::make_unique<ExtTestCaseUpdaterDialog>(this, editTask, editSettings, -1, NO_EDIT, 1,
		                                                         NO_EDIT, NO_EDIT, NO_EDIT);

		if (dialog->exec() == QDialog::Accepted) {
			editTask->getTestCase(who)->setInputFiles(loc, dialog->getInput());
			editTask->getTestCase(who)->setOutputFiles(loc, dialog->getOutput());
		}
	}

	refresh();
}

void ExtTestCaseModifier::moveUpSelected() {
	auto selectedSubtasks = ui->testCaseTable->getSelectedHaveSub();
	auto partialSelections = ui->testCaseTable->getSelectedResSub();

	auto selectRange = ui->testCaseTable->getSelectRange();
	int selectionDelta = 0;

	if (! partialSelections.empty()) {
		int l = partialSelections[0].second.first, r = partialSelections[0].second.second;
		selectionDelta = -1;

		for (int i = l; i <= r; i++)
			editTask->getTestCaseList()[partialSelections[0].first]->swapFiles(i - 1, i);
	} else {
		int l = selectedSubtasks.front(), r = selectedSubtasks.back();
		selectionDelta = -editTask->getTestCase(l - 1)->getInputFiles().size();

		for (int i = l; i <= r; i++)
			editTask->swapTestCase(i - 1, i);

		for (int i = l - 1; i <= r - 1; i++) {
			remapTestCaseDeps(editTask->getTestCase(i), [l, r](int a) -> std::optional<int> {
				if (a == l - 1)
					return std::nullopt;

				if (l <= a && a <= r)
					return a - 1;

				return a;
			});
		}

		int totalCases = editTask->getTestCaseList().size();

		for (int i = r + 1; i < totalCases; i++) {
			remapTestCaseDeps(editTask->getTestCase(i), [l, r](int a) -> std::optional<int> {
				if (l <= a && a <= r)
					return a - 1;

				if (a == l - 1)
					return r;

				return a;
			});
		}
	}

	refresh();

	ui->testCaseTable->modifySelected(selectRange.first + selectionDelta,
	                                  selectRange.second + selectionDelta);
}

void ExtTestCaseModifier::moveDownSelected() {
	auto selectedSubtasks = ui->testCaseTable->getSelectedHaveSub();
	auto partialSelections = ui->testCaseTable->getSelectedResSub();

	auto selectRange = ui->testCaseTable->getSelectRange();
	int selectionDelta = 0;

	if (! partialSelections.empty()) {
		int l = partialSelections[0].second.first, r = partialSelections[0].second.second;
		selectionDelta = 1;

		for (int i = r; i >= l; i--)
			editTask->getTestCaseList()[partialSelections[0].first]->swapFiles(i, i + 1);
	} else {
		int l = selectedSubtasks.front(), r = selectedSubtasks.back();
		selectionDelta = editTask->getTestCase(r + 1)->getInputFiles().size();

		for (int i = r; i >= l; i--)
			editTask->swapTestCase(i, i + 1);

		remapTestCaseDeps(editTask->getTestCase(l), [l, r](int a) -> std::optional<int> {
			if (l <= a && a <= r)
				return std::nullopt;

			return a;
		});

		for (int i = l + 1; i <= r + 1; i++) {
			remapTestCaseDeps(editTask->getTestCase(i), [l, r](int a) -> std::optional<int> {
				if (l <= a && a <= r)
					return a + 1;

				return a;
			});
		}

		int totalCases = editTask->getTestCaseList().size();

		for (int i = r + 1; i < totalCases; i++) {
			remapTestCaseDeps(editTask->getTestCase(i), [l, r](int a) -> std::optional<int> {
				if (l <= a && a <= r)
					return a + 1;

				if (a == r + 1)
					return l;

				return a;
			});
		}
	}

	refresh();

	ui->testCaseTable->modifySelected(selectRange.first + selectionDelta,
	                                  selectRange.second + selectionDelta);
}

void ExtTestCaseModifier::removeSelected() {
	auto selectedSubtasks = ui->testCaseTable->getSelectedHaveSub();
	auto partialSelections = ui->testCaseTable->getSelectedResSub();

	int skipSub1 = -1, skipSub2 = -1;

	for (auto i : std::as_const(partialSelections)) {
		if (skipSub1 < 0)
			skipSub1 = i.first;
		else
			skipSub2 = i.first;

		int l = i.second.first, r = i.second.second;

		for (int j = l; j <= r; j++)
			editTask->getTestCaseList()[i.first]->deleteSingleCase(l);
	}

	int l = selectedSubtasks.front(), r = selectedSubtasks.back();

	while (l == skipSub1 || l == skipSub2)
		l++;

	while (r == skipSub1 || r == skipSub2)
		r--;

	for (int i = l; i <= r; i++)
		editTask->deleteTestCase(l);

	int totalCases = editTask->getTestCaseList().size();

	for (int i = l + 1; i < totalCases; i++) {
		remapTestCaseDeps(editTask->getTestCase(i), [l, r](int a) -> std::optional<int> {
			if (l <= a && a <= r)
				return std::nullopt;

			if (a > r)
				return a - (r - l);

			return a;
		});
	}

	refresh();
}

void ExtTestCaseModifier::mergeSelected() {
	auto selectedSubtasks = ui->testCaseTable->getSelectedHaveSub();

	auto selectRange = ui->testCaseTable->getSelectRange();

	int l = selectedSubtasks.front(), r = selectedSubtasks.back();

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

	ui->testCaseTable->modifySelected(selectRange.first, selectRange.second);
}

void ExtTestCaseModifier::splitSelected() {
	auto selectedSubtasks = ui->testCaseTable->getSelectedHaveSub();

	auto selectRange = ui->testCaseTable->getSelectRange();

	QList<TestCase *> ans;

	int l = selectedSubtasks.front(), r = selectedSubtasks.back();

	for (int i = l; i <= r; i++) {
		TestCase *now = editTask->getTestCase(i);
		auto in = now->getInputFiles();
		auto out = now->getOutputFiles();
		int allScore = now->getFullScore();

		// Distribute the total score evenly across test cases.
		// The first baseCaseCount cases get floor(allScore / n),
		// the remaining cases get ceil(allScore / n).
		int baseCaseCount = 0;

		if (! in.empty())
			baseCaseCount = in.size() - allScore % in.size();

		for (int j = 0; j < in.size(); j++) {
			auto *app = new TestCase;
			app->setFullScore((allScore / in.size()) + (j >= baseCaseCount));
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

	ui->testCaseTable->modifySelected(selectRange.first, selectRange.second);
}

void ExtTestCaseModifier::appendNewSub() {
	auto dialog = std::make_unique<ExtTestCaseUpdaterDialog>(
	    this, editTask, editSettings, editTask->getTestCaseList().size() + 1, EDIT_WITH_DEFAULT, 1,
	    EDIT_WITH_DEFAULT, EDIT_WITH_DEFAULT, 1);

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

	refresh();
}

void ExtTestCaseModifier::appendNewCase() {
	int who = ui->testCaseTable->getSelectedHaveSub().constLast();

	auto dialog = std::make_unique<ExtTestCaseUpdaterDialog>(this, editTask, editSettings, who + 1, NO_EDIT,
	                                                         1, NO_EDIT, NO_EDIT, NO_EDIT);

	if (dialog->exec() == QDialog::Accepted) {
		editTask->getTestCase(who)->addSingleCase(dialog->getInput(), dialog->getOutput());
	}

	refresh();
}
