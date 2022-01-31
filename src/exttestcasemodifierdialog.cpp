/*
 * SPDX-FileCopyrightText: 2020-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "exttestcasemodifierdialog.h"
#include "ui_exttestcasemodifierdialog.h"
//
#include "core/task.h"
//
#include <QtWidgets>

ExtTestCaseModifierDialog::ExtTestCaseModifierDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ExtTestCaseModifierDialog) {
	ui->setupUi(this);
	setWindowTitle(QString(tr("Advanced Test Case Modifier")));
	connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this,
	        &ExtTestCaseModifierDialog::accept);
	connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this,
	        &ExtTestCaseModifierDialog::reject);
}

ExtTestCaseModifierDialog::~ExtTestCaseModifierDialog() { delete ui; }

void ExtTestCaseModifierDialog::init(Task *theTask, const Settings *theSettings) {
	editTask = new Task;

	theTask->copyTo(editTask);

	ui->widget->init(editTask, theSettings);
}

auto ExtTestCaseModifierDialog::getEditTask() -> Task * { return editTask; }
