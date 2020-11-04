/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "summarytree.h"
//
#include "addtestcaseswizard.h"
#include "core/contest.h"
#include "core/settings.h"
#include "core/task.h"
#include "core/testcase.h"
#include "exttestcasemodifierdialog.h"

SummaryTree::SummaryTree(QWidget *parent) : QTreeWidget(parent)
{
	curContest = nullptr;
	addCount = 0;
	addTaskAction = new QAction(tr("Add a New Task"), this);
	addTestCaseAction = new QAction(tr("Add a Test Case"), this);
	addTestCasesAction = new QAction(tr("Add Test Cases ..."), this);
	deleteTaskAction = new QAction(tr("Delete Current Task"), this);
	deleteTestCaseAction = new QAction(tr("Delete Current Test Case"), this);
	ExtTestCaseModifierAction = new QAction(tr("Advanced Test Case Modifier"), this);
	addTaskKeyAction = new QAction(this);
	addTestCaseKeyAction = new QAction(this);
	deleteTaskKeyAction = new QAction(this);
	deleteTestCaseKeyAction = new QAction(this);
	addTaskKeyAction->setShortcutContext(Qt::WidgetShortcut);
	addTestCaseKeyAction->setShortcutContext(Qt::WidgetShortcut);
	deleteTaskKeyAction->setShortcutContext(Qt::WidgetShortcut);
	deleteTestCaseKeyAction->setShortcutContext(Qt::WidgetShortcut);
	addTaskKeyAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Insert));
	addTestCaseKeyAction->setShortcut(QKeySequence(Qt::Key_Insert));
	deleteTaskKeyAction->setShortcut(QKeySequence(Qt::Key_Delete));
	deleteTestCaseKeyAction->setShortcut(QKeySequence(Qt::Key_Delete));
	addTaskKeyAction->setEnabled(true);
	addTestCaseKeyAction->setEnabled(false);
	deleteTaskKeyAction->setEnabled(false);
	deleteTestCaseKeyAction->setEnabled(false);
	addAction(addTaskKeyAction);
	addAction(addTestCaseKeyAction);
	addAction(deleteTaskKeyAction);
	addAction(deleteTestCaseKeyAction);
	connect(addTaskAction, &QAction::triggered, this, &SummaryTree::addTask);
	connect(addTestCaseAction, &QAction::triggered, this, &SummaryTree::addTestCase);
	connect(addTestCasesAction, &QAction::triggered, this, &SummaryTree::addTestCases);
	connect(ExtTestCaseModifierAction, &QAction::triggered, this, &SummaryTree::launchExtTestCaseModifier);
	connect(addTaskKeyAction, &QAction::triggered, this, &SummaryTree::addTask);
	connect(addTestCaseKeyAction, &QAction::triggered, this, &SummaryTree::addTestCase);
	connect(deleteTaskAction, &QAction::triggered, this, &SummaryTree::deleteTask);
	connect(deleteTestCaseAction, &QAction::triggered, this, &SummaryTree::deleteTestCase);
	connect(deleteTaskKeyAction, &QAction::triggered, this, &SummaryTree::deleteTask);
	connect(deleteTestCaseKeyAction, &QAction::triggered, this, &SummaryTree::deleteTestCase);
	connect(this, &SummaryTree::currentItemChanged, this, &SummaryTree::selectionChanged);
	connect(this, &SummaryTree::itemChanged, this, &SummaryTree::itemChanged);
}

void SummaryTree::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		addTaskAction->setText(QApplication::translate("SummaryTree", "Add a New Task", nullptr));
		addTestCaseAction->setText(QApplication::translate("SummaryTree", "Add a Test Case", nullptr));
		addTestCasesAction->setText(QApplication::translate("SummaryTree", "Add Test Cases ...", nullptr));
		deleteTaskAction->setText(QApplication::translate("SummaryTree", "Delete Current Task", nullptr));
		deleteTestCaseAction->setText(
		    QApplication::translate("SummaryTree", "Delete Current Test Case", nullptr));
		ExtTestCaseModifierAction->setText(
		    QApplication::translate("SummaryTree", "Advanced Test Case Modifier", nullptr));

		for (int i = 0; i < topLevelItemCount(); i++)
		{
			QTreeWidgetItem *taskItem = topLevelItem(i);

			for (int j = 0; j < taskItem->childCount(); j++)
			{
				taskItem->child(j)->setText(
				    0, QApplication::translate("SummaryTree", "Test Case #%1", nullptr).arg(j + 1));
			}
		}
	}
}

