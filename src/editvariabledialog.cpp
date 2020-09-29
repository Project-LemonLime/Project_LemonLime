/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "editvariabledialog.h"
#include "ui_editvariabledialog.h"
#include <QPushButton>

EditVariableDialog::EditVariableDialog(QWidget *parent) : QDialog(parent), ui(new Ui::EditVariableDialog)
{
	ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	connect(ui->variableName, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
	connect(ui->variableValue, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
}

EditVariableDialog::~EditVariableDialog() { delete ui; }

void EditVariableDialog::setVariableName(const QString &variable) { ui->variableName->setText(variable); }

void EditVariableDialog::setVariableValue(const QString &value) { ui->variableValue->setText(value); }

auto EditVariableDialog::getVariableName() const -> QString { return ui->variableName->text(); }

auto EditVariableDialog::getVariableValue() const -> QString { return ui->variableValue->text(); }

void EditVariableDialog::textChanged()
{
	if (! ui->variableName->text().isEmpty() && ! ui->variableValue->text().isEmpty())
	{
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	}
	else
	{
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
}
