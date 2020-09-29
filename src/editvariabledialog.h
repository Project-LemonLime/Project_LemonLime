/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

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
