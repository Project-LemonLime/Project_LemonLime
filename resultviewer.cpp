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

#include "resultviewer.h"
#include "judgingdialog.h"
#include "contestant.h"
#include "settings.h"
#include "contest.h"
#include "task.h"
#include "detaildialog.h"
#include <QMessageBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>

ResultViewer::ResultViewer(QWidget *parent) :
    QTableWidget(parent)
{
    curContest = 0;
    
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
    if (event->type() == QEvent::LanguageChange) {
        deleteContestantAction->setText(QApplication::translate("ResultViewer", "Delete",
                                                                0));
        detailInformationAction->setText(QApplication::translate("ResultViewer", "Details",
                                                                 0));
        judgeSelectedAction->setText(QApplication::translate("ResultViewer", "Judge",
                                                             0));
    }
}

void ResultViewer::contextMenuEvent(QContextMenuEvent *event)
{
    QList<QTableWidgetSelectionRange> selectionRange = selectedRanges();
    if (selectionRange.size() == 0) return;
    QMenu *contextMenu = new QMenu(this);
    if (selectionRange.size() == 1 && selectionRange[0].rowCount() == 1) {
        contextMenu->addAction(detailInformationAction);
        contextMenu->setDefaultAction(detailInformationAction);
    }
    if (selectionRange.size() > 0) {
        contextMenu->addAction(judgeSelectedAction);
    }
    contextMenu->addAction(deleteContestantAction);
    contextMenu->exec(QCursor::pos());
    delete contextMenu;
}

void ResultViewer::setContest(Contest *contest)
{
    if (curContest) {
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
    headerList << tr("Name") << tr("Rank");
    QList<Task*> taskList = curContest->getTaskList();
    for (int i = 0; i < taskList.size(); i ++) {
        headerList << taskList[i]->getProblemTile();
    }
    headerList << tr("Total Score") << tr("Total Used Time (s)") << tr("Judging Time");
    setColumnCount(taskList.size() + 5);
    setHorizontalHeaderLabels(headerList);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    
    QList<Contestant*> contestantList = curContest->getContestantList();
    QList< QPair<int, QString> > sortList;
    setRowCount(contestantList.size());
    for (int i = 0; i < contestantList.size(); i ++) {
        setItem(i, 0, new QTableWidgetItem(contestantList[i]->getContestantName()));
        setItem(i, 1, new QTableWidgetItem());
        for (int j = 0; j < taskList.size(); j ++) {
            setItem(i, j + 2, new QTableWidgetItem());
            int score = contestantList[i]->getTaskScore(j);
            if (score != -1) {
                item(i, j + 2)->setData(Qt::DisplayRole, score);
            } else {
                item(i, j + 2)->setText(tr("Invalid"));
            }
        }
        setItem(i, taskList.size() + 2, new QTableWidgetItem());
        setItem(i, taskList.size() + 3, new QTableWidgetItem());
        setItem(i, taskList.size() + 4, new QTableWidgetItem());
        int totalScore = contestantList[i]->getTotalScore();
        int totalUsedTime = contestantList[i]->getTotalUsedTime();
        QDateTime judgingTime = contestantList[i]->getJudingTime();
        if (totalScore != -1) {
            item(i, taskList.size() + 2)->setData(Qt::DisplayRole, totalScore);
            item(i, taskList.size() + 3)->setData(Qt::DisplayRole, double(totalUsedTime) / 1000);
            item(i, taskList.size() + 4)->setData(Qt::DisplayRole, judgingTime);
            sortList.append(qMakePair(-totalScore, contestantList[i]->getContestantName()));
        } else {
            item(i, taskList.size() + 2)->setText(tr("Invalid"));
            item(i, taskList.size() + 3)->setText(tr("Invalid"));
            item(i, taskList.size() + 4)->setText(tr("Invalid"));
        }
    }
    
    qSort(sortList);
    QMap<QString, int> rankList;
    for (int i = 0; i < sortList.size(); i ++) {
        if (i > 0 && sortList[i].first == sortList[i-1].first) {
            rankList.insert(sortList[i].second, rankList[sortList[i-1].second]);
        } else {
            rankList.insert(sortList[i].second, i);
        }
    }
    for (int i = 0; i < rowCount(); i ++) {
        if (rankList.contains(contestantList[i]->getContestantName())) {
            item(i, 1)->setData(Qt::DisplayRole, rankList[contestantList[i]->getContestantName()] + 1);
        } else {
            item(i, 1)->setText(tr("Invalid"));
        }
    }
    
    for (int i = 0; i < rowCount(); i ++) {
        for (int j = 0; j < columnCount(); j ++) {
            item(i, j)->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        }
    }
}

void ResultViewer::judgeSelected()
{
    QStringList nameList;
    QList<QTableWidgetSelectionRange> selectionRange = selectedRanges();
    for (int i = 0; i < selectionRange.size(); i ++) {
        for (int j = selectionRange[i].topRow(); j <= selectionRange[i].bottomRow(); j ++) {
            nameList.append(item(j, 0)->text());
        }
    }
    JudgingDialog *dialog = new JudgingDialog(this);
    dialog->setModal(true);
    dialog->setContest(curContest);
    dialog->show();
    dialog->judge(nameList);
    delete dialog;
    refreshViewer();
}

void ResultViewer::judgeAll()
{
    JudgingDialog *dialog = new JudgingDialog(this);
    dialog->setModal(true);
    dialog->setContest(curContest);
    dialog->show();
    dialog->judgeAll();
    delete dialog;
    refreshViewer();
}

void ResultViewer::judgeSingleTask(int taskID)
{
    //qDebug() << taskID;
    JudgingDialog *dialog = new JudgingDialog(this);
    dialog->setModal(true);
    dialog->setContest(curContest);
    dialog->show();
    dialog->judgeSingleTask(taskID);
    delete dialog;
    refreshViewer();
}

void ResultViewer::clearPath(const QString &curDir)
{
    QDir dir(curDir);
    QStringList fileList = dir.entryList(QDir::Files);
    for (int i = 0; i < fileList.size(); i ++) {
        dir.remove(fileList[i]);
    }
    QStringList dirList = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < dirList.size(); i ++) {
        clearPath(curDir + dirList[i] + QDir::separator());
        dir.rmdir(dirList[i]);
    }
}

