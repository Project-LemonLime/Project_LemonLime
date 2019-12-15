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

QString StatisticsBrowser::getScoreNormalChart(QMap<int,int> scoreCount, int listSize, int totalScore)
{
	QString buffer = "";

	long long overallScoreSum = 0;
	double scoreDiscrim = 0, scoreStandardDevia = 0;
	int scoreTierPrefix = 0, lastScoreTier = -1;

	for (auto i = scoreCount.constEnd(); i != scoreCount.constBegin();)
	{
		i--;
		int curScoreTier = i.key(), curScoreTierNum = i.value();
		overallScoreSum += 1ll * curScoreTier * curScoreTierNum;

		if(lastScoreTier >= 0)scoreDiscrim += qLn(1 + 10.00 * (lastScoreTier - curScoreTier) / totalScore);
		lastScoreTier = curScoreTier;
	}

	double scoreAverage = 1.00 * overallScoreSum / listSize;

	buffer += "<table border=\"-1\">";
	buffer += QString("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th></tr>").arg(tr("Score")).arg(tr("Count")).arg(tr("Ratio")).arg(tr("Prefix")).arg(tr("Suffix"));
	for (auto i = scoreCount.constEnd(); i != scoreCount.constBegin();)
	{
		i--;
		int curScoreTier = i.key(), curScoreTierNum = i.value();

		scoreStandardDevia += qPow(curScoreTier - scoreAverage, 2) * curScoreTierNum;

		buffer += "<tr>";
		buffer += QString("<td align=\"right\"><nobr>%1 Pt</nobr></td>").arg(curScoreTier);
		buffer += QString("<td align=\"right\"><nobr>%1</nobr></td>").arg(curScoreTierNum);
		buffer += QString("<td align=\"left\"><nobr>%1%</nobr></td>").arg(100.00 * curScoreTierNum / listSize);
		buffer += QString("<td align=\"left\"><nobr>%1 (%2%)</nobr></td>").arg(listSize - scoreTierPrefix).arg(100.00 - 100.00 * scoreTierPrefix / listSize);
		scoreTierPrefix += curScoreTierNum;
		buffer += QString("<td align=\"left\"><nobr>%1 (%2%)</nobr></td>").arg(scoreTierPrefix).arg(100.00 * scoreTierPrefix / listSize);
		buffer += "</tr>";
	}
	buffer += "</table>";

	scoreStandardDevia = qSqrt(scoreStandardDevia / listSize);
	scoreDiscrim = scoreDiscrim * scoreDiscrim;

	buffer += "<p>" + tr("Average") + " : " + QString::number(scoreAverage) + " / " + QString::number(totalScore) + "</p>";
	buffer += "<p>" + tr("Standard Deviation") + " : " + QString::number(scoreStandardDevia) + "<p>";
	buffer += "<p>" + tr("Score Discrimination Power") + " : " + QString::number(scoreDiscrim) + "<p>";

	return buffer;
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

	int totalScore = curContest->getTotalScore();

	buffer += "<html><head>";
	buffer += "<style type=\"text/css\">th, td {padding-left: 1em; padding-right: 1em;}</style>";
	buffer += "</head><body>";

	buffer += "<h1>" + QString("%1 %2").arg(tr("Contest")).arg(curContest->getContestTitle()) + "</h1>";

	buffer += "<h2>" + tr("Overall") + "</h2>";

	int haveError = 0;

	QMap<int, int> scoreCount;
	for (int i = 0; i < contestantList.size(); i++)
	{
		int contestantTotalScore = 0;
		for (int j = 0; j < taskList.size(); j++)
		{
			contestantTotalScore += contestantList[i]->getTaskScore(j);
			if(contestantList[i]->getTaskScore(j) < 0) haveError = 1;
		}
		scoreCount[contestantTotalScore]++;
	}

	if(haveError)
	{
		buffer += "<p style=\"font-size: large; color: red;\">" + tr("Warning: Judgement is not finished.") + "</p><br>";
	}

	buffer += getScoreNormalChart(scoreCount, contestantList.size(), totalScore);
	buffer += "<br>";

	buffer += "<h2>" + tr("Problems") + "</h2>";

	for (int i = 0; i < taskList.size(); i++)
	{
		buffer += "<h3>";
		buffer += QString("%1 %2: %3").arg(tr("Task")).arg(i + 1).arg(taskList[i]->getProblemTile());
		buffer += "</h3>";

		int numberSubmitted = 0;

		QMap<int, int> cnts;
		for (int j = 0; j < contestantList.size(); j++)
		{
			cnts[contestantList[j]->getTaskScore(i)]++;
			if(contestantList[j]->getCompileState(i) != NoValidSourceFile)
				numberSubmitted ++;
		}

		buffer += getScoreNormalChart(cnts, contestantList.size(), taskList[i]->getTotalScore());
		buffer += "<p>" + tr("Number of answer submitted") + " : " + QString::number(numberSubmitted) + " / " + QString::number(contestantList.size()) + " (" + QString::number(100.00 * numberSubmitted / contestantList.size()) + "%)</p>";
		buffer += "<br>";
	}

	buffer += "</body></html>";
	ui->textBrowser->setHtml(buffer);
}
