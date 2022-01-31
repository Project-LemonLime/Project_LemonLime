/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "optionsdialog.h"
#include "ui_optionsdialog.h"
//
#include "base/LemonLog.hpp"
#include "base/settings.h"
//
#include <QtWidgets>
#define LEMON_MODULE_NAME "OptionsDialog"

OptionsDialog::OptionsDialog(QWidget *parent) : QDialog(parent), ui(new Ui::OptionsDialog) {
	ui->setupUi(this);
	editSettings = new Settings();
	connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this,
	        &OptionsDialog::okayButtonClicked);
}

OptionsDialog::~OptionsDialog() { delete ui; }

auto OptionsDialog::getEditSettings() -> Settings * { return editSettings; }

void OptionsDialog::resetEditSettings(Settings *settings) {
	editSettings->copyFrom(settings);
	ui->generalSettings->resetEditSettings(editSettings);
	ui->compilerSettings->resetEditSettings(editSettings);
	ui->visualMainSettings->resetEditSettings(editSettings);
	ui->tabWidget->setCurrentIndex(0);
}

void OptionsDialog::okayButtonClicked() {
	ui->tabWidget->setCurrentIndex(0);

	if (! ui->generalSettings->checkValid())
		return;

	ui->tabWidget->setCurrentIndex(1);

	if (! ui->compilerSettings->checkValid())
		return;

	accept();
}
