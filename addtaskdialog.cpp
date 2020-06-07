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
 * addtaskdialog.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * addcompilerwizard.cpp @Project LemonLime
 * Update 2019 iotang
 **/

#include "addtaskdialog.h"
#include "settings.h"
#include "ui_addtaskdialog.h"

AddTaskDialog::AddTaskDialog(QWidget *parent) : QDialog(parent), ui(new Ui::AddTaskDialog)
{
	ui->setupUi(this);
	ui->fullScore->setValidator(new QIntValidator(1, Settings::upperBoundForFullScore() * 100, this));
	ui->timeLimit->setValidator(new QIntValidator(1, Settings::upperBoundForTimeLimit(), this));
	ui->memoryLimit->setValidator(new QIntValidator(1, Settings::upperBoundForMemoryLimit(), this));
	connect(ui->taskBox, SIGNAL(currentIndexChanged(int)), this, SLOT(taskBoxIndexChanged()));
	connect(ui->fullScore, SIGNAL(textChanged(QString)), this, SLOT(fullScoreChanged()));
	connect(ui->timeLimit, SIGNAL(textChanged(QString)), this, SLOT(timeLimitChanged()));
	connect(ui->memoryLimit, SIGNAL(textChanged(QString)), this, SLOT(memoryLimitChanged()));
}

AddTaskDialog::~AddTaskDialog() { delete ui; }

void AddTaskDialog::addTask(const QString &title, int _fullScore, int _timeLimit, int _memoryLimit)
{
	fullScore.append(_fullScore);
	timeLimit.append(_timeLimit);
	memoryLimit.append(_memoryLimit);
	ui->taskBox->addItem(title);
	ui->taskBox->setCurrentIndex(0);
}

auto AddTaskDialog::getFullScore(int index) const -> int
{
	if (0 <= index && index < fullScore.size())
	{
		return fullScore[index];
	}

	return 0;
}

auto AddTaskDialog::getTimeLimit(int index) const -> int
{
	if (0 <= index && index < timeLimit.size())
	{
		return timeLimit[index];
	}

	return 0;
}

auto AddTaskDialog::getMemoryLimit(int index) const -> int
{
	if (0 <= index && index < memoryLimit.size())
	{
		return memoryLimit[index];
	}

	return 0;
}

void AddTaskDialog::taskBoxIndexChanged()
{
	int index = ui->taskBox->currentIndex();
	ui->fullScore->setText(QString("%1").arg(fullScore[index]));
	ui->timeLimit->setText(QString("%1").arg(timeLimit[index]));
	ui->memoryLimit->setText(QString("%1").arg(memoryLimit[index]));
}

void AddTaskDialog::fullScoreChanged()
{
	int index = ui->taskBox->currentIndex();
	fullScore[index] = ui->fullScore->text().toInt();
}

void AddTaskDialog::timeLimitChanged()
{
	int index = ui->taskBox->currentIndex();
	timeLimit[index] = ui->timeLimit->text().toInt();
}

void AddTaskDialog::memoryLimitChanged()
{
	int index = ui->taskBox->currentIndex();
	memoryLimit[index] = ui->memoryLimit->text().toInt();
}
