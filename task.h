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
 * task.h @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * task.h @Project LemonLime
 * Update 2019 iotang
 **/

#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QtCore>

class TestCase;
class Settings;

class Task : public QObject
{
	Q_OBJECT
public:
	enum TaskType
	{
		Traditional,
		AnswersOnly,
		Interaction,
		Communication
	};
	enum ComparisonMode
	{
		LineByLineMode,
		IgnoreSpacesMode,
		ExternalToolMode,
		RealNumberMode,
		SpecialJudgeMode
	};

	explicit Task(QObject *parent = nullptr, TaskType taskType = Traditional,
	              ComparisonMode comparisonMode = IgnoreSpacesMode,
	              QString diffArguments = "--ignore-space-change --text --brief", int realPrecision = 3,
	              bool standardInputCheck = false, bool standardOutputCheck = false,
	              bool subFolderCheck = false);

	void copyTo(Task *);

	const QList<TestCase *> &getTestCaseList() const;
	const QString &getProblemTile() const;
	bool getSubFolderCheck() const;
	const QString &getSourceFileName() const;
	const QString &getInputFileName() const;
	const QString &getOutputFileName() const;
	bool getStandardInputCheck() const;
	bool getStandardOutputCheck() const;
	TaskType getTaskType() const;
	ComparisonMode getComparisonMode() const;
	const QString &getDiffArguments() const;
	int getRealPrecision() const;
	const QString &getSpecialJudge() const;
	const QString &getInteractor() const;
	const QString &getInteractorName() const;
	const QString &getGrader() const;
	QString getCompilerConfiguration(const QString &) const;
	const QString &getAnswerFileExtension() const;
	const QStringList &getSourceFilesPath() const;
	const QStringList &getSourceFilesName() const;
	const QStringList &getGraderFilesPath() const;
	const QStringList &getGraderFilesName() const;

	void setProblemTitle(const QString &);
	void setSubFolderCheck(bool);
	void setSourceFileName(const QString &);
	void setInputFileName(const QString &);
	void setOutputFileName(const QString &);
	void setStandardInputCheck(bool);
	void setStandardOutputCheck(bool);
	void setTaskType(TaskType);
	void setComparisonMode(ComparisonMode);
	void setDiffArguments(const QString &);
	void setRealPrecision(int);
	void setSpecialJudge(const QString &);
	void setInteractor(const QString &);
	void setInteractorName(const QString &);
	void setGrader(const QString &);
	void setCompilerConfiguration(const QString &, const QString &);
	void setAnswerFileExtension(const QString &);
	void setSourceFilesPath(const QStringList &);
	void setSourceFilesName(const QStringList &);
	void setGraderFilesPath(const QStringList &);
	void setGraderFilesName(const QStringList &);
	void appendSourceFiles(const QString &, const QString &);
	void appendGraderFiles(const QString &, const QString &);
	void removeSourceFilesAt(int);
	void removeGraderFilesAt(int);

	void addTestCase(TestCase *);
	void addTestCase(TestCase *, int);
	TestCase *getTestCase(int) const;
	void deleteTestCase(int);
	void swapTestCase(int, int);
	void refreshCompilerConfiguration(Settings *);
	int getTotalTimeLimit() const;
	int getTotalScore() const;
	void writeToStream(QDataStream &);
	void readFromStream(QDataStream &);

private:
	QList<TestCase *> testCaseList;
	QString problemTitle;
	QString sourceFileName;
	QString inputFileName;
	QString outputFileName;
	TaskType taskType;
	ComparisonMode comparisonMode;
	QString diffArguments;
	int realPrecision;
	bool standardInputCheck;
	bool standardOutputCheck;
	bool subFolderCheck;
	QString specialJudge;
	QString interactor;
	QString interactorName;
	QString grader;
	QStringList sourceFilesPath;
	QStringList sourceFilesName;
	QStringList graderFilesPath;
	QStringList graderFilesName;
	QMap<QString, QString> compilerConfiguration;
	QString answerFileExtension;

signals:
	void problemTitleChanged(const QString &);
	void compilerConfigurationRefreshed();
};

#endif // TASK_H
