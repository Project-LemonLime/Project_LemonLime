/*
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QWidget>

namespace Ui {
	class VisualSettings;
}

class ColorTheme;

class VisualSettings : public QWidget {
	Q_OBJECT

  public:
	explicit VisualSettings(QWidget *parent = nullptr);
	void resetColorTheme(ColorTheme *);
	void refresh();
	~VisualSettings();

  private:
	Ui::VisualSettings *ui;
	ColorTheme *editColorTheme{};

  private slots:
	void themeNameChanged(const QString &);
};
