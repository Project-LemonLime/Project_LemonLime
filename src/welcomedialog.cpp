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
 * welcomedialog.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "welcomedialog.h"
#include "ui_welcomedialog.h"
#include <QPushButton>

WelcomeDialog::WelcomeDialog(QWidget *parent) : QDialog(parent), ui(new Ui::WelcomeDialog)
{
	ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	connect(ui->openContestWidget, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
	connect(ui->newContestWidget, SIGNAL(informationChanged()), this, SLOT(informationChanged()));
	connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabIndexChanged(int)));
	connect(ui->openContestWidget, SIGNAL(rowDoubleClicked()), this, SLOT(accept()));
}

WelcomeDialog::~WelcomeDialog() { delete ui; }

void WelcomeDialog::setRecentContest(const QStringList &list)
{
	ui->openContestWidget->setRecentContest(list);
}

auto WelcomeDialog::getContestTitle() -> QString { return ui->newContestWidget->getContestTitle(); }

auto WelcomeDialog::getSavingName() -> QString { return ui->newContestWidget->getSavingName(); }

auto WelcomeDialog::getContestPath() -> QString { return ui->newContestWidget->getContestPath(); }

auto WelcomeDialog::getRecentContest() const -> const QStringList &
{
	return ui->openContestWidget->getRecentContest();
}

auto WelcomeDialog::getSelectedContest() -> QString
{
	return ui->openContestWidget->getRecentContest().at(ui->openContestWidget->getCurrentRow());
}

auto WelcomeDialog::getCurrentTab() const -> int { return ui->tabWidget->currentIndex(); }

void WelcomeDialog::selectionChanged()
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

void WelcomeDialog::informationChanged()
{
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->newContestWidget->checkReady());
}

void WelcomeDialog::tabIndexChanged(int index)
{
	if (index == 0)
	{
		selectionChanged();
	}
	else
	{
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->newContestWidget->checkReady());
	}
}
