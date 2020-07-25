/***************************************************************************
    This file is part of Project LemonLime
    Copyright (C) 2020 iotang

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#ifndef VISUALSETTINGS_H
#define VISUALSETTINGS_H

#include <QWidget>

namespace Ui
{
	class VisualSettings;
}

class Settings;

class VisualSettings : public QWidget
{
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

#endif // VISUALSETTINGS_H
