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

#include "base/LemonType.hpp"
#include <QDialog>
#include <QtCore>
#include <QtWidgets>

class Contest;

namespace Ui {
	class JudgingDialog;
}

class JudgingDialog : public QDialog {
	Q_OBJECT

  public:
	explicit JudgingDialog(QWidget *parent = nullptr);
	~JudgingDialog();
	void setContest(Contest *);
	void judge(const QStringList &);
	void judge(const QString &, int);
	void judge(const QList<std::pair<QString, QSet<int>>> &);
	void judgeAll();
	void reject();

  private slots:
	void stopJudgingSlot();
	static void skipJudging();
	void sendNotify(QString, QString);

  private:
	Ui::JudgingDialog *ui;
	Contest *curContest{};
	QTextCursor *cursor;
	bool stopJudging{};

  public slots:
	void dialogAlert(const QString &);
	void singleCaseFinished(int, int, int, int, int, int, int);
	void singleSubtaskDependenceFinished(int, int, int);
	void taskJudgingStarted(const QString &);
	void taskJudgedDisplay(const QString &, const QList<QList<int>> &, const int);
	void contestantJudgingStart(const QString &);
	void contestantJudgingFinished();
	void contestantJudgedDisplay(const QString &, const int, const int);
	void compileError(int, int);

  signals:
	void stopJudgingSignal();
};