void SummaryTree::setContest(Contest *contest)
{
	if (curContest)
	{
		QList<Task *> taskList = curContest->getTaskList();

		for (auto &i : taskList)
			disconnect(i, SIGNAL(problemTitleChanged(QString)), this, SLOT(titleChanged(QString)));
	}

	curContest = contest;
	emit taskChanged();

	if (! curContest)
		return;

	setEnabled(false);
	clear();
	QList<Task *> taskList = curContest->getTaskList();

	for (auto &i : taskList)
	{
		connect(i, SIGNAL(problemTitleChanged(QString)), this, SLOT(titleChanged(QString)));
		auto *newTaskItem = new QTreeWidgetItem(this);
		newTaskItem->setText(0, i->getProblemTile());
		newTaskItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		for (int j = 0; j < i->getTestCaseList().size(); j++)
		{
			auto *newTestCaseItem = new QTreeWidgetItem(newTaskItem);
			newTestCaseItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			newTestCaseItem->setText(0, tr("Test Case #%1").arg(newTaskItem->childCount()));
		}
	}

	if (! taskList.empty())
		setCurrentItem(topLevelItem(0));

	setEnabled(true);
	emit currentItemChanged(nullptr, nullptr);
}

void SummaryTree::setSettings(Settings *_settings) { settings = _settings; }

void SummaryTree::contextMenuEvent(QContextMenuEvent * /*event*/)
{
	auto *contextMenu = new QMenu(this);
	QTreeWidgetItem *curItem = currentItem();

	if (! curItem)
	{
		contextMenu->addAction(addTaskAction);
		contextMenu->exec(QCursor::pos());
		delete contextMenu;
		return;
	}

	int index = indexOfTopLevelItem(curItem);

	if (index != -1)
	{
		contextMenu->addAction(addTaskAction);
		contextMenu->addAction(deleteTaskAction);
		contextMenu->addSeparator();
		contextMenu->addAction(addTestCaseAction);
		contextMenu->addAction(addTestCasesAction);
		contextMenu->addSeparator();
		contextMenu->addAction(ExtTestCaseModifierAction);
		contextMenu->exec(QCursor::pos());
		delete contextMenu;
	}
	else
	{
		contextMenu->addAction(addTaskAction);
		contextMenu->addAction(deleteTaskAction);
		contextMenu->addSeparator();
		contextMenu->addAction(addTestCaseAction);
		contextMenu->addAction(addTestCasesAction);
		contextMenu->addAction(deleteTestCaseAction);
		contextMenu->exec(QCursor::pos());
		delete contextMenu;
	}
}

