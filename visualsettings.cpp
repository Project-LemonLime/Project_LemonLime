/***************************************************************************
	 This file is part of Project LemonLime
	 Copyright (C) 2020 iotang

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

#include "visualsettings.h"
#include "ui_visualsettings.h"
#include "settings.h"

VisualSettings::VisualSettings(QWidget *parent) :
   QWidget(parent),
   ui(new Ui::VisualSettings)
{
	ui->setupUi(this);

	connect(ui->spinBoxMxH, SIGNAL(valueChanged(int)),
	        this, SLOT(colorMxHChanged(int)));
	connect(ui->doubleSpinBoxMxS, SIGNAL(valueChanged(double)),
	        this, SLOT(colorMxSChanged(double)));
	connect(ui->doubleSpinBoxMxL, SIGNAL(valueChanged(double)),
	        this, SLOT(colorMxLChanged(double)));
	connect(ui->spinBoxMiH, SIGNAL(valueChanged(int)),
	        this, SLOT(colorMiHChanged(int)));
	connect(ui->doubleSpinBoxMiS, SIGNAL(valueChanged(double)),
	        this, SLOT(colorMiSChanged(double)));
	connect(ui->doubleSpinBoxMiL, SIGNAL(valueChanged(double)),
	        this, SLOT(colorMiLChanged(double)));
	connect(ui->spinBoxNfH, SIGNAL(valueChanged(int)),
	        this, SLOT(colorNfHChanged(int)));
	connect(ui->doubleSpinBoxNfS, SIGNAL(valueChanged(double)),
	        this, SLOT(colorNfSChanged(double)));
	connect(ui->doubleSpinBoxNfL, SIGNAL(valueChanged(double)),
	        this, SLOT(colorNfLChanged(double)));
	connect(ui->spinBoxCeH, SIGNAL(valueChanged(int)),
	        this, SLOT(colorCeHChanged(int)));
	connect(ui->doubleSpinBoxCeS, SIGNAL(valueChanged(double)),
	        this, SLOT(colorCeSChanged(double)));
	connect(ui->doubleSpinBoxCeL, SIGNAL(valueChanged(double)),
	        this, SLOT(colorCeLChanged(double)));

	connect(ui->resetToDefaultButton, SIGNAL(clicked()),
	        this, SLOT(resetToDefault()));
}

void VisualSettings::resetToDefault()
{
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
}

void VisualSettings::resetEditSettings(Settings *settings)
{
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
}

void VisualSettings::colorMxHChanged(const int &x)
{
	editSettings->setColorMxH(x);
}

void VisualSettings::colorMxSChanged(const double &x)
{
	editSettings->setColorMxS(x);
}

void VisualSettings::colorMxLChanged(const double &x)
{
	editSettings->setColorMxL(x);
}

void VisualSettings::colorMiHChanged(const int &x)
{
	editSettings->setColorMiH(x);
}

void VisualSettings::colorMiSChanged(const double &x)
{
	editSettings->setColorMiS(x);
}

void VisualSettings::colorMiLChanged(const double &x)
{
	editSettings->setColorMiL(x);
}

void VisualSettings::colorNfHChanged(const int &x)
{
	editSettings->setColorNfH(x);
}

void VisualSettings::colorNfSChanged(const double &x)
{
	editSettings->setColorNfS(x);
}

void VisualSettings::colorNfLChanged(const double &x)
{
	editSettings->setColorNfL(x);
}

void VisualSettings::colorCeHChanged(const int &x)
{
	editSettings->setColorCeH(x);
}

void VisualSettings::colorCeSChanged(const double &x)
{
	editSettings->setColorCeS(x);
}

void VisualSettings::colorCeLChanged(const double &x)
{
	editSettings->setColorCeL(x);
}

VisualSettings::~VisualSettings()
{
	delete ui;
}
