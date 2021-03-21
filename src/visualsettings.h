/*
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
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

class Settings;

class VisualSettings : public QWidget {
	Q_OBJECT

  public:
	explicit VisualSettings(QWidget *parent = nullptr);
	void resetEditSettings(Settings *);
	void resetToDefault();
	~VisualSettings();

  private:
	Ui::VisualSettings *ui;
	Settings *editSettings{};

  private slots:
	void colorMxHChanged(const int &);
	void colorMxSChanged(const double &);
	void colorMxLChanged(const double &);
	void colorMiHChanged(const int &);
	void colorMiSChanged(const double &);
	void colorMiLChanged(const double &);
	void colorNfHChanged(const int &);
	void colorNfSChanged(const double &);
	void colorNfLChanged(const double &);
	void colorCeHChanged(const int &);
	void colorCeSChanged(const double &);
	void colorCeLChanged(const double &);
	void grandCompHChanged(const double &);
	void grandCompSChanged(const double &);
	void grandCompLChanged(const double &);
	void grandRateHChanged(const double &);
	void grandRateSChanged(const double &);
	void grandRateLChanged(const double &);
	void splashTimeChanged(const int &);

	void whenResetToDefault();
};
