/*
 * SPDX-FileCopyrightText: 2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "themeeditdialog.h"
#include "settings.h"
#include "ui_themeeditdialog.h"
#include "visualsettings.h"
#include <QtWidgets>

ThemeEditDialog::ThemeEditDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ThemeEditDialog) {
	ui->setupUi(this);
	editTheme = nullptr;
	connect(ui->buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this,
	        &ThemeEditDialog::okayButtonClicked);
}

ThemeEditDialog::~ThemeEditDialog() { delete ui; }

void ThemeEditDialog::resetEditTheme(ColorTheme *colorTheme) {
	editTheme = colorTheme;
	ui->ThemeEdit->resetColorTheme(editTheme);
}

ColorTheme *ThemeEditDialog::getEditTheme() { return editTheme; }

void ThemeEditDialog::okayButtonClicked() {
	ui->ThemeEdit->refresh();
	accept();
}
