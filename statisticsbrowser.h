/***************************************************************************
	 This file is part of Project LemonLime
	 Copyright (C) 2019 iotang

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

#ifndef STATISTICSBROWSER_H
#define STATISTICSBROWSER_H

#include <QWidget>

namespace Ui
{
	class StatisticsBrowser;
}

class Contest;

class StatisticsBrowser : public QWidget
{
	Q_OBJECT

public:
	explicit StatisticsBrowser(QWidget *parent = nullptr);
	void setContest(Contest *);
	~StatisticsBrowser();

public slots:
	void refresh();

private:
	Ui::StatisticsBrowser *ui;
	Contest *curContest;
	QString getScoreNormalChart(QMap<int,int>, int, int);
};

#endif // STATISTICSBROWSER_H
