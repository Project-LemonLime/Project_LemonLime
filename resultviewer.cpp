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
 * resultviewer.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * resultviewer.cpp @Project LemonLime
 * Update 2019 iotang
 **/

#include "resultviewer.h"
#include "judgingdialog.h"
#include "globaltype.h"
#include "contestant.h"
#include "settings.h"
#include "contest.h"
#include "task.h"
#include "detaildialog.h"
#include <algorithm>
#include <QMessageBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>

ResultViewer::ResultViewer(QWidget *parent) :
	QTableWidget(parent)
{
	curContest = nullptr;

	deleteContestantAction = new QAction(tr("Delete"), this);
	detailInformationAction = new QAction(tr("Details"), this);
	judgeSelectedAction = new QAction(tr("Judge"), this);
	deleteContestantKeyAction = new QAction(this);
	deleteContestantKeyAction->setShortcut(QKeySequence::Delete);
	deleteContestantKeyAction->setShortcutContext(Qt::WidgetShortcut);
	addAction(deleteContestantKeyAction);
	connect(deleteContestantAction, SIGNAL(triggered()),
	        this, SLOT(deleteContestant()));
	connect(detailInformationAction, SIGNAL(triggered()),
	        this, SLOT(detailInformation()));
	connect(judgeSelectedAction, SIGNAL(triggered()),
	        this, SLOT(judgeSelected()));
	connect(deleteContestantKeyAction, SIGNAL(triggered()),
	        this, SLOT(deleteContestant()));
	connect(this, SIGNAL(cellDoubleClicked(int, int)),
	        this, SLOT(detailInformation()));
}

void ResultViewer::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		deleteContestantAction->setText(QApplication::translate("ResultViewer", "Delete",
		                                nullptr));
		detailInformationAction->setText(QApplication::translate("ResultViewer", "Details",
		                                 nullptr));
		judgeSelectedAction->setText(QApplication::translate("ResultViewer", "Judge",
		                             nullptr));
	}
}

void ResultViewer::contextMenuEvent(QContextMenuEvent */*event*/)
{
	QList<QTableWidgetSelectionRange> selectionRange = selectedRanges();

	if (selectionRange.empty()) return;

	auto *contextMenu = new QMenu(this);

	if (selectionRange.size() == 1 && selectionRange[0].rowCount() == 1)
	{
		contextMenu->addAction(detailInformationAction);
		contextMenu->setDefaultAction(detailInformationAction);
	}

	if (!selectionRange.empty())
	{
		contextMenu->addAction(judgeSelectedAction);
	}

	contextMenu->addAction(deleteContestantAction);
	contextMenu->exec(QCursor::pos());
	delete contextMenu;
}

void ResultViewer::setContest(Contest *contest)
{
	if (curContest)
	{
		disconnect(curContest, SIGNAL(taskAddedForViewer()),
		           this, SLOT(refreshViewer()));
		disconnect(curContest, SIGNAL(taskDeletedForViewer(int)),
		           this, SLOT(refreshViewer()));
		disconnect(curContest, SIGNAL(problemTitleChanged()),
		           this, SLOT(refreshViewer()));
		disconnect(curContest, SIGNAL(taskJudgingFinished()),
		           this, SLOT(refreshViewer()));
	}

	curContest = contest;

	if (! curContest) return;

	connect(curContest, SIGNAL(taskAddedForViewer()),
	        this, SLOT(refreshViewer()));
	connect(curContest, SIGNAL(taskDeletedForViewer(int)),
	        this, SLOT(refreshViewer()));
	connect(curContest, SIGNAL(problemTitleChanged()),
	        this, SLOT(refreshViewer()));
	connect(curContest, SIGNAL(taskJudgingFinished()),
	        this, SLOT(refreshViewer()));
}

