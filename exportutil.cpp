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
#include "task.h"
#include "testcase.h"
#include "contestant.h"
#include "globaltype.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>

ExportUtil::ExportUtil(QObject *parent) :
	QObject(parent)
{
}

QString ExportUtil::getContestantHtmlCode(Contest *contest, Contestant *contestant)
{
	QString htmlCode;

	QList<Task *> taskList = contest->getTaskList();

	for(int i = 0; i < taskList.size(); i ++)
	{
		htmlCode += "<p><span style=\"font-weight:bold; font-size:large;\">";
		htmlCode += QString("%1 %2</span><br>").arg(tr("Task")).arg(taskList[i]->getProblemTile());

		if(! contestant->getCheckJudged(i))
		{
			htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Not judged"));
			continue;
		}

		if(taskList[i]->getTaskType() == Task::Traditional || taskList[i]->getTaskType() == Task::Interaction)
		{
			if(contestant->getCompileState(i) != CompileSuccessfully)
			{
				switch(contestant->getCompileState(i))
				{
					case NoValidSourceFile:
						htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Cannot find valid source file"));
						break;

					case CompileTimeLimitExceeded:
						htmlCode += QString("&nbsp;&nbsp;%1%2<br>").arg(tr("Source file: "))
						            .arg(contestant->getSourceFile(i));
						htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Compile time limit exceeded"));
						break;

					case InvalidCompiler:
						htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Cannot run given compiler"));
						break;

					case CompileError:
						htmlCode += QString("&nbsp;&nbsp;%1%2<br>").arg(tr("Source file: "))
						            .arg(contestant->getSourceFile(i));
						htmlCode += QString("&nbsp;&nbsp;%1").arg(tr("Compile error"));

						if(! contestant->getCompileMessage(i).isEmpty())
						{
							QString compileMessage = contestant->getCompileMessage(i);
							compileMessage.replace("\r\n", "<br>");
							compileMessage.replace("\n", "<br>");
							compileMessage.replace("\r", "<br>");

							if(compileMessage.endsWith("<br>"))
								compileMessage.chop(4);

							htmlCode += "<table border=\"1\" cellpadding=\"1\">";
							htmlCode += "<tr><td style=\"padding: 0.5em\"><code>";
							htmlCode += compileMessage;
							htmlCode += "</code></td></tr></table>";
						}

						htmlCode += "</p>";
						break;
				}

				continue;
			}

			htmlCode += QString("&nbsp;&nbsp;%1%2").arg(tr("Source file: ")).arg(contestant->getSourceFile(i));
		}

		htmlCode += "<table border=\"1\" cellpadding=\"1\"><tr>";
		htmlCode += QString("<th scope=\"col\" nowrap=\"nowrap\">%1</th>").arg(tr("Test Case"));
		htmlCode += QString("<th scope=\"col\" nowrap=\"nowrap\">%1</th>").arg(tr("Input File"));
		htmlCode += QString("<th scope=\"col\">%1</th>").arg(tr("Result"));
		htmlCode += QString("<th scope=\"col\" nowrap=\"nowrap\">%1</th>").arg(tr("Time Used"));
		htmlCode += QString("<th scope=\"col\" nowrap=\"nowrap\">%1</th>").arg(tr("Memory Used"));
		htmlCode += QString("<th scope=\"col\" nowrap=\"nowrap\">%1</th></tr>").arg(tr("Score"));

		QList<QStringList> inputFiles = contestant->getInputFiles(i);
		QList< QList<ResultState> > result = contestant->getResult(i);
		QList<QStringList> message = contestant->getMessage(i);
		QList< QList<int> > timeUsed = contestant->getTimeUsed(i);
		QList< QList<int> > memoryUsed = contestant->getMemoryUsed(i);
		QList< QList<int> > score = contestant->getSocre(i);

		for(int j = 0; j < inputFiles.size(); j ++)
		{
			for(int k = 0; k < inputFiles[j].size(); k ++)
			{
				htmlCode += "<tr>";

				if(k == 0)
				{
					if(score[j].size() == inputFiles[j].size())
						htmlCode += QString("<td nowrap=\"nowrap\" rowspan=\"%1\" align=\"center\" valign=\"middle\">#%2</td>")
						            .arg(inputFiles[j].size()).arg(j + 1);
					else
						htmlCode += QString("<td nowrap=\"nowrap\" rowspan=\"%1\" align=\"center\" valign=\"middle\">#%2<br>%3:%4</td>")
						            .arg(inputFiles[j].size()).arg(j + 1).arg(tr("Subtask Dependence Score")).arg(score[j].back());
				}

				htmlCode += QString("<td nowrap=\"nowrap\" align=\"center\">%1</td>").arg(inputFiles[j][k]);

				QString text;

				switch(result[j][k])
				{
					case CorrectAnswer:
						text = tr("Correct Answer");
						break;

					case WrongAnswer:
						text = tr("Wrong Answer");
						break;

					case PartlyCorrect:
						text = tr("Partly Correct");
						break;

					case TimeLimitExceeded:
						text = tr("Time Limit Exceeded");
						break;

					case MemoryLimitExceeded:
						text = tr("Memory Limit Exceeded");
						break;

					case CannotStartProgram:
						text = tr("Cannot Start Program");
						break;

					case FileError:
						text = tr("File Error");
						break;

					case RunTimeError:
						text = tr("Run Time Error");
						break;

					case InvalidSpecialJudge:
						text = tr("Invalid Special Judge");
						break;

					case SpecialJudgeTimeLimitExceeded:
						text = tr("Special Judge Time Limit Exceeded");
						break;

					case SpecialJudgeRunTimeError:
						text = tr("Special Judge Run Time Error");
						break;

					case Skipped:
						text = tr("Skipped");
						break;

					case InteractorError:
						text = tr("Interactor Error");
						break;
				}

				htmlCode += QString("<td align=\"center\">%1").arg(text);

				if(! message[j][k].isEmpty())
				{
					QString tmp = message[j][k];
					tmp.replace("\n", "\\n");
					tmp.replace("\"", "\\&quot;");
					htmlCode += QString("<a href=\"javascript:alert(&quot;%1&quot;)\"> (...)")
					            .arg(tmp);
				}

				htmlCode += "</td>";

				htmlCode += "<td nowrap=\"nowrap\" align=\"center\">";

				if(timeUsed[j][k] != -1)
				{
					htmlCode += QString("").sprintf("%.3lf s", double (timeUsed[j][k]) / 1000);
				}
				else
				{
					htmlCode += tr("Invalid");
				}

				htmlCode += "</td>";

				htmlCode += "<td nowrap=\"nowrap\" align=\"center\">";

				if(memoryUsed[j][k] != -1)
				{
					htmlCode += QString("").sprintf("%.3lf MB", double (memoryUsed[j][k]) / 1024 / 1024);
				}
				else
				{
					htmlCode += tr("Invalid");
				}

				htmlCode += "</td>";

				if(k == 0)
				{
					htmlCode += QString("<td rowspan=\"%1\" align=\"center\" valign=\"middle\">").arg(inputFiles[j].size());
					int minv = 1000000000;

					for(int t = 0; t < inputFiles[j].size(); t ++)
						if(score[j][t] < minv) minv = score[j][t];

					htmlCode += QString("%1</td>").arg(minv);
				}

				htmlCode += "</tr>";
			}
		}

		htmlCode += "</table><br></p>";
	}

	htmlCode += QString("<p><a href=\"#top\">%1</a></p>").arg(tr("Return to top"));
	return htmlCode;
}

