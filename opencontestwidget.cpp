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
 * opencontestwidget.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "opencontestwidget.h"
#include "ui_opencontestwidget.h"
#include "contest.h"
#include <QFileDialog>

OpenContestWidget::OpenContestWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::OpenContestWidget)
{
	ui->setupUi(this);
	connect(ui->recentContest, SIGNAL(itemSelectionChanged()),
	        this, SIGNAL(selectionChanged()));
	connect(ui->recentContest, SIGNAL(cellDoubleClicked(int, int)),
	        this, SIGNAL(rowDoubleClicked()));
	connect(ui->addButton, SIGNAL(clicked()),
	        this, SLOT(addContest()));
	connect(ui->deleteButton, SIGNAL(clicked()),
	        this, SLOT(deleteContest()));
	connect(ui->recentContest, SIGNAL(currentCellChanged(int, int, int, int)),
	        this, SLOT(currentRowChanged()));
}

OpenContestWidget::~OpenContestWidget()
{
	delete ui;
}

void OpenContestWidget::setRecentContest(const QStringList &list)
{
	recentContest = list;
	refreshContestList();
}

void OpenContestWidget::refreshContestList()
{
	ui->recentContest->setRowCount(0);

	for(int i = 0; i < recentContest.size();)
	{
		QFile file(recentContest[i]);

		if(! file.open(QFile::ReadOnly))
		{
			recentContest.removeAt(i);
			continue;
		}

		QDataStream _in(&file);
		unsigned checkNumber;
		_in >> checkNumber;

		if(checkNumber != unsigned(MagicNumber))
		{
			recentContest.removeAt(i);
			continue;
		}

		quint16 checksum;
		int len;
		_in >> checksum >> len;
		char *raw = new char[len];
		_in.readRawData(raw, len);

		if(qChecksum(raw, len) != checksum)
		{
			delete[] raw;
			recentContest.removeAt(i);
			continue;
		}

		QByteArray data(raw, len);
		delete[] raw;
		data = qUncompress(data);
		QDataStream in (data);
		QString title;
		in >> title;
		ui->recentContest->setRowCount(i + 1);
		ui->recentContest->setItem(i, 0, new QTableWidgetItem(title));
		ui->recentContest->setItem(i, 1, new QTableWidgetItem(recentContest[i]));
		ui->recentContest->item(i, 0)->setTextAlignment(Qt::AlignCenter);
		i ++;
	}

	QHeaderView *header = ui->recentContest->horizontalHeader();
	header->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void OpenContestWidget::addContest()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Add Contest"), QDir::homePath(),
	                   tr("Lemon contest data file (*.cdf)"));

	if(fileName.isEmpty()) return;

	fileName = fileName.replace('/', QDir::separator());
	QFile file(fileName);

	if(! file.open(QFile::ReadOnly))
	{
		QMessageBox::warning(this, tr("Error"), tr("Cannot open selected file"), QMessageBox::Close);
		return;
	}

	QDataStream in (&file);
	unsigned checkNumber;
	in >> checkNumber;

	if(checkNumber != unsigned(MagicNumber))
	{
		QMessageBox::warning(this, tr("Error"), tr("Broken contest data file"), QMessageBox::Close);
		return;
	}

	quint16 checksum;
	int len;
	in >> checksum >> len;
	char *raw = new char[len];
	in.readRawData(raw, len);

	if(qChecksum(raw, len) != checksum)
	{
		QMessageBox::warning(this, tr("Error"), tr("Broken contest data file"), QMessageBox::Close);
		delete[] raw;
		return;
	}

	delete[] raw;
	recentContest.prepend(fileName);
	refreshContestList();
}

void OpenContestWidget::deleteContest()
{
	int index = ui->recentContest->currentRow();
	recentContest.removeAt(index);
	refreshContestList();
}

void OpenContestWidget::currentRowChanged()
{
	int index = ui->recentContest->currentRow();

	if(index != -1)
	{
		ui->deleteButton->setEnabled(true);
	}
	else
	{
		ui->deleteButton->setEnabled(false);
	}
}

const QStringList &OpenContestWidget::getRecentContest() const
{
	return recentContest;
}

int OpenContestWidget::getCurrentRow() const
{
	return ui->recentContest->currentRow();
}
