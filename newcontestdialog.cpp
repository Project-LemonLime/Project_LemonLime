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
 * newcontestdialog.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "newcontestdialog.h"
#include "ui_newcontestdialog.h"
#include <QPushButton>

NewContestDialog::NewContestDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::NewContestDialog)
{
	ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	connect(ui->newContestWidget, SIGNAL(informationChanged()),
	        this, SLOT(informationChanged()));
}

NewContestDialog::~NewContestDialog()
{
	delete ui;
}

QString NewContestDialog::getContestTitle()
{
	return ui->newContestWidget->getContestTitle();
}

QString NewContestDialog::getSavingName()
{
	return ui->newContestWidget->getSavingName();
}

QString NewContestDialog::getContestPath()
{
	return ui->newContestWidget->getContestPath();
}

void NewContestDialog::informationChanged()
{
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->newContestWidget->checkReady());
}
