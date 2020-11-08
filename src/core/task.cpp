/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "task.h"

#include "core/compiler.h"
#include "core/settings.h"
#include "core/testcase.h"
#include <utility>

Task::Task(QObject *parent, TaskType taskType, ComparisonMode comparisonMode, QString diffArguments,
           int realPrecision, bool standardInputCheck, bool standardOutputCheck, bool subFolderCheck)
    : QObject(parent), taskType(taskType), comparisonMode(comparisonMode),
      diffArguments(std::move(diffArguments)), realPrecision(realPrecision),
      standardInputCheck(standardInputCheck), standardOutputCheck(standardOutputCheck),
      subFolderCheck(subFolderCheck)
{
}

void Task::copyTo(Task *to)
{
	QByteArray data;
	QDataStream tmpin(&data, QIODevice::WriteOnly);
	writeToStream(tmpin);
	QDataStream tmpout(&data, QIODevice::ReadOnly);
	to->readFromStream(tmpout);
}

auto Task::getTestCaseList() const -> const QList<TestCase *> & { return testCaseList; }

auto Task::getProblemTile() const -> const QString & { return problemTitle; }

auto Task::getSubFolderCheck() const -> bool { return subFolderCheck; }

auto Task::getSourceFileName() const -> const QString & { return sourceFileName; }

auto Task::getInputFileName() const -> const QString & { return inputFileName; }

auto Task::getOutputFileName() const -> const QString & { return outputFileName; }

auto Task::getStandardInputCheck() const -> bool { return standardInputCheck; }

auto Task::getStandardOutputCheck() const -> bool { return standardOutputCheck; }

auto Task::getTaskType() const -> Task::TaskType { return taskType; }

auto Task::getComparisonMode() const -> Task::ComparisonMode { return comparisonMode; }

auto Task::getDiffArguments() const -> const QString & { return diffArguments; }

auto Task::getRealPrecision() const -> int { return realPrecision; }

auto Task::getSpecialJudge() const -> const QString & { return specialJudge; }

auto Task::getInteractor() const -> const QString & { return interactor; }

auto Task::getInteractorName() const -> const QString & { return interactorName; }

auto Task::getGrader() const -> const QString & { return grader; }

auto Task::getCompilerConfiguration(const QString &compilerName) const -> QString
{
	return compilerConfiguration.value(compilerName);
}

auto Task::getAnswerFileExtension() const -> const QString & { return answerFileExtension; }

auto Task::getSourceFilesPath() const -> const QStringList & { return sourceFilesPath; }

auto Task::getSourceFilesName() const -> const QStringList & { return sourceFilesName; }

auto Task::getGraderFilesPath() const -> const QStringList & { return graderFilesPath; }

auto Task::getGraderFilesName() const -> const QStringList & { return graderFilesName; }

void Task::setProblemTitle(const QString &title)
{
	bool changed = problemTitle != title;
	problemTitle = title;

	if (changed)
		emit problemTitleChanged(title);
}

void Task::setSubFolderCheck(bool check) { subFolderCheck = check; }

void Task::setSourceFileName(const QString &fileName) { sourceFileName = fileName; }

void Task::setInputFileName(const QString &fileName) { inputFileName = fileName; }

void Task::setOutputFileName(const QString &fileName) { outputFileName = fileName; }

void Task::setStandardInputCheck(bool check) { standardInputCheck = check; }

void Task::setStandardOutputCheck(bool check) { standardOutputCheck = check; }

void Task::setTaskType(Task::TaskType type) { taskType = type; }

void Task::setComparisonMode(Task::ComparisonMode mode) { comparisonMode = mode; }

void Task::setDiffArguments(const QString &argumentsList) { diffArguments = argumentsList; }

void Task::setRealPrecision(int precision) { realPrecision = precision; }

void Task::setSpecialJudge(const QString &fileName) { specialJudge = fileName; }

void Task::setInteractor(const QString &fileName) { interactor = fileName; }

void Task::setInteractorName(const QString &fileName) { interactorName = fileName; }

void Task::setGrader(const QString &fileName) { grader = fileName; }

void Task::setCompilerConfiguration(const QString &compiler, const QString &configuration)
{
	compilerConfiguration.insert(compiler, configuration);
}

void Task::setAnswerFileExtension(const QString &extension) { answerFileExtension = extension; }

void Task::setSourceFilesPath(const QStringList &pathList) { sourceFilesPath = pathList; }

void Task::setSourceFilesName(const QStringList &nameList) { sourceFilesName = nameList; }

void Task::setGraderFilesPath(const QStringList &pathList) { graderFilesPath = pathList; }

void Task::setGraderFilesName(const QStringList &nameList) { graderFilesName = nameList; }

void Task::appendSourceFiles(const QString &path, const QString &name)
{
	sourceFilesPath.append(path);
	sourceFilesName.append(name);
}

void Task::appendGraderFiles(const QString &path, const QString &name)
{
	graderFilesPath.append(path);
	graderFilesName.append(name);
}

