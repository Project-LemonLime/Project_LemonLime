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
 * judgingdialog.h @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * judgingdialog.h @Project LemonLime
 * Update 2019 iotang
 **/

#ifndef JUDGINGDIALOG_H
#define JUDGINGDIALOG_H

#include <QtCore>
#include <QtGui>
#include <QDialog>
#include "globaltype.h"

class Contest;

namespace Ui
{
	class JudgingDialog;
}

class JudgingDialog : public QDialog
{
	Q_OBJECT

public:
	explicit JudgingDialog(QWidget *parent = 0);
	~JudgingDialog();
	void setContest(Contest *);
	void judge(const QStringList &);
	void judge(const QString &, int);
	void judge(const QList<QPair<QString, QSet<int> > > &);
	void judgeAll();
	void reject();

private slots:
	void stopJudgingSlot();
	void skipJudging();

private:
	Ui::JudgingDialog *ui;
	Contest *curContest;
	QTextCursor *cursor;
	bool stopJudging;

public slots:
	void singleCaseFinished(int, int, int, int, int, int, int);
	void singleSubtaskDependenceFinished(int, int, double);
	void taskJudgingStarted(const QString &);
	void contestantJudgingStart(const QString &);
	void contestantJudgingFinished();
	void compileError(int, int);

signals:
	void stopJudgingSignal();
};

#endif // JUDGINGDIALOG_H
