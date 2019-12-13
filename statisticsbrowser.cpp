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

#include "statisticsbrowser.h"
#include "ui_statisticsbrowser.h"
#include "globaltype.h"
#include "contestant.h"
#include "settings.h"
#include "contest.h"
#include "task.h"
#include <QMap>
#include <QMessageBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>

StatisticsBrowser::StatisticsBrowser(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::StatisticsBrowser)
{
	ui->setupUi(this);
	curContest = nullptr;
}

StatisticsBrowser::~StatisticsBrowser()
{
	delete ui;
}

void StatisticsBrowser::setContest(Contest *contest)
{
	curContest = contest;
}

void StatisticsBrowser::refresh()
{
	QString buffer;

	if (! curContest)
	{
		buffer = tr("No contest yet");
		ui->textBrowser->setHtml(buffer);
		return;
	}

	QList<Task *> taskList = curContest->getTaskList();
	QList<Contestant *> contestantList = curContest->getContestantList();

	if (taskList.size() <= 0)
	{
		buffer = tr("No task yet");
		ui->textBrowser->setHtml(buffer);
		return;
	}

	if (contestantList.size() <= 0)
	{
		buffer = tr("No contestant yet");
		ui->textBrowser->setHtml(buffer);
		return;
	}

	QList<int> fullScore;
	int sfullScore = curContest->getTotalScore();

	for (int i = 0; i < taskList.size(); i++)
	{
		fullScore.append(taskList[i]->getTotalScore());
	}

	buffer += "<html><head>";
	buffer += "<style type=\"text/css\">th, td {padding-left: 1em; padding-right: 1em;}</style>";
	buffer += "</head><body>";

	buffer += "<h1>" + QString("%1 %2").arg(tr("Contest")).arg(curContest->getContestTitle()) + "</h1>";

	buffer += "<h2>" + tr("Overall") + "</h2>";

	long long scoreSum = 0;

	QMap<int, int> scoreCount;
	for (int i = 0; i < contestantList.size(); i++)
	{
		int allscore = 0;
		for (int j = 0; j < taskList.size(); j++)
			allscore += contestantList[i]->getTaskScore(j);
		scoreCount[allscore]++;
		scoreSum += allscore;
	}

	int temp = 0;
	buffer += "<table border=\"-1\">";
	buffer += QString("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th></tr>").arg(tr("Score")).arg(tr("Count")).arg(tr("Ratio")).arg(tr("Prefix")).arg(tr("Suffix"));
	for (auto i = scoreCount.constEnd(); i != scoreCount.constBegin();)
	{
		i--;
		buffer += "<tr>";
		buffer += QString("<td align=\"right\"><nobr>%1 Pt</nobr></td>").arg(i.key());
		buffer += QString("<td align=\"right\"><nobr>%1</nobr></td>").arg(i.value());
		buffer += QString("<td align=\"left\"><nobr>%1%</nobr></td>").arg(100.00 * i.value() / contestantList.size());
		buffer += QString("<td align=\"left\"><nobr>%1 (%2%)</nobr></td>").arg(contestantList.size() - temp).arg(100.00 - 100.00 * temp / contestantList.size());
		temp += i.value();
		buffer += QString("<td align=\"left\"><nobr>%1 (%2%)</nobr></td>").arg(temp).arg(100.00 * temp / contestantList.size());
		buffer += "</tr>";
	}
	buffer += "</table>";

	buffer += "<p>" + tr("Average") + " : " + QString::number(1.00 * scoreSum / contestantList.size()) + " / " + QString::number(sfullScore) + "</p>";


	buffer += "<h2>" + tr("Problems") + "</h2>";

	for (int i = 0; i < taskList.size(); i++)
	{
		buffer += "<h3>";
		buffer += QString("%1 %2: %3").arg(tr("Task")).arg(i + 1).arg(taskList[i]->getProblemTile());
		buffer += "</h3>";

		scoreSum = 0;
		QMap<int, int> cnts;
		for (int j = 0; j < contestantList.size(); j++)
		{
			scoreSum += contestantList[j]->getTaskScore(i);
			cnts[contestantList[j]->getTaskScore(i)]++;
		}

		temp = 0;
		buffer += "<table border=\"-1\">";
		buffer += QString("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th></tr>").arg(tr("Score")).arg(tr("Count")).arg(tr("Ratio")).arg(tr("Prefix")).arg(tr("Suffix"));
		for (auto j = cnts.constEnd(); j != cnts.constBegin();)
		{
			j--;
			buffer += "<tr>";
			buffer += QString("<td align=\"right\"><nobr>%1 Pt</nobr></td>").arg(j.key());
			buffer += QString("<td align=\"right\"><nobr>%1</nobr></td>").arg(j.value());
			buffer += QString("<td align=\"left\"><nobr>%1%</nobr></td>").arg(100.00 * j.value() / contestantList.size());
			buffer += QString("<td align=\"left\"><nobr>%1 (%2%)</nobr></td>").arg(contestantList.size() - temp).arg(100.00 - 100.00 * temp / contestantList.size());
			temp += j.value();
			buffer += QString("<td align=\"left\"><nobr>%1 (%2%)</nobr></td>").arg(temp).arg(100.00 * temp / contestantList.size());
			buffer += "</tr>";
		}

		buffer += "</table>";

		buffer += "<p>" + tr("Average") + " : " + QString::number(1.00 * scoreSum / contestantList.size()) + " / " + QString::number(taskList[i]->getTotalScore()) + "</p>";
	}

	buffer += "</body></html>";
	ui->textBrowser->setHtml(buffer);
}
