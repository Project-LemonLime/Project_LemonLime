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

