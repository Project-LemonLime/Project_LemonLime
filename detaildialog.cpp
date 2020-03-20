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
 * detaildialog.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * detaildialog.cpp @Project LemonLime
 * Update 2019 iotang
 **/

#include "detaildialog.h"
#include "ui_detaildialog.h"
#include "task.h"
#include "testcase.h"
#include "contest.h"
#include "contestant.h"
#include "globaltype.h"
#include "judgingdialog.h"
#include <QMessageBox>
#include <QScrollBar>

DetailDialog::DetailDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::DetailDialog)
{
	ui->setupUi(this);
	connect(ui->detailViewer, SIGNAL(anchorClicked(QUrl)),
	        this, SLOT(anchorClicked(QUrl)));
}

DetailDialog::~DetailDialog()
{
	delete ui;
}

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
		htmlCode += QString(R"(%1 %2 (%5 / %6) <a href="Rejudge %3" style="text-decoration: none">(%4)</span>)")
		            .arg(tr("Task")).arg(taskList[i]->getProblemTile()).arg(i).arg(tr("Rejudge")).arg(contestant->getTaskScore(i)).arg(taskList[i]->getTotalScore());
	}

	htmlCode += "<HR>";

	for (int i = 0; i < taskList.size(); i ++)
	{
		htmlCode += "<p><span style=\"font-weight:bold; font-size:large;\">";
		htmlCode += QString(R"(%1 %2 (%5 / %6) <a href="Rejudge %3" style="text-decoration: none">(%4)</span><br>)")
		            .arg(tr("Task")).arg(taskList[i]->getProblemTile()).arg(i).arg(tr("Rejudge")).arg(contestant->getTaskScore(i)).arg(taskList[i]->getTotalScore());

		if (! contestant->getCheckJudged(i))
		{
			htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Not judged"));
			continue;
		}

		if (taskList[i]->getTaskType() == Task::Traditional || taskList[i]->getTaskType() == Task::Interaction || taskList[i]->getTaskType() == Task::Communication)
		{
			if (contestant->getCompileState(i) != CompileSuccessfully)
			{
				switch (contestant->getCompileState(i))
				{
					case NoValidSourceFile:
						htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Cannot find valid source file"));
						break;

					case CompileTimeLimitExceeded:
						htmlCode += QString("&nbsp;&nbsp;%1%2<br>").arg(tr("Source file: ")).arg(contestant->getSourceFile(i));
						htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Compile time limit exceeded"));
						break;

					case InvalidCompiler:
						htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Cannot run given compiler"));
						break;

					case CompileError:
						htmlCode += QString("&nbsp;&nbsp;%1%2<br>").arg(tr("Source file: ")).arg(contestant->getSourceFile(i));
						htmlCode += QString("&nbsp;&nbsp;%1").arg(tr("Compile error"));

						if (! contestant->getCompileMessage(i).isEmpty())
							htmlCode += QString(R"(<a href="CompileMessage %1" style="text-decoration: none"> (...))").arg(i);

						htmlCode += "</p>";
						break;

					case CompileSuccessfully:
						break;
				}

				continue;
			}

			htmlCode += QString("&nbsp;&nbsp;%1%2").arg(tr("Source file: ")).arg(contestant->getSourceFile(i));
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
		QList< QList<ResultState>> result = contestant->getResult(i);
		QList<QStringList> message = contestant->getMessage(i);
		QList< QList<int>> timeUsed = contestant->getTimeUsed(i);
		QList< QList<int>> memoryUsed = contestant->getMemoryUsed(i);
		QList< QList<int>> score = contestant->getSocre(i);

		for (int j = 0; j < inputFiles.size(); j ++)
		{
			for (int k = 0; k < inputFiles[j].size(); k ++)
			{
				htmlCode += "<tr>";

				if (k == 0)
				{
					if (score[j].size() == inputFiles[j].size())
						htmlCode += QString(R"(<td nowrap="nowrap" rowspan="%1" align="center" valign="middle">#%2</td>)")
						            .arg(inputFiles[j].size()).arg(j + 1);
					else
						htmlCode += QString(R"(<td nowrap="nowrap" rowspan="%1" align="center" valign="middle">#%2<br>%3:%4</td>)")
						            .arg(inputFiles[j].size()).arg(j + 1).arg(tr("Subtask Dependence Status")).arg(score[j].back() == -1 ? tr("Success") : tr("Failed"));
				}

				htmlCode += QString(R"(<td nowrap="nowrap" align="center" valign="middle">%1</td>)").arg(inputFiles[j][k]);

				QString text;
				QString bgColor = "rgb(255, 255, 255)";
				QString FrColor = "rgb(0, 0, 0)";

				switch (result[j][k])
				{
					case CorrectAnswer:
						text = tr("Correct Answer");
						bgColor = "rgb(192, 255, 192)";
						break;

					case WrongAnswer:
						text = tr("Wrong Answer");
						bgColor = "rgb(255, 192, 192)";
						break;

					case PartlyCorrect:
						text = tr("Partly Correct");
						bgColor = "rgb(192, 255, 255)";
						break;

					case TimeLimitExceeded:
						text = tr("Time Limit Exceeded");
						bgColor = "rgb(255, 255, 192)";
						break;

					case MemoryLimitExceeded:
						text = tr("Memory Limit Exceeded");
						bgColor = "rgb(192, 192, 255)";
						break;

					case CannotStartProgram:
						text = tr("Cannot Start Program");
						FrColor = "rgb(255, 64, 64)";
						bgColor = "rgb(192, 192, 192)";
						break;

					case FileError:
						text = tr("File Error");
						FrColor = "rgb(255, 255, 64)";
						bgColor = "rgb(192, 192, 192)";
						break;

					case RunTimeError:
						text = tr("Run Time Error");
						bgColor = "rgb(255, 192, 255)";
						break;

					case InvalidSpecialJudge:
						text = tr("Invalid Special Judge");
						FrColor = "rgb(255, 255, 255)";
						bgColor = "rgb(128, 0, 0)";
						break;

					case SpecialJudgeTimeLimitExceeded:
						text = tr("Special Judge Time Limit Exceeded");
						FrColor = "rgb(255, 255, 255)";
						bgColor = "rgb(128, 128, 0)";
						break;

					case SpecialJudgeRunTimeError:
						text = tr("Special Judge Run Time Error");
						FrColor = "rgb(255, 255, 255)";
						bgColor = "rgb(128, 0, 128)";
						break;

					case Skipped:
						text = tr("Skipped");
						FrColor = "rgb(192, 192, 192)";
						bgColor = "rgb(255, 255, 255)";
						break;

					case InteractorError:
						text = tr("Interactor Error");
						FrColor = "rgb(255, 255, 255)";
						bgColor = "rgb(0, 0, 128)";
						break;
				}

				htmlCode += QString(R"(<td align="center" valign="middle" style="background-color: %2; color: %3;">%1)").arg(text).arg(bgColor).arg(FrColor);

				if (! message[j][k].isEmpty())
				{
					htmlCode += QString(R"(<a href="Message %1 %2 %3" style="text-decoration: none"> (...)</a>)").arg(i).arg(j).arg(k);
				}

				htmlCode += "</td>";

				htmlCode += R"(<td nowrap="nowrap" align="center" valign="middle">)";

				if (timeUsed[j][k] != -1)
				{
					htmlCode += QString("").asprintf("%.3lf s", double (timeUsed[j][k]) / 1000);
				}
				else
				{
					htmlCode += tr("Invalid");
				}

				htmlCode += "</td>";

				htmlCode += R"(<td nowrap="nowrap" align="center" valign="middle">)";

				if (memoryUsed[j][k] != -1)
				{
					htmlCode += QString("").asprintf("%.3lf MB", double (memoryUsed[j][k]) / 1024 / 1024);
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

					for (int t = 0; t < inputFiles[j].size(); t ++)
						if (score[j][t] < minv) minv = score[j][t];

					QString bgColor = "rgb(255, 192, 192)";

					if (minv >= maxv) bgColor = "rgb(192, 255, 192)";
					else if (minv > 0) bgColor = "rgb(192, 255, 255)";

					htmlCode += QString(R"(<td rowspan="%1" align="center" valign="middle" style="background-color: %2;"><a style="font-weight: bold; font-size: 14px;">%3</a> / %4</td>)").arg(inputFiles[j].size()).arg(bgColor).arg(minv).arg(maxv);
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
	QStringList list = url.path().split(' ', QString::SkipEmptyParts);

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
		            QMessageBox::Close, this).exec();
	}

	if (list[0] == "Message")
	{
		QList<QStringList> message = contestant->getMessage(list[1].toInt());
		QMessageBox(QMessageBox::NoIcon, tr("Message"),
		            QString("<code>%1<br></code>").arg(message[list[2].toInt()][list[3].toInt()]),
		            QMessageBox::Close, this).exec();
	}
}
