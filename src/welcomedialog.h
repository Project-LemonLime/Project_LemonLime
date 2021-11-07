/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QDialog>

namespace Ui {
	class WelcomeDialog;
}

class WelcomeDialog : public QDialog {
	Q_OBJECT

  public:
	explicit WelcomeDialog(QWidget *parent = nullptr);
	~WelcomeDialog();
	void setRecentContest(const QStringList &);
	QString getContestTitle();
	QString getSavingName();
	QString getContestPath();
	const QStringList &getRecentContest() const;
	QString getSelectedContest();
	int getCurrentTab() const;

  private:
	Ui::WelcomeDialog *ui;

  private slots:
	void selectionChanged();
	void informationChanged();
	void tabIndexChanged(int);
};
