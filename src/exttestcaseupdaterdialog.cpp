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

#include "exttestcaseupdaterdialog.h"
#include "settings.h"
#include "task.h"
#include "ui_exttestcaseupdaterdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QValidator>

ExtTestCaseUpdaterDialog::ExtTestCaseUpdaterDialog(QWidget *parent, Task *nowTask,
                                                   const Settings *nowSettings, int nowCaseNumber,
                                                   int editScore, int editData, int editTime, int editMemory,
                                                   int editDepend, QList<int> tempDepends)
    : QDialog(parent), ui(new Ui::ExtTestCaseUpdaterDialog), nowTask(nowTask), nowSettings(nowSettings),
      nowCaseNumber(nowCaseNumber), editScore(editScore), editData(editData), editTime(editTime),
      editMemory(editMemory), editDepend(editDepend)
{
	ui->setupUi(this);

	if (! nowTask)
		return;

	if (nowCaseNumber > 0)
		setWindowTitle(QString(tr("Configure Test Case #%1")).arg(nowCaseNumber));
	else
		setWindowTitle(QString(tr("Configure")));

	ui->lineEditScore->setValidator(new QIntValidator(0, nowSettings->upperBoundForFullScore(), this));
	ui->lineEditTime->setValidator(new QIntValidator(1, nowSettings->upperBoundForTimeLimit(), this));
	ui->lineEditMemory->setValidator(new QIntValidator(1, nowSettings->upperBoundForMemoryLimit(), this));
	ui->lineEditInput->setFilters(QDir::Files);
	ui->lineEditOutput->setFilters(QDir::Files);
	QRegExp regx("[0-9,]+$");
	QValidator *subtaskDependenceValidator = new QRegExpValidator(regx, ui->lineEditDepends);
	ui->lineEditDepends->setValidator(subtaskDependenceValidator);

	connect(ui->lineEditScore, SIGNAL(textChanged(QString)), this, SLOT(fullScoreChanged(QString)));
	connect(ui->lineEditTime, SIGNAL(textChanged(QString)), this, SLOT(timeLimitChanged(QString)));
	connect(ui->lineEditMemory, SIGNAL(textChanged(QString)), this, SLOT(memoryLimitChanged(QString)));
	connect(ui->buttonFindInput, SIGNAL(clicked()), this, SLOT(whenButtonFindInputClicked()));
	connect(ui->buttonFindOutput, SIGNAL(clicked()), this, SLOT(whenButtonFindOutputClicked()));
	connect(ui->lineEditInput, SIGNAL(textChanged(QString)), this, SLOT(inputFileChanged(QString)));
	connect(ui->lineEditOutput, SIGNAL(textChanged(QString)), this, SLOT(outputFileChanged(QString)));
	connect(ui->lineEditDepends, SIGNAL(textChanged(QString)), this, SLOT(dependsChanged(QString)));

	if (editScore == NO_EDIT)
		ui->labelScore->hide(), ui->lineEditScore->hide(), defScore = NO_EDIT;
	else if (editScore == MAY_EDIT)
		score = defScore = MAY_EDIT;
	else if (editScore == EDIT_WITH_DEFAULT)
		score = defScore = nowSettings->getDefaultFullScore(),
		ui->lineEditScore->setText(QString::number(defScore));
	else
		score = defScore = editScore, ui->lineEditScore->setText(QString::number(defScore));

	if (editData == NO_EDIT)
	{
		ui->labelInput->hide(), ui->labelOutput->hide();
		ui->lineEditInput->hide(), ui->lineEditOutput->hide();
		ui->buttonFindInput->hide(), ui->buttonFindOutput->hide();
	}

	if (nowTask->getTaskType() == Task::AnswersOnly)
		editTime = editMemory = NO_EDIT;

	if (editTime == NO_EDIT)
	{
		ui->labelTimeLimit->hide(), ui->labelMemoryLimit->hide();
		ui->lineEditTime->hide(), ui->lineEditMemory->hide();
		ui->label->hide(), ui->label_2->hide();
	}
	else if (editTime == MAY_EDIT)
	{
		timeLimit = defTimeLimit = MAY_EDIT;
		memoryLimit = defMemoryLimit = MAY_EDIT;
	}
	else if (editTime == EDIT_WITH_DEFAULT)
	{
		timeLimit = defTimeLimit = nowSettings->getDefaultTimeLimit();
		memoryLimit = defMemoryLimit = nowSettings->getDefaultMemoryLimit();
		ui->lineEditTime->setText(QString::number(defTimeLimit));
		ui->lineEditMemory->setText(QString::number(defMemoryLimit));
	}
	else
	{
		timeLimit = defTimeLimit = editTime;
		memoryLimit = defMemoryLimit = editMemory;
		ui->lineEditTime->setText(QString::number(defTimeLimit));
		ui->lineEditMemory->setText(QString::number(defMemoryLimit));
	}

	if (editDepend == NO_EDIT)
		ui->labelDepend->hide(), ui->lineEditDepends->hide();

	QString tempStr = "";

	for (auto i : tempDepends)
		tempStr = tempStr + QString::number(i) + ",";

	if (! tempStr.isEmpty())
		tempStr.resize(tempStr.size() - 1);

	ui->lineEditDepends->setText(tempStr);
}