void SummaryTree::addTask()
{
	Task *newTask = new Task;
	newTask->setAnswerFileExtension(settings->getDefaultOutputFileExtension());
	curContest->addTask(newTask);
	newTask->refreshCompilerConfiguration(settings);
	connect(newTask, SIGNAL(problemTitleChanged(QString)), this, SLOT(titleChanged(QString)));
	auto *newItem = new QTreeWidgetItem(this);
	setCurrentItem(newItem);
	newItem->setFlags(Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	newItem->setText(0, tr("Problem %1").arg(++addCount));
	editItem(newItem);
	emit taskChanged();
}

void SummaryTree::addTestCase()
{
	QTreeWidgetItem *curItem = currentItem();

	if (indexOfTopLevelItem(curItem) == -1)
	{
		curItem = curItem->parent();
	}

	int index = indexOfTopLevelItem(curItem);
	Task *curTask = curContest->getTask(index);
	auto *newTestCase = new TestCase;
	newTestCase->setFullScore(settings->getDefaultFullScore());
	newTestCase->setTimeLimit(settings->getDefaultTimeLimit());
	newTestCase->setMemoryLimit(settings->getDefaultMemoryLimit());
	curTask->addTestCase(newTestCase);
	auto *newItem = new QTreeWidgetItem(curItem);
	newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	newItem->setText(0, tr("Test Case #%1").arg(curItem->childCount()));
	setCurrentItem(newItem);
}

void SummaryTree::addTestCases()
{
	QTreeWidgetItem *curItem = currentItem();

	if (indexOfTopLevelItem(curItem) == -1)
	{
		curItem = curItem->parent();
	}

	int index = indexOfTopLevelItem(curItem);
	Task *curTask = curContest->getTask(index);
	auto *wizard = new AddTestCasesWizard(this);
	wizard->setSettings(settings, curTask->getTaskType() == Task::Traditional ||
	                                  curTask->getTaskType() == Task::Interaction ||
	                                  curTask->getTaskType() == Task::Communication);

	if (wizard->exec() == QDialog::Accepted)
	{
		QList<QStringList> inputFiles = wizard->getMatchedInputFiles();
		QList<QStringList> outputFiles = wizard->getMatchedOutputFiles();

		for (int i = 0; i < inputFiles.size(); i++)
		{
			addTestCase();
			QTreeWidgetItem *curItem = currentItem();
			QTreeWidgetItem *parentItem = curItem->parent();
			int taskIndex = indexOfTopLevelItem(parentItem);
			int testCaseIndex = parentItem->indexOfChild(curItem);
			Task *curTask = curContest->getTask(taskIndex);
			TestCase *curTestCase = curTask->getTestCase(testCaseIndex);
			curTestCase->setFullScore(wizard->getFullScore());
			curTestCase->setTimeLimit(wizard->getTimeLimit());
			curTestCase->setMemoryLimit(wizard->getMemoryLimit());

			for (int j = 0; j < inputFiles[i].size(); j++)
			{
				curTestCase->addSingleCase(inputFiles[i][j], outputFiles[i][j]);
			}

			setCurrentItem(parentItem);
			setCurrentItem(curItem);
		}
	}

	delete wizard;
}

void SummaryTree::deleteTask()
{
	if (QMessageBox::warning(this, tr("LemonLime"), tr("Are you sure to delete this task?"), QMessageBox::Yes,
	                         QMessageBox::Cancel) == QMessageBox::Cancel)
	{
		return;
	}

	QTreeWidgetItem *curItem = currentItem();

	if (indexOfTopLevelItem(curItem) == -1)
	{
		curItem = curItem->parent();
	}

	int index = indexOfTopLevelItem(curItem);

	if (index + 1 < topLevelItemCount())
	{
		setCurrentItem(topLevelItem(index + 1));
	}
	else
	{
		if (index - 1 >= 0)
		{
			setCurrentItem(topLevelItem(index - 1));
		}
		else
		{
			setCurrentItem(nullptr);
		}
	}

	delete curItem;
	curContest->deleteTask(index);
	emit taskChanged();
}

void SummaryTree::deleteTestCase()
{
	QTreeWidgetItem *curItem = currentItem();
	QTreeWidgetItem *parentItem = curItem->parent();
	int taskIndex = indexOfTopLevelItem(parentItem);
	int testCaseIndex = parentItem->indexOfChild(curItem);
	Task *curTask = curContest->getTask(taskIndex);
	delete curItem;
	curTask->deleteTestCase(testCaseIndex);

	for (int i = 0; i < parentItem->childCount(); i++)
	{
		parentItem->child(i)->setText(0, tr("Test Case #%1").arg(i + 1));
	}
}

void SummaryTree::selectionChanged()
{
	if (! isEnabled())
		return;

	QTreeWidgetItem *curItem = currentItem();

	if (! curItem)
	{
		addTestCaseKeyAction->setEnabled(false);
		deleteTaskKeyAction->setEnabled(false);
		deleteTestCaseKeyAction->setEnabled(false);
		return;
	}

	int index = indexOfTopLevelItem(curItem);

	if (index != -1)
	{
		addTestCaseKeyAction->setEnabled(true);
		deleteTaskKeyAction->setEnabled(true);
		deleteTestCaseKeyAction->setEnabled(false);
	}
	else
	{
		addTestCaseKeyAction->setEnabled(true);
		deleteTaskKeyAction->setEnabled(false);
		deleteTestCaseKeyAction->setEnabled(true);
	}
}

void SummaryTree::itemChanged(QTreeWidgetItem *item)
{
	int index = indexOfTopLevelItem(item);

	if (index != -1)
	{
		Task *curTask = curContest->getTask(index);
		curTask->setProblemTitle(item->text(0));
	}
}

void SummaryTree::titleChanged(const QString &title)
{
	QTreeWidgetItem *curItem = currentItem();

	if (curItem)
		curItem->setText(0, title);

	emit taskChanged();
}

void SummaryTree::launchExtTestCaseModifier()
{
	QTreeWidgetItem *curItem = currentItem();

	if (indexOfTopLevelItem(curItem) == -1)
	{
		curItem = curItem->parent();
	}

	int index = indexOfTopLevelItem(curItem);
	Task *curTask = curContest->getTask(index);

	auto *dialog = new ExtTestCaseModifierDialog;

	dialog->init(curTask, settings);

	if (dialog->exec() == QDialog::Accepted)
	{
		dialog->getEditTask()->copyTo(curTask);
	}

	setContest(curContest);
}
