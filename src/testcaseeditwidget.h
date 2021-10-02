/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QHeaderView>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QWidget>
#include <QtCore>
#include <QtWidgets>

namespace Ui {
	class TestCaseEditWidget;
}

class Settings;
class TestCase;

class TestCaseEditWidget : public QWidget {
	Q_OBJECT

  public:
	explicit TestCaseEditWidget(QWidget *parent = nullptr);
	~TestCaseEditWidget();
	void changeEvent(QEvent *);
	void setEditTestCase(TestCase *, bool);
	void setSettings(Settings *);

  private:
	Ui::TestCaseEditWidget *ui;
	TestCase *editTestCase;
	Settings *settings{};
	QAction *deleteAction;
	void refreshFileList();

  private slots:
	void addSingleCase();
	void deleteSingleCase();
	void fullScoreChanged(const QString &);
	void timeLimitChanged(const QString &);
	void memoryLimitChanged(const QString &);
	void subtaskDependenceChanged();
	void subtaskDependenceClear();
	void fileListSelectionChanged();
	void fileListItemChanged(QTableWidgetItem *);

  signals:
	void dataPathChanged();
};
