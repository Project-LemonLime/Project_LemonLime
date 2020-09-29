/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

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

auto ExtTestCaseModifierDialog::getEditTask() -> Task * { return editTask; }