ExtTestCaseUpdaterDialog::~ExtTestCaseUpdaterDialog() { delete ui; }

int ExtTestCaseUpdaterDialog::getScore() { return score; }

QString ExtTestCaseUpdaterDialog::getInput() { return input; }

QString ExtTestCaseUpdaterDialog::getOutput() { return output; }

int ExtTestCaseUpdaterDialog::getTimeLimit() { return timeLimit; }

int ExtTestCaseUpdaterDialog::getMemoryLimit() { return memoryLimit; }

QStringList ExtTestCaseUpdaterDialog::getDepends() { return depends; }

void ExtTestCaseUpdaterDialog::whenButtonFindInputClicked()
{
	QString filter = tr("Input Data") + " (";

	auto temp = nowSettings->getInputFileExtensions();

	for (const auto &i : temp)
		filter = filter + "*." + i + " ";

	filter.resize(filter.size() - 1);

	filter = filter + ");;" + tr("All Files (*)");

	QString curPath = QDir::currentPath() + nowSettings->dataPath();
	QString filePath = QFileDialog::getOpenFileName(
	   this, tr("Choose Input File"), nowSettings->dataPath() + nowTask->getSourceFileName(), filter);

	if (! filePath.isEmpty())
	{
		QString realPath = filePath.mid(curPath.length() + 1);
		ui->lineEditInput->setText(realPath);
	}
}

void ExtTestCaseUpdaterDialog::whenButtonFindOutputClicked()
{
	QString filter = tr("Output Data") + " (";

	auto temp = nowSettings->getOutputFileExtensions();

	for (const auto &i : temp)
		filter = filter + "*." + i + " ";

	filter.resize(filter.size() - 1);

	filter = filter + ");;" + tr("All Files (*)");

	QString curPath = QDir::currentPath() + nowSettings->dataPath();
	QString filePath = QFileDialog::getOpenFileName(
	   this, tr("Choose Output File"), nowSettings->dataPath() + nowTask->getSourceFileName(), filter);

	if (! filePath.isEmpty())
	{
		QString realPath = filePath.mid(curPath.length() + 1);
		ui->lineEditOutput->setText(realPath);
	}
}

void ExtTestCaseUpdaterDialog::accept()
{
	if (editData != NO_EDIT && input.isEmpty())
	{
		QMessageBox::warning(this, tr("Error"), tr("Input File is Empty!"), QMessageBox::Close);
		return;
	}

	if (editData != NO_EDIT && output.isEmpty())
	{
		QMessageBox::warning(this, tr("Error"), tr("Output File is Empty!"), QMessageBox::Close);
		return;
	}

	if (editDepend != NO_EDIT && checkDepends() != 0)
	{
		QMessageBox::warning(this, tr("Error"), tr("Dependence subtask index error!"), QMessageBox::Close);
		return;
	}

	return QDialog::accept();
}

void ExtTestCaseUpdaterDialog::fullScoreChanged(const QString &text)
{
	if (text.isEmpty())
	{
		if (editScore == MAY_EDIT)
			score = MAY_EDIT;
		else
			ui->lineEditScore->setText(QString::number(defScore));
	}
	else
		score = text.toInt();
}

void ExtTestCaseUpdaterDialog::timeLimitChanged(const QString &text)
{
	if (text.isEmpty())
	{
		if (editTime == MAY_EDIT)
			timeLimit = MAY_EDIT;
		else
			ui->lineEditTime->setText(QString::number(defTimeLimit));
	}
	else
		timeLimit = text.toInt();
}

void ExtTestCaseUpdaterDialog::memoryLimitChanged(const QString &text)
{
	if (text.isEmpty())
	{
		if (editMemory == MAY_EDIT)
			memoryLimit = MAY_EDIT;
		else
			ui->lineEditMemory->setText(QString::number(defMemoryLimit));
	}
	else
		memoryLimit = text.toInt();
}

void ExtTestCaseUpdaterDialog::inputFileChanged(const QString &text) { input = text; }

void ExtTestCaseUpdaterDialog::outputFileChanged(const QString &text) { output = text; }

void ExtTestCaseUpdaterDialog::dependsChanged(const QString &text)
{
	depends = text.isEmpty() ? QStringList() : text.split(',');
}

int ExtTestCaseUpdaterDialog::checkDepends()
{
	QSet<int> hav;

	for (const auto &i_ : qAsConst(depends))
	{
		int i = i_.toInt();

		if (i <= 0 || i >= nowCaseNumber || hav.contains(i))
			return -1;

		hav.insert(i);
	}

	return 0;
}
