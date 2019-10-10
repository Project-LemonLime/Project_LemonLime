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
 * editvariabledialog.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "editvariabledialog.h"
#include "ui_editvariabledialog.h"
#include <QPushButton>

EditVariableDialog::EditVariableDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EditVariableDialog)
{
	ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	connect(ui->variableName, SIGNAL(textChanged(QString)),
	        this, SLOT(textChanged()));
	connect(ui->variableValue, SIGNAL(textChanged(QString)),
	        this, SLOT(textChanged()));
}

EditVariableDialog::~EditVariableDialog()
{
	delete ui;
}

void EditVariableDialog::setVariableName(const QString &variable)
{
	ui->variableName->setText(variable);
}

void EditVariableDialog::setVariableValue(const QString &value)
{
	ui->variableValue->setText(value);
}

QString EditVariableDialog::getVariableName() const
{
	return ui->variableName->text();
}

QString EditVariableDialog::getVariableValue() const
{
	return ui->variableValue->text();
}

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
