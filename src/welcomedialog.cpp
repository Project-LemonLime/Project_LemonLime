/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "welcomedialog.h"
#include "ui_welcomedialog.h"
//
#include <QPushButton>

WelcomeDialog::WelcomeDialog(QWidget *parent) : QDialog(parent), ui(new Ui::WelcomeDialog) {
	ui->setupUi(this);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	connect(ui->openContestWidget, &OpenContestWidget::selectionChanged, this,
	        &WelcomeDialog::selectionChanged);
	connect(ui->newContestWidget, &NewContestWidget::informationChanged, this,
	        &WelcomeDialog::informationChanged);
	connect(ui->tabWidget, &QTabWidget::currentChanged, this, &WelcomeDialog::tabIndexChanged);
	connect(ui->openContestWidget, &OpenContestWidget::rowDoubleClicked, this, &WelcomeDialog::accept);
}

WelcomeDialog::~WelcomeDialog() { delete ui; }

void WelcomeDialog::setRecentContest(const QStringList &list) {
	ui->openContestWidget->setRecentContest(list);
}

auto WelcomeDialog::getContestTitle() -> QString { return ui->newContestWidget->getContestTitle(); }

auto WelcomeDialog::getSavingName() -> QString { return ui->newContestWidget->getSavingName(); }

auto WelcomeDialog::getContestPath() -> QString { return ui->newContestWidget->getContestPath(); }

auto WelcomeDialog::getRecentContest() const -> const QStringList & {
	return ui->openContestWidget->getRecentContest();
}

auto WelcomeDialog::getSelectedContest() -> QString {
	return ui->openContestWidget->getRecentContest().at(ui->openContestWidget->getCurrentRow());
}

auto WelcomeDialog::getCurrentTab() const -> int { return ui->tabWidget->currentIndex(); }

void WelcomeDialog::selectionChanged() {
	if (ui->openContestWidget->getCurrentRow() != -1) {
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
	} else {
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
}

void WelcomeDialog::informationChanged() {
	ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->newContestWidget->checkReady());
}

void WelcomeDialog::tabIndexChanged(int index) {
	if (index == 0) {
		selectionChanged();
	} else {
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->newContestWidget->checkReady());
	}
}
