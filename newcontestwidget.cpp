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
 * newcontestwidget.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "newcontestwidget.h"
#include "ui_newcontestwidget.h"
#include <QFileDialog>

NewContestWidget::NewContestWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::NewContestWidget)
{
	ui->setupUi(this);
	connect(ui->selectButton, SIGNAL(clicked()),
			  this, SLOT(selectContestPath()));
	connect(ui->savingName, SIGNAL(textChanged(QString)),
			  this, SLOT(savingNameChanged()));
	connect(ui->contestTitle, SIGNAL(textChanged(QString)),
			  this, SIGNAL(informationChanged()));
	connect(ui->savingName, SIGNAL(textChanged(QString)),
			  this, SIGNAL(informationChanged()));
	connect(ui->contestPath, SIGNAL(textChanged(QString)),
			  this, SIGNAL(informationChanged()));
}

NewContestWidget::~NewContestWidget()
{
	delete ui;
}

QString NewContestWidget::getContestTitle()
{
	return ui->contestTitle->text();
}

QString NewContestWidget::getSavingName()
{
	return ui->savingName->text();
}

QString NewContestWidget::getContestPath()
{
	return ui->contestPath->text();
}

bool NewContestWidget::checkReady() const
{
	return ! ui->contestTitle->text().isEmpty() && ! ui->contestPath->text().isEmpty() && ! ui->savingName->text().isEmpty();
}

void NewContestWidget::selectContestPath()
{
	QString path = QFileDialog::getExistingDirectory(this, tr("Select Contest Path"), QDir::homePath());

	if(! path.isEmpty()) ui->contestPath->setText(QDir::toNativeSeparators(path));
}

void NewContestWidget::savingNameChanged()
{
	QString path = QDir::homePath();
	path = QDir::toNativeSeparators(path);
	path += QDir::separator();
	path += ui->savingName->text();
	ui->contestPath->setText(path);
}
