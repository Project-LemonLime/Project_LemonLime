/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QTableWidget>
#include <QtCore>
#include <QtGui>

class Contest;

class ResultViewer : public QTableWidget {
	Q_OBJECT
  public:
	explicit ResultViewer(QWidget *parent = nullptr);
	void changeEvent(QEvent *);
	void contextMenuEvent(QContextMenuEvent *);
	void setContest(Contest *);

  public slots:
	void refreshViewer();
	void judgeSelected();
	void judgeAll();
	void judgeUnjudged();
	void judgeGrey();
	void judgeMagenta();

  private:
	Contest *curContest;
	QAction *deleteContestantAction;
	QAction *detailInformationAction;
	QAction *judgeSelectedAction;
	QAction *deleteContestantKeyAction;
	void clearPath(const QString &);

  private slots:
	void deleteContestant();
	void detailInformation();

  signals:
	void contestantDeleted();
};
