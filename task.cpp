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
 * task.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * task.cpp @Project LemonLime
 * Update 2019 iotang
 **/

#include "task.h"
#include "testcase.h"
#include "settings.h"
#include "compiler.h"

Task::Task(QObject *parent) :
	QObject(parent)
{
	taskType = Traditional;
	comparisonMode = IgnoreSpacesMode;
	diffArguments = "--ignore-space-change --text --brief";
	realPrecision = 3;
	standardInputCheck = false;
	standardOutputCheck = false;
	subFolderCheck = false;
}

const QList<TestCase *> &Task::getTestCaseList() const
{
	return testCaseList;
}

const QString &Task::getProblemTile() const
{
	return problemTitle;
}

bool Task::getSubFolderCheck() const
{
	return subFolderCheck;
}

const QString &Task::getSourceFileName() const
{
	return sourceFileName;
}

const QString &Task::getInputFileName() const
{
	return inputFileName;
}

const QString &Task::getOutputFileName() const
{
	return outputFileName;
}

bool Task::getStandardInputCheck() const
{
	return standardInputCheck;
}

bool Task::getStandardOutputCheck() const
{
	return standardOutputCheck;
}

Task::TaskType Task::getTaskType() const
{
	return taskType;
}

Task::ComparisonMode Task::getComparisonMode() const
{
	return comparisonMode;
}

const QString &Task::getDiffArguments() const
{
	return diffArguments;
}

int Task::getRealPrecision() const
{
	return realPrecision;
}

const QString &Task::getSpecialJudge() const
{
	return specialJudge;
}

const QString &Task::getInteractor() const
{
	return interactor;
}

const QString &Task::getInteractorName() const
{
	return interactorName;
}

const QString &Task::getGrader() const
{
	return grader;
}

QString Task::getCompilerConfiguration(const QString &compilerName) const
{
	return compilerConfiguration.value(compilerName);
}

const QString &Task::getAnswerFileExtension() const
{
	return answerFileExtension;
}

void Task::setProblemTitle(const QString &title)
{
	bool changed = problemTitle != title;
	problemTitle = title;

	if (changed) emit problemTitleChanged(title);
}

void Task::setSubFolderCheck(bool check)
{
	subFolderCheck = check;
}

void Task::setSourceFileName(const QString &fileName)
{
	sourceFileName = fileName;
}

void Task::setInputFileName(const QString &fileName)
{
	inputFileName = fileName;
}

void Task::setOutputFileName(const QString &fileName)
{
	outputFileName = fileName;
}

void Task::setStandardInputCheck(bool check)
{
	standardInputCheck = check;
}

void Task::setStandardOutputCheck(bool check)
{
	standardOutputCheck = check;
}

void Task::setTaskType(Task::TaskType type)
{
	taskType = type;
}

void Task::setComparisonMode(Task::ComparisonMode mode)
{
	comparisonMode = mode;
}

void Task::setDiffArguments(const QString &argumentsList)
{
	diffArguments = argumentsList;
}

void Task::setRealPrecision(int precision)
{
	realPrecision = precision;
}

void Task::setSpecialJudge(const QString &fileName)
{
	specialJudge = fileName;
}

void Task::setInteractor(const QString &fileName)
{
	interactor = fileName;
}

void Task::setInteractorName(const QString &fileName)
{
	interactorName = fileName;
}

void Task::setGrader(const QString &fileName)
{
	grader = fileName;
}

void Task::setCompilerConfiguration(const QString &compiler, const QString &configuration)
{
	compilerConfiguration.insert(compiler, configuration);
}

void Task::setAnswerFileExtension(const QString &extension)
{
	answerFileExtension = extension;
}

void Task::addTestCase(TestCase *testCase)
{
	testCase->setParent(this);
	testCase->setIndex(testCaseList.size() + 1);
	testCaseList.append(testCase);
}

TestCase *Task::getTestCase(int index) const
{
	if (0 <= index && index < testCaseList.size())
	{
		return testCaseList[index];
	}
	else
	{
		return 0;
	}
}

