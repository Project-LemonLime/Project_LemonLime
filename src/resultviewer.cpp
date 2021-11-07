/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "resultviewer.h"
//
#include "base/LemonLog.hpp"
#include "base/LemonType.hpp"
#include "base/settings.h"
#include "core/contest.h"
#include "core/contestant.h"
#include "core/task.h"
#include "detaildialog.h"
#include "judgingdialog.h"
//
#include <QApplication>
#include <QCheckBox>
#include <QDir>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <algorithm>

#define LEMON_MODULE_NAME "ResultViewer"

ResultViewer::ResultViewer(QWidget *parent) : QTableWidget(parent) {
	curContest = nullptr;
	deleteContestantAction = new QAction(tr("Delete"), this);
	detailInformationAction = new QAction(tr("Details"), this);
	judgeSelectedAction = new QAction(tr("Judge"), this);
	deleteContestantKeyAction = new QAction(this);
	deleteContestantKeyAction->setShortcut(QKeySequence::Delete);
	deleteContestantKeyAction->setShortcutContext(Qt::WidgetShortcut);
	addAction(deleteContestantKeyAction);
	connect(deleteContestantAction, &QAction::triggered, this, &ResultViewer::deleteContestant);
	connect(detailInformationAction, &QAction::triggered, this, &ResultViewer::detailInformation);
	connect(judgeSelectedAction, &QAction::triggered, this, &ResultViewer::judgeSelected);
	connect(deleteContestantKeyAction, &QAction::triggered, this, &ResultViewer::deleteContestant);
	connect(this, &ResultViewer::cellDoubleClicked, this, &ResultViewer::detailInformation);
}

void ResultViewer::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		deleteContestantAction->setText(QApplication::translate("ResultViewer", "Delete", nullptr));
		detailInformationAction->setText(QApplication::translate("ResultViewer", "Details", nullptr));
		judgeSelectedAction->setText(QApplication::translate("ResultViewer", "Judge", nullptr));
	}
}

void ResultViewer::contextMenuEvent(QContextMenuEvent * /*event*/) {
	QList<QTableWidgetSelectionRange> selectionRange = selectedRanges();

	if (selectionRange.empty())
		return;

	auto *contextMenu = new QMenu(this);

	if (selectionRange.size() == 1 && selectionRange[0].rowCount() == 1) {
		contextMenu->addAction(detailInformationAction);
		contextMenu->setDefaultAction(detailInformationAction);
	}

	if (! selectionRange.empty()) {
		contextMenu->addAction(judgeSelectedAction);
	}

	contextMenu->addAction(deleteContestantAction);
	contextMenu->exec(QCursor::pos());
	delete contextMenu;
}

void ResultViewer::setContest(Contest *contest) {
	if (curContest) {
		disconnect(curContest, &Contest::taskAddedForViewer, this, &ResultViewer::refreshViewer);
		disconnect(curContest, &Contest::taskDeletedForViewer, this, &ResultViewer::refreshViewer);
		disconnect(curContest, &Contest::problemTitleChanged, this, &ResultViewer::refreshViewer);
		disconnect(curContest, &Contest::taskJudgingFinished, this, &ResultViewer::refreshViewer);
	}

	curContest = contest;

	if (! curContest)
		return;

	connect(curContest, &Contest::taskAddedForViewer, this, &ResultViewer::refreshViewer);
	connect(curContest, &Contest::taskDeletedForViewer, this, &ResultViewer::refreshViewer);
	connect(curContest, &Contest::problemTitleChanged, this, &ResultViewer::refreshViewer);
	connect(curContest, &Contest::taskJudgingFinished, this, &ResultViewer::refreshViewer);
}

