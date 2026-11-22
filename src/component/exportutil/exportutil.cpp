/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "exportutil.h"
//
#include "base/LemonType.hpp"
#include "base/settings.h"
#include "core/contest.h"
#include "core/contestant.h"
#include "core/subtaskdependencelib.h"
#include "core/task.h"
#include "core/testcase.h"
#include "settings.h"
#include "visualmainsettings.h"
#include "visualsettings.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <ranges>

namespace {

	struct RankedContestant {
		const Contestant *contestant;
		int totalScore;
		int sourceIndex;
		int rank = 0;
	};

	QList<RankedContestant> rankContestants(const QList<Contestant *> &contestants) {
		QList<RankedContestant> entries;
		entries.reserve(contestants.size());
		for (int i = 0; i < contestants.size(); ++i)
			entries.append({contestants[i], contestants[i]->getTotalScore(), i});

		std::ranges::sort(entries, [](const auto &left, const auto &right) {
			if (left.totalScore != right.totalScore)
				return left.totalScore > right.totalScore;
			return left.contestant->getContestantName() < right.contestant->getContestantName();
		});
		for (int i = 0; i < entries.size(); ++i) {
			entries[i].rank =
			    i > 0 && entries[i].totalScore == entries[i - 1].totalScore ? entries[i - 1].rank : i + 1;
		}
		return entries;
	}

} // namespace

ExportUtil::ExportUtil(QObject *parent) : QObject(parent) {}

