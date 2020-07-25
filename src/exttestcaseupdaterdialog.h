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

#ifndef EXTTESTCASEUPDATERDIALOG_H
#define EXTTESTCASEUPDATERDIALOG_H

#include <QDialog>

namespace Ui
{
	class ExtTestCaseUpdaterDialog;
}

class Task;
class Settings;

const int NO_EDIT = -400;
const int MAY_EDIT = -300;
const int EDIT_WITH_DEFAULT = -200;

class ExtTestCaseUpdaterDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ExtTestCaseUpdaterDialog(QWidget *parent = nullptr, Task *nowTask = nullptr,
	                                  const Settings *nowSettings = nullptr, int nowCaseNumber = 0,
	                                  int editScore = NO_EDIT, int editData = NO_EDIT, int editTime = NO_EDIT,
	                                  int editMemory = NO_EDIT, int editDepend = NO_EDIT,
	                                  QList<int> tempDepends = QList<int>());
	~ExtTestCaseUpdaterDialog();

	int getScore();
	QString getInput();
	QString getOutput();
	int getTimeLimit();
	int getMemoryLimit();
	QStringList getDepends();

	int checkDepends();

protected:
	virtual void accept() override;

private:
	Ui::ExtTestCaseUpdaterDialog *ui;
	Task *nowTask;
	const Settings *nowSettings;
	int nowCaseNumber;
	int editScore;
	int editData;
	int editTime;
	int editMemory;
	int editDepend;

	int score;
	QString input, output;
	int timeLimit, memoryLimit;
	QStringList depends;

	int defScore, defTimeLimit, defMemoryLimit;

private slots:

	void whenButtonFindInputClicked();
	void whenButtonFindOutputClicked();
	void fullScoreChanged(const QString &);
	void timeLimitChanged(const QString &);
	void memoryLimitChanged(const QString &);
	void inputFileChanged(const QString &);
	void outputFileChanged(const QString &);
	void dependsChanged(const QString &);
};

#endif // EXTTESTCASEUPDATERDIALOG_H
