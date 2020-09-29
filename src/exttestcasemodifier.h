/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QWidget>

namespace Ui
{
	class ExtTestCaseModifier;
}

class Task;
class Settings;

class ExtTestCaseModifier : public QWidget
{
	Q_OBJECT

  public:
	explicit ExtTestCaseModifier(QWidget *parent = nullptr);
	~ExtTestCaseModifier();

	void refresh();
	void init(Task *, const Settings *);

  private:
	Ui::ExtTestCaseModifier *ui;

	Task *editTask{};
	const Settings *editSettings{};

  private slots:
	void whenTestCaseSelectionChanged();

	void modifySelected();
	void moveUpSelected();
	void moveDownSelected();
	void removeSelected();
	void mergeSelected();
	void splitSelected();

	void appendNewSub();
	void appendNewCase();
};
