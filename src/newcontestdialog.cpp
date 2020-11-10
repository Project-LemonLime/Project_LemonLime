/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "newcontestdialog.h"
#include "ui_newcontestdialog.h"
//
#include <QPushButton>

NewContestDialog::NewContestDialog(QWidget *parent) : QDialog(parent), ui(new Ui::NewContestDialog) {
	ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	connect(ui->newContestWidget, &NewContestWidget::informationChanged, this,
	        &NewContestDialog::informationChanged);
}

NewContestDialog::~NewContestDialog() { delete ui; }

auto NewContestDialog::getContestTitle() -> QString { return ui->newContestWidget->getContestTitle(); }

auto NewContestDialog::getSavingName() -> QString { return ui->newContestWidget->getSavingName(); }

auto NewContestDialog::getContestPath() -> QString { return ui->newContestWidget->getContestPath(); }

void NewContestDialog::informationChanged() {
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->newContestWidget->checkReady());
}
