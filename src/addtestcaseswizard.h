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

#include <QWizard>
#include <QtCore>
#include <QtGui>

namespace Ui
{
	class AddTestCasesWizard;
}

class Settings;

class AddTestCasesWizard : public QWizard
{
	Q_OBJECT

  public:
	explicit AddTestCasesWizard(QWidget *parent = nullptr);
	~AddTestCasesWizard();
	void setSettings(Settings *, bool);
	int getFullScore() const;
	int getTimeLimit() const;
	int getMemoryLimit() const;
	const QList<QStringList> &getMatchedInputFiles() const;
	const QList<QStringList> &getMatchedOutputFiles() const;

  private:
	Ui::AddTestCasesWizard *ui;
	Settings *settings{};
	int fullScore{};
	int timeLimit{};
	int memoryLimit{};
	QString inputFilesPattern;
	QString outputFilesPattern;
	QList<QStringList> matchedInputFiles;
	QList<QStringList> matchedOutputFiles;
	void refreshButtonState();
	void getFiles(const QString &, const QString &, QStringList &);
	QString getFullRegExp(const QString &);
	QStringList getMatchedPart(const QString &, const QString &);
	void searchMatchedFiles();
	bool validateCurrentPage();
	static bool compareFileName(const QString &, const QString &);

  private slots:
	void fullScoreChanged(const QString &);
	void timeLimitChanged(const QString &);
	void memoryLimitChanged(const QString &);
	void inputFilesPatternChanged(const QString &);
	void outputFilesPatternChanged(const QString &);
	void addArgument();
	void deleteArgument();
};
