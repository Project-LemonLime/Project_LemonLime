/*
 * SPDX-FileCopyrightText: 2021-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef THEMEEDITDIALOG_H
#define THEMEEDITDIALOG_H

#include <QDialog>

namespace Ui {
	class ThemeEditDialog;
}

class ColorTheme;

class ThemeEditDialog : public QDialog {
	Q_OBJECT

  public:
	explicit ThemeEditDialog(QWidget *parent = nullptr);
	void resetEditTheme(ColorTheme *);
	ColorTheme *getEditTheme();

	~ThemeEditDialog();

  private:
	Ui::ThemeEditDialog *ui;
	ColorTheme *editTheme{};

  private slots:
	void okayButtonClicked();
};

#endif // THEMEEDITDIALOG_H
