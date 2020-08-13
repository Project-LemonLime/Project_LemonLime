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
 * opencontestwidget.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef OPENCONTESTWIDGET_H
#define OPENCONTESTWIDGET_H

#include <QMessageBox>
#include <QWidget>
#include <QtCore>
#include <QtGui>

namespace Ui
{
	class OpenContestWidget;
}

class OpenContestWidget : public QWidget
{
	Q_OBJECT

  public:
	explicit OpenContestWidget(QWidget *parent = nullptr);
	~OpenContestWidget();
	void setRecentContest(const QStringList &);
	const QStringList &getRecentContest() const;
	int getCurrentRow() const;

  private:
	Ui::OpenContestWidget *ui;
	QStringList recentContest;
	void refreshContestList();

  private slots:
	void addContest();
	void deleteContest();
	void currentRowChanged();

  signals:
	void selectionChanged();
	void rowDoubleClicked();
};

#endif // OPENCONTESTWIDGET_H
