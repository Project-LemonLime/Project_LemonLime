/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "detaildialog.h"
#include "ui_detaildialog.h"
//
#include "base/LemonType.hpp"
#include "base/settings.h"
#include "core/contest.h"
#include "core/contestant.h"
#include "core/subtaskdependencelib.h"
#include "core/task.h"
#include "core/testcase.h"
#include "judgingdialog.h"
//
#include <QMessageBox>
#include <QScrollBar>

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define QT_SkipEmptyParts Qt::SkipEmptyParts
#else
#define QT_SkipEmptyParts QString::SkipEmptyParts
#endif

DetailDialog::DetailDialog(QWidget *parent) : QDialog(parent), ui(new Ui::DetailDialog)
{
	ui->setupUi(this);
	connect(ui->detailViewer, &QTextBrowser::anchorClicked, this, &DetailDialog::anchorClicked);
}

DetailDialog::~DetailDialog() { delete ui; }

void DetailDialog::refreshViewer(Contest *_contest, Contestant *_contestant)
{
	contest = _contest;
	contestant = _contestant;
	setWindowTitle(tr("Contestant: %1").arg(contestant->getContestantName()));
	ui->detailViewer->clear();
	QString htmlCode;
	htmlCode += "<html><head>";
	htmlCode += "<style type=\"text/css\">th, td {padding-left: 1em; padding-right: 1em;}</style>";
	htmlCode += "</head><body>";
	QList<Task *> taskList = contest->getTaskList();

	for (int i = 0; i < taskList.size(); i++)
	{
		htmlCode += "<p><span style=\"font-weight:bold; font-size:large;\">";
		htmlCode +=
		    QString(R"(%1 %2 (%5 / %6) <a href="Rejudge %3" style="text-decoration: none">(%4)</span>)")
		        .arg(tr("Task"))
		        .arg(taskList[i]->getProblemTile())
		        .arg(i)
		        .arg(tr("Rejudge"))
		        .arg(contestant->getTaskScore(i))
		        .arg(taskList[i]->getTotalScore());
	}

	htmlCode += "<HR>";

	for (int i = 0; i < taskList.size(); i++)
	{
		htmlCode += "<p><span style=\"font-weight:bold; font-size:large;\">";
		htmlCode +=
		    QString(R"(%1 %2 (%5 / %6) <a href="Rejudge %3" style="text-decoration: none">(%4)</span><br>)")
		        .arg(tr("Task"))
		        .arg(taskList[i]->getProblemTile())
		        .arg(i)
		        .arg(tr("Rejudge"))
		        .arg(contestant->getTaskScore(i))
		        .arg(taskList[i]->getTotalScore());

		if (! contestant->getCheckJudged(i))
		{
			htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Not judged"));
			continue;
		}

		if (taskList[i]->getTaskType() == Task::Traditional ||
		    taskList[i]->getTaskType() == Task::Interaction ||
		    taskList[i]->getTaskType() == Task::Communication)
		{
			if (contestant->getCompileState(i) != CompileSuccessfully)
			{
				switch (contestant->getCompileState(i))
				{
					case NoValidSourceFile:
						htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Cannot find valid source file"));
						break;

					case CompileTimeLimitExceeded:
						htmlCode += QString("&nbsp;&nbsp;%1%2<br>")
						                .arg(tr("Source file: "))
						                .arg(contestant->getSourceFile(i));
						htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Compile time limit exceeded"));
						break;

					case InvalidCompiler:
						htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Cannot run given compiler"));
						break;

					case CompileError:
						htmlCode += QString("&nbsp;&nbsp;%1%2<br>")
						                .arg(tr("Source file: "))
						                .arg(contestant->getSourceFile(i));
						htmlCode += QString("&nbsp;&nbsp;%1").arg(tr("Compile error"));

						if (! contestant->getCompileMessage(i).isEmpty())
							htmlCode +=
							    QString(R"(<a href="CompileMessage %1" style="text-decoration: none"> (...))")
							        .arg(i);

						htmlCode += "</p>";
						break;

					case CompileSuccessfully:
						break;
				}

				continue;
			}

			htmlCode +=
			    QString("&nbsp;&nbsp;%1%2").arg(tr("Source file: ")).arg(contestant->getSourceFile(i));
		}

		htmlCode += R"(<table width="100%" cellpadding="1" border="1"><tr>)";
		htmlCode += QString(R"(<th scope="col" nowrap="nowrap">%1</th>)").arg(tr("Test Case"));
		htmlCode += QString(R"(<th scope="col" nowrap="nowrap">%1</th>)").arg(tr("Input File"));
		htmlCode += QString("<th scope=\"col\">%1</th>").arg(tr("Result"));
		htmlCode += QString(R"(<th scope="col" nowrap="nowrap">%1</th>)").arg(tr("Time Used"));
		htmlCode += QString(R"(<th scope="col" nowrap="nowrap">%1</th>)").arg(tr("Memory Used"));
		htmlCode += QString(R"(<th scope="col" nowrap="nowrap">%1</th></tr>)").arg(tr("Score"));
		QList<TestCase *> testCases = taskList[i]->getTestCaseList();
		QList<QStringList> inputFiles = contestant->getInputFiles(i);
		QList<QList<ResultState>> result = contestant->getResult(i);
		QList<QStringList> message = contestant->getMessage(i);
		QList<QList<int>> timeUsed = contestant->getTimeUsed(i);
		QList<QList<int>> memoryUsed = contestant->getMemoryUsed(i);
		QList<QList<int>> score = contestant->getSocre(i);

		for (int j = 0; j < inputFiles.size(); j++)
		{
			for (int k = 0; k < inputFiles[j].size(); k++)
			{
				htmlCode += "<tr>";

				if (k == 0)
				{
					if (score[j].size() == inputFiles[j].size())
						htmlCode +=
						    QString(
						        R"(<td nowrap="nowrap" rowspan="%1" align="center" valign="middle">#%2</td>)")
						        .arg(inputFiles[j].size())
						        .arg(j + 1);
					else
						htmlCode +=
						    QString(
						        R"(<td nowrap="nowrap" rowspan="%1" align="center" valign="middle">#%2<br>%3:%4</td>)")
						        .arg(inputFiles[j].size())
						        .arg(j + 1)
						        .arg(tr("Subtask Dependence Status"))
						        .arg(statusRankingText(score[j].back()));
				}

				htmlCode += QString(R"(<td nowrap="nowrap" align="center" valign="middle">%1</td>)")
				                .arg(inputFiles[j][k]);
				QString text;
				QString bgColor = "rgb(255, 255, 255)";
				QString frColor = "rgb(0, 0, 0)";
				Settings::setTextAndColor(result[j][k], text, frColor, bgColor);
				htmlCode +=
				    QString(
				        R"(<td align="center" valign="middle" style="background-color: %2; color: %3;">%1)")
				        .arg(text)
				        .arg(bgColor)
				        .arg(frColor);

				if (! message[j][k].isEmpty())
				{
					htmlCode +=
					    QString(R"(<a href="Message %1 %2 %3" style="text-decoration: none"> (...)</a>)")
					        .arg(i)
					        .arg(j)
					        .arg(k);
				}

				htmlCode += "</td>";
				htmlCode += R"(<td nowrap="nowrap" align="center" valign="middle">)";

				if (timeUsed[j][k] != -1)
				{
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
					htmlCode += QString("").asprintf("%.3lf s", double(timeUsed[j][k]) / 1000);
#else
					htmlCode += QString("").sprintf("%.3lf s", double(timeUsed[j][k]) / 1000);
#endif
				}
				else
				{
					htmlCode += tr("Invalid");
				}

				htmlCode += "</td>";
				htmlCode += R"(<td nowrap="nowrap" align="center" valign="middle">)";

				if (memoryUsed[j][k] != -1)
				{
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
					htmlCode += QString("").asprintf("%.3lf MB", double(memoryUsed[j][k]) / 1024 / 1024);
#else
					htmlCode += QString("").sprintf("%.3lf MB", double(memoryUsed[j][k]) / 1024 / 1024);
#endif
				}
				else
				{
					htmlCode += tr("Invalid");
				}

				htmlCode += "</td>";

				if (k == 0)
				{
					int minv = 2147483647;
					int maxv = testCases[j]->getFullScore();

					for (int t = 0; t < inputFiles[j].size(); t++)
						if (score[j][t] < minv)
							minv = score[j][t];

					int tempStatus = mxDependValue + 1;

					if (! testCases[j]->getDependenceSubtask().empty())
					{
						tempStatus = score[j].back();
						minv = qMin(minv, statusToScore(score[j].back(), maxv));
					}

					QString bgColor = "rgb(192, 255, 192)";

					for (int t = 0; t < inputFiles[j].size(); t++)
						if (result[j][t] != CorrectAnswer)
							bgColor = "rgb(192, 255, 255)";

					if (tempStatus < mxDependValue)
						bgColor = "rgb(192, 255, 255)";

					for (int t = 0; t < inputFiles[j].size(); t++)
						if (result[j][t] != CorrectAnswer && result[j][t] != PartlyCorrect)
							bgColor = "rgb(255, 192, 192)";

					if (tempStatus < 0)
						bgColor = "rgb(255, 192, 192)";

					htmlCode +=
					    QString(
					        R"(<td rowspan="%1" align="center" valign="middle" style="background-color: %2;"><a style="font-weight: bold; font-size: large;">%3</a> / %4</td>)")
					        .arg(inputFiles[j].size())
					        .arg(bgColor)
					        .arg(minv)
					        .arg(maxv);
				}

				htmlCode += "</tr>";
			}
		}

		htmlCode += "</table><br></p>";
	}

	htmlCode += "</body></html>";
	ui->detailViewer->setHtml(htmlCode);
}

