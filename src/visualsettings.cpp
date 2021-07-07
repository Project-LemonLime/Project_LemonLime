/*
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "visualsettings.h"
#include "ui_visualsettings.h"
//
#include "base/LemonLog.hpp"
#include "base/settings.h"
#define LEMON_MODULE_NAME "VisualSettings"

VisualSettings::VisualSettings(QWidget *parent) : QWidget(parent), ui(new Ui::VisualSettings) {
	ui->setupUi(this);

	editColorTheme = nullptr;
	connect(ui->themeName, &QLineEdit::textChanged, this, &VisualSettings::themeNameChanged);
}

void VisualSettings::resetColorTheme(ColorTheme *colorTheme) {
	editColorTheme = colorTheme;

	ui->themeName->setText(editColorTheme->getName());
	ui->spinBoxMxH->setValue(editColorTheme->getMxColor().h);
	ui->doubleSpinBoxMxS->setValue(editColorTheme->getMxColor().s);
	ui->doubleSpinBoxMxL->setValue(editColorTheme->getMxColor().l);
	ui->spinBoxMiH->setValue(editColorTheme->getMiColor().h);
	ui->doubleSpinBoxMiS->setValue(editColorTheme->getMiColor().s);
	ui->doubleSpinBoxMiL->setValue(editColorTheme->getMiColor().l);
	ui->spinBoxNfH->setValue(editColorTheme->getNfColor().h);
	ui->doubleSpinBoxNfS->setValue(editColorTheme->getNfColor().s);
	ui->doubleSpinBoxNfL->setValue(editColorTheme->getNfColor().l);
	ui->spinBoxCeH->setValue(editColorTheme->getCeColor().h);
	ui->doubleSpinBoxCeS->setValue(editColorTheme->getCeColor().s);
	ui->doubleSpinBoxCeL->setValue(editColorTheme->getCeColor().l);

	ui->grandCompH->setValue(editColorTheme->getGrandComp().h);
	ui->grandCompS->setValue(editColorTheme->getGrandComp().s);
	ui->grandCompL->setValue(editColorTheme->getGrandComp().l);
	ui->grandRateH->setValue(editColorTheme->getGrandRate().h);
	ui->grandRateS->setValue(editColorTheme->getGrandRate().s);
	ui->grandRateL->setValue(editColorTheme->getGrandRate().l);
}

void VisualSettings::themeNameChanged(const QString &x) { editColorTheme->setName(x); }

void VisualSettings::refresh() {
	editColorTheme->setColor(
	    hslTuple(ui->spinBoxMxH->value(), ui->doubleSpinBoxMxS->value(), ui->doubleSpinBoxMxL->value()),
	    hslTuple(ui->spinBoxMiH->value(), ui->doubleSpinBoxMiS->value(), ui->doubleSpinBoxMiL->value()),
	    hslTuple(ui->spinBoxNfH->value(), ui->doubleSpinBoxNfS->value(), ui->doubleSpinBoxNfL->value()),
	    hslTuple(ui->spinBoxCeH->value(), ui->doubleSpinBoxCeS->value(), ui->doubleSpinBoxCeL->value()),
	    dddTuple(ui->grandCompH->value(), ui->grandCompS->value(), ui->grandCompL->value()),
	    dddTuple(ui->grandRateH->value(), ui->grandRateS->value(), ui->grandRateL->value()));
}

VisualSettings::~VisualSettings() { delete ui; }
