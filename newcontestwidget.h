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
 * newcontestwidget.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef NEWCONTESTWIDGET_H
#define NEWCONTESTWIDGET_H

#include <QtCore>
#include <QtGui>
#include <QWidget>

namespace Ui
{
	class NewContestWidget;
}

class NewContestWidget : public QWidget
{
	Q_OBJECT

public:
	explicit NewContestWidget(QWidget *parent = 0);
	~NewContestWidget();
	QString getContestTitle();
	QString getSavingName();
	QString getContestPath();
	bool checkReady() const;

private:
	Ui::NewContestWidget *ui;

signals:
	void informationChanged();

private slots:
	void selectContestPath();
	void savingNameChanged();
};

#endif // NEWCONTESTWIDGET_H
