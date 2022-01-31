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

#include <QWidget>

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
