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
	class ExtTestCaseUpdaterDialog;
}

class Task;
class Settings;

const int NO_EDIT = -400;
const int MAY_EDIT = -300;
const int EDIT_WITH_DEFAULT = -200;

class ExtTestCaseUpdaterDialog : public QDialog {
	Q_OBJECT

  public:
	explicit ExtTestCaseUpdaterDialog(QWidget *parent = nullptr, Task *nowTask = nullptr,
	                                  const Settings *nowSettings = nullptr, int nowCaseNumber = 0,
	                                  int editScore = NO_EDIT, int editData = NO_EDIT, int editTime = NO_EDIT,
	                                  int editMemory = NO_EDIT, int editDepend = NO_EDIT,
	                                  QList<int> tempDepends = QList<int>());
	~ExtTestCaseUpdaterDialog();

	int getScore() const;
	QString getInput() const;
	QString getOutput() const;
	int getTimeLimit() const;
	int getMemoryLimit() const;
	QStringList getDepends() const;

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
