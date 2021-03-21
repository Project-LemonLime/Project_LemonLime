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

#include <QDialog>
#include <QtCore>
#include <QtWidgets>

namespace Ui {
	class NewContestDialog;
}

class NewContestDialog : public QDialog {
	Q_OBJECT

  public:
	explicit NewContestDialog(QWidget *parent = nullptr);
	~NewContestDialog();
	QString getContestTitle();
	QString getSavingName();
	QString getContestPath();

  private:
	Ui::NewContestDialog *ui;

  private slots:
	void informationChanged();
};
