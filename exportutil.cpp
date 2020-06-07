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
 * exportutil.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * exportutil.cpp @Project LemonLime
 * Update 2019 iotang
 **/

#include "exportutil.h"
#include "contest.h"
#include "contestant.h"
#include "globaltype.h"
#include "settings.h"
#include "subtaskdependencelib.h"
#include "task.h"
#include "testcase.h"
#include "visualsettings.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <algorithm>

ExportUtil::ExportUtil(QObject *parent) : QObject(parent) {}

auto ExportUtil::getContestantHtmlCode(Contest *contest, Contestant *contestant, int num) -> QString
{
	QString htmlCode;
	QList<Task *> taskList = contest->getTaskList();

	for (int i = 0; i < taskList.size(); i++)
	{
		htmlCode +=
		   QString(R"(<a name="c%1p%2"><p><span style="font-weight:bold; font-size:large;">)").arg(num).arg(i);
		htmlCode += QString("%1 %2</span><br>").arg(tr("Task")).arg(taskList[i]->getProblemTile());

		if (! contestant->getCheckJudged(i))
		{
			htmlCode += QString("&nbsp;&nbsp;%1</p></a>").arg(tr("Not judged"));
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
						htmlCode += QString("&nbsp;&nbsp;%1</p></a>").arg(tr("Cannot find valid source file"));
						break;

					case CompileTimeLimitExceeded:
						htmlCode += QString("&nbsp;&nbsp;%1%2<br>")
						               .arg(tr("Source file: "))
						               .arg(contestant->getSourceFile(i));
						htmlCode += QString("&nbsp;&nbsp;%1</p></a>").arg(tr("Compile time limit exceeded"));
						break;

					case InvalidCompiler:
						htmlCode += QString("&nbsp;&nbsp;%1</p></a>").arg(tr("Cannot run given compiler"));
						break;

					case CompileError:
						htmlCode += QString("&nbsp;&nbsp;%1%2<br>")
						               .arg(tr("Source file: "))
						               .arg(contestant->getSourceFile(i));
						htmlCode += QString("&nbsp;&nbsp;%1").arg(tr("Compile error"));

						if (! contestant->getCompileMessage(i).isEmpty())
						{
							QString compileMessage = contestant->getCompileMessage(i);
							compileMessage.replace("\r\n", "<br>");
							compileMessage.replace("\n", "<br>");
							compileMessage.replace("\r", "<br>");

							if (compileMessage.endsWith("<br>"))
								compileMessage.chop(4);

							htmlCode += R"(<table border="1"  cellpadding="1">)";
							htmlCode += "<tr><td style=\"padding: 0.5em; text-align: left;\"><code>";
							htmlCode += compileMessage;
							htmlCode += "</code></td></tr></table>";
						}

						htmlCode += "</p></a>";
						break;

					default:
						break;
				}

				continue;
			}

			htmlCode += QString("&nbsp;&nbsp;%1%2").arg(tr("Source file: ")).arg(contestant->getSourceFile(i));
		}

		htmlCode += "<table style=\"border: solid;\"><tr>";
		htmlCode +=
		   QString(
		      R"(<th scope="col" style="border-style: none solid solid none; border-width: 1px 3px; border-color: #ccc;">%1</th>)")
		      .arg(tr("Test Case"));
		htmlCode +=
		   QString(
		      R"(<th scope="col" style="border-style: none solid solid none; border-width: 1px 3px; border-color: #ccc;">%1</th>)")
		      .arg(tr("Input File"));
		htmlCode +=
		   QString(
		      R"(<th scope="col" style="border-style: none solid solid none; border-width: 1px 3px; border-color: #ccc;">%1</th>)")
		      .arg(tr("Result"));
		htmlCode +=
		   QString(
		      R"(<th scope="col" style="border-style: none solid solid none; border-width: 1px 3px; border-color: #ccc;">%1</th>)")
		      .arg(tr("Time Used"));
		htmlCode +=
		   QString(
		      R"(<th scope="col" style="border-style: none solid solid none; border-width: 1px 3px; border-color: #ccc;">%1</th>)")
		      .arg(tr("Memory Used"));
		htmlCode +=
		   QString(
		      R"(<th scope="col" style="border-style: none solid solid none; border-width: 1px 3px; border-color: #ccc;">%1</th></tr>)")
		      .arg(tr("Score"));
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
						      R"(<td rowspan="%1" style="border-style: none solid solid none; border-width: 1px 3px; border-color: #ccc;">#%2</td>)")
						      .arg(inputFiles[j].size())
						      .arg(j + 1);
					else
						htmlCode +=
						   QString(
						      R"(<td rowspan="%1" style="border-style: none solid solid none; border-width: 1px 3px; border-color: #ccc;">#%2<br>%3:%4</td>)")
						      .arg(inputFiles[j].size())
						      .arg(j + 1)
						      .arg(tr("Subtask Dependence Status"))
						      .arg(statusRankingText(score[j].back()));
				}

				htmlCode += QString("<td style=\"border-style: none solid solid none; border-width: 1px 3px; "
				                    "border-color: #ccc;\">%1</td>")
				               .arg(inputFiles[j][k]);
				QString text;
				QString bgColor = "rgb(255, 255, 255)";
				QString frColor = "rgb(0, 0, 0)";
				Settings::setTextAndColor(result[j][k], text, frColor, bgColor);
				htmlCode += QString("<td style=\"border-style: none solid solid none; border-width: 1px 3px; "
				                    "border-color: #ccc; background-color: %2; color: %3;\">%1")
				               .arg(text)
				               .arg(bgColor)
				               .arg(frColor);

				if (! message[j][k].isEmpty())
				{
					QString tmp = message[j][k];
					tmp.replace("\n", "\\n");
					tmp.replace("\"", "\\&quot;");
					htmlCode += QString("<a href=\"javascript:alert(&quot;%1&quot;)\"> (...)").arg(tmp);
				}

				htmlCode += "</td>";
				htmlCode += "<td style=\"border-style: none solid solid none; border-width: 1px 3px; "
				            "border-color: #ccc;\">";

				if (timeUsed[j][k] != -1)
				{
					htmlCode += QString("").asprintf("%.3lf s", double(timeUsed[j][k]) / 1000);
				}
				else
				{
					htmlCode += tr("Invalid");
				}

				htmlCode += "</td>";
				htmlCode += "<td style=\"border-style: none solid solid none; border-width: 1px 3px; "
				            "border-color: #ccc;\">";

				if (memoryUsed[j][k] != -1)
				{
					htmlCode += QString("").asprintf("%.3lf MB", double(memoryUsed[j][k]) / 1024 / 1024);
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

					QString bgColor = "rgb(255, 192, 192)";

					if (minv >= maxv)
						bgColor = "rgb(192, 255, 192)";
					else if (minv > 0)
						bgColor = "rgb(192, 255, 255)";

					htmlCode +=
					   QString(
					      R"(<td rowspan="%1" style="border-style: none solid solid none; border-width: 1px 3px; border-color: #ccc; background-color: %2;"><a style="font-weight: bold; font-size: large;">%3</a> / %4</td>)")
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

	htmlCode += QString("<p><a href=\"#top\">%1</a></p>").arg(tr("Return to top"));
	return htmlCode;
}

void ExportUtil::exportHtml(QWidget *widget, Contest *contest, const QString &fileName, int isDefaultTheme)
{
	Settings colors;
	contest->copySettings(colors);

	if (isDefaultTheme)
	{
		VisualSettings tempVisualSettings;
		tempVisualSettings.resetEditSettings(&colors);
		tempVisualSettings.resetToDefault();
	}

	QFile file(fileName);

	if (! file.open(QFile::WriteOnly))
	{
		QMessageBox::warning(widget, tr("LemonLime"), tr("Cannot open file %1").arg(QFileInfo(file).fileName()),
		                     QMessageBox::Ok);
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	QTextStream out(&file);
	QList<Contestant *> contestantList = contest->getContestantList();
	QList<Task *> taskList = contest->getTaskList();
	out.setCodec("UTF-8");
	out << "<html><head>";
	out << R"(<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />)";
	out << "<style type=\"text/css\">th, td {padding-left: 1em; padding-right: 1em; white-space: nowrap; "
	       "text-align: center; verticle-align: middle;}</style>";
	out << "<title>" << tr("Contest Result") << "</title>";
	out << "</head><body>";
	QList<QPair<int, QString>> sortList;

	for (auto &i : contestantList)
	{
		int totalScore = i->getTotalScore();

		if (totalScore != -1)
		{
			sortList.append(qMakePair(-totalScore, i->getContestantName()));
		}
		else
		{
			sortList.append(qMakePair(1, i->getContestantName()));
		}
	}

	std::sort(sortList.begin(), sortList.end());
	QMap<QString, int> rankList;

	for (int i = 0; i < sortList.size(); i++)
	{
		if (i > 0 && sortList[i].first == sortList[i - 1].first)
		{
			rankList.insert(sortList[i].second, rankList[sortList[i - 1].second]);
		}
		else
		{
			rankList.insert(sortList[i].second, i);
		}
	}

	QHash<Contestant *, int> loc;

	for (int i = 0; i < contestantList.size(); i++)
	{
		loc.insert(contestantList[i], i);
	}

	out << "<p><span style=\"font-size:x-large; font-weight:bold;\">";
	out << "<a name=\"top\"></a>" << tr("Rank List") << "</span></p>";
	out << "<p style=\"font-size: small;\">"
	    << tr("Click names or task scores to jump to details. Judged By LemonLime") << "</p>";
	out << R"(<p><table cellpadding="1" style="border-style: solid;"><tr>)";
	out
	   << QString(
	         R"(<th style="border-style: none solid solid none; border-width: 3px 3px; border-color: #000;" scope="col">%1</th>)")
	         .arg(tr("Rank"));
	out
	   << QString(
	         R"(<th style="border-style: none solid solid none; border-width: 3px 3px; border-color: #000;" scope="col">%1</th>)")
	         .arg(tr("Name"));
	out
	   << QString(
	         R"(<th style="border-style: none solid solid none; border-width: 3px 2px; border-color: #000;" scope="col">%1</th>)")
	         .arg(tr("Total Score"));

	for (auto &i : taskList)
		out
		   << QString(
		         R"(<th style="border-style: none solid solid none; border-width: 3px 2px; border-color: #000;" scope="col">%1</th>)")
		         .arg(i->getProblemTile());

	out << "</tr>";
	QList<int> fullScore;
	int sfullScore = contest->getTotalScore();

	for (auto &i : taskList)
	{
		fullScore.append(i->getTotalScore());
	}

	for (auto &i : sortList)
	{
		Contestant *contestant = contest->getContestant(i.second);
		out << QString("<tr><td style=\"border-style: none solid solid none; border-width: 1px 3px; "
		               "border-color: #ccc;\">%1</td>")
		          .arg(rankList[contestant->getContestantName()] + 1);
		out
		   << QString(
		         R"(<td style="border-style: none solid solid none; border-width: 1px 3px; border-color: #ccc;"><a href="#c%1" style="color: black; text-decoration: none;">%2</a></td>)")
		         .arg(loc[contestant])
		         .arg(i.second);
		int allScore = contestant->getTotalScore();

		if (allScore >= 0)
		{
			double h;
			double s;
			double l;
			colors.getColorGrand(allScore, sfullScore).getHslF(&h, &s, &l);
			h *= 360, s *= 100, l *= 100;
			out << QString("<td style=\"background-color: hsl(%2,%3%,%4%); border-radius: 5px; font-weight: "
			               "bold; border: 2px solid hsl(%2,%3%,%5%);\">%1</td>")
			          .arg(allScore)
			          .arg(h)
			          .arg(s)
			          .arg(l)
			          .arg(qMax(l - 20, 0.00));
		}
		else
		{
			out << QString("<td style=\"border-radius: 5px; font-weight: bold;\">%1</td>").arg(tr("Invalid"));
		}

		for (int j = 0; j < taskList.size(); j++)
		{
			int score = contestant->getTaskScore(j);

			if (score != -1)
			{
				double h;
				double s;
				double l;
				QColor col = colors.getColorPer(score, fullScore[j]);
				col.getHslF(&h, &s, &l);

				if (taskList[j]->getTaskType() != Task::AnswersOnly &&
				    contestant->getCompileState(j) != CompileSuccessfully)
				{
					if (contestant->getCompileState(j) == NoValidSourceFile)
					{
						colors.getColorNf().getHslF(&h, &s, &l);
					}
					else
					{
						colors.getColorCe().getHslF(&h, &s, &l);
					}
				}

				h *= 360, s *= 100, l *= 100;
				out
				   << QString(
				         R"(<td style="background-color: hsl(%2,%3%,%4%); border-radius: 5px;"><a href="#c%5p%6" style="color: black; text-decoration: none;">%1</a></td>)")
				         .arg(score)
				         .arg(h)
				         .arg(s)
				         .arg(l)
				         .arg(loc[contestant])
				         .arg(j);
			}
			else
			{
				out
				   << QString(
				         R"(<td style="border-radius: 5px;"><a href="#c%2p%3" style="color: black; text-decoration: none;">%1</a></td>)")
				         .arg(tr("Invalid"))
				         .arg(loc[contestant])
				         .arg(j);
			}
		}
	}

	out << "</table></p>";

	for (int i = 0; i < contestantList.size(); i++)
	{
		out << QString("<a name=\"c%1\"><hr><a>").arg(i)
		    << "<span style=\"font-size:x-large; font-weight:bold;\">";
		out << tr("Contestant: %1").arg(contestantList[i]->getContestantName()) << "</span>";
		out << getContestantHtmlCode(contest, contestantList[i], i);
	}

	out << "</body></html>";
	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
}

auto ExportUtil::getSmallerContestantHtmlCode(Contest *contest, Contestant *contestant) -> QString
{
	QString htmlCode;
	QList<Task *> taskList = contest->getTaskList();

	for (int i = 0; i < taskList.size(); i++)
	{
		htmlCode += "<p><span style=\"font-weight:bold; font-size:large;\">";
		htmlCode += QString("%1 %2</span><br>").arg(tr("Task")).arg(taskList[i]->getProblemTile());

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
						{
							QString compileMessage = contestant->getCompileMessage(i);
							compileMessage.replace("\r\n", "<br>");
							compileMessage.replace("\n", "<br>");
							compileMessage.replace("\r", "<br>");

							if (compileMessage.endsWith("<br>"))
								compileMessage.chop(4);

							htmlCode += R"(<table border="1" cellpadding="1">)";
							htmlCode += "<tr><td style=\"padding: 0.5em; text-align: left;\"><code>";
							htmlCode += compileMessage;
							htmlCode += "</code></td></tr></table>";
						}

						htmlCode += "</p>";
						break;

					default:
						break;
				}

				continue;
			}

			htmlCode += QString("&nbsp;&nbsp;%1%2").arg(tr("Source file: ")).arg(contestant->getSourceFile(i));
		}

		htmlCode += R"(<table border="1"  cellpadding="1"><tr>)";
		htmlCode += QString("<th scope=\"col\">%1</th>").arg(tr("Test Case"));
		htmlCode += QString("<th scope=\"col\">%1</th>").arg(tr("Input File"));
		htmlCode += QString("<th scope=\"col\">%1</th>").arg(tr("Result"));
		htmlCode += QString("<th scope=\"col\">%1</th>").arg(tr("Time Used"));
		htmlCode += QString("<th scope=\"col\">%1</th>").arg(tr("Memory Used"));
		htmlCode += QString("<th scope=\"col\">%1</th></tr>").arg(tr("Score"));
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
						htmlCode += QString("<td rowspan=\"%1\">#%2</td>").arg(inputFiles[j].size()).arg(j + 1);
					else
						htmlCode += QString("<td rowspan=\"%1\">#%2<br>%3:%4</td>")
						               .arg(inputFiles[j].size())
						               .arg(j + 1)
						               .arg(tr("Subtask Dependence Status"))
						               .arg(statusRankingText(score[j].back()));
				}

				htmlCode += QString("<td>%1</td>").arg(inputFiles[j][k]);
				QString text;
				QString bgColor;
				QString frColor;
				Settings::setTextAndColor(result[j][k], text, frColor, bgColor);
				htmlCode += QString("<td>%1").arg(text);

				if (! message[j][k].isEmpty())
				{
					QString tmp = message[j][k];
					tmp.replace("\n", "\\n");
					tmp.replace("\"", "\\&quot;");
					htmlCode += QString("<a href=\"javascript:alert(&quot;%1&quot;)\"> (...)").arg(tmp);
				}

				htmlCode += "</td>";
				htmlCode += "<td>";

				if (timeUsed[j][k] != -1)
				{
					htmlCode += QString("").asprintf("%.3lf s", double(timeUsed[j][k]) / 1000);
				}
				else
				{
					htmlCode += tr("Invalid");
				}

				htmlCode += "</td>";
				htmlCode += "<td>";

				if (memoryUsed[j][k] != -1)
				{
					htmlCode += QString("").asprintf("%.3lf MB", double(memoryUsed[j][k]) / 1024 / 1024);
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

					htmlCode +=
					   QString(
					      R"(<td rowspan="%1"><a style="font-weight: bold; font-size: large;">%2</a> / %3</td>)")
					      .arg(inputFiles[j].size())
					      .arg(minv)
					      .arg(maxv);
				}

				htmlCode += "</tr>";
			}
		}

		htmlCode += "</table><br></p>";
	}

	htmlCode += QString("<p><a href=\"#top\">%1</a></p>").arg(tr("Return to top"));
	return htmlCode;
}

void ExportUtil::exportSmallerHtml(QWidget *widget, Contest *contest, const QString &fileName)
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
	QList<Contestant *> contestantList = contest->getContestantList();
	QList<Task *> taskList = contest->getTaskList();
	out.setCodec("UTF-8");
	out << "<html><head>";
	out << R"(<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />)";
	out << "<style type=\"text/css\">th, td {padding-left: 1em; padding-right: 1em; white-space: nowrap; "
	       "text-align: center; verticle-align: middle;}</style>";
	out << "<title>" << tr("Contest Result") << "</title>";
	out << "</head><body>";
	QList<QPair<int, QString>> sortList;

	for (auto &i : contestantList)
	{
		int totalScore = i->getTotalScore();

		if (totalScore != -1)
		{
			sortList.append(qMakePair(-totalScore, i->getContestantName()));
		}
		else
		{
			sortList.append(qMakePair(1, i->getContestantName()));
		}
	}

	std::sort(sortList.begin(), sortList.end());
	QMap<QString, int> rankList;

	for (int i = 0; i < sortList.size(); i++)
	{
		if (i > 0 && sortList[i].first == sortList[i - 1].first)
		{
			rankList.insert(sortList[i].second, rankList[sortList[i - 1].second]);
		}
		else
		{
			rankList.insert(sortList[i].second, i);
		}
	}

	QHash<Contestant *, int> loc;

	for (int i = 0; i < contestantList.size(); i++)
	{
		loc.insert(contestantList[i], i);
	}

	out << "<p><span style=\"font-size:x-large; font-weight:bold;\">";
	out << "<a name=\"top\"></a>" << tr("Rank List") << "</span></p>";
	out << "<p style=\"font-size: small;\">" << tr("Judged By LemonLime") << "</p>";
	out << R"(<p><table border="1" cellpadding="1"><tr>)";
	out << QString("<th scope=\"col\">%1</th>").arg(tr("Rank"));
	out << QString("<th scope=\"col\">%1</th>").arg(tr("Name"));
	out << QString("<th scope=\"col\">%1</th>").arg(tr("Total Score"));

	for (auto &i : taskList)
		out << QString("<th scope=\"col\">%1</th>").arg(i->getProblemTile());

	out << QString("</tr>");
	QList<int> fullScore;

	for (auto &i : taskList)
	{
		int a = i->getTotalScore();
		fullScore.append(a);
	}

	for (auto &i : sortList)
	{
		Contestant *contestant = contest->getContestant(i.second);
		out << QString("<tr><td>%1</td>").arg(rankList[contestant->getContestantName()] + 1);
		out << QString("<td><a href=\"#c%1\">%2</a></td>").arg(loc[contestant]).arg(i.second);
		int allScore = contestant->getTotalScore();

		if (allScore != -1)
		{
			out << QString("<td style=\"font-weight: bold;\">%1</td>").arg(allScore);
		}
		else
		{
			out << QString("<td style=\"font-weight: bold;\">%1</td>").arg(tr("Invalid"));
		}

		for (int j = 0; j < taskList.size(); j++)
		{
			int score = contestant->getTaskScore(j);

			if (score != -1)
			{
				out << QString("<td>%1</td>").arg(score);
			}
			else
			{
				out << QString("<td>%1</td>").arg(tr("Invalid"));
			}
		}
	}

	out << "</table></p>";

	for (int i = 0; i < contestantList.size(); i++)
	{
		out << QString("<a name=\"c%1\"><hr><a>").arg(i)
		    << "<span style=\"font-size:x-large; font-weight:bold;\">";
		out << tr("Contestant: %1").arg(contestantList[i]->getContestantName()) << "</span>";
		out << getSmallerContestantHtmlCode(contest, contestantList[i]);
	}

	out << "</body></html>";
	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
}

void ExportUtil::exportCsv(QWidget *widget, Contest *contest, const QString &fileName)
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
	QList<Contestant *> contestantList = contest->getContestantList();
	QList<Task *> taskList = contest->getTaskList();
	QList<QPair<int, QString>> sortList;

	for (auto &i : contestantList)
	{
		int totalScore = i->getTotalScore();

		if (totalScore != -1)
		{
			sortList.append(qMakePair(-totalScore, i->getContestantName()));
		}
		else
		{
			sortList.append(qMakePair(1, i->getContestantName()));
		}
	}

	std::sort(sortList.begin(), sortList.end());
	QMap<QString, int> rankList;

	for (int i = 0; i < sortList.size(); i++)
	{
		if (i > 0 && sortList[i].first == sortList[i - 1].first)
		{
			rankList.insert(sortList[i].second, rankList[sortList[i - 1].second]);
		}
		else
		{
			rankList.insert(sortList[i].second, i);
		}
	}

	QHash<Contestant *, int> loc;

	for (int i = 0; i < contestantList.size(); i++)
	{
		loc.insert(contestantList[i], i);
	}

	out << "\"" << tr("Rank") << "\""
	    << ","
	    << "\"" << tr("Name") << "\""
	    << ",";

	for (auto &i : taskList)
	{
		out << "\"" << i->getProblemTile() << "\""
		    << ",";
	}

	out << "\"" << tr("Total Score") << "\"" << Qt::endl;

	for (auto &i : sortList)
	{
		Contestant *contestant = contest->getContestant(i.second);
		out << "\"" << rankList[contestant->getContestantName()] + 1 << "\""
		    << ",";
		out << "\"" << i.second << "\""
		    << ",";

		for (int j = 0; j < taskList.size(); j++)
		{
			int score = contestant->getTaskScore(j);

			if (score != -1)
			{
				out << "\"" << score << "\""
				    << ",";
			}
			else
			{
				out << "\"" << tr("Invalid") << "\""
				    << ",";
			}
		}

		int score = contestant->getTotalScore();

		if (score != -1)
		{
			out << "\"" << score << "\"" << Qt::endl;
		}
		else
		{
			out << "\"" << tr("Invalid") << "\"" << Qt::endl;
		}
	}

	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
}

void ExportUtil::exportXls(QWidget *widget, Contest *contest, const QString &fileName)
{
#ifdef Q_OS_WIN32

	if (QFile(fileName).exists())
	{
		if (! QFile(fileName).remove())
		{
			QMessageBox::warning(widget, tr("LemonLime"),
			                     tr("Cannot open file %1").arg(QFileInfo(fileName).fileName()), QMessageBox::Ok);
			return;
		}
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	QList<Contestant *> contestantList = contest->getContestantList();
	QList<Task *> taskList = contest->getTaskList();
	QList<QPair<int, QString>> sortList;

	for (int i = 0; i < contestantList.size(); i++)
	{
		int totalScore = contestantList[i]->getTotalScore();

		if (totalScore != -1)
		{
			sortList.append(qMakePair(-totalScore, contestantList[i]->getContestantName()));
		}
		else
		{
			sortList.append(qMakePair(1, contestantList[i]->getContestantName()));
		}
	}

	qSort(sortList);
	QMap<QString, int> rankList;

	for (int i = 0; i < sortList.size(); i++)
	{
		if (i > 0 && sortList[i].first == sortList[i - 1].first)
		{
			rankList.insert(sortList[i].second, rankList[sortList[i - 1].second]);
		}
		else
		{
			rankList.insert(sortList[i].second, i);
		}
	}

	QMap<Contestant *, int> loc;

	for (int i = 0; i < contestantList.size(); i++)
	{
		loc.insert(contestantList[i], i);
	}

	QAxObject *excel = new QAxObject("Excel.Application", widget);
	QAxObject *workbook = excel->querySubObject("Workbooks")->querySubObject("Add");
	QAxObject *sheet = workbook->querySubObject("ActiveSheet");
	sheet->setProperty("Name", QDate::currentDate().toString("yyyy-MM-dd"));
	sheet->querySubObject("Cells(int, int)", 1, 1)->setProperty("Value", tr("Rank"));
	sheet->querySubObject("Cells(int, int)", 1, 2)->setProperty("Value", tr("Name"));

	for (int i = 0; i < taskList.size(); i++)
		sheet->querySubObject("Cells(int, int)", 1, 3 + i)->setProperty("Value", taskList[i]->getProblemTile());

	sheet->querySubObject("Cells(int, int)", 1, 3 + taskList.size())->setProperty("Value", tr("Total Score"));

	for (int i = 0; i < taskList.size() + 3; i++)
		sheet->querySubObject("Cells(int, int)", 1, i + 1)->querySubObject("Font")->setProperty("Bold", true);

	for (int i = 0; i < sortList.size(); i++)
	{
		Contestant *contestant = contest->getContestant(sortList[i].second);
		sheet->querySubObject("Cells(int, int)", 2 + i, 1)
		   ->setProperty("Value", rankList[contestant->getContestantName()] + 1);
		sheet->querySubObject("Cells(int, int)", 2 + i, 2)->setProperty("Value", sortList[i].second);

		for (int j = 0; j < taskList.size(); j++)
		{
			int score = contestant->getTaskScore(j);

			if (score != -1)
			{
				sheet->querySubObject("Cells(int, int)", 2 + i, 3 + j)->setProperty("Value", score);
			}
			else
			{
				sheet->querySubObject("Cells(int, int)", 2 + i, 3 + j)->setProperty("Value", tr("Invalid"));
			}
		}

		int score = contestant->getTotalScore();

		if (score != -1)
		{
			sheet->querySubObject("Cells(int, int)", 2 + i, 3 + taskList.size())->setProperty("Value", score);
		}
		else
		{
			sheet->querySubObject("Cells(int, int)", 2 + i, 3 + taskList.size())
			   ->setProperty("Value", tr("Invalid"));
		}
	}

	workbook->dynamicCall("SaveAs(const QString&, int)", QDir::toNativeSeparators(fileName), -4143);
	excel->dynamicCall("Quit()");
	delete excel;
	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
#endif
}

void ExportUtil::exportResult(QWidget *widget, Contest *contest)
{
	QList<Contestant *> contestantList = contest->getContestantList();
	QList<Task *> taskList = contest->getTaskList();

	if (contestantList.isEmpty())
	{
		QMessageBox::warning(widget, tr("LemonLime"), tr("No contestant in current contest"), QMessageBox::Ok);
		return;
	}

	if (taskList.isEmpty())
	{
		QMessageBox::warning(widget, tr("LemonLime"), tr("No task in current contest"), QMessageBox::Ok);
		return;
	}

	QString filter = tr("HTML Document (*.html *.htm);;CSV (*.csv)");
#ifdef Q_OS_WIN32
	QAxObject *excel = new QAxObject("Excel.Application", widget);

	if (! excel->isNull())
		filter = filter + tr(";;Excel Workbook (*.xls)");

	delete excel;
#endif
	QString fileName = QFileDialog::getSaveFileName(
	   widget, tr("Export Result"), QDir::currentPath() + QDir::separator() + "result.html", filter);

	if (fileName.isEmpty())
		return;

	if (QFileInfo(fileName).suffix() == "html")
	{
		QMessageBox::StandardButton res =
		   QMessageBox::warning(widget, tr("Export Result"), tr("Use Default Color Theme?"),
		                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		exportHtml(widget, contest, fileName, res == QMessageBox::Yes);
	}

	if (QFileInfo(fileName).suffix() == "htm")
		exportSmallerHtml(widget, contest, fileName);

	if (QFileInfo(fileName).suffix() == "csv")
		exportCsv(widget, contest, fileName);

	if (QFileInfo(fileName).suffix() == "xls")
		exportXls(widget, contest, fileName);
}
