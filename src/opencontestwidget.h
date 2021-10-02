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

#include <QMessageBox>
#include <QWidget>
#include <QtCore>
#include <QtWidgets>

namespace Ui {
	class OpenContestWidget;
}

class OpenContestWidget : public QWidget {
	Q_OBJECT

  public:
	explicit OpenContestWidget(QWidget *parent = nullptr);
	~OpenContestWidget();
	void setRecentContest(const QStringList &);
	const QStringList &getRecentContest() const;
	int getCurrentRow() const;

  private:
	Ui::OpenContestWidget *ui;
	QStringList recentContest;
	void refreshContestList();

  private slots:
	void addContest();
	void deleteContest();
	void currentRowChanged();

  signals:
	void selectionChanged();
	void rowDoubleClicked();
};