void ResultViewer::deleteContestant()
{
    QMessageBox *messageBox = new QMessageBox(QMessageBox::Warning, tr("Lemon"),
                                              QString("<span style=\"font-size:large\">")
                                              + tr("Are you sure to delete selected contestant(s)?") + "</span>",
                                              QMessageBox::Ok | QMessageBox::Cancel, this);
    //QHBoxLayout *layout = new QHBoxLayout;
    QCheckBox *checkBox = new QCheckBox(tr("Delete directories in the hard disk as well"));
    //layout->addWidget(checkBox);
    //layout->setAlignment(checkBox, Qt::AlignHCenter);
    //dynamic_cast<QGridLayout*>(messageBox->layout())->addLayout(layout, 1, 1);
    //dynamic_cast<QGridLayout*>(messageBox->layout())->setVerticalSpacing(10);
    messageBox->setCheckBox(checkBox);
    if (messageBox->exec() != QMessageBox::Ok) return;
    
    QList<QTableWidgetSelectionRange> selectionRange = selectedRanges();
    for (int i = 0; i < selectionRange.size(); i ++) {
        for (int j = selectionRange[i].topRow(); j <= selectionRange[i].bottomRow(); j ++) {
            curContest->deleteContestant(item(j, 0)->text());
            if (checkBox->isChecked()) {
                clearPath(Settings::sourcePath() + item(j, 0)->text() + QDir::separator());
                QDir(Settings::sourcePath()).rmdir(item(j, 0)->text());
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
    DetailDialog *dialog = new DetailDialog(this);
    dialog->setModal(true);
    dialog->refreshViewer(curContest, curContest->getContestant(item(index, 0)->text()));
    connect(dialog, SIGNAL(rejudgeSignal()), this, SLOT(refreshViewer()));
    dialog->showDialog();
    delete dialog;
}
