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
#include <QFileDialog>

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

auto StatisticsBrowser::getScoreNormalChart(const QMap<int, int> &scoreCount, int listSize, int totalScore) -> QString
{
	QString buffer = "";

	long long overallScoreSum = 0;
	double scoreDiscrim = 0;
	double scoreStandardDevia = 0;
	int scoreTierPrefix = 0;
	int lastScoreTier = -1;
	int lastScoreTierNum = -1;

	for (auto i = scoreCount.constEnd(); i != scoreCount.constBegin();)
	{
		i--;
		int curScoreTier = i.key();
		int curScoreTierNum = i.value();
		overallScoreSum += 1LL * curScoreTier * curScoreTierNum;

		if (lastScoreTier >= 0)scoreDiscrim += qLn(1 + 10.00 * (lastScoreTier - curScoreTier) / totalScore) * (1.00 - 1.00 * lastScoreTierNum * curScoreTierNum / listSize / listSize);
		lastScoreTier = curScoreTier;
		lastScoreTierNum = curScoreTierNum;
	}

	double scoreAverage = 1.00 * overallScoreSum / listSize;

	buffer += "<table border=\"-1\">";
	buffer += QString("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th></tr>").arg(tr("Score")).arg(tr("Count")).arg(tr("Ratio")).arg(tr("Prefix")).arg(tr("Suffix"));
	for (auto i = scoreCount.constEnd(); i != scoreCount.constBegin();)
	{
		i--;
		int curScoreTier = i.key();
		int curScoreTierNum = i.value();

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

auto StatisticsBrowser::getTestcaseScoreChart(QList<TestCase *> testCaseList, QList<QList<QList<int>>> scoreList, QList<QList<QList<ResultState>>> resultList) -> QString
{
	QString buffer = "";

	buffer += "<table border=\"-1\">";
	buffer += QString("<tr><th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th><th>%6</th><th>%7</th></tr>").arg(tr("No.")).arg(tr("Input")).arg(tr("Output")).arg(tr("Pure")).arg(tr("Far")).arg(tr("Lost")).arg(tr("Average"));

	for (int i = 0; i < testCaseList.length(); i++)
	{
		QStringList inFileList = testCaseList[i]->getInputFiles();
		QStringList outFileList = testCaseList[i]->getOutputFiles();

		if (inFileList.length() != outFileList.length())return "Wrong File List Length Please Refresh and Rejudge";

		int mxScore = testCaseList[i]->getFullScore();
		QList<int> miScoreRecord;
		QList<int> miStatRecord;
		for (int j = 0; j < scoreList.length(); j++)
		{
			miScoreRecord.append(mxScore);
			miStatRecord.append(2);
		}

		for (int j = 0; j < inFileList.length(); j++)
		{
			int cntFail = 0;
			int cntPati = 0;
			int cntSucc = 0;
			long long sumscore = 0;
			for (int k = 0; k < scoreList.length(); k++)
			{
				int score = 0;
				int statVal = 2;
				ResultState stat = WrongAnswer;
				if (scoreList[k].length() > i && scoreList[k][i].length() > j)
				{
					score = scoreList[k][i][j];
					stat = resultList[k][i][j];
				}
				if (stat == CorrectAnswer)cntSucc++, statVal = 2;
				else if (stat == PartlyCorrect)cntPati++, statVal = 1;
				else cntFail++, statVal = 0;
				sumscore += score;
				miScoreRecord[k] = qMin(miScoreRecord[k], score);
				miStatRecord[k] = qMin(miStatRecord[k], statVal);
			}
			buffer += "<tr>";
			buffer += "<td align=\"left\"><nobr>" + QString("%1.%2").arg(i + 1).arg(j + 1) + "</nobr></td>";
			buffer += "<td align=\"left\"><nobr>" + QString("%1").arg(inFileList[j]) + "</nobr></td>";
			buffer += "<td align=\"left\"><nobr>" + QString("%1").arg(outFileList[j]) + "</nobr></td>";
			buffer += "<td align=\"left\"><nobr>" + QString("%1 (%2%)").arg(cntSucc).arg(100.00 * cntSucc / scoreList.length()) + "</nobr></td>";
			buffer += "<td align=\"left\"><nobr>" + QString("%1 (%2%)").arg(cntPati).arg(100.00 * cntPati / scoreList.length()) + "</nobr></td>";
			buffer += "<td align=\"left\"><nobr>" + QString("%1 (%2%)").arg(cntFail).arg(100.00 * cntFail / scoreList.length()) + "</nobr></td>";
			buffer += "<td align=\"right\"><nobr>" + QString("%1 / %2").arg(1.00 * sumscore / scoreList.length()).arg(mxScore) + "</nobr></td>";
			buffer += "</tr>";
		}

		if (inFileList.length() > 1)
		{
			int sumCntFail = 0;
			int sumCntPati = 0;
			int sumCntSucc = 0;
			long long sumSumScore = 0;
			for (int j = 0; j < scoreList.length(); j++)
			{
				sumSumScore += miScoreRecord[j];
				if (miStatRecord[j] >= 2)sumCntSucc++;
				else if (miStatRecord[j] == 1)sumCntPati++;
				else sumCntFail++;
			}

			buffer += "<tr>";
			buffer += "<td align=\"left\"><nobr>" + QString("%1 %2").arg(i + 1).arg("Overall") + "</nobr></td>";
			buffer += "<td align=\"left\"><nobr>" + QString("%1 %2").arg(inFileList.length()).arg(tr("Files")) + "</nobr></td>";
			buffer += "<td align=\"left\"><nobr>" + QString("%1 %2").arg(outFileList.length()).arg(tr("Files")) + "</nobr></td>";
			buffer += "<td align=\"left\"><nobr>" + QString("%1 (%2%)").arg(sumCntSucc).arg(100.00 * sumCntSucc / scoreList.length()) + "</nobr></td>";
			buffer += "<td align=\"left\"><nobr>" + QString("%1 (%2%)").arg(sumCntPati).arg(100.00 * sumCntPati / scoreList.length()) + "</nobr></td>";
			buffer += "<td align=\"left\"><nobr>" + QString("%1 (%2%)").arg(sumCntFail).arg(100.00 * sumCntFail / scoreList.length()) + "</nobr></td>";
			buffer += "<td align=\"right\"><nobr>" + QString("%1 / %2").arg(1.00 * sumSumScore / scoreList.length()).arg(mxScore) + "</nobr></td>";
			buffer += "</tr>";
		}
	}
	buffer += "</table>";

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

	if (taskList.empty())
	{
		buffer = tr("No task yet");
		ui->textBrowser->setHtml(buffer);
		return;
	}

	if (contestantList.empty())
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
	for (auto &i : contestantList)
	{
		int contestantTotalScore = 0;
		for (int j = 0; j < taskList.size(); j++)
		{
			contestantTotalScore += i->getTaskScore(j);
			if (i->getTaskScore(j) < 0) haveError = 1;
		}
		scoreCount[contestantTotalScore]++;
	}

	if (haveError)
	{
		buffer += "<p style=\"font-size: large; color: red;\">" + tr("Warning: Judgement is not finished.") + "</p><br>";
	}

	buffer += getScoreNormalChart(scoreCount, contestantList.size(), totalScore);
	buffer += "<br>";
	buffer += "<br>";

	buffer += "<h2>" + tr("Problems") + "</h2>";

	for (int i = 0; i < taskList.size(); i++)
	{
		buffer += "<h3>";
		buffer += QString("%1 %2: %3").arg(tr("Task")).arg(i + 1).arg(taskList[i]->getProblemTile());
		buffer += "</h3>";

		int numberSubmitted = 0;

		QMap<int, int> cnts;
		QList<QList<QList<int>>> TestcaseScoreList;
		QList<QList<QList<ResultState>>> resultList;
		for (auto &j : contestantList)
		{
			cnts[j->getTaskScore(i)]++;
			if (j->getCompileState(i) != NoValidSourceFile)
				numberSubmitted ++;
			TestcaseScoreList.append(j->getSocre(i));
			resultList.append(j->getResult(i));
		}

		buffer += getScoreNormalChart(cnts, contestantList.size(), taskList[i]->getTotalScore());
		buffer += "<p>" + tr("Number of answer submitted") + " : " + QString::number(numberSubmitted) + " / " + QString::number(contestantList.size()) + " (" + QString::number(100.00 * numberSubmitted / contestantList.size()) + "%)</p>";
		buffer += getTestcaseScoreChart(taskList[i]->getTestCaseList(), TestcaseScoreList, resultList);
		buffer += "<br>";
		buffer += "<br>";
	}

	buffer += "</body></html>";
	ui->textBrowser->setHtml(buffer);
	nowBrowserText = buffer;
}

void StatisticsBrowser::exportStatsticsHtml(QWidget *widget, const QString &fileName)
{
	QFile file(fileName);

	if (! file.open(QFile::WriteOnly))
	{
		QMessageBox::warning(widget, tr("LemonLime"), tr("Cannot open file %1").arg(QFileInfo(file).fileName()),
		                     QMessageBox::Ok);
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	QTextStream out(&file);

	out.setCodec("UTF-8");
	out << nowBrowserText;

	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
}

void StatisticsBrowser::exportStatstics(QWidget *widget, Contest *curContest)
{
	QString buffer;

	if (! curContest)
	{
		QMessageBox::warning(widget, tr("LemonLime"), tr("No contest yet"), QMessageBox::Ok);
		return;
	}

	QList<Task *> taskList = curContest->getTaskList();
	QList<Contestant *> contestantList = curContest->getContestantList();

	if (taskList.empty())
	{
		QMessageBox::warning(widget, tr("LemonLime"), tr("No task yet"), QMessageBox::Ok);
		return;
	}

	if (contestantList.empty())
	{
		QMessageBox::warning(widget, tr("LemonLime"), tr("No contestant yet"), QMessageBox::Ok);
		return;
	}

	QString filter = tr("HTML Document (*.html)");

	QString fileName = QFileDialog::getSaveFileName(widget, tr("Export Statstics"),
	                   QDir::currentPath() + QDir::separator() + "statstics.html", filter);

	if (fileName.isEmpty()) return;

	if (QFileInfo(fileName).suffix() == "html") exportStatsticsHtml(widget, fileName);

}