void ResultViewer::refreshViewer()
{
	clear();
	setRowCount(0);
	setColumnCount(0);

	if (! curContest) return;

	QStringList headerList;
	headerList << tr("Rank") << tr("Name") << tr("Total Score");
	QList<Task *> taskList = curContest->getTaskList();

	Settings setting;
	curContest->copySettings(setting);

	for (auto &i : taskList)
	{
		headerList << i->getProblemTile();
	}

	headerList << tr("Total Used Time (s)") << tr("Judging Time");
	setColumnCount(taskList.size() + 5);
	setHorizontalHeaderLabels(headerList);
	horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	QList<Contestant *> contestantList = curContest->getContestantList();
	QList< QPair<int, QString> > sortList;
	QList<int> fullScore;
	int sfullScore = curContest->getTotalScore();

	for (auto &i : taskList)
	{
		fullScore.append(i->getTotalScore());
	}

	setRowCount(contestantList.size());

	for (int i = 0; i < contestantList.size(); i ++)
	{
		setItem(i, 0, new QTableWidgetItem());
		setItem(i, 1, new QTableWidgetItem(contestantList[i]->getContestantName()));
		setItem(i, 2, new QTableWidgetItem());

		for (int j = 0; j < taskList.size(); j ++)
		{
			setItem(i, j + 3, new QTableWidgetItem());
			int score = contestantList[i]->getTaskScore(j);

			if (score != -1)
			{
				item(i, j + 3)->setData(Qt::DisplayRole, score);

				QColor bg = QColor::fromHsl(0, 0, 255);

				if (taskList[j]->getTaskType() != Task::AnswersOnly && contestantList[i]->getCompileState(j) != CompileSuccessfully)
				{
					if (contestantList[i]->getCompileState(j) == NoValidSourceFile)
						bg = setting.getColorNf();
					//	bg = QColor::fromHslF(nofBaseColorHF, nofBaseColorSF, nofBaseColorLF);
					else bg = setting.getColorCe();
					// bg = QColor::fromHslF(cmeBaseColorHF, cmeBaseColorSF, cmeBaseColorLF);
				}
				else bg = setting.getColorPer(score, fullScore[j]);
				//bg = QColor::fromHslF(oriBaseColorHF, oriBaseColorSF, oriBaseColorLF(score, fullScore[j], 0.3));

				item(i, j + 3)->setBackground(bg);
			}
			else
			{
				item(i, j + 3)->setText(tr("Invalid"));
			}
		}

		setItem(i, taskList.size() + 3, new QTableWidgetItem());
		setItem(i, taskList.size() + 4, new QTableWidgetItem());
		int totalScore = contestantList[i]->getTotalScore();
		int totalUsedTime = contestantList[i]->getTotalUsedTime();
		QDateTime judgingTime = contestantList[i]->getJudingTime();

		if (totalScore != -1)
		{
			item(i, 2)->setData(Qt::DisplayRole, totalScore);
			item(i, 2)->setBackground(setting.getColorGrand(totalScore, sfullScore));

			QFont font;
			font.setBold(true);
			item(i, 2)->setFont(font);

			item(i, taskList.size() + 3)->setData(Qt::DisplayRole, double (totalUsedTime) / 1000);
			item(i, taskList.size() + 4)->setData(Qt::DisplayRole, judgingTime.toString("yyyy-MM-dd hh:mm:ss"));
			sortList.append(qMakePair(-totalScore, contestantList[i]->getContestantName()));
		}
		else
		{
			item(i, 2)->setText(tr("Invalid"));
			item(i, taskList.size() + 3)->setText(tr("Invalid"));
			item(i, taskList.size() + 4)->setText(tr("Invalid"));
		}
	}

	std::sort(sortList.begin(), sortList.end());
	QMap<QString, int> rankList;

	for (int i = 0; i < sortList.size(); i ++)
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

	for (int i = 0; i < rowCount(); i ++)
	{
		if (rankList.contains(contestantList[i]->getContestantName()))
		{
			item(i, 0)->setData(Qt::DisplayRole, rankList[contestantList[i]->getContestantName()] + 1);
		}
		else
		{
			item(i, 0)->setText(tr("Invalid"));
		}
	}

	for (int i = 0; i < rowCount(); i ++)
	{
		for (int j = 0; j < columnCount(); j ++)
		{
			item(i, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		}
	}

	sortByColumn(0, Qt::AscendingOrder);
}

void ResultViewer::judgeSelected()
{
	QList<QTableWidgetSelectionRange> selectionRange = selectedRanges();

	QMap<QString, QSet<int> > mapping;

	QList<Task *> taskList = curContest->getTaskList();
	int taskSize = taskList.size();

	for (auto &i : selectionRange)
	{
		for (int j = i.topRow(); j <= i.bottomRow(); j ++)
		{
			for (int k = i.leftColumn(); k <= i.rightColumn(); k++)
			{
				if (3 <= k && k < 3 + taskSize) mapping[item(j, 1)->text()].insert(k - 3);
				else
				{
					for (int a = 0; a < taskSize; a++) mapping[item(j, 1)->text()].insert(a);
				}
			}
		}
	}

	QList<QPair<QString, QSet<int> > > judgeList;

	for (QMap<QString, QSet<int> >::const_iterator i = mapping.constBegin(); i != mapping.constEnd(); i++)
	{
		judgeList.append(qMakePair(i.key(), i.value()));
	}

	auto *dialog = new JudgingDialog(this);
	dialog->setModal(true);
	dialog->setContest(curContest);
	dialog->show();
	dialog->judge(judgeList);
	delete dialog;
	refreshViewer();
}

void ResultViewer::judgeAll()
{
	auto *dialog = new JudgingDialog(this);
	dialog->setModal(true);
	dialog->setContest(curContest);
	dialog->show();
	dialog->judgeAll();
	delete dialog;
	refreshViewer();
}

void ResultViewer::judgeUnjudged()
{
	QMap<QString, QSet<int> > mapping;

	QList<Contestant *> contestantList = curContest->getContestantList();
	QList<Task *> taskList = curContest->getTaskList();
	int contestantSize = contestantList.size();
	int taskSize = taskList.size();

	for (int i = 0; i < contestantSize; i++)
	{
		for (int j = 0; j < taskSize; j++)
		{
			if (item(i, j + 3)->text() == tr("Invalid"))
			{
				mapping[item(i, 1)->text()].insert(j);
			}
		}
	}

	QList<QPair<QString, QSet<int> > > judgeList;

	for (QMap<QString, QSet<int> >::const_iterator i = mapping.constBegin(); i != mapping.constEnd(); i++)
	{
		judgeList.append(qMakePair(i.key(), i.value()));
	}

	auto *dialog = new JudgingDialog(this);
	dialog->setModal(true);
	dialog->setContest(curContest);
	dialog->show();
	dialog->judge(judgeList);
	delete dialog;
	refreshViewer();
}

void ResultViewer::judgeGrey()
{
	QMap<QString, QSet<int> > mapping;

	QList<Contestant *> contestantList = curContest->getContestantList();
	QList<Task *> taskList = curContest->getTaskList();
	int contestantSize = contestantList.size();
	int taskSize = taskList.size();

	for (int i = 0; i < contestantSize; i++)
	{
		for (int j = 0; j < taskSize; j++)
		{
			if (contestantList[i]->getCompileState(j) == NoValidSourceFile)
			{
				mapping[contestantList[i]->getContestantName()].insert(j);
			}
		}
	}

	QList<QPair<QString, QSet<int> > > judgeList;

	for (QMap<QString, QSet<int> >::const_iterator i = mapping.constBegin(); i != mapping.constEnd(); i++)
	{
		judgeList.append(qMakePair(i.key(), i.value()));
	}

	auto *dialog = new JudgingDialog(this);
	dialog->setModal(true);
	dialog->setContest(curContest);
	dialog->show();
	dialog->judge(judgeList);
	delete dialog;
	refreshViewer();
}

void ResultViewer::judgeMagenta()
{
	QMap<QString, QSet<int> > mapping;

	QList<Contestant *> contestantList = curContest->getContestantList();
	QList<Task *> taskList = curContest->getTaskList();
	int contestantSize = contestantList.size();
	int taskSize = taskList.size();

	for (int i = 0; i < contestantSize; i++)
	{
		for (int j = 0; j < taskSize; j++)
		{
			if (taskList[j]->getTaskType() != Task::AnswersOnly && contestantList[i]->getCompileState(j) != CompileSuccessfully && contestantList[i]->getCompileState(j) != NoValidSourceFile)
			{
				mapping[contestantList[i]->getContestantName()].insert(j);
			}
		}
	}

	QList<QPair<QString, QSet<int> > > judgeList;

	for (QMap<QString, QSet<int> >::const_iterator i = mapping.constBegin(); i != mapping.constEnd(); i++)
	{
		judgeList.append(qMakePair(i.key(), i.value()));
	}

	auto *dialog = new JudgingDialog(this);
	dialog->setModal(true);
	dialog->setContest(curContest);
	dialog->show();
	dialog->judge(judgeList);
	delete dialog;
	refreshViewer();
}

void ResultViewer::clearPath(const QString &curDir)
{
	QDir dir(curDir);
	QStringList fileList = dir.entryList(QDir::Files);

	for (int i = 0; i < fileList.size(); i ++)
	{
		dir.remove(fileList[i]);
	}

	QStringList dirList = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (int i = 0; i < dirList.size(); i ++)
	{
		clearPath(curDir + dirList[i] + QDir::separator());
		dir.rmdir(dirList[i]);
	}
}

void ResultViewer::deleteContestant()
{
	auto *messageBox = new QMessageBox(QMessageBox::Warning, tr("LemonLime"),
	                                   QString("<span style=\"font-size:large\">")
	                                   + tr("Are you sure to delete selected contestant(s)?") + "</span>",
	                                   QMessageBox::Ok | QMessageBox::Cancel, this);
	//QHBoxLayout *layout = new QHBoxLayout;
	auto *checkBox = new QCheckBox(tr("Delete data in the disk as well"));
	//layout->addWidget(checkBox);
	//layout->setAlignment(checkBox, Qt::AlignHCenter);
	//dynamic_cast<QGridLayout*>(messageBox->layout())->addLayout(layout, 1, 1);
	//dynamic_cast<QGridLayout*>(messageBox->layout())->setVerticalSpacing(10);
	messageBox->setCheckBox(checkBox);

	if (messageBox->exec() != QMessageBox::Ok) return;

	QList<QTableWidgetSelectionRange> selectionRange = selectedRanges();

	for (auto &i : selectionRange)
	{
		for (int j = i.topRow(); j <= i.bottomRow(); j ++)
		{
			curContest->deleteContestant(item(j, 1)->text());

			if (checkBox->isChecked())
			{
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

void ResultViewer::detailInformation()
{
	QList<QTableWidgetSelectionRange> selectionRange = selectedRanges();
	int index = selectionRange[0].topRow();
	auto *dialog = new DetailDialog(this);
	dialog->setModal(true);
	dialog->refreshViewer(curContest, curContest->getContestant(item(index, 1)->text()));
	connect(dialog, SIGNAL(rejudgeSignal()), this, SLOT(refreshViewer()));
	dialog->showDialog();
	delete dialog;
}