void ExportUtil::exportHtml(QWidget *widget, Contest *contest, const QString &fileName)
{
	QFile file(fileName);

	if(! file.open(QFile::WriteOnly))
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
	out << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" />";
	out << "<style type=\"text/css\">th, td {padding-left: 1em; padding-right: 1em;}</style>";
	out << "<title>" << tr("Contest Result") << "</title>";
	out << "</head><body>";

	QList< QPair<int, QString> > sortList;

	for(int i = 0; i < contestantList.size(); i ++)
	{
		int totalScore = contestantList[i]->getTotalScore();

		if(totalScore != -1)
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

	for(int i = 0; i < sortList.size(); i ++)
	{
		if(i > 0 && sortList[i].first == sortList[i - 1].first)
		{
			rankList.insert(sortList[i].second, rankList[sortList[i - 1].second]);
		}
		else
		{
			rankList.insert(sortList[i].second, i);
		}
	}

	QMap<Contestant *, int> loc;

	for(int i = 0; i < contestantList.size(); i ++)
	{
		loc.insert(contestantList[i], i);
	}

	out << "<p><span style=\"font-size:x-large; font-weight:bold;\">";
	out << "<a name=\"top\"></a>" << tr("Rank List") << "</span></p>";
	out << "<p><table border=\"1\" cellpadding=\"1\"><tr>";
	out << QString("<th scope=\"col\" nowrap=\"nowrap\">%1</th>").arg(tr("Rank"));
	out << QString("<th scope=\"col\" nowrap=\"nowrap\">%1</th>").arg(tr("Name"));

	for(int i = 0; i < taskList.size(); i ++)
		out << QString("<th scope=\"col\" nowrap=\"nowrap\">%1</th>").arg(taskList[i]->getProblemTile());

	out << QString("<th scope=\"col\" nowrap=\"nowrap\">%1</th></tr>").arg(tr("Total Score"));

	QList<int> fullScore;
	int sfullScore = 0;

	for(int i = 0; i < taskList.size(); i++)
	{
		int a = taskList[i]->getTotalScore();
		fullScore.append(a), sfullScore += a;
	}

	for(int i = 0; i < sortList.size(); i ++)
	{
		Contestant *contestant = contest->getContestant(sortList[i].second);
		out << QString("<tr><td nowrap=\"nowrap\" align=\"center\">%1</td>")
		    .arg(rankList[contestant->getContestantName()] + 1);
		out << QString("<td nowrap=\"nowrap\" align=\"center\"><a href=\"#c%1\">%2</a></td>")
		    .arg(loc[contestant]).arg(sortList[i].second);

		for(int j = 0; j < taskList.size(); j ++)
		{
			int score = contestant->getTaskScore(j);

			if(score != -1)
			{
				int basCol = oriBaseColorHI;
				double colFix = oriBaseColorSF * 100;
				double colLevel = oriBaseColorLF(score, fullScore[j], 0.30) * 100;

				if(contestant->getCompileState(j) != CompileSuccessfully)
				{
					if(contestant->getCompileState(j) == NoValidSourceFile)
						basCol = nofBaseColorHI, colFix = nofBaseColorSF * 100, colLevel = nofBaseColorLF * 100;
					else basCol = cmeBaseColorHI, colFix = cmeBaseColorSF * 100, colLevel = cmeBaseColorLF * 100;
				}

				out << QString("<td style=\"background-color: hsl(%2,%3\%,%4\%);\" nowrap=\"nowrap\" align=\"center\">%1</td>").arg(score).arg(basCol).arg(colFix).arg(colLevel);
			}
			else
			{
				out << QString("<td nowrap=\"nowrap\" align=\"center\">%1</td>").arg(tr("Invalid"));
			}
		}

		int score = contestant->getTotalScore();

		if(score != -1)
		{
			int basCol = oriBaseColorHI;
			double colFix = oriBaseColorSF * 100;
			double colLevel = oriBaseColorLF(score, sfullScore, 0.40) * 100;

			out << QString("<td style=\"background-color: hsl(%2,%3\%,%4\%); font-weight: bold;\" nowrap=\"nowrap\" align=\"center\">%1</td>").arg(score).arg(basCol).arg(colFix).arg(colLevel);
		}
		else
		{
			out << QString("<td style=\"font-weight: bold;\" nowrap=\"nowrap\" align=\"center\">%1</td>").arg(tr("Invalid"));
		}
	}

	out << "</table></p>";
	//out << "<p>" << tr("Judge with LemonPlus") << "</p>";
	//out << "<p>" << tr("Github Page: ") << QString("<a href=\"https://github.com/Dust1404/Project_LemonPlus\">https://github.com/Dust1404/Project_LemonPlus</a>") << "</p>";
	//remove project info in result.html

	for(int i = 0; i < contestantList.size(); i ++)
	{
		out << QString("<a name=\"c%1\"><hr><a>").arg(i) << "<span style=\"font-size:x-large; font-weight:bold;\">";
		out << tr("Contestant: %1").arg(contestantList[i]->getContestantName()) << "</span>";
		out << getContestantHtmlCode(contest, contestantList[i]);
	}

	out << "</body></html>";

	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
}

void ExportUtil::exportCsv(QWidget *widget, Contest *contest, const QString &fileName)
{
	QFile file(fileName);

	if(! file.open(QFile::WriteOnly))
	{
		QMessageBox::warning(widget, tr("LemonLime"), tr("Cannot open file %1").arg(QFileInfo(file).fileName()),
		                     QMessageBox::Ok);
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	QTextStream out(&file);

	QList<Contestant *> contestantList = contest->getContestantList();
	QList<Task *> taskList = contest->getTaskList();
	QList< QPair<int, QString> > sortList;

	for(int i = 0; i < contestantList.size(); i ++)
	{
		int totalScore = contestantList[i]->getTotalScore();

		if(totalScore != -1)
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

	for(int i = 0; i < sortList.size(); i ++)
	{
		if(i > 0 && sortList[i].first == sortList[i - 1].first)
		{
			rankList.insert(sortList[i].second, rankList[sortList[i - 1].second]);
		}
		else
		{
			rankList.insert(sortList[i].second, i);
		}
	}

	QMap<Contestant *, int> loc;

	for(int i = 0; i < contestantList.size(); i ++)
	{
		loc.insert(contestantList[i], i);
	}

	out << "\"" << tr("Rank") << "\"" << "," << "\"" << tr("Name") << "\"" << ",";

	for(int i = 0; i < taskList.size(); i ++)
	{
		out << "\"" << taskList[i]->getProblemTile() << "\"" << ",";
	}

	out << "\"" << tr("Total Score") << "\"" << endl;

	for(int i = 0; i < sortList.size(); i ++)
	{
		Contestant *contestant = contest->getContestant(sortList[i].second);
		out << "\"" << rankList[contestant->getContestantName()] + 1 << "\"" << ",";
		out << "\"" << sortList[i].second << "\"" << ",";

		for(int j = 0; j < taskList.size(); j ++)
		{
			int score = contestant->getTaskScore(j);

			if(score != -1)
			{
				out << "\"" << score << "\"" << ",";
			}
			else
			{
				out << "\"" << tr("Invalid") << "\"" << ",";
			}
		}

		int score = contestant->getTotalScore();

		if(score != -1)
		{
			out << "\"" << score << "\"" << endl;
		}
		else
		{
			out << "\"" << tr("Invalid") << "\"" << endl;
		}
	}

	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
}

void ExportUtil::exportXls(QWidget *widget, Contest *contest, const QString &fileName)
{
#ifdef Q_OS_WIN32

	if(QFile(fileName).exists())
	{
		if(! QFile(fileName).remove())
		{
			QMessageBox::warning(widget, tr("LemonLime"), tr("Cannot open file %1").arg(QFileInfo(fileName).fileName()),
			                     QMessageBox::Ok);
			return;
		}
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	QList<Contestant *> contestantList = contest->getContestantList();
	QList<Task *> taskList = contest->getTaskList();
	QList< QPair<int, QString> > sortList;

	for(int i = 0; i < contestantList.size(); i ++)
	{
		int totalScore = contestantList[i]->getTotalScore();

		if(totalScore != -1)
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

	for(int i = 0; i < sortList.size(); i ++)
	{
		if(i > 0 && sortList[i].first == sortList[i - 1].first)
		{
			rankList.insert(sortList[i].second, rankList[sortList[i - 1].second]);
		}
		else
		{
			rankList.insert(sortList[i].second, i);
		}
	}

	QMap<Contestant *, int> loc;

	for(int i = 0; i < contestantList.size(); i ++)
	{
		loc.insert(contestantList[i], i);
	}

	QAxObject *excel = new QAxObject("Excel.Application", widget);
	QAxObject *workbook = excel->querySubObject("Workbooks")->querySubObject("Add");
	QAxObject *sheet = workbook->querySubObject("ActiveSheet");
	sheet->setProperty("Name", QDate::currentDate().toString("yyyy-MM-dd"));

	sheet->querySubObject("Cells(int, int)", 1, 1)->setProperty("Value", tr("Rank"));
	sheet->querySubObject("Cells(int, int)", 1, 2)->setProperty("Value", tr("Name"));

	for(int i = 0; i < taskList.size(); i ++)
		sheet->querySubObject("Cells(int, int)", 1, 3 + i)->setProperty("Value", taskList[i]->getProblemTile());

	sheet->querySubObject("Cells(int, int)", 1, 3 + taskList.size())->setProperty("Value", tr("Total Score"));

	for(int i = 0; i < taskList.size() + 3; i ++)
		sheet->querySubObject("Cells(int, int)", 1, i + 1)->querySubObject("Font")->setProperty("Bold", true);

	for(int i = 0; i < sortList.size(); i ++)
	{
		Contestant *contestant = contest->getContestant(sortList[i].second);
		sheet->querySubObject("Cells(int, int)", 2 + i, 1)->setProperty("Value",
		        rankList[contestant->getContestantName()] + 1);
		sheet->querySubObject("Cells(int, int)", 2 + i, 2)->setProperty("Value", sortList[i].second);

		for(int j = 0; j < taskList.size(); j ++)
		{
			int score = contestant->getTaskScore(j);

			if(score != -1)
			{
				sheet->querySubObject("Cells(int, int)", 2 + i, 3 + j)->setProperty("Value", score);
			}
			else
			{
				sheet->querySubObject("Cells(int, int)", 2 + i, 3 + j)->setProperty("Value", tr("Invalid"));
			}
		}

		int score = contestant->getTotalScore();

		if(score != -1)
		{
			sheet->querySubObject("Cells(int, int)", 2 + i, 3 + taskList.size())->setProperty("Value", score);
		}
		else
		{
			sheet->querySubObject("Cells(int, int)", 2 + i, 3 + taskList.size())->setProperty("Value", tr("Invalid"));
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

	if(contestantList.isEmpty())
	{
		QMessageBox::warning(widget, tr("LemonLime"), tr("No contestant in current contest"), QMessageBox::Ok);
		return;
	}

	if(taskList.isEmpty())
	{
		QMessageBox::warning(widget, tr("LemonLime"), tr("No task in current contest"), QMessageBox::Ok);
		return;
	}

	QString filter = tr("HTML Document (*.html);;CSV (*.csv)");

#ifdef Q_OS_WIN32
	QAxObject *excel = new QAxObject("Excel.Application", widget);

	if(! excel->isNull()) filter = filter + tr(";;Excel Workbook (*.xls)");

	delete excel;
#endif

	QString fileName = QFileDialog::getSaveFileName(widget, tr("Export Result"),
	                   QDir::currentPath() + QDir::separator() + "result.html", filter);

	if(fileName.isEmpty()) return;

	if(QFileInfo(fileName).suffix() == "html") exportHtml(widget, contest, fileName);

	if(QFileInfo(fileName).suffix() == "csv") exportCsv(widget, contest, fileName);

	if(QFileInfo(fileName).suffix() == "xls") exportXls(widget, contest, fileName);
}
