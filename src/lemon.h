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
 * lemon.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef LEMON_H
#define LEMON_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>

namespace Ui
{
	class Lemon;
}

class Contest;
class Settings;
class OptionsDialog;

class Lemon : public QMainWindow
{
	Q_OBJECT

public:
	explicit Lemon(QWidget *parent = nullptr);
	~Lemon();
	void changeEvent(QEvent *);
	void closeEvent(QCloseEvent *);
	int getSplashTime();
	void welcome();

private:
	Ui::Lemon *ui;
	Contest *curContest;
	Settings *settings;
	QFileSystemWatcher *dataDirWatcher;
	QString curFile;
	QList<QAction *> languageActions;
	QTranslator *appTranslator;
	QTranslator *qtTranslator;
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
	void setUiLanguage();

signals:
	void dataPathChanged();
};

#endif // LEMON_H
