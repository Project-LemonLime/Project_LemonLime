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
 * optionsdialog.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "settings.h"
//#include <QtGui>
#include <QtWidgets>

OptionsDialog::OptionsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::OptionsDialog)
{
	ui->setupUi(this);
	editSettings = new Settings(this);
	connect(ui->buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
	        this, SLOT(okayButtonClicked()));
}

OptionsDialog::~OptionsDialog()
{
	delete ui;
}

Settings *OptionsDialog::getEditSettings()
{
	return editSettings;
}

void OptionsDialog::resetEditSettings(Settings *settings)
{
	editSettings->copyFrom(settings);
	ui->generalSettings->resetEditSettings(editSettings);
	ui->compilerSettings->resetEditSettings(editSettings);
	ui->tabWidget->setCurrentIndex(0);
}

void OptionsDialog::okayButtonClicked()
{
	ui->tabWidget->setCurrentIndex(0);

	if (! ui->generalSettings->checkValid()) return;

	ui->tabWidget->setCurrentIndex(1);

	if (! ui->compilerSettings->checkValid()) return;

	accept();
}