void DetailDialog::showDialog()
{
	show();
	QScrollBar *bar = ui->detailViewer->verticalScrollBar();
	bar->setValue(bar->minimum());
	bar = ui->detailViewer->horizontalScrollBar();
	bar->setValue(bar->minimum());
	exec();
}

void DetailDialog::anchorClicked(const QUrl &url)
{
	QStringList list = url.path().split(' ', QT_SkipEmptyParts);

	if (list[0] == "Rejudge")
	{
		auto *dialog = new JudgingDialog(this);
		dialog->setModal(true);
		dialog->setContest(contest);
		dialog->show();
		dialog->judge(contestant->getContestantName(), list[1].toInt());
		delete dialog;
		emit rejudgeSignal();
		refreshViewer(contest, contestant);
	}

	if (list[0] == "CompileMessage")
	{
		QMessageBox(QMessageBox::NoIcon, tr("Compile Message"),
		            QString("<code>%1</code>").arg(contestant->getCompileMessage(list[1].toInt())),
		            QMessageBox::Close, this)
		    .exec();
	}

	if (list[0] == "Message")
	{
		QList<QStringList> message = contestant->getMessage(list[1].toInt());
		QMessageBox(QMessageBox::NoIcon, tr("Message"),
		            QString("<code>%1<br></code>").arg(message[list[2].toInt()][list[3].toInt()]),
		            QMessageBox::Close, this)
		    .exec();
	}
}
