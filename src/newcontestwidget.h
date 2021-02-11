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

#include <QWidget>
#include <QtCore>
#include <QtGui>

namespace Ui {
	class NewContestWidget;
}

class NewContestWidget : public QWidget {
	Q_OBJECT

  public:
	explicit NewContestWidget(QWidget *parent = nullptr);
	~NewContestWidget();
	QString getContestTitle();
	QString getSavingName();
	QString getContestPath();
	bool checkReady() const;

  private:
	Ui::NewContestWidget *ui;

  signals:
	void informationChanged();

  private slots:
	void selectContestPath();
	void savingNameChanged();
};
