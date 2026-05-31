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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <algorithm>
#include <cmath>
#include <iostream>

ExportUtil::ExportUtil(QObject *parent) : QObject(parent) {}

QJsonObject ExportUtil::buildExportJson(Contest *contest) {
	Settings settings;
	contest->copySettings(settings);
	ColorTheme colors = settings.getCurrentColorTheme();

	QList<Contestant *> contestantList = contest->getContestantList();
	QList<Task *> taskList = contest->getTaskList();
	int sfullScore = contest->getTotalScore();

	QList<std::pair<int, QString>> sortList;

	for (auto &i : contestantList) {
		int totalScore = i->getTotalScore();

		if (totalScore != -1) {
			sortList.append(std::make_pair(totalScore, i->getContestantName()));
		} else {
			sortList.append(std::make_pair(-1, i->getContestantName()));
		}
	}

	std::sort(sortList.begin(), sortList.end(), std::greater<>());
	QMap<QString, int> rankList;

	for (int i = 0; i < sortList.size(); i++) {
		if (i > 0 && sortList[i].first == sortList[i - 1].first) {
			rankList.insert(sortList[i].second, rankList[sortList[i - 1].second]);
		} else {
			rankList.insert(sortList[i].second, i);
		}
	}

	QJsonObject root;
	root["name"] = contest->getContestTitle();
	root["version"] = QString("Lemonlime Version %1:%2").arg(LEMON_VERSION_STRING).arg(LEMON_VERSION_BUILD);

	QJsonObject i18n;
	i18n["rank_list"] = tr("Rank List");
	i18n["hint"] = tr("Click names or task scores to jump to details. Judged By LemonLime");
	i18n["rank"] = tr("Rank");
	i18n["name"] = tr("Name");
	i18n["total"] = tr("Total Score");
	i18n["contestant"] = tr("Contestant");
	i18n["task"] = tr("Task");
	i18n["testcase"] = tr("Test Case");
	i18n["input"] = tr("Input File");
	i18n["result"] = tr("Result");
	i18n["time"] = tr("Time Used");
	i18n["memory"] = tr("Memory Used");
	i18n["score"] = tr("Score");
	i18n["return_to_top"] = tr("Return to top");
	root["i18n"] = i18n;

	QJsonArray taskNames;

	for (auto &task : taskList) {
		taskNames.append(task->getProblemTitle());
	}

	root["task_names"] = taskNames;

	QJsonArray contestantsArr;

	for (int idx = 0; idx < contestantList.size(); idx++) {
		Contestant *contestant = contestantList[idx];
		QJsonObject cObj;
		cObj["name"] = contestant->getContestantName();
		cObj["rank"] = rankList[contestant->getContestantName()] + 1;

		int allScore = contestant->getTotalScore();

		if (allScore >= 0) {
			cObj["total_score"] = allScore;

			float h = NAN;
			float s = NAN;
			float l = NAN;

			colors.getColorGrand(allScore, sfullScore).getHslF(&h, &s, &l);
			h *= 360, s *= 100, l *= 100;
			cObj["total_bg"] = QString("%1, %2%, %3%").arg(h).arg(s).arg(l);
			cObj["total_border"] = QString("%1, %2%, %3%").arg(h).arg(s).arg(qMax(l - 20, 0.00));
		} else {
			cObj["total_score"] = 0;
			cObj["total_bg"] = QString("0, 0%, 90%");
			cObj["total_border"] = QString("0, 0%, 70%");
		}

		QJsonArray tasksArr;

		for (int j = 0; j < taskList.size(); j++) {
			QJsonObject tObj;
			int score = contestant->getTaskScore(j);

			if (score >= 0) {
				tObj["score"] = score;

				float h = NAN;
				float s = NAN;
				float l = NAN;

				QColor col = colors.getColorPer(score, taskList[j]->getTotalScore());
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
				tObj["bg"] = QString("%1, %2%, %3%").arg(h).arg(s).arg(l);
			} else {
				tObj["score"] = 0;
				tObj["bg"] = QString("0, 0%, 90%");
			}

			if (taskList[j]->getTaskType() == Task::Traditional ||
			    taskList[j]->getTaskType() == Task::Interaction ||
			    taskList[j]->getTaskType() == Task::Communication ||
			    taskList[j]->getTaskType() == Task::CommunicationExec) {
				QString info;

				if (! contestant->getCheckJudged(j)) {
					info = tr("Not judged");
				} else {
					switch (contestant->getCompileState(j)) {
						case CompileSuccessfully:
							info = tr("Source file: ") + contestant->getSourceFile(j);
							break;
						case NoValidGraderFile:
							info = tr("Main grader (grader.*) cannot be found");
							break;
						case NoValidSourceFile:
							info = tr("Cannot find valid source file");
							break;
						case CompileTimeLimitExceeded:
							info = tr("Source file: ") + contestant->getSourceFile(j) +
							       QString(", ") + tr("Compile time limit exceeded");
							break;
						case InvalidCompiler:
							info = tr("Cannot run given compiler");
							break;
						case CompileError:
							info = tr("Source file: ") + contestant->getSourceFile(j) +
							       QString(", ") + tr("Compile error");
							break;
						default:
							break;
					}
				}

				if (! info.isEmpty()) {
					tObj["info"] = info;
				}

				if (contestant->getCheckJudged(j) &&
				    contestant->getCompileState(j) == CompileError &&
				    ! contestant->getCompileMessage(j).isEmpty()) {
					tObj["compile_message"] = contestant->getCompileMessage(j);
				}
			} else if (! contestant->getCheckJudged(j)) {
				tObj["info"] = tr("Not judged");
			}

			bool isAnswersOnly = taskList[j]->getTaskType() == Task::AnswersOnly;
			bool canShowDetails = contestant->getCheckJudged(j) &&
			                      (isAnswersOnly || contestant->getCompileState(j) == CompileSuccessfully);

			if (canShowDetails) {
				QList<TestCase *> testCases = taskList[j]->getTestCaseList();
				QList<QStringList> inputFiles = contestant->getInputFiles(j);
				QList<QList<ResultState>> result = contestant->getResult(j);
				QList<QStringList> message = contestant->getMessage(j);
				QList<QList<int>> timeUsed = contestant->getTimeUsed(j);
				QList<QList<qint64>> memoryUsed = contestant->getMemoryUsed(j);
				QList<QList<int>> score = contestant->getScore(j);

				QJsonArray detailsArr;

				for (int jj = 0; jj < inputFiles.size(); jj++) {
					for (int k = 0; k < inputFiles[jj].size(); k++) {
						QJsonObject dObj;

						if (k == 0) {
							QString label = QString("#%1").arg(jj + 1);

							if (score[jj].size() != inputFiles[jj].size()) {
								label += QString("<br>%1:%2")
								             .arg(tr("Subtask Dependence Status"))
								             .arg(statusRankingText(score[jj].back()));
							}

							dObj["label"] = label;
							dObj["row_span"] = inputFiles[jj].size();
						} else {
							dObj["label"] = QString("");
							dObj["row_span"] = 0;
						}

						dObj["input"] = inputFiles[jj][k];

						QString text;
						QString frColor;
						QString bgColor;
						Settings::setTextAndColor(result[jj][k], text, frColor, bgColor);
						dObj["result"] = text;

						dObj["bg"] = bgColor;

						if (timeUsed[jj][k] != -1) {
							dObj["time"] = QString("").asprintf("%.3lf s", double(timeUsed[jj][k]) / 1000);
						} else {
							dObj["time"] = tr("Invalid");
						}

						if (memoryUsed[jj][k] != -1) {
							dObj["memory"] =
							    QString("").asprintf("%.3lf MiB", double(memoryUsed[jj][k]) / 1024 / 1024);
						} else {
							dObj["memory"] = tr("Invalid");
						}

						if (k == 0) {
							int minv = 2147483647;
							int maxv = testCases[jj]->getFullScore();

							for (int t = 0; t < inputFiles[jj].size(); t++)
								if (score[jj][t] < minv)
									minv = score[jj][t];

							dObj["score"] = minv;
							dObj["full_score"] = maxv;
						} else {
							dObj["score"] = 0;
							dObj["full_score"] = 0;
						}

						if (! message[jj][k].isEmpty()) {
							dObj["info"] = message[jj][k];
						}

						detailsArr.append(dObj);
					}
				}

				tObj["details"] = detailsArr;
			}

			tasksArr.append(tObj);
		}

		cObj["tasks"] = tasksArr;
		contestantsArr.append(cObj);
	}

	root["contestants"] = contestantsArr;
	return root;
}

