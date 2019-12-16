/***************************************************************************
    This file is part of Project Lemon
    Copyright (C) 2011 Zhipeng Jia

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
 * testcaseeditwidget.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "testcaseeditwidget.h"
#include "ui_testcaseeditwidget.h"
#include "settings.h"
#include "testcase.h"
#include <QAction>

TestCaseEditWidget::TestCaseEditWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::TestCaseEditWidget)
{
	ui->setupUi(this);

	editTestCase = nullptr;

	deleteAction = new QAction(this);
	deleteAction->setShortcutContext(Qt::WidgetShortcut);
	deleteAction->setShortcut(QKeySequence::Delete);
	deleteAction->setEnabled(false);
	ui->fileList->addAction(deleteAction);

	QRegExp regx("[0-9,]+$");
	QValidator *subtaskDependenceValidator = new QRegExpValidator(regx, ui->subtaskDependecne);
	ui->subtaskDependecne->setValidator(subtaskDependenceValidator);

	ui->fullScore->setValidator(new QIntValidator(1, Settings::upperBoundForFullScore(), this));
	ui->timeLimit->setValidator(new QIntValidator(1, Settings::upperBoundForTimeLimit(), this));
	ui->memoryLimit->setValidator(new QIntValidator(1, Settings::upperBoundForMemoryLimit(), this));
	ui->fileList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	ui->inputFileEdit->setFilters(QDir::Files);
	ui->outputFileEdit->setFilters(QDir::Files);
	connect(this, SIGNAL(dataPathChanged()),
	        ui->inputFileEdit, SLOT(refreshFileList()));
	connect(this, SIGNAL(dataPathChanged()),
	        ui->outputFileEdit, SLOT(refreshFileList()));
	connect(ui->subtaskDependecne, SIGNAL(editingFinished()),
	        this, SLOT(subtaskDependenceChanged()));    //auto save subtaskDependence

	connect(ui->addButton, SIGNAL(clicked()),
	        this, SLOT(addSingleCase()));
	connect(deleteAction, SIGNAL(triggered()),
	        this, SLOT(deleteSingleCase()));
	connect(ui->fullScore, SIGNAL(textChanged(QString)),
	        this, SLOT(fullScoreChanged(QString)));
	connect(ui->timeLimit, SIGNAL(textChanged(QString)),
	        this, SLOT(timeLimitChanged(QString)));
	connect(ui->memoryLimit, SIGNAL(textChanged(QString)),
	        this, SLOT(memoryLimitChanged(QString)));
	connect(ui->subtaskDependenceClearButton, SIGNAL(clicked()),
	        this, SLOT(subtaskDependenceClear()));
	connect(ui->fileList, SIGNAL(itemSelectionChanged()),
	        this, SLOT(fileListSelectionChanged()));
	connect(ui->fileList, SIGNAL(itemChanged(QTableWidgetItem *)),
	        this, SLOT(fileListItemChanged(QTableWidgetItem *)));
}

TestCaseEditWidget::~TestCaseEditWidget()
{
	delete ui;
}

void TestCaseEditWidget::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		TestCase *bak = editTestCase;
		setEditTestCase(nullptr, true);
		ui->retranslateUi(this);
		setEditTestCase(bak, true);
	}
}

void TestCaseEditWidget::refreshFileList()
{
	if (! editTestCase) return;

	ui->fileList->setRowCount(0);
	QStringList inputFiles = editTestCase->getInputFiles();
	QStringList outputFiles = editTestCase->getOutputFiles();
	ui->fileList->setRowCount(inputFiles.size());

	for (int i = 0; i < ui->fileList->rowCount(); i ++)
	{
		QTableWidgetItem *inputFile = new QTableWidgetItem(inputFiles[i]);
		QTableWidgetItem *outputFile = new QTableWidgetItem(outputFiles[i]);
		ui->fileList->setItem(i, 0, inputFile);
		ui->fileList->setItem(i, 1, outputFile);
	}
}

void TestCaseEditWidget::setEditTestCase(TestCase *testCase, bool check)
{
	editTestCase = testCase;

	if (! editTestCase) return;

	ui->fullScore->setText(QString("%1").arg(editTestCase->getFullScore()));
	ui->timeLimit->setText(QString("%1").arg(editTestCase->getTimeLimit()));
	ui->memoryLimit->setText(QString("%1").arg(editTestCase->getMemoryLimit()));
	refreshFileList();
	QStringList temp;
	const QList<int> &dependenceSubtask = editTestCase->getDependenceSubtask();

	for (int i = 0; i != dependenceSubtask.size(); ++i)
		temp.push_back(QString("%1").arg(dependenceSubtask[i]));

	ui->subtaskDependecne->setText(temp.join(','));
	ui->timeLimit->setEnabled(check);
	ui->timeLimitLabel->setEnabled(check);
	ui->msLabel->setEnabled(check);
	ui->memoryLimit->setEnabled(check);
	ui->memoryLimitLabel->setEnabled(check);
	ui->mbLabel->setEnabled(check);
	//ui->outputFileEdit->setDisabled(check2);
}

void TestCaseEditWidget::setSettings(Settings *_settings)
{
	settings = _settings;
	ui->inputFileEdit->setFileExtensions(settings->getInputFileExtensions());
	ui->outputFileEdit->setFileExtensions(settings->getOutputFileExtensions());
}

void TestCaseEditWidget::addSingleCase()
{
	if (ui->inputFileEdit->text().isEmpty())
	{
		ui->inputFileEdit->setFocus();
		QMessageBox::warning(this, tr("Error"), tr("Empty input file name!"), QMessageBox::Close);
		return;
	}

	if (ui->outputFileEdit->text().isEmpty())
	{
		ui->outputFileEdit->setFocus();
		QMessageBox::warning(this, tr("Error"), tr("Empty output file name!"), QMessageBox::Close);
		return;
	}

	ui->fileList->setRowCount(ui->fileList->rowCount() + 1);
	editTestCase->addSingleCase(ui->inputFileEdit->text(), ui->outputFileEdit->text());
	ui->fileList->setItem(ui->fileList->rowCount() - 1, 0,
	                      new QTableWidgetItem(ui->inputFileEdit->text()));
	ui->fileList->setItem(ui->fileList->rowCount() - 1, 1,
	                      new QTableWidgetItem(ui->outputFileEdit->text()));
	ui->inputFileEdit->clear();
	ui->outputFileEdit->clear();
	ui->subtaskDependecne->clear();
}

void TestCaseEditWidget::deleteSingleCase()
{
	QTableWidgetSelectionRange range = ui->fileList->selectedRanges().at(0);

	for (int i = 0; i <= range.rowCount(); i ++)
	{
		editTestCase->deleteSingleCase(range.topRow());
	}

	refreshFileList();
}

void TestCaseEditWidget::fileListSelectionChanged()
{
	if (! editTestCase) return;

	QList<QTableWidgetSelectionRange> range = ui->fileList->selectedRanges();

	if (range.size() == 1 && range[0].columnCount() == 2)
	{
		deleteAction->setEnabled(true);
	}
	else
	{
		deleteAction->setEnabled(false);
	}
}

void TestCaseEditWidget::fileListItemChanged(QTableWidgetItem *item)
{
	if (! editTestCase) return;

	if (item)
	{
		if (item->column() == 0)
		{
			editTestCase->setInputFiles(item->row(), item->text());
		}
		else
		{
			editTestCase->setOutputFiles(item->row(), item->text());
		}
	}
}

void TestCaseEditWidget::subtaskDependenceChanged()
{
	const QString &text = ui->subtaskDependecne->text();

	if (! editTestCase) return;

	QStringList list = text.isEmpty() ? QStringList() : text.split(',');

	if (editTestCase->checkDependenceSubtask(list))
	{
		//QMessageBox::information(this, tr("Information"), tr("Finished!"), QMessageBox::Close);
		editTestCase->setDependenceSubtask(list);
	}
	else
	{
		ui->subtaskDependecne->setFocus();
		//ui->subtaskDependecne->set
		QMessageBox::warning(this, tr("Error"), tr("Dependence subtask index error!"), QMessageBox::Close);
	}
}

void TestCaseEditWidget::subtaskDependenceClear()
{
	ui->subtaskDependecne->clear();

	if (!editTestCase)
		return;

	editTestCase->clearDependenceSubtask();
}

void TestCaseEditWidget::fullScoreChanged(const QString &text)
{
	if (! editTestCase) return;

	editTestCase->setFullScore(text.toInt());
}

void TestCaseEditWidget::timeLimitChanged(const QString &text)
{
	if (! editTestCase) return;

	editTestCase->setTimeLimit(text.toInt());
}

void TestCaseEditWidget::memoryLimitChanged(const QString &text)
{
	if (! editTestCase) return;

	editTestCase->setMemoryLimit(text.toInt());
}