void Task::deleteTestCase(int index)
{
	if (0 <= index && index < testCaseList.size())
	{
		for (int i = index; i < testCaseList.size(); ++i)
			testCaseList[i]->clearDependenceSubtask();

		delete testCaseList[index];
		testCaseList.removeAt(index);
	}
}

void Task::refreshCompilerConfiguration(Settings *settings)
{
	QList<Compiler *> compilerList = settings->getCompilerList();
	QStringList compilerNames;

	for (int i = 0; i < compilerList.size(); i ++)
		compilerNames.append(compilerList[i]->getCompilerName());

	QMap<QString, QString>::iterator p;

	for (p = compilerConfiguration.begin(); p != compilerConfiguration.end();)
	{
		if (! compilerNames.contains(p.key()))
		{
			p = compilerConfiguration.erase(p);
		}
		else
		{
			p ++;
		}
	}

	for (int i = 0; i < compilerList.size(); i ++)
	{
		if (compilerConfiguration.contains(compilerList[i]->getCompilerName()))
		{
			const QString &config = compilerConfiguration.value(compilerList[i]->getCompilerName());
			const QStringList &configurationNames = compilerList[i]->getConfigurationNames();

			if (! configurationNames.contains(config))
			{
				compilerConfiguration.insert(compilerList[i]->getCompilerName(), "default");
			}
		}
		else
		{
			compilerConfiguration.insert(compilerList[i]->getCompilerName(), "default");
		}
	}

	emit compilerConfigurationRefreshed();
}

int Task::getTotalTimeLimit() const
{
	int total = 0;

	for (int i = 0; i < testCaseList.size(); i ++)
	{
		total += testCaseList[i]->getTimeLimit() * testCaseList[i]->getInputFiles().size();
	}

	return total;
}

int Task::getTotalScore() const
{
	int total = 0;

	for (int i = 0; i < testCaseList.size(); i ++)
	{
		total += testCaseList[i]->getFullScore();
	}

	return total;
}

void Task::writeToStream(QDataStream &out)
{
	out << problemTitle;
	out << sourceFileName;
	out << inputFileName;
	out << outputFileName;
	out << standardInputCheck;
	out << standardOutputCheck;
	out << (int (taskType) | (((int) subFolderCheck) << 8));
	out << int (comparisonMode);
	out << diffArguments;
	out << realPrecision;
	QString _specialJudge = specialJudge;
	_specialJudge.replace(QDir::separator(), '/');
	out << _specialJudge;

	if (taskType == Task::Interaction)
	{
		QString _temp = interactor;
		_temp.replace(QDir::separator(), '/');
		out << _temp;
		_temp = grader;
		_temp.replace(QDir::separator(), '/');
		out << _temp;
		out << interactorName;
	}

	out << compilerConfiguration;
	out << answerFileExtension;
	out << testCaseList.size();

	for (int i = 0; i < testCaseList.size(); i ++)
	{
		testCaseList[i]->writeToStream(out);
	}
}

void Task::readFromStream(QDataStream &in)
{
	int tmp, count;
	in >> problemTitle;
	in >> sourceFileName;
	in >> inputFileName;
	in >> outputFileName;
	in >> standardInputCheck;
	in >> standardOutputCheck;
	in >> tmp;
	taskType = TaskType(tmp & 0xFF);
	subFolderCheck = tmp >> 8;
	in >> tmp;
	comparisonMode = ComparisonMode(tmp);
	in >> diffArguments;
	in >> realPrecision;
	in >> specialJudge;
	specialJudge.replace('/', QDir::separator());

	if (taskType == Task::Interaction)
	{
		in >> interactor;
		interactor.replace('/', QDir::separator());
		in >> grader;
		grader.replace('/', QDir::separator());
		in >> interactorName;
	}

	in >> compilerConfiguration;
	in >> answerFileExtension;
	in >> count;

	for (int i = 0; i < count; i ++)
	{
		TestCase *newTestCase = new TestCase(this);
		newTestCase->readFromStream(in);
		newTestCase->setIndex(testCaseList.size() + 1);
		testCaseList.append(newTestCase);
	}
}
