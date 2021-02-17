/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019-2021 Project LemonLime
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
	class EnvironmentVariablesDialog;
}

class EnvironmentVariablesDialog : public QDialog {
	Q_OBJECT

  public:
	explicit EnvironmentVariablesDialog(QWidget *parent = nullptr);
	~EnvironmentVariablesDialog();
	void setProcessEnvironment(const QProcessEnvironment &);
	QProcessEnvironment getProcessEnvironment() const;

  private:
	Ui::EnvironmentVariablesDialog *ui;

  private slots:
	void addButtonClicked();
	void editButtonClicked();
	void deleteButtonClicked();
	void viewerSelectionChanged();
};
