/*
 * SPDX-FileCopyrightText: 2019  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "visualsettings.h"
#include "ui_visualsettings.h"
//
#include "base/settings.h"

VisualSettings::VisualSettings(QWidget *parent) : QWidget(parent), ui(new Ui::VisualSettings) {
	ui->setupUi(this);
	connect(ui->spinBoxMxH, qOverload<int>(&QSpinBox::valueChanged), this, &VisualSettings::colorMxHChanged);
	connect(ui->doubleSpinBoxMxS, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::colorMxSChanged);
	connect(ui->doubleSpinBoxMxL, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::colorMxLChanged);
	connect(ui->spinBoxMiH, qOverload<int>(&QSpinBox::valueChanged), this, &VisualSettings::colorMiHChanged);
	connect(ui->doubleSpinBoxMiS, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::colorMiSChanged);
	connect(ui->doubleSpinBoxMiL, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::colorMiLChanged);
	connect(ui->spinBoxNfH, qOverload<int>(&QSpinBox::valueChanged), this, &VisualSettings::colorNfHChanged);
	connect(ui->doubleSpinBoxNfS, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::colorNfSChanged);
	connect(ui->doubleSpinBoxNfL, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::colorNfLChanged);
	connect(ui->spinBoxCeH, qOverload<int>(&QSpinBox::valueChanged), this, &VisualSettings::colorCeHChanged);
	connect(ui->doubleSpinBoxCeS, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::colorCeSChanged);
	connect(ui->doubleSpinBoxCeL, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::colorCeLChanged);
	connect(ui->grandCompH, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::grandCompHChanged);
	connect(ui->grandCompS, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::grandCompSChanged);
	connect(ui->grandCompL, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::grandCompLChanged);
	connect(ui->grandRateH, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::grandRateHChanged);
	connect(ui->grandRateS, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::grandRateSChanged);
	connect(ui->grandRateL, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
	        &VisualSettings::grandRateLChanged);
	connect(ui->splashTime, qOverload<int>(&QSpinBox::valueChanged), this,
	        &VisualSettings::splashTimeChanged);
	connect(ui->resetToDefaultButton, &QPushButton::clicked, this, &VisualSettings::whenResetToDefault);
}

void VisualSettings::resetToDefault() {
	ui->spinBoxMxH->setValue(120);
	ui->doubleSpinBoxMxS->setValue(50);
	ui->doubleSpinBoxMxL->setValue(57.5);
	ui->spinBoxMiH->setValue(120);
	ui->doubleSpinBoxMiS->setValue(50);
	ui->doubleSpinBoxMiL->setValue(100);
	ui->spinBoxNfH->setValue(0);
	ui->doubleSpinBoxNfS->setValue(0);
	ui->doubleSpinBoxNfL->setValue(91.67);
	ui->spinBoxCeH->setValue(300);
	ui->doubleSpinBoxCeS->setValue(100);
	ui->doubleSpinBoxCeL->setValue(83.33);
	ui->grandCompH->setValue(0);
	ui->grandCompS->setValue(0);
	ui->grandCompL->setValue(0);
	ui->grandRateH->setValue(1);
	ui->grandRateS->setValue(1);
	ui->grandRateL->setValue(1.33);
	ui->splashTime->setValue(500);
	editSettings->setColorMxH(ui->spinBoxMxH->value());
	editSettings->setColorMxS(ui->doubleSpinBoxMxS->value());
	editSettings->setColorMxL(ui->doubleSpinBoxMxL->value());
	editSettings->setColorMiH(ui->spinBoxMiH->value());
	editSettings->setColorMiS(ui->doubleSpinBoxMiS->value());
	editSettings->setColorMiL(ui->doubleSpinBoxMiL->value());
	editSettings->setColorNfH(ui->spinBoxNfH->value());
	editSettings->setColorNfS(ui->doubleSpinBoxNfS->value());
	editSettings->setColorNfL(ui->doubleSpinBoxNfL->value());
	editSettings->setColorCeH(ui->spinBoxCeH->value());
	editSettings->setColorCeS(ui->doubleSpinBoxCeS->value());
	editSettings->setColorCeL(ui->doubleSpinBoxCeL->value());
	editSettings->setGrandCompH(ui->grandCompH->value());
	editSettings->setGrandCompS(ui->grandCompS->value());
	editSettings->setGrandCompL(ui->grandCompL->value());
	editSettings->setGrandRateH(ui->grandRateH->value());
	editSettings->setGrandRateS(ui->grandRateS->value());
	editSettings->setGrandRateL(ui->grandRateL->value());
	editSettings->setSplashTime(ui->splashTime->value());
}

void VisualSettings::whenResetToDefault() { VisualSettings::resetToDefault(); }

void VisualSettings::resetEditSettings(Settings *settings) {
	editSettings = settings;
	ui->spinBoxMxH->setValue(editSettings->getColorMxH());
	ui->doubleSpinBoxMxS->setValue(editSettings->getColorMxS());
	ui->doubleSpinBoxMxL->setValue(editSettings->getColorMxL());
	ui->spinBoxMiH->setValue(editSettings->getColorMiH());
	ui->doubleSpinBoxMiS->setValue(editSettings->getColorMiS());
	ui->doubleSpinBoxMiL->setValue(editSettings->getColorMiL());
	ui->spinBoxNfH->setValue(editSettings->getColorNfH());
	ui->doubleSpinBoxNfS->setValue(editSettings->getColorNfS());
	ui->doubleSpinBoxNfL->setValue(editSettings->getColorNfL());
	ui->spinBoxCeH->setValue(editSettings->getColorCeH());
	ui->doubleSpinBoxCeS->setValue(editSettings->getColorCeS());
	ui->doubleSpinBoxCeL->setValue(editSettings->getColorCeL());
	ui->grandCompH->setValue(editSettings->getGrandCompH());
	ui->grandCompS->setValue(editSettings->getGrandCompS());
	ui->grandCompL->setValue(editSettings->getGrandCompL());
	ui->grandRateH->setValue(editSettings->getGrandRateH());
	ui->grandRateS->setValue(editSettings->getGrandRateS());
	ui->grandRateL->setValue(editSettings->getGrandRateL());
	ui->splashTime->setValue(editSettings->getSplashTime());
}

void VisualSettings::colorMxHChanged(const int &x) { editSettings->setColorMxH(x); }

void VisualSettings::colorMxSChanged(const double &x) { editSettings->setColorMxS(x); }

void VisualSettings::colorMxLChanged(const double &x) { editSettings->setColorMxL(x); }

void VisualSettings::colorMiHChanged(const int &x) { editSettings->setColorMiH(x); }

void VisualSettings::colorMiSChanged(const double &x) { editSettings->setColorMiS(x); }

void VisualSettings::colorMiLChanged(const double &x) { editSettings->setColorMiL(x); }

void VisualSettings::colorNfHChanged(const int &x) { editSettings->setColorNfH(x); }

void VisualSettings::colorNfSChanged(const double &x) { editSettings->setColorNfS(x); }

void VisualSettings::colorNfLChanged(const double &x) { editSettings->setColorNfL(x); }

void VisualSettings::colorCeHChanged(const int &x) { editSettings->setColorCeH(x); }

void VisualSettings::colorCeSChanged(const double &x) { editSettings->setColorCeS(x); }

void VisualSettings::colorCeLChanged(const double &x) { editSettings->setColorCeL(x); }

void VisualSettings::grandCompHChanged(const double &x) { editSettings->setGrandCompH(x); }

void VisualSettings::grandCompSChanged(const double &x) { editSettings->setGrandCompS(x); }

void VisualSettings::grandCompLChanged(const double &x) { editSettings->setGrandCompL(x); }

void VisualSettings::grandRateHChanged(const double &x) { editSettings->setGrandRateH(x); }

void VisualSettings::grandRateSChanged(const double &x) { editSettings->setGrandRateS(x); }

void VisualSettings::grandRateLChanged(const double &x) { editSettings->setGrandRateL(x); }

void VisualSettings::splashTimeChanged(const int &x) { editSettings->setSplashTime(x); }

VisualSettings::~VisualSettings() { delete ui; }
