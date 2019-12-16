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
 * addtaskdialog.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef ADDTASKDIALOG_H
#define ADDTASKDIALOG_H

#include <QDialog>

namespace Ui
{
	class AddTaskDialog;
}

class AddTaskDialog : public QDialog
{
	Q_OBJECT

public:
	explicit AddTaskDialog(QWidget *parent = nullptr);
	~AddTaskDialog();
	void addTask(const QString &, int, int, int);
	int getFullScore(int) const;
	int getTimeLimit(int) const;
	int getMemoryLimit(int) const;

private:
	Ui::AddTaskDialog *ui;
	QList<int> fullScore;
	QList<int> timeLimit;
	QList<int> memoryLimit;

private slots:
	void taskBoxIndexChanged();
	void fullScoreChanged();
	void timeLimitChanged();
	void memoryLimitChanged();
};

#endif // ADDTASKDIALOG_H
