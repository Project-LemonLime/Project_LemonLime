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
 * taskeditwidget.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef TASKEDITWIDGET_H
#define TASKEDITWIDGET_H

#include <QtCore>
#include <QtGui>
#include <QWidget>

namespace Ui
{
class TaskEditWidget;
}

class Settings;
class Task;

class TaskEditWidget : public QWidget
{
	Q_OBJECT

public:
	explicit TaskEditWidget(QWidget *parent = 0);
	~TaskEditWidget();
	void changeEvent(QEvent*);
	void setEditTask(Task*);
	void setSettings(Settings*);

private:
	Ui::TaskEditWidget *ui;
	Settings *settings;
	Task *editTask;
	void refreshWidgetState();

private slots:
	void problemTitleChanged(const QString&);
	void setToTraditional(bool);
	void setToAnswersOnly(bool);
	void setToInteraction(bool);
	void sourceFileNameChanged(const QString&);
	void subFolderCheckChanged();
	void inputFileNameChanged(const QString&);
	void outputFileNameChanged(const QString&);
	void standardInputCheckChanged();
	void standardOutputCheckChanged();
	void comparisonModeChanged();
	void diffArgumentsChanged(const QString&);
	void realPrecisionChanged(int);
	void specialJudgeChanged(const QString&);
	void interactorChanged(const QString&);
	void interactorNameChanged(const QString&);
	void graderChanged(const QString&);
	void refreshProblemTitle(const QString&);
	void refreshCompilerConfiguration();
	void compilerSelectionChanged();
	void configurationSelectionChanged();
	void answerFileExtensionChanged(const QString&);

signals:
	void dataPathChanged();
};

#endif // TASKEDITWIDGET_H