void Task::removeSourceFilesAt(int num)
{
	if (num < sourceFilesPath.length())
	{
		sourceFilesPath.removeAt(num);
		sourceFilesName.removeAt(num);
	}
}

void Task::removeGraderFilesAt(int num)
{
	if (num < graderFilesPath.length())
	{
		graderFilesPath.removeAt(num);
		graderFilesName.removeAt(num);
	}
}

void Task::addTestCase(TestCase *testCase)
{
	testCase->setParent(this);
	testCase->setIndex(testCaseList.size() + 1);
	testCaseList.append(testCase);
}

void Task::addTestCase(TestCase *testCase, int loc)
{
	testCase->setParent(this);
	testCaseList.insert(loc, testCase);
}

auto Task::getTestCase(int index) const -> TestCase *
{
	if (0 <= index && index < testCaseList.size())
	{
		return testCaseList[index];
	}

	return nullptr;
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

void Task::swapTestCase(int a, int b)
{
	if (a < 0 || a >= testCaseList.size())
		return;

	if (b < 0 || b >= testCaseList.size())
		return;

	qSwap(testCaseList[a], testCaseList[b]);
}

void Task::refreshCompilerConfiguration(Settings *settings)
{
	QList<Compiler *> compilerList = settings->getCompilerList();
	QStringList compilerNames;

	for (auto &i : compilerList)
		compilerNames.append(i->getCompilerName());

	QMap<QString, QString>::iterator p;

	for (p = compilerConfiguration.begin(); p != compilerConfiguration.end();)
	{
		if (! compilerNames.contains(p.key()))
		{
			p = compilerConfiguration.erase(p);
		}
		else
		{
			++p;
		}
	}

	for (auto &i : compilerList)
	{
		if (compilerConfiguration.contains(i->getCompilerName()))
		{
			const QString &config = compilerConfiguration.value(i->getCompilerName());
			const QStringList &configurationNames = i->getConfigurationNames();

			if (! configurationNames.contains(config))
			{
				compilerConfiguration.insert(i->getCompilerName(), "default");
			}
		}
		else
		{
			compilerConfiguration.insert(i->getCompilerName(), "default");
		}
	}

	emit compilerConfigurationRefreshed();
}

auto Task::getTotalTimeLimit() const -> int
{
	int total = 0;

	for (auto *i : testCaseList)
	{
		total += i->getTimeLimit() * i->getInputFiles().size();
	}

	return total;
}

auto Task::getTotalScore() const -> int
{
	int total = 0;

	for (auto *i : testCaseList)
	{
		total += i->getFullScore();
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
	out << (static_cast<int>(taskType) | ((static_cast<int>(subFolderCheck)) << 8));
	out << int(comparisonMode);
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

	if (taskType == Task::Communication)
	{
		out << sourceFilesPath.length();

		for (int i = 0; i < sourceFilesPath.length(); i++)
		{
			QString temp = sourceFilesPath[i];
			temp.replace(QDir::separator(), '/');
			out << temp;
			temp = sourceFilesName[i];
			temp.replace(QDir::separator(), '/');
			out << temp;
		}

		out << graderFilesPath.length();

		for (int i = 0; i < graderFilesPath.length(); i++)
		{
			QString temp = graderFilesPath[i];
			temp.replace(QDir::separator(), '/');
			out << temp;
			temp = graderFilesName[i];
			temp.replace(QDir::separator(), '/');
			out << temp;
		}
	}

	out << compilerConfiguration;
	out << answerFileExtension;
	out << testCaseList.size();

	for (auto &i : testCaseList)
	{
		i->writeToStream(out);
	}
}

void Task::readFromStream(QDataStream &in)
{
	int tmp = 0;
	int count = 0;
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

	if (taskType == Task::Communication)
	{
		int length = 0;
		in >> length;
		sourceFilesPath.clear();
		sourceFilesName.clear();

		for (int i = 0; i < length; i++)
		{
			QString temp;
			in >> temp;
			temp.replace('/', QDir::separator());
			sourceFilesPath.append(temp);
			in >> temp;
			temp.replace('/', QDir::separator());
			sourceFilesName.append(temp);
		}

		in >> length;
		graderFilesPath.clear();
		graderFilesName.clear();

		for (int i = 0; i < length; i++)
		{
			QString temp;
			in >> temp;
			temp.replace('/', QDir::separator());
			graderFilesPath.append(temp);
			in >> temp;
			temp.replace('/', QDir::separator());
			graderFilesName.append(temp);
		}
	}

	in >> compilerConfiguration;
	in >> answerFileExtension;
	in >> count;
	testCaseList.clear();

	for (int i = 0; i < count; i++)
	{
		auto *newTestCase = new TestCase(this);
		newTestCase->readFromStream(in);
		newTestCase->setIndex(testCaseList.size() + 1);
		testCaseList.append(newTestCase);
	}
}
