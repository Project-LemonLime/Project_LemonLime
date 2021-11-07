/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "opencontestwidget.h"
#include "ui_opencontestwidget.h"
//
#include "core/contest.h"
//
#include <QFileDialog>
#include <QJsonDocument>
OpenContestWidget::OpenContestWidget(QWidget *parent) : QWidget(parent), ui(new Ui::OpenContestWidget) {
	ui->setupUi(this);
	connect(ui->recentContest, &QTableWidget::itemSelectionChanged, this,
	        &OpenContestWidget::selectionChanged);
	connect(ui->recentContest, &QTableWidget::cellDoubleClicked, this, &OpenContestWidget::rowDoubleClicked);
	connect(ui->addButton, &QToolButton::clicked, this, &OpenContestWidget::addContest);
	connect(ui->deleteButton, &QToolButton::clicked, this, &OpenContestWidget::deleteContest);
	connect(ui->recentContest, &QTableWidget::currentCellChanged, this,
	        &OpenContestWidget::currentRowChanged);
}

OpenContestWidget::~OpenContestWidget() { delete ui; }

void OpenContestWidget::setRecentContest(const QStringList &list) {
	recentContest = list;
	refreshContestList();
}

void OpenContestWidget::refreshContestList() {
	ui->recentContest->setRowCount(0);

	for (int i = 0; i < recentContest.size();) {
		QFile file(recentContest[i]);

		if (! file.open(QFile::ReadOnly)) {
			recentContest.removeAt(i);
			continue;
		}
		QString title;
		char firstChar;
		file.peek(&firstChar, 1);
		// Don't support RFC 7159, but support RFC 4627
		if (firstChar == '[' || firstChar == '{') {
			QJsonParseError parseError;
			QJsonObject inObj(QJsonDocument::fromJson(file.readAll(), &parseError).object());
			if (parseError.error != 0) {
				recentContest.removeAt(i);
				continue;
			}
			title = inObj["contestTitle"].toString();
		} else {
			QDataStream _in(&file);
			unsigned checkNumber = 0;
			_in >> checkNumber;

			if (checkNumber != unsigned(MagicNumber)) {
				recentContest.removeAt(i);
				continue;
			}

			quint16 checksum = 0;
			int len = 0;
			_in >> checksum >> len;
			char *raw = new char[len];
			_in.readRawData(raw, len);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			if (qChecksum(QByteArrayView(raw, static_cast<uint>(len))) != checksum)
#else
			if (qChecksum(raw, static_cast<uint>(len)) != checksum)
#endif
			{
				delete[] raw;
				recentContest.removeAt(i);
				continue;
			}

			QByteArray data(raw, len);
			delete[] raw;
			data = qUncompress(data);
			QDataStream in(data);
			in >> title;
		}
		ui->recentContest->setRowCount(i + 1);
		ui->recentContest->setItem(i, 0, new QTableWidgetItem(title));
		ui->recentContest->setItem(i, 1, new QTableWidgetItem(recentContest[i]));
		ui->recentContest->item(i, 0)->setTextAlignment(Qt::AlignCenter);
		i++;
	}

	QHeaderView *header = ui->recentContest->horizontalHeader();
	header->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void OpenContestWidget::addContest() {
	QString fileName = QFileDialog::getOpenFileName(this, tr("Add Contest"), QDir::homePath(),
	                                                tr("Lemon contest data file (*.cdf)"));

	if (fileName.isEmpty())
		return;

	fileName = fileName.replace('/', QDir::separator());
	QFile file(fileName);

	if (! file.open(QFile::ReadOnly)) {
		QMessageBox::warning(this, tr("Error"), tr("Cannot open selected file"), QMessageBox::Close);
		return;
	}

	char firstChar;
	file.peek(&firstChar, 1);
	// Don't support RFC 7159, but support RFC 4627
	if (firstChar == '[' || firstChar == '{') {
		QJsonParseError parseError;
		QJsonObject inObj(QJsonDocument::fromJson(file.readAll(), &parseError).object());
		if (parseError.error != 0) {
			QMessageBox::warning(this, tr("Error"), tr("Broken contest data file"), QMessageBox::Close);
			return;
		}
	} else {
		QDataStream in(&file);
		unsigned checkNumber = 0;
		in >> checkNumber;

		if (checkNumber != unsigned(MagicNumber)) {
			QMessageBox::warning(this, tr("Error"), tr("Broken contest data file"), QMessageBox::Close);
			return;
		}

		quint16 checksum = 0;
		int len = 0;
		in >> checksum >> len;
		char *raw = new char[len];
		in.readRawData(raw, len);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		if (qChecksum(QByteArrayView(raw, static_cast<uint>(len))) != checksum)
#else
		if (qChecksum(raw, static_cast<uint>(len)) != checksum)
#endif
		{
			QMessageBox::warning(this, tr("Error"), tr("Broken contest data file"), QMessageBox::Close);
			delete[] raw;
			return;
		}

		delete[] raw;
	}
	recentContest.prepend(fileName);
	refreshContestList();
}

void OpenContestWidget::deleteContest() {
	int index = ui->recentContest->currentRow();
	recentContest.removeAt(index);
	refreshContestList();
}

void OpenContestWidget::currentRowChanged() {
	int index = ui->recentContest->currentRow();

	if (index != -1) {
		ui->deleteButton->setEnabled(true);
	} else {
		ui->deleteButton->setEnabled(false);
	}
}

auto OpenContestWidget::getRecentContest() const -> const QStringList & { return recentContest; }

auto OpenContestWidget::getCurrentRow() const -> int { return ui->recentContest->currentRow(); }
