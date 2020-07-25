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

#include "contestant.h"
#include "globaltype.h"
#include "task.h"
#include "testcase.h"
#include <QWidget>

namespace Ui
{
	class StatisticsBrowser;
}

class Contest;
class TestCase;

static QString nowBrowserText;

class StatisticsBrowser : public QWidget
{
	Q_OBJECT

public:
	explicit StatisticsBrowser(QWidget *parent = nullptr);
	void setContest(Contest *);
	static void exportStatstics(QWidget *, Contest *);
	~StatisticsBrowser();

public slots:
	void refresh();

private:
	Ui::StatisticsBrowser *ui;
	Contest *curContest;
	static int checkValid(QList<Task *>, const QList<Contestant *> &);
	static QString getScoreNormalChart(const QMap<int, int> &, int, int);
	static QString getTestcaseScoreChart(QList<TestCase *>, QList<QList<QList<int>>>,
	                                     QList<QList<QList<ResultState>>>);
	static void exportStatsticsHtml(QWidget *, const QString &);
};

#endif // STATISTICSBROWSER_H
