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
 * addtestcasewizard.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef ADDTESTCASESWIZARD_H
#define ADDTESTCASESWIZARD_H

#include <QtCore>
#include <QtGui>
#include <QWizard>

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

#endif // ADDTESTCASESWIZARD_H
