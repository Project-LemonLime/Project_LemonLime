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

#ifndef EXTTESTCASEMODIFIER_H
#define EXTTESTCASEMODIFIER_H

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

	Task *editTask;
	const Settings *editSettings;

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

#endif // EXTTESTCASEMODIFIER_H
