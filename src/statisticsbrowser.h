/*
 * SPDX-FileCopyrightText: 2019  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include "base/LemonType.hpp"
#include "core/contestant.h"
#include "core/task.h"
#include "core/testcase.h"
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
