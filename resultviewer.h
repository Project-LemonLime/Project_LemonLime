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
 * resultviewer.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef RESULTVIEWER_H
#define RESULTVIEWER_H

#include <QtCore>
#include <QtGui>
#include <QTableWidget>

class Contest;

class ResultViewer : public QTableWidget
{
    Q_OBJECT
public:
    explicit ResultViewer(QWidget *parent = 0);
    void changeEvent(QEvent*);
    void contextMenuEvent(QContextMenuEvent*);
    void setContest(Contest*);

public slots:
    void refreshViewer();
    void judgeSelected();
    void judgeAll();
    void judgeSingleTask(int);

private:
    Contest *curContest;
    QAction *deleteContestantAction;
    QAction *detailInformationAction;
    QAction *judgeSelectedAction;
    QAction *deleteContestantKeyAction;
    void clearPath(const QString&);

private slots:
    void deleteContestant();
    void detailInformation();

signals:
    void contestantDeleted();
};

#endif // RESULTVIEWER_H
