/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "opencontestdialog.h"
#include "ui_opencontestdialog.h"
//
#include <QPushButton>

OpenContestDialog::OpenContestDialog(QWidget *parent) : QDialog(parent), ui(new Ui::OpenContestDialog) {
	ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	connect(ui->openContestWidget, &OpenContestWidget::selectionChanged, this,
	        &OpenContestDialog::selectionChanged);
	connect(ui->openContestWidget, &OpenContestWidget::rowDoubleClicked, this, &OpenContestDialog::accept);
}

OpenContestDialog::~OpenContestDialog() { delete ui; }

void OpenContestDialog::setRecentContest(const QStringList &list) {
	ui->openContestWidget->setRecentContest(list);
}

void OpenContestDialog::selectionChanged() {
	if (ui->openContestWidget->getCurrentRow() != -1) {
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	} else {
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
}

auto OpenContestDialog::getRecentContest() const -> const QStringList & {
	return ui->openContestWidget->getRecentContest();
}

auto OpenContestDialog::getSelectedContest() -> QString {
	return ui->openContestWidget->getRecentContest().at(ui->openContestWidget->getCurrentRow());
}
