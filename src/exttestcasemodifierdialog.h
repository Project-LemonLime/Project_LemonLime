/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QDialog>

namespace Ui {
	class ExtTestCaseModifierDialog;
}

class Task;
class Settings;

class ExtTestCaseModifierDialog : public QDialog {
	Q_OBJECT

  public:
	explicit ExtTestCaseModifierDialog(QWidget *parent = nullptr);
	~ExtTestCaseModifierDialog();

	void init(Task *, const Settings *);
	Task *getEditTask();

  private:
	Ui::ExtTestCaseModifierDialog *ui;

	Task *editTask{};
};
