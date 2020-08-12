/***************************************************************************
    This file is part of Project Lemon
    Copyright (C) 2011 Zhipeng Jia

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
/**
 * optionsdialog.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QtCore>
#include <QtGui>

namespace Ui
{
	class OptionsDialog;
}

class Settings;

class OptionsDialog : public QDialog
{
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

#endif // OPTIONSDIALOG_H