void ResultViewer::refreshViewer() {
	clear();
	setRowCount(0);
	setColumnCount(0);

	if (! curContest)
		return;

	QStringList headerList;
	headerList << tr("Rank") << tr("Name") << tr("Total Score");
	QList<Task *> taskList = curContest->getTaskList();
	Settings setting;
	curContest->copySettings(setting);
	const ColorTheme *colors = setting.getCurrentColorTheme();

	for (auto &i : taskList) {
		headerList << i->getProblemTitle();
	}

	headerList << tr("Total Used Time (s)") << tr("Judging Time");
	setColumnCount(taskList.size() + 5);
	setHorizontalHeaderLabels(headerList);
	horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	QList<Contestant *> contestantList = curContest->getContestantList();
	QList<std::pair<int, QString>> sortList;
	QList<int> fullScore;
	int sfullScore = curContest->getTotalScore();

	for (auto &i : taskList) {
		fullScore.append(i->getTotalScore());
	}

	setRowCount(contestantList.size());

	for (int i = 0; i < contestantList.size(); i++) {
		setItem(i, 0, new QTableWidgetItem());
		setItem(i, 1, new QTableWidgetItem(contestantList[i]->getContestantName()));
		setItem(i, 2, new QTableWidgetItem());

		for (int j = 0; j < taskList.size(); j++) {
			setItem(i, j + 3, new QTableWidgetItem());
			int score = contestantList[i]->getTaskScore(j);

			if (score != -1) {
				item(i, j + 3)->setData(Qt::DisplayRole, score);
				QColor bg = QColor::fromHsl(0, 0, 255);

				if (taskList[j]->getTaskType() != Task::AnswersOnly &&
				    contestantList[i]->getCompileState(j) != CompileSuccessfully) {
					if (contestantList[i]->getCompileState(j) == NoValidSourceFile)
						bg = colors->getColorNf();
					else
						bg = colors->getColorCe();
				} else
					bg = colors->getColorPer(score, fullScore[j]);

				item(i, j + 3)->setBackground(bg);

				// qDebug() << i << j << bg;
			} else {
				item(i, j + 3)->setText(tr("Invalid"));
			}
		}

		setItem(i, taskList.size() + 3, new QTableWidgetItem());
		setItem(i, taskList.size() + 4, new QTableWidgetItem());
		int totalScore = contestantList[i]->getTotalScore();
		int totalUsedTime = contestantList[i]->getTotalUsedTime();
		QDateTime judgingTime = contestantList[i]->getJudingTime();

		if (totalScore != -1) {
			item(i, 2)->setData(Qt::DisplayRole, totalScore);
			item(i, 2)->setBackground(colors->getColorGrand(totalScore, sfullScore));
			QFont font;
			font.setBold(true);
			item(i, 2)->setFont(font);
			item(i, taskList.size() + 3)->setData(Qt::DisplayRole, double(totalUsedTime) / 1000);
			item(i, taskList.size() + 4)
			    ->setData(Qt::DisplayRole, judgingTime.toString("yyyy-MM-dd hh:mm:ss"));
			sortList.append(std::make_pair(-totalScore, contestantList[i]->getContestantName()));
		} else {
			item(i, 2)->setText(tr("Invalid"));
			item(i, taskList.size() + 3)->setText(tr("Invalid"));
			item(i, taskList.size() + 4)->setText(tr("Invalid"));
		}
	}

	std::sort(sortList.begin(), sortList.end());
	QMap<QString, int> rankList;

	for (int i = 0; i < sortList.size(); i++) {
		if (i > 0 && sortList[i].first == sortList[i - 1].first) {
			rankList.insert(sortList[i].second, rankList[sortList[i - 1].second]);
		} else {
			rankList.insert(sortList[i].second, i);
		}
	}

	for (int i = 0; i < rowCount(); i++) {
		if (rankList.contains(contestantList[i]->getContestantName())) {
			item(i, 0)->setData(Qt::DisplayRole, rankList[contestantList[i]->getContestantName()] + 1);
		} else {
			item(i, 0)->setText(tr("Invalid"));
		}
	}

	for (int i = 0; i < rowCount(); i++) {
		for (int j = 0; j < columnCount(); j++) {
			item(i, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		}
	}

	sortByColumn(0, Qt::AscendingOrder);
}

void ResultViewer::judgeSelected() {
	QList<QTableWidgetSelectionRange> selectionRange = selectedRanges();
	QMap<QString, QSet<int>> mapping;
	QList<Task *> taskList = curContest->getTaskList();
	int taskSize = taskList.size();

	for (auto &i : selectionRange) {
		for (int j = i.topRow(); j <= i.bottomRow(); j++) {
			for (int k = i.leftColumn(); k <= i.rightColumn(); k++) {
				if (3 <= k && k < 3 + taskSize)
					mapping[item(j, 1)->text()].insert(k - 3);
				else {
					for (int a = 0; a < taskSize; a++)
						mapping[item(j, 1)->text()].insert(a);
				}
			}
		}
	}

	QList<std::pair<QString, QVector<int>>> judgeList;

	for (QMap<QString, QSet<int>>::const_iterator i = mapping.constBegin(); i != mapping.constEnd(); ++i) {
		judgeList.append(std::make_pair(i.key(), QVector<int>(i.value().constBegin(), i.value().constEnd())));
	}

	auto *dialog = new JudgingDialog(this);
	dialog->setModal(true);
	dialog->setContest(curContest);
	dialog->show();
	dialog->judge(judgeList);
	delete dialog;
	refreshViewer();
}

void ResultViewer::judgeAll() {
	auto *dialog = new JudgingDialog(this);
	dialog->setModal(true);
	dialog->setContest(curContest);
	dialog->show();
	dialog->judgeAll();
	delete dialog;
	refreshViewer();
}

void ResultViewer::judgeUnjudged() {
	QMap<QString, QVector<int>> mapping;
	QList<Contestant *> contestantList = curContest->getContestantList();
	QList<Task *> taskList = curContest->getTaskList();
	int contestantSize = contestantList.size();
	int taskSize = taskList.size();

	for (int i = 0; i < contestantSize; i++) {
		for (int j = 0; j < taskSize; j++) {
			if (item(i, j + 3)->text() == tr("Invalid")) {
				mapping[item(i, 1)->text()].push_back(j);
			}
		}
	}

	QList<std::pair<QString, QVector<int>>> judgeList;

	for (QMap<QString, QVector<int>>::const_iterator i = mapping.constBegin(); i != mapping.constEnd(); ++i) {
		judgeList.push_back({i.key(), i.value()});
	}

	auto *dialog = new JudgingDialog(this);
	dialog->setModal(true);
	dialog->setContest(curContest);
	dialog->show();
	dialog->judge(judgeList);
	delete dialog;
	refreshViewer();
}

void ResultViewer::judgeGrey() {
	QMap<QString, QVector<int>> mapping;
	QList<Contestant *> contestantList = curContest->getContestantList();
	QList<Task *> taskList = curContest->getTaskList();
	int contestantSize = contestantList.size();
	int taskSize = taskList.size();

	for (int i = 0; i < contestantSize; i++) {
		for (int j = 0; j < taskSize; j++) {
			if (contestantList[i]->getCompileState(j) == NoValidSourceFile) {
				mapping[contestantList[i]->getContestantName()].push_back(j);
			}
		}
	}

	QList<std::pair<QString, QVector<int>>> judgeList;

	for (QMap<QString, QVector<int>>::const_iterator i = mapping.constBegin(); i != mapping.constEnd(); ++i) {
		judgeList.append(std::make_pair(i.key(), i.value()));
	}

	auto *dialog = new JudgingDialog(this);
	dialog->setModal(true);
	dialog->setContest(curContest);
	dialog->show();
	dialog->judge(judgeList);
	delete dialog;
	refreshViewer();
}

void ResultViewer::judgeMagenta() {
	QMap<QString, QVector<int>> mapping;
	QList<Contestant *> contestantList = curContest->getContestantList();
	QList<Task *> taskList = curContest->getTaskList();
	int contestantSize = contestantList.size();
	int taskSize = taskList.size();

	for (int i = 0; i < contestantSize; i++) {
		for (int j = 0; j < taskSize; j++) {
			if (taskList[j]->getTaskType() != Task::AnswersOnly &&
			    contestantList[i]->getCompileState(j) != CompileSuccessfully &&
			    contestantList[i]->getCompileState(j) != NoValidSourceFile &&
			    contestantList[i]->getCompileState(j) != NoValidGraderFile) {
				mapping[contestantList[i]->getContestantName()].push_back(j);
			}
		}
	}

	QList<std::pair<QString, QVector<int>>> judgeList;

	for (QMap<QString, QVector<int>>::const_iterator i = mapping.constBegin(); i != mapping.constEnd(); ++i) {
		judgeList.append(std::make_pair(i.key(), i.value()));
	}

	auto *dialog = new JudgingDialog(this);
	dialog->setModal(true);
	dialog->setContest(curContest);
	dialog->show();
	dialog->judge(judgeList);
	delete dialog;
	refreshViewer();
}

void ResultViewer::clearPath(const QString &curDir) {
	QDir dir(curDir);
	QStringList fileList = dir.entryList(QDir::Files);

	for (int i = 0; i < fileList.size(); i++) {
		dir.remove(fileList[i]);
	}

	QStringList dirList = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (int i = 0; i < dirList.size(); i++) {
		clearPath(curDir + dirList[i] + QDir::separator());
		dir.rmdir(dirList[i]);
	}
}

void ResultViewer::deleteContestant() {
	auto *messageBox = new QMessageBox(QMessageBox::Warning, tr("LemonLime"),
	                                   QString("<span style=\"font-size:large\">") +
	                                       tr("Are you sure to delete selected contestant(s)?") + "</span>",
	                                   QMessageBox::Ok | QMessageBox::Cancel, this);
	// QHBoxLayout *layout = new QHBoxLayout;
	auto *checkBox = new QCheckBox(tr("Delete data in the disk as well"));
	// layout->addWidget(checkBox);
	// layout->setAlignment(checkBox, Qt::AlignHCenter);
	// dynamic_cast<QGridLayout*>(messageBox->layout())->addLayout(layout, 1, 1);
	// dynamic_cast<QGridLayout*>(messageBox->layout())->setVerticalSpacing(10);
	messageBox->setCheckBox(checkBox);

	if (messageBox->exec() != QMessageBox::Ok)
		return;

	QList<QTableWidgetSelectionRange> selectionRange = selectedRanges();

	for (auto &i : selectionRange) {
		for (int j = i.topRow(); j <= i.bottomRow(); j++) {
			curContest->deleteContestant(item(j, 1)->text());

			if (checkBox->isChecked()) {
				clearPath(Settings::sourcePath() + item(j, 1)->text() + QDir::separator());
				QDir(Settings::sourcePath()).rmdir(item(j, 1)->text());
			}
		}
	}

	delete checkBox;
	delete messageBox;
	refreshViewer();
	emit contestantDeleted();
}

void ResultViewer::detailInformation() {
	QList<QTableWidgetSelectionRange> selectionRange = selectedRanges();
	int index = selectionRange[0].topRow();
	auto *dialog = new DetailDialog(this);
	dialog->setModal(true);
	dialog->refreshViewer(curContest, curContest->getContestant(item(index, 1)->text()));
	connect(dialog, &DetailDialog::rejudgeSignal, this, &ResultViewer::refreshViewer);
	dialog->showDialog();
	delete dialog;
}
