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

#include "exttestcasemodifierdialog.h"
#include "task.h"
#include "ui_exttestcasemodifierdialog.h"
#include <QtWidgets>

ExtTestCaseModifierDialog::ExtTestCaseModifierDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ExtTestCaseModifierDialog)
{
	ui->setupUi(this);
	setWindowTitle(QString(tr("Advanced Test Case Modifier")));
	connect(ui->buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
}

ExtTestCaseModifierDialog::~ExtTestCaseModifierDialog() { delete ui; }

void ExtTestCaseModifierDialog::init(Task *theTask, const Settings *theSettings)
{
	editTask = new Task;

	theTask->copyTo(editTask);

	ui->widget->init(editTask, theSettings);
}

Task *ExtTestCaseModifierDialog::getEditTask() { return editTask; }
