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
 * opencontestdialog.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "opencontestdialog.h"
#include "ui_opencontestdialog.h"
#include <QPushButton>

OpenContestDialog::OpenContestDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::OpenContestDialog)
{
	ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	connect(ui->openContestWidget, SIGNAL(selectionChanged()),
	        this, SLOT(selectionChanged()));
	connect(ui->openContestWidget, SIGNAL(rowDoubleClicked()),
	        this, SLOT(accept()));
}

OpenContestDialog::~OpenContestDialog()
{
	delete ui;
}

void OpenContestDialog::setRecentContest(const QStringList &list)
{
	ui->openContestWidget->setRecentContest(list);
}

void OpenContestDialog::selectionChanged()
{
	if (ui->openContestWidget->getCurrentRow() != -1)
	{
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	}
	else
	{
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
}

auto OpenContestDialog::getRecentContest() const -> const QStringList &
{
	return ui->openContestWidget->getRecentContest();
}

auto OpenContestDialog::getSelectedContest() -> QString
{
	return ui->openContestWidget->getRecentContest().at(ui->openContestWidget->getCurrentRow());
}