const auto resultStateMap = []() {
	QMap<ResultState, std::tuple<QString, QString, QString>> resMap;
	for (int i = static_cast<int>(CorrectAnswer); i != static_cast<int>(LastResultState); i++) {
		QString text, frColor, bgColor;
		Settings::setTextAndColor(static_cast<ResultState>(i), text, frColor, bgColor);
		resMap[static_cast<ResultState>(i)] = {text, frColor, bgColor};
	}
	return resMap;
}();
auto ExportUtil::getContestantHtmlCode(Contest *contest, Contestant *contestant, int num) -> QString {
	QString htmlCode;
	QList<Task *> taskList = contest->getTaskList();

	for (int i = 0; i < taskList.size(); i++) {
		htmlCode += QString(R"(<div id="c%1p%2"><p><span>)").arg(num).arg(i);
		htmlCode +=
		    QString("%1 %2</span><br>").arg(tr("Task"), taskList[i]->getProblemTitle().toHtmlEscaped());

		if (! contestant->getCheckJudged(i)) {
			htmlCode += QString("&nbsp;&nbsp;%1</p></div>").arg(tr("Not judged"));
			continue;
		}

		if (taskList[i]->getTaskType() == Task::Traditional ||
		    taskList[i]->getTaskType() == Task::Interaction ||
		    taskList[i]->getTaskType() == Task::Communication ||
		    taskList[i]->getTaskType() == Task::CommunicationExec) {
			if (contestant->getCompileState(i) != CompileSuccessfully) {
				switch (contestant->getCompileState(i)) {
					case NoValidGraderFile:
						htmlCode += QString("&nbsp;&nbsp;%1</p></div>")
						                .arg(tr("Main grader (grader.*) cannot be found"));
						break;

					case NoValidSourceFile:
						htmlCode +=
						    QString("&nbsp;&nbsp;%1</p></div>").arg(tr("Cannot find valid source file"));
						break;

					case CompileTimeLimitExceeded:
						htmlCode += QString("&nbsp;&nbsp;%1%2<br>")
						                .arg(tr("Source file: "))
						                .arg(contestant->getSourceFile(i).toHtmlEscaped());
						htmlCode +=
						    QString("&nbsp;&nbsp;%1</p></div>").arg(tr("Compile time limit exceeded"));
						break;

					case InvalidCompiler:
						htmlCode += QString("&nbsp;&nbsp;%1</p></div>").arg(tr("Cannot run given compiler"));
						break;

					case CompileError:
						htmlCode += QString("&nbsp;&nbsp;%1%2<br>")
						                .arg(tr("Source file: "))
						                .arg(contestant->getSourceFile(i).toHtmlEscaped());
						htmlCode += QString("&nbsp;&nbsp;%1").arg(tr("Compile error"));

						if (! contestant->getCompileMessage(i).isEmpty()) {
							QString compileMessage = contestant->getCompileMessage(i).toHtmlEscaped();
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

						htmlCode += "</p></div>";
						break;

					default:
						break;
				}

				continue;
			}

			htmlCode += QString("&nbsp;&nbsp;%1%2")
			                .arg(tr("Source file: "), contestant->getSourceFile(i).toHtmlEscaped());
		}

		htmlCode += "<table><tr>";
		htmlCode += QString(R"(<th>%1</th>)").arg(tr("Test Case"));
		htmlCode += QString(R"(<th>%1</th>)").arg(tr("Input File"));
		htmlCode += QString(R"(<th>%1</th>)").arg(tr("Result"));
		htmlCode += QString(R"(<th>%1</th>)").arg(tr("Time Used"));
		htmlCode += QString(R"(<th>%1</th>)").arg(tr("Memory Used"));
		htmlCode += QString(R"(<th>%1</th></tr>)").arg(tr("Score"));
		QList<TestCase *> testCases = taskList[i]->getTestCaseList();
		QList<QStringList> inputFiles = contestant->getInputFiles(i);
		QList<QList<ResultState>> result = contestant->getResult(i);
		QList<QStringList> message = contestant->getMessage(i);
		QList<QList<int>> timeUsed = contestant->getTimeUsed(i);
		QList<QList<qint64>> memoryUsed = contestant->getMemoryUsed(i);
		QList<QList<int>> score = contestant->getScore(i);

		for (int j = 0; j < inputFiles.size(); j++) {
			for (int k = 0; k < inputFiles[j].size(); k++) {
				htmlCode += "<tr>";

				if (k == 0) {
					if (score[j].size() == inputFiles[j].size())
						htmlCode +=
						    QString(R"(<td rowspan="%1">#%2</td>)").arg(inputFiles[j].size()).arg(j + 1);
					else
						htmlCode += QString(R"(<td rowspan="%1">#%2<br>%3:%4</td>)")
						                .arg(inputFiles[j].size())
						                .arg(j + 1)
						                .arg(tr("Subtask Dependence Status"))
						                .arg(statusRankingText(score[j].back()));
				}

				htmlCode += QString("<td>%1</td>").arg(inputFiles[j][k].toHtmlEscaped());
				QString text, bgColor, frColor;
				Settings::setTextAndColor(result[j][k], text, frColor, bgColor);
				htmlCode +=
				    QString("<td class=\"result%2\">%1").arg(text).arg(static_cast<int>(result[j][k]));

				if (! message[j][k].isEmpty()) {
					htmlCode +=
					    QString(
					        R"(<a href="#" data-message="%1" onclick="alert(this.dataset.message); return false;"> (...)</a>)")
					        .arg(message[j][k].toHtmlEscaped());
				}

				htmlCode += "</td>";
				htmlCode += "<td>";

				if (timeUsed[j][k] != -1) {
					htmlCode += QString("").asprintf("%.3lf s", double(timeUsed[j][k]) / 1000);
				} else {
					htmlCode += tr("Invalid");
				}

				htmlCode += "</td>";
				htmlCode += "<td>";

				if (memoryUsed[j][k] != -1) {
					htmlCode += QString("").asprintf("%.3lf MiB", double(memoryUsed[j][k]) / 1024 / 1024);
				} else {
					htmlCode += tr("Invalid");
				}

				htmlCode += "</td>";

				if (k == 0) {
					const int minv = std::ranges::min(score[j] | std::views::take(inputFiles[j].size()));
					int maxv = testCases[j]->getFullScore();

					QString bgClass = "zero-score";

					if (minv >= maxv)
						bgClass = "full-score";
					else if (minv > 0)
						bgClass = "partial-score";

					htmlCode += QString(R"(<td rowspan="%1" class="%2"><span class="c">%3</span> / %4</td>)")
					                .arg(inputFiles[j].size())
					                .arg(bgClass)
					                .arg(minv)
					                .arg(maxv);
				}

				htmlCode += "</tr>";
			}
		}

		htmlCode += "</table><br></p></div>";
	}

	return htmlCode;
}
/*
 * Generate the HTML code for the summary page
 * Might be difficult to maintain
 * Use Javascript to shrink the filesize
 */
void ExportUtil::exportHtml(QWidget *widget, Contest *contest, const QString &fileName) {
	QFile file(fileName);

	if (! file.open(QFile::WriteOnly)) {
		QMessageBox::warning(widget, tr("LemonLime"),
		                     tr("Cannot open file %1").arg(QFileInfo(file).fileName()), QMessageBox::Ok);
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	QTextStream out(&file);
	writeHtml(out, contest);
	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
}

void ExportUtil::writeHtml(QTextStream &out, Contest *contest) {
	Settings settings;
	contest->copySettings(settings);
	ColorTheme colors = settings.getCurrentColorTheme();
	QList<Contestant *> contestantList = contest->getContestantList();
	QList<Task *> taskList = contest->getTaskList();
	out << "<html><head>";
	out << R"(<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />)";

	// Style sheet
	out << R"(<style type="text/css" id="lemon-style">)"
	       "th, td {padding-left: 1em; padding-right: 1em; white-space: nowrap; "
	       "text-align: center; verticle-align: middle;}"
	       ".td-0 {border-style: none solid solid none; border-width: 1px 3px; border-color: #ccc;}"
	       ".th-0 {border-style: none solid solid none; border-width: 3px 3px; border-color: #000;}"
	       ".th-1 {border-style: none solid solid none; border-width: 3px 2px; border-color: #000;}"
	       ".a-0 {color: black; text-decoration: none;} .c {font-weight: bold; font-size: large;}"
	       ".td-2 {border-radius: 5px; font-weight: bold;}"
	       ".td-3 {border-radius: 5px;}"
	       ".full-score {background: rgb(192, 255, 192)}"
	       ".partial-score {background: rgb(192, 255, 255)}"
	       ".zero-score {background: rgb(255, 192, 192)}";
	for (auto [k, v] : resultStateMap.toStdMap()) {
		out << ".result" << static_cast<int>(k);
		out << QString(" {color: %1;background: %2;}").arg(std::get<1>(v)).arg(std::get<2>(v));
	}
	out << "</style>";

	out << "<title>" << contest->getContestTitle().toHtmlEscaped() << " : " << tr("Contest Result")
	    << "</title>";
	out << QString(R"(</head><body data-popup-blocked="%1">)")
	           .arg(tr("The browser blocked the popup window. Please allow popups for this page and try again.")
	                    .toHtmlEscaped());

	out << "<p><span class=\"d\">";
	out << "<a name=\"top\"></a>" << contest->getContestTitle().toHtmlEscaped() << " : " << tr("Rank List")
	    << "</span></p>";
	out << "<p>"
	    << tr("Click names or task scores to view details in a popup window. Judged By LemonLime") << "</p>";
	out << R"(<p><table cellpadding="1" style="border-style: solid;"><tr>)";
	out << QString(R"(<th class="th-0" scope="col">%1</th>)").arg(tr("Rank"));
	out << QString(R"(<th class="th-0" scope="col">%1</th>)").arg(tr("Name"));
	out << QString(R"(<th class="th-1" scope="col">%1</th>)").arg(tr("Total Score"));

	for (auto &i : taskList)
		out << QString(R"(<th class="th-1" scope="col">%1</th>)").arg(i->getProblemTitle().toHtmlEscaped());

	out << "</tr>";
	QList<int> fullScore;
	int sfullScore = contest->getTotalScore();

	for (auto &i : taskList) {
		fullScore.append(i->getTotalScore());
	}

	for (const auto &[contestant, allScore, sourceIndex, rank] : rankContestants(contestantList)) {
		out << "<tr>";
		out << QString("<td class=\"td-0\">%1</td>").arg(rank);
		out << QString(R"LEMON(<td class="td-0"><a href="#" onclick="return lemonOpenDetail(%1)" class="a-0">%2</a></td>)LEMON")
		           .arg(sourceIndex)
		           .arg(contestant->getContestantName().toHtmlEscaped());

		if (allScore >= 0) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			float h = NAN;
			float s = NAN;
			float l = NAN;
#else
			double h = NAN;
			double s = NAN;
			double l = NAN;
#endif
			colors.getColorGrand(allScore, sfullScore).getHslF(&h, &s, &l);
			h *= 360, s *= 100, l *= 100;
			out << QString("<td class=\"td-2\" style=\"background: hsl(%2,%3%,%4%); border: 2px solid "
			               "hsl(%2,%3%,%5%);\">%1</td>")
			           .arg(allScore)
			           .arg(h)
			           .arg(s)
			           .arg(l)
			           .arg(qMax(l - 20, 0.00));
		} else {
			out << QString("<td class=\"td-2\">%1</td>").arg(tr("Invalid"));
		}

		for (int j = 0; j < taskList.size(); j++) {
			int score = contestant->getTaskScore(j);

			if (score != -1) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
				float h = NAN;
				float s = NAN;
				float l = NAN;
#else
				double h = NAN;
				double s = NAN;
				double l = NAN;
#endif
				QColor col = colors.getColorPer(score, fullScore[j]);
				col.getHslF(&h, &s, &l);

				if (taskList[j]->getTaskType() != Task::AnswersOnly &&
				    contestant->getCompileState(j) != CompileSuccessfully) {
					if (contestant->getCompileState(j) == NoValidSourceFile) {
						colors.getColorNf().getHslF(&h, &s, &l);
					} else {
						colors.getColorCe().getHslF(&h, &s, &l);
					}
				}

				h *= 360, s *= 100, l *= 100;
				out << QString(
				           R"LEMON(<td class="td-3" style="background: hsl(%2,%3%,%4%);"><a href="#" onclick="return lemonOpenDetail(%5,%6)" class="a-0">%1</a></td>)LEMON")
				           .arg(score)
				           .arg(h)
				           .arg(s)
				           .arg(l)
				           .arg(sourceIndex)
				           .arg(j);
			} else {
				out << QString(R"LEMON(<td class="td-3"><a href="#" onclick="return lemonOpenDetail(%2,%3)" class="a-0">%1</a></td>)LEMON")
				           .arg(tr("Invalid"))
				           .arg(sourceIndex)
				           .arg(j);
			}
		}
		out << "</tr>";
	}

	out << "</table></p>";

	// Contestant details are stored as text templates instead of DOM nodes, so that
	// browsers only render the details actually opened in a popup window (#235)
	for (int i = 0; i < contestantList.size(); i++) {
		out << QString(R"(<script type="text/template" id="c%1">)").arg(i);
		out << "<span class=\"d\">";
		out << tr("Contestant: %1").arg(contestantList[i]->getContestantName().toHtmlEscaped()) << "</span>";
		out << getContestantHtmlCode(contest, contestantList[i], i);
		out << "</script>";
	}

	out << QString(R"(<footer><p><i>Lemonlime Version %1:%2</i></p></footer>)")
	           .arg(LEMON_VERSION_STRING)
	           .arg(LEMON_VERSION_BUILD);

	out << R"LEMONJS(
	<script>
	(function () {
		"use strict";
		function styleDetailDocument(doc) {
			doc.querySelectorAll("div[id^='c'] th").forEach(e => e.classList.add("td-0"));
			doc.querySelectorAll("div[id^='c'] td").forEach(e => e.classList.add("td-0"));
			doc.querySelectorAll("div[id^='c']>p>span").forEach(e => {e.style.fontWeight = "bold"; e.style.fontSize = "large";});
			doc.querySelectorAll("div[id^='c']>p>table").forEach(e => e.style.border = "solid");
			doc.querySelectorAll("div[id^='c']>p>table th").forEach(e => e.setAttribute("scope", "col"));
		}
		window.lemonOpenDetail = function (contestantIndex, taskIndex) {
			const template = document.getElementById("c" + contestantIndex);
			if (!template)
				return false;
			const popup = window.open("", "_blank");
			if (!popup) {
				alert(document.body.dataset.popupBlocked || "Popup blocked");
				return false;
			}
			popup.document.write("<html><head><meta charset=\"utf-8\"><style>" +
				document.getElementById("lemon-style").textContent +
				"</style></head><body>" + template.textContent + "</body></html>");
			popup.document.close();
			popup.document.title = document.title;
			styleDetailDocument(popup.document);
			if (typeof taskIndex === "number") {
				const target = popup.document.getElementById("c" + contestantIndex + "p" + taskIndex);
				if (target)
					target.scrollIntoView();
			}
			popup.focus();
			return false;
		};
	})();
	</script>
	)LEMONJS";
	out << "</body>";
	out << "</html>";
}

auto ExportUtil::getSmallerContestantHtmlCode(Contest *contest, Contestant *contestant) -> QString {
	QString htmlCode;
	QList<Task *> taskList = contest->getTaskList();

	for (int i = 0; i < taskList.size(); i++) {
		htmlCode += "<p><span style=\"font-weight:bold; font-size:large;\">";
		htmlCode +=
		    QString("%1 %2</span><br>").arg(tr("Task"), taskList[i]->getProblemTitle().toHtmlEscaped());

		if (! contestant->getCheckJudged(i)) {
			htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Not judged"));
			continue;
		}

		if (taskList[i]->getTaskType() == Task::Traditional ||
		    taskList[i]->getTaskType() == Task::Interaction ||
		    taskList[i]->getTaskType() == Task::Communication ||
		    taskList[i]->getTaskType() == Task::CommunicationExec) {
			if (contestant->getCompileState(i) != CompileSuccessfully) {
				switch (contestant->getCompileState(i)) {
					case NoValidGraderFile:
						htmlCode += QString("&nbsp;&nbsp;%1</p>")
						                .arg(tr("Main grader (grader.*) cannot be found"));
						break;

					case NoValidSourceFile:
						htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Cannot find valid source file"));
						break;

					case CompileTimeLimitExceeded:
						htmlCode += QString("&nbsp;&nbsp;%1%2<br>")
						                .arg(tr("Source file: "))
						                .arg(contestant->getSourceFile(i).toHtmlEscaped());
						htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Compile time limit exceeded"));
						break;

					case InvalidCompiler:
						htmlCode += QString("&nbsp;&nbsp;%1</p>").arg(tr("Cannot run given compiler"));
						break;

					case CompileError:
						htmlCode += QString("&nbsp;&nbsp;%1%2<br>")
						                .arg(tr("Source file: "))
						                .arg(contestant->getSourceFile(i).toHtmlEscaped());
						htmlCode += QString("&nbsp;&nbsp;%1").arg(tr("Compile error"));

						if (! contestant->getCompileMessage(i).isEmpty()) {
							QString compileMessage = contestant->getCompileMessage(i).toHtmlEscaped();
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

			htmlCode += QString("&nbsp;&nbsp;%1%2")
			                .arg(tr("Source file: "), contestant->getSourceFile(i).toHtmlEscaped());
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
		QList<QList<qint64>> memoryUsed = contestant->getMemoryUsed(i);
		QList<QList<int>> score = contestant->getScore(i);

		for (int j = 0; j < inputFiles.size(); j++) {
			for (int k = 0; k < inputFiles[j].size(); k++) {
				htmlCode += "<tr>";

				if (k == 0) {
					if (score[j].size() == inputFiles[j].size())
						htmlCode +=
						    QString("<td rowspan=\"%1\">#%2</td>").arg(inputFiles[j].size()).arg(j + 1);
					else
						htmlCode += QString("<td rowspan=\"%1\">#%2<br>%3:%4</td>")
						                .arg(inputFiles[j].size())
						                .arg(j + 1)
						                .arg(tr("Subtask Dependence Status"))
						                .arg(statusRankingText(score[j].back()));
				}

				htmlCode += QString("<td>%1</td>").arg(inputFiles[j][k].toHtmlEscaped());
				QString text;
				QString bgColor;
				QString frColor;
				Settings::setTextAndColor(result[j][k], text, frColor, bgColor);
				htmlCode += QString("<td>%1").arg(text);

				if (! message[j][k].isEmpty()) {
					htmlCode +=
					    QString(
					        R"(<a href="#" data-message="%1" onclick="alert(this.dataset.message); return false;"> (...)</a>)")
					        .arg(message[j][k].toHtmlEscaped());
				}

				htmlCode += "</td>";
				htmlCode += "<td>";

				if (timeUsed[j][k] != -1) {
					htmlCode += QString("").asprintf("%.3lf s", double(timeUsed[j][k]) / 1000);
				} else {
					htmlCode += tr("Invalid");
				}

				htmlCode += "</td>";
				htmlCode += "<td>";

				if (memoryUsed[j][k] != -1) {
					htmlCode += QString("").asprintf("%.3lf MiB", double(memoryUsed[j][k]) / 1024 / 1024);
				} else {
					htmlCode += tr("Invalid");
				}

				htmlCode += "</td>";

				if (k == 0) {
					const int minv = std::ranges::min(score[j] | std::views::take(inputFiles[j].size()));
					int maxv = testCases[j]->getFullScore();

					htmlCode += QString(R"(<td rowspan="%1"><span class="c">%2</span> / %3</td>)")
					                .arg(inputFiles[j].size())
					                .arg(minv)
					                .arg(maxv);
				}

				htmlCode += "</tr>";
			}
		}

		htmlCode += "</table><br></p>";
	}

	return htmlCode;
}

void ExportUtil::exportSmallerHtml(QWidget *widget, Contest *contest, const QString &fileName) {
	QFile file(fileName);

	if (! file.open(QFile::WriteOnly)) {
		QMessageBox::warning(widget, tr("LemonLime"),
		                     tr("Cannot open file %1").arg(QFileInfo(file).fileName()), QMessageBox::Ok);
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	QTextStream out(&file);
	writeSmallerHtml(out, contest);
	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
}

void ExportUtil::writeSmallerHtml(QTextStream &out, Contest *contest) {
	QList<Contestant *> contestantList = contest->getContestantList();
	QList<Task *> taskList = contest->getTaskList();
	out << "<html><head>";
	out << R"(<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />)";
	out << "<style type=\"text/css\" id=\"lemon-style\">th, td {padding-left: 1em; padding-right: 1em; white-space: nowrap; "
	       "text-align: center; verticle-align: middle;} .a {border-style: none solid solid none; "
	       "border-width: 1px 3px; border-color: #ccc;} .b {border-style: none solid solid none; "
	       "border-width: 3px 3px; border-color: #000;} .c {font-weight: bold; font-size: large;} "
	       ".d {font-size:x-large; font-weight:bold;} .e {font-size: small;} .f {font-weight: bold;}"
	       ".g {border-style: none solid solid none; border-width: 3px 2px; border-color: #000;}</style>";
	out << "<title>" << contest->getContestTitle().toHtmlEscaped() << " : " << tr("Contest Result")
	    << "</title>";
	out << QString(R"(</head><body data-popup-blocked="%1">)")
	           .arg(tr("The browser blocked the popup window. Please allow popups for this page and try again.")
	                    .toHtmlEscaped());

	out << "<p><span class=\"d\">";
	out << "<a name=\"top\"></a>" << contest->getContestTitle().toHtmlEscaped() << " : " << tr("Rank List")
	    << "</span></p>";
	out << "<p class=\"e\">"
	    << tr("Click names to view details in a popup window. Judged By LemonLime") << "</p>";
	out << R"(<p><table border="1" cellpadding="1"><tr>)";
	out << QString("<th scope=\"col\">%1</th>").arg(tr("Rank"));
	out << QString("<th scope=\"col\">%1</th>").arg(tr("Name"));
	out << QString("<th scope=\"col\">%1</th>").arg(tr("Total Score"));

	for (auto &i : taskList)
		out << QString("<th scope=\"col\">%1</th>").arg(i->getProblemTitle().toHtmlEscaped());

	out << QString("</tr>");
	for (const auto &[contestant, allScore, sourceIndex, rank] : rankContestants(contestantList)) {
		out << QString("<tr><td>%1</td>").arg(rank);
		out << QString(R"LEMON(<td><a href="#" onclick="return lemonOpenDetail(%1)">%2</a></td>)LEMON")
		           .arg(sourceIndex)
		           .arg(contestant->getContestantName().toHtmlEscaped());

		if (allScore != -1) {
			out << QString("<td class=\"f\">%1</td>").arg(allScore);
		} else {
			out << QString("<td class=\"f\">%1</td>").arg(tr("Invalid"));
		}

		for (int j = 0; j < taskList.size(); j++) {
			int score = contestant->getTaskScore(j);

			if (score != -1) {
				out << QString("<td>%1</td>").arg(score);
			} else {
				out << QString("<td>%1</td>").arg(tr("Invalid"));
			}
		}
	}

	out << "</table></p>";

	// Contestant details are stored as text templates instead of DOM nodes, so that
	// browsers only render the details actually opened in a popup window (#235)
	for (int i = 0; i < contestantList.size(); i++) {
		out << QString(R"(<script type="text/template" id="c%1">)").arg(i);
		out << "<span class=\"d\">";
		out << tr("Contestant: %1").arg(contestantList[i]->getContestantName().toHtmlEscaped()) << "</span>";
		out << getSmallerContestantHtmlCode(contest, contestantList[i]);
		out << "</script>";
	}

	out << R"LEMONJS(
	<script>
	(function () {
		"use strict";
		window.lemonOpenDetail = function (contestantIndex) {
			const template = document.getElementById("c" + contestantIndex);
			if (!template)
				return false;
			const popup = window.open("", "_blank");
			if (!popup) {
				alert(document.body.dataset.popupBlocked || "Popup blocked");
				return false;
			}
			popup.document.write("<html><head><meta charset=\"utf-8\"><style>" +
				document.getElementById("lemon-style").textContent +
				"</style></head><body>" + template.textContent + "</body></html>");
			popup.document.close();
			popup.document.title = document.title;
			popup.focus();
			return false;
		};
	})();
	</script>
	)LEMONJS";
	out << "</body></html>";
}

void ExportUtil::exportCsv(QWidget *widget, Contest *contest, const QString &fileName) {
	QFile file(fileName);

	if (! file.open(QFile::WriteOnly)) {
		QMessageBox::warning(widget, tr("LemonLime"),
		                     tr("Cannot open file %1").arg(QFileInfo(file).fileName()), QMessageBox::Ok);
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	QTextStream out(&file);
	QList<Task *> taskList = contest->getTaskList();

	out << "\"" << tr("Rank") << "\"" << "," << "\"" << tr("Name") << "\"" << ",";

	for (auto &i : taskList) {
		out << "\"" << i->getProblemTitle() << "\"" << ",";
	}

	out << "\"" << tr("Total Score") << "\"" << Qt::endl;

	for (const auto &entry : rankContestants(contest->getContestantList())) {
		out << "\"" << entry.rank << "\"" << ",";
		out << "\"" << entry.contestant->getContestantName() << "\"" << ",";

		for (int j = 0; j < taskList.size(); j++) {
			int score = entry.contestant->getTaskScore(j);

			if (score != -1) {
				out << "\"" << score << "\"" << ",";
			} else {
				out << "\"" << tr("Invalid") << "\"" << ",";
			}
		}

		int score = entry.totalScore;

		if (score != -1) {
			out << "\"" << score << "\"" << Qt::endl;
		} else {
			out << "\"" << tr("Invalid") << "\"" << Qt::endl;
		}
	}

	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
}

#ifdef ENABLE_XLS_EXPORT
void ExportUtil::exportXls(QWidget *widget, Contest *contest, const QString &fileName) {

	if (QFile(fileName).exists()) {
		if (! QFile(fileName).remove()) {
			QMessageBox::warning(widget, tr("LemonLime"),
			                     tr("Cannot open file %1").arg(QFileInfo(fileName).fileName()),
			                     QMessageBox::Ok);
			return;
		}
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	QList<Task *> taskList = contest->getTaskList();
	const auto standings = rankContestants(contest->getContestantList());

	QAxObject *excel = new QAxObject("Excel.Application", widget);
	QAxObject *workbook = excel->querySubObject("Workbooks")->querySubObject("Add");
	QAxObject *sheet = workbook->querySubObject("ActiveSheet");
	sheet->setProperty("Name", QDate::currentDate().toString("yyyy-MM-dd"));
	sheet->querySubObject("Cells(int, int)", 1, 1)->setProperty("Value", tr("Rank"));
	sheet->querySubObject("Cells(int, int)", 1, 2)->setProperty("Value", tr("Name"));

	for (int i = 0; i < taskList.size(); i++)
		sheet->querySubObject("Cells(int, int)", 1, 3 + i)
		    ->setProperty("Value", taskList[i]->getProblemTitle());

	sheet->querySubObject("Cells(int, int)", 1, 3 + taskList.size())->setProperty("Value", tr("Total Score"));

	for (int i = 0; i < taskList.size() + 3; i++)
		sheet->querySubObject("Cells(int, int)", 1, i + 1)->querySubObject("Font")->setProperty("Bold", true);

	for (int i = 0; i < standings.size(); i++) {
		const auto &entry = standings[i];
		sheet->querySubObject("Cells(int, int)", 2 + i, 1)->setProperty("Value", entry.rank);
		sheet->querySubObject("Cells(int, int)", 2 + i, 2)
		    ->setProperty("Value", entry.contestant->getContestantName());

		for (int j = 0; j < taskList.size(); j++) {
			int score = entry.contestant->getTaskScore(j);

			if (score != -1) {
				sheet->querySubObject("Cells(int, int)", 2 + i, 3 + j)->setProperty("Value", score);
			} else {
				sheet->querySubObject("Cells(int, int)", 2 + i, 3 + j)->setProperty("Value", tr("Invalid"));
			}
		}

		int score = entry.totalScore;

		if (score != -1) {
			sheet->querySubObject("Cells(int, int)", 2 + i, 3 + taskList.size())->setProperty("Value", score);
		} else {
			sheet->querySubObject("Cells(int, int)", 2 + i, 3 + taskList.size())
			    ->setProperty("Value", tr("Invalid"));
		}
	}

	workbook->dynamicCall("SaveAs(const QString&, int)", QDir::toNativeSeparators(fileName), -4143);
	excel->dynamicCall("Quit()");
	delete excel;
	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
}
#endif

void ExportUtil::exportResult(QWidget *widget, Contest *contest) {
	QList<Contestant *> contestantList = contest->getContestantList();
	QList<Task *> taskList = contest->getTaskList();

	if (contestantList.isEmpty()) {
		QMessageBox::warning(widget, tr("LemonLime"), tr("No contestant in current contest"),
		                     QMessageBox::Ok);
		return;
	}

	if (taskList.isEmpty()) {
		QMessageBox::warning(widget, tr("LemonLime"), tr("No task in current contest"), QMessageBox::Ok);
		return;
	}

	QString filter = tr("HTML Document (*.html *.htm);;CSV (*.csv)");
#ifdef ENABLE_XLS_EXPORT
	QAxObject *excel = new QAxObject("Excel.Application", widget);

	if (! excel->isNull())
		filter = filter + tr(";;Excel Workbook (*.xls)");

	delete excel;
#endif
	QString fileName = QFileDialog::getSaveFileName(
	    widget, tr("Export Result"), QDir::currentPath() + QDir::separator() + "result.html", filter);

	if (fileName.isEmpty())
		return;
	// TODO: refactor
	if (QFileInfo(fileName).suffix() == "html")
		exportHtml(widget, contest, fileName);

	if (QFileInfo(fileName).suffix() == "htm")
		exportSmallerHtml(widget, contest, fileName);

	if (QFileInfo(fileName).suffix() == "csv")
		exportCsv(widget, contest, fileName);
#ifdef ENABLE_XLS_EXPORT
	if (QFileInfo(fileName).suffix() == "xls")
		exportXls(widget, contest, fileName);
#endif
}
