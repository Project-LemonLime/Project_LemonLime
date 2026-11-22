/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QWidget>

namespace Ui {
	class TaskEditWidget;
}

class Settings;
class Task;

class TaskEditWidget : public QWidget {
	Q_OBJECT

  public:
	explicit TaskEditWidget(QWidget *parent = nullptr);
	~TaskEditWidget();
	void changeEvent(QEvent *);
	void setEditTask(Task *);
	void setSettings(Settings *);

  private:
	Ui::TaskEditWidget *ui;
	Settings *settings{};
	Task *editTask;
	void refreshWidgetState();
	void addSourceFiles(const QString &, const QString &);
	void addGraderFiles(const QString &, const QString &);
	void rmSourceFilesAt(int);
	void rmGraderFilesAt(int);
	void multiFilesRefresh();

  private slots:
	void sourceFileNameChanged(const QString &);
	void subFolderCheckChanged();
	void standardInputCheckChanged();
	void standardOutputCheckChanged();
	void comparisonModeChanged();
	void realPrecisionChanged(int);
	void refreshProblemTitle(const QString &);
	void refreshCompilerConfiguration();
	void compilerSelectionChanged();
	void configurationSelectionChanged();
	void addSourceFileClicked();
	void addGraderFileClicked();
	void rmSourceFileClicked();
	void rmGraderFileClicked();

  signals:
	void dataPathChanged();
};
