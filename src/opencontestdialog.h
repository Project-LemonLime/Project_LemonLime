/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QDialog>
#include <QtCore>
#include <QtGui>

namespace Ui {
	class OpenContestDialog;
}

class OpenContestDialog : public QDialog {
	Q_OBJECT

  public:
	explicit OpenContestDialog(QWidget *parent = nullptr);
	~OpenContestDialog();
	void setRecentContest(const QStringList &);
	const QStringList &getRecentContest() const;
	QString getSelectedContest();

  private:
	Ui::OpenContestDialog *ui;

  private slots:
	void selectionChanged();
};
