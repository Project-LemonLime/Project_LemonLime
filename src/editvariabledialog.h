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
 * editvariabledialog.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef EDITVARIABLEDIALOG_H
#define EDITVARIABLEDIALOG_H

#include <QDialog>
#include <QtCore>
#include <QtGui>

namespace Ui
{
	class EditVariableDialog;
}

class EditVariableDialog : public QDialog
{
	Q_OBJECT

public:
	explicit EditVariableDialog(QWidget *parent = nullptr);
	~EditVariableDialog();
	void setVariableName(const QString &);
	void setVariableValue(const QString &);
	QString getVariableName() const;
	QString getVariableValue() const;

private:
	Ui::EditVariableDialog *ui;

private slots:
	void textChanged();
};

#endif // EDITVARIABLEDIALOG_H
