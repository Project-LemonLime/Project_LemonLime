/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QMainWindow>
#include <QtCore>
#include <QtGui>

namespace Ui {
	class LemonLime;
}

class Contest;
class Settings;
class OptionsDialog;

class LemonLime : public QMainWindow {
	Q_OBJECT

  public:
	explicit LemonLime(QWidget *parent = nullptr);
	~LemonLime();
	void changeEvent(QEvent *);
	void closeEvent(QCloseEvent *);
	int getSplashTime();
	void welcome();

  private:
	Ui::LemonLime *ui;
	Contest *curContest;
	Settings *settings;
	QFileSystemWatcher *dataDirWatcher;
	QString curFile;
	QSignalMapper *signalMapper;
	QMenu *TaskMenu;
	QList<QAction *> TaskList;
	void judgeExtButtonFlip(bool);
	void loadUiLanguage();
	void insertWatchPath(const QString &, QFileSystemWatcher *);
	void newContest(const QString &, const QString &, const QString &);
	void saveContest(const QString &);
	void loadContest(const QString &);
	static void getFiles(const QString &, const QStringList &, QMap<QString, QString> &);
	void addTask(const QString &, const QList<QPair<QString, QString>> &, int, int, int);
	void addTaskWithScoreScale(const QString &, const QList<QPair<QString, QString>> &, int, int, int);
	static bool compareFileName(const QPair<QString, QString> &, const QPair<QString, QString> &);

  private slots:
	void summarySelectionChanged();
	void refreshSummary();
	void resetDataWatcher();
	void showOptionsDialog();
	void refreshButtonClicked();
	void cleanupButtonClicked();
	void tabIndexChanged(int);
	void moveUpTask();
	void moveDownTask();
	void viewerSelectionChanged();
	void contestantDeleted();
	void newAction();
	void saveAction();
	static void openFolderAction();
	void closeAction();
	void loadAction();
	void addTasksAction();
	void exportResult();
	void exportStatstics();
	void changeContestName();
	void aboutLemon();
	void actionManual();
	static void actionMore();

  signals:
	void dataPathChanged();
};