void ExportUtil::exportHtml(QWidget *widget, Contest *contest, const QString &fileName) {
	QFile file(fileName);

	if (! file.open(QFile::WriteOnly)) {
		QMessageBox::warning(widget, tr("LemonLime"),
		                     tr("Cannot open file %1").arg(QFileInfo(file).fileName()), QMessageBox::Ok);
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	QFile templateFile(":/export/export_template.html");

	if (! templateFile.open(QFile::ReadOnly | QFile::Text)) {
		QApplication::restoreOverrideCursor();
		QMessageBox::warning(widget, tr("LemonLime"), tr("Cannot read export template"), QMessageBox::Ok);
		return;
	}

	QString htmlTemplate = templateFile.readAll();
	templateFile.close();

	QJsonObject jsonData = buildExportJson(contest);
	QJsonDocument doc(jsonData);
	QString jsonStr = doc.toJson(QJsonDocument::Compact);

	htmlTemplate.replace("%%DATA%%", jsonStr);

	QTextStream out(&file);
	out << htmlTemplate;
	out.flush();
	file.close();

	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
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
	QList<Contestant *> contestantList = contest->getContestantList();
	QList<Task *> taskList = contest->getTaskList();
	QList<std::pair<int, QString>> sortList;

	for (auto &i : contestantList) {
		int totalScore = i->getTotalScore();

		if (totalScore != -1) {
			sortList.append(std::make_pair(totalScore, i->getContestantName()));
		} else {
			sortList.append(std::make_pair(-1, i->getContestantName()));
		}
	}

	std::sort(sortList.begin(), sortList.end(), std::greater<>());
	QMap<QString, int> rankList;

	for (int i = 0; i < sortList.size(); i++) {
		if (i > 0 && sortList[i].first == sortList[i - 1].first) {
			rankList.insert(sortList[i].second, rankList[sortList[i - 1].second]);
		} else {
			rankList.insert(sortList[i].second, i);
		}
	}

	out << "\"" << tr("Rank") << "\"" << "," << "\"" << tr("Name") << "\"" << ",";

	for (auto &i : taskList) {
		out << "\"" << i->getProblemTitle() << "\"" << ",";
	}

	out << "\"" << tr("Total Score") << "\"" << Qt::endl;

	for (auto &i : sortList) {
		Contestant *contestant = contest->getContestant(i.second);
		out << "\"" << rankList[contestant->getContestantName()] + 1 << "\"" << ",";
		out << "\"" << i.second << "\"" << ",";

		for (int j = 0; j < taskList.size(); j++) {
			int score = contestant->getTaskScore(j);

			if (score != -1) {
				out << "\"" << score << "\"" << ",";
			} else {
				out << "\"" << tr("Invalid") << "\"" << ",";
			}
		}

		int score = contestant->getTotalScore();

		if (score != -1) {
			out << "\"" << score << "\"" << Qt::endl;
		} else {
			out << "\"" << tr("Invalid") << "\"" << Qt::endl;
		}
	}

	out.flush();
	file.close();

	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonLime"), tr("Export is done"), QMessageBox::Ok);
}

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

	QString filter = tr("HTML Document (*.html);;CSV (*.csv)");
	QString fileName = QFileDialog::getSaveFileName(
	    widget, tr("Export Result"), QDir::currentPath() + QDir::separator() + "result.html", filter);

	if (fileName.isEmpty())
		return;

	if (QFileInfo(fileName).suffix() == "html")
		exportHtml(widget, contest, fileName);

	if (QFileInfo(fileName).suffix() == "csv")
		exportCsv(widget, contest, fileName);
}
