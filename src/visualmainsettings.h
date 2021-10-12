/*
 * SPDX-FileCopyrightText: 2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef VISUALMAINSETTINGS_H
#define VISUALMAINSETTINGS_H

#include <QWidget>

namespace Ui {
	class VisualMainSettings;
}

class Settings;

class VisualMainSettings : public QWidget {
	Q_OBJECT

  public:
	explicit VisualMainSettings(QWidget *parent = nullptr);
	~VisualMainSettings();
	void resetEditSettings(Settings *);

  private:
	Ui::VisualMainSettings *ui;
	Settings *editSettings{};

  private slots:
	void splashTimeChanged(const QString &);
	void themeChanged(const int);
	void whenThemeEdit();
	void whenThemeAdd();
	void whenThemeDelete();
};

#endif // VISUALMAINSETTINGS_H
