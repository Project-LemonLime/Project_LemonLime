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
#include <QtCore>
#include <QtWidgets>

namespace Ui {
	class OptionsDialog;
}

class Settings;

class OptionsDialog : public QDialog {
	Q_OBJECT

  public:
	explicit OptionsDialog(QWidget *parent = nullptr);
	~OptionsDialog();
	void resetEditSettings(Settings *);
	Settings *getEditSettings();

  private:
	Ui::OptionsDialog *ui;
	Settings *editSettings;

  private slots:
	void okayButtonClicked();
};
