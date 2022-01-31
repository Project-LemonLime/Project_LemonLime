/*
 * SPDX-FileCopyrightText: 2021-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "visualmainsettings.h"
#include "themeeditdialog.h"
#include "ui_visualmainsettings.h"
//
#include "base/LemonLog.hpp"
#include "base/settings.h"
//
#include <QMessageBox>
//
#define LEMON_MODULE_NAME "VisualMainSettings"

VisualMainSettings::VisualMainSettings(QWidget *parent) : QWidget(parent), ui(new Ui::VisualMainSettings) {
	ui->setupUi(this);

	ui->splashTimeEdit->setValidator(new QIntValidator(0, 3000, this));

	connect(ui->splashTimeEdit, &QLineEdit::textChanged, this, &VisualMainSettings::splashTimeChanged);
	connect(ui->editThemeButton, &QPushButton::clicked, this, &VisualMainSettings::whenThemeEdit);
	connect(ui->themeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
	        &VisualMainSettings::themeChanged);
	connect(ui->addThemeButton, &QToolButton::clicked, this, &VisualMainSettings::whenThemeAdd);
	connect(ui->deleteThemeButton, &QToolButton::clicked, this, &VisualMainSettings::whenThemeDelete);
}

void VisualMainSettings::resetEditSettings(Settings *settings) {
	editSettings = settings;
	int index = editSettings->getCurrentColorThemeIndex();

	ui->themeComboBox->clear();

	auto themeList = editSettings->getColorThemeList();

	for (auto i : themeList) {
		ui->themeComboBox->addItem(i->getName());
	}

	ui->themeComboBox->setCurrentIndex(index);

	ui->splashTimeEdit->setText(QString("%1").arg(editSettings->getSplashTime()));
}

void VisualMainSettings::splashTimeChanged(const QString &x) { editSettings->setSplashTime(x.toInt()); }

void VisualMainSettings::themeChanged(const int x) { editSettings->setCurrentColorThemeIndex(x); }

void VisualMainSettings::whenThemeEdit() {
	int index = ui->themeComboBox->currentIndex();
	auto *dialog = new ThemeEditDialog(this);

	ColorTheme *temp = new ColorTheme;
	temp->copyFrom(editSettings->getColorTheme(index));

	dialog->resetEditTheme(temp);

	if (dialog->exec() == QDialog::Accepted) {
		editSettings->setColorTheme(dialog->getEditTheme(), index);
	}

	delete dialog;

	resetEditSettings(editSettings);
	ui->themeComboBox->setCurrentIndex(index);
}

void VisualMainSettings::whenThemeAdd() {
	ColorTheme *temp = new ColorTheme();
	temp->setName(tr("New Theme"));
	editSettings->addColorTheme(temp);
	resetEditSettings(editSettings);
	ui->themeComboBox->setCurrentIndex(editSettings->getColorThemeList().size() - 1);
}

void VisualMainSettings::whenThemeDelete() {
	if (editSettings->getColorThemeList().size() <= 1)
		return;
	int index = ui->themeComboBox->currentIndex();
	editSettings->deleteColorTheme(index);
	if (index >= editSettings->getColorThemeList().size())
		index--;
	ui->themeComboBox->setCurrentIndex(index);
	resetEditSettings(editSettings);
}

VisualMainSettings::~VisualMainSettings() { delete ui; }
