/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QDialog>
#include <QString>

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
