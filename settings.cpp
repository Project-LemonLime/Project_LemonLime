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
 * settings.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "settings.h"
#include "compiler.h"

Settings::Settings(QObject *parent) :
	QObject(parent)
{
}

int Settings::getDefaultFullScore() const
{
	return defaultFullScore;
}

int Settings::getDefaultTimeLimit() const
{
	return defaultTimeLimit;
}

int Settings::getDefaultMemoryLimit() const
{
	return defaultMemoryLimit;
}

int Settings::getCompileTimeLimit() const
{
	return compileTimeLimit;
}

int Settings::getSpecialJudgeTimeLimit() const
{
	return specialJudgeTimeLimit;
}

int Settings::getFileSizeLimit() const
{
	return fileSizeLimit;
}

int Settings::getRejudgeTimes() const
{
	return rejudgeTimes;
}

const QString& Settings::getDefaultInputFileExtension() const
{
	return defaultInputFileExtension;
}

const QString& Settings::getDefaultOutputFileExtension() const
{
	return defaultOutputFileExtension;
}

const QStringList& Settings::getInputFileExtensions() const
{
	return inputFileExtensions;
}

const QStringList& Settings::getOutputFileExtensions() const
{
	return outputFileExtensions;
}

const QStringList& Settings::getRecentContest() const
{
	return recentContest;
}

const QList<Compiler*>& Settings::getCompilerList() const
{
	return compilerList;
}

const QString& Settings::getUiLanguage() const
{
	return uiLanguage;
}

const QString& Settings::getDiffPath() const
{
	return diffPath;
}

void Settings::setDefaultFullScore(int score)
{
	defaultFullScore = score;
}

void Settings::setDefaultTimeLimit(int limit)
{
	defaultTimeLimit = limit;
}

void Settings::setDefaultMemoryLimit(int limit)
{
	defaultMemoryLimit = limit;
}

void Settings::setCompileTimeLimit(int limit)
{
	compileTimeLimit = limit;
}

void Settings::setSpecialJudgeTimeLimit(int limit)
{
	specialJudgeTimeLimit = limit;
}

void Settings::setFileSizeLimit(int limit)
{
	fileSizeLimit = limit;
}

void Settings::setRejudgeTimes(int number)
{
	rejudgeTimes = number;
}

void Settings::setDefaultInputFileExtension(const QString &extension)
{
	defaultInputFileExtension = extension;
}

void Settings::setDefaultOutputFileExtension(const QString &extension)
{
	defaultOutputFileExtension = extension;
}

void Settings::setInputFileExtensions(const QString &extensions)
{
	inputFileExtensions = extensions.split(";", QString::SkipEmptyParts);
}

void Settings::setOutputFileExtensions(const QString &extensions)
{
	outputFileExtensions = extensions.split(";", QString::SkipEmptyParts);
}

void Settings::setRecentContest(const QStringList &list)
{
	recentContest = list;
}

void Settings::setUiLanguage(const QString &language)
{
	uiLanguage = language;
}

void Settings::addCompiler(Compiler *compiler)
{
	compiler->setParent(this);
	compilerList.append(compiler);
}

void Settings::deleteCompiler(int index)
{
	if(0 <= index && index < compilerList.size())
	{
		delete compilerList[index];
		compilerList.removeAt(index);
	}
}

Compiler* Settings::getCompiler(int index)
{
	if(0 <= index && index < compilerList.size())
	{
		return compilerList[index];
	}
	else
	{
		return 0;
	}
}

void Settings::swapCompiler(int a, int b)
{
	if(0 <= a && a < compilerList.size())
	{
		if(0 <= b && b < compilerList.size())
		{
			compilerList.swap(a, b);
		}
	}
}

void Settings::copyFrom(Settings *other)
{
	setDefaultFullScore(other->getDefaultFullScore());
	setDefaultTimeLimit(other->getDefaultTimeLimit());
	setDefaultMemoryLimit(other->getDefaultMemoryLimit());
	setCompileTimeLimit(other->getCompileTimeLimit());
	setSpecialJudgeTimeLimit(other->getSpecialJudgeTimeLimit());
	setFileSizeLimit(other->getFileSizeLimit());
	setRejudgeTimes(other->getRejudgeTimes());
	setDefaultInputFileExtension(other->getDefaultInputFileExtension());
	setDefaultOutputFileExtension(other->getDefaultOutputFileExtension());
	setInputFileExtensions(other->getInputFileExtensions().join(";"));
	setOutputFileExtensions(other->getOutputFileExtensions().join(";"));

	for(int i = 0; i < compilerList.size(); i ++)
	{
		delete compilerList[i];
	}

	compilerList.clear();
	const QList<Compiler*> &list = other->getCompilerList();

	for(int i = 0; i < list.size(); i ++)
	{
		Compiler *compiler = new Compiler;
		compiler->copyFrom(list[i]);
		addCompiler(compiler);
	}
}

void Settings::saveSettings()
{
	QSettings settings("Crash", "Lemon");

	settings.setValue("UiLanguage", uiLanguage);

	settings.beginGroup("GeneralSettings");
	settings.setValue("DefaultFullScore", defaultFullScore);
	settings.setValue("DefaultTimeLimit", defaultTimeLimit);
	settings.setValue("DefaultMemoryLimit", defaultMemoryLimit);
	settings.setValue("CompileTimeLimit", compileTimeLimit);
	settings.setValue("SpecialJudgeTimeLimit", specialJudgeTimeLimit);
	settings.setValue("FileSizeLimit", fileSizeLimit);
	settings.setValue("MaximumRejudgeTimes", rejudgeTimes);
	settings.setValue("DefaultInputFileExtension", defaultInputFileExtension);
	settings.setValue("DefaultOutputFileExtension", defaultOutputFileExtension);
	settings.setValue("InputFileExtensions", inputFileExtensions);
	settings.setValue("OutputFileExtensions", outputFileExtensions);
	settings.endGroup();

	settings.beginWriteArray("v1.2/CompilerSettings");

	for(int i = 0; i < compilerList.size(); i ++)
	{
		settings.setArrayIndex(i);
		settings.setValue("CompilerType", (int)compilerList[i]->getCompilerType());
		settings.setValue("CompilerName", compilerList[i]->getCompilerName());
		settings.setValue("SourceExtensions", compilerList[i]->getSourceExtensions());
		settings.setValue("CompilerLocation", compilerList[i]->getCompilerLocation());
		settings.setValue("InterpreterLocation", compilerList[i]->getInterpreterLocation());
		settings.setValue("BytecodeExtensions", compilerList[i]->getBytecodeExtensions());
		settings.setValue("TimeLimitRatio", compilerList[i]->getTimeLimitRatio());
		settings.setValue("MemoryLimitRatio", compilerList[i]->getMemoryLimitRatio());
		settings.setValue("DisableMemoryLimitCheck", compilerList[i]->getDisableMemoryLimitCheck());
		QStringList configurationNames = compilerList[i]->getConfigurationNames();
		QStringList compilerArguments = compilerList[i]->getCompilerArguments();
		QStringList interpreterArguments = compilerList[i]->getInterpreterArguments();
		settings.beginWriteArray("Configuration");

		for(int j = 0; j < configurationNames.size(); j ++)
		{
			settings.setArrayIndex(j);
			settings.setValue("Name", configurationNames[j]);
			settings.setValue("CompilerArguments", compilerArguments[j]);
			settings.setValue("InterpreterArguments", interpreterArguments[j]);
		}

		settings.setValue("EnvironmentVariables", compilerList[i]->getEnvironment().toStringList());
		settings.endArray();
	}

	settings.endArray();

	settings.beginWriteArray("v1.2/RecentContest");

	for(int i = 0; i < recentContest.size(); i ++)
	{
		settings.setArrayIndex(i);
		settings.setValue("Location", recentContest[i]);
	}

	settings.endArray();
}

void Settings::loadSettings()
{
	for(int i = 0; i < compilerList.size(); i ++)
		delete compilerList[i];

	compilerList.clear();
	recentContest.clear();

	QSettings settings("Crash", "Lemon");

	uiLanguage = settings.value("UiLanguage", QLocale::system().name()).toString();

	settings.beginGroup("GeneralSettings");
	defaultFullScore = settings.value("DefaultFullScore", 10).toInt();
	defaultTimeLimit = settings.value("DefaultTimeLimit", 1000).toInt();
	defaultMemoryLimit = settings.value("DefaultMemoryLimit", 256).toInt();
	compileTimeLimit = settings.value("CompileTimeLimit", 10000).toInt();
	specialJudgeTimeLimit = settings.value("SpecialJudgeTimeLimit", 10000).toInt();
	fileSizeLimit = settings.value("FileSizeLimit", 50).toInt();
	rejudgeTimes = settings.value("MaximumRejudgeTimes", 1).toInt();
	defaultInputFileExtension = settings.value("DefaultInputFileExtension", "in").toString();
	defaultOutputFileExtension = settings.value("DefaultOuputFileExtension", "out").toString();
	inputFileExtensions = settings.value("InputFileExtensions", QStringList() << "in").toStringList();
	outputFileExtensions = settings.value("OutputFileExtensions", QStringList() << "out" << "ans").toStringList();
	settings.endGroup();

	int compilerCount = settings.beginReadArray("v1.2/CompilerSettings");

	for(int i = 0; i < compilerCount; i ++)
	{
		settings.setArrayIndex(i);
		Compiler *compiler = new Compiler;
		compiler->setCompilerType((Compiler::CompilerType)settings.value("CompilerType").toInt());
		compiler->setCompilerName(settings.value("CompilerName").toString());
		compiler->setSourceExtensions(settings.value("SourceExtensions").toStringList().join(";"));
		compiler->setCompilerLocation(settings.value("CompilerLocation").toString());
		compiler->setInterpreterLocation(settings.value("InterpreterLocation").toString());
		compiler->setBytecodeExtensions(settings.value("BytecodeExtensions").toStringList().join(";"));
		compiler->setTimeLimitRatio(settings.value("TimeLimitRatio").toDouble());
		compiler->setMemoryLimitRatio(settings.value("MemoryLimitRatio").toDouble());
		compiler->setDisableMemoryLimitCheck(settings.value("DisableMemoryLimitCheck").toBool());
		int configurationCount = settings.beginReadArray("Configuration");

		for(int j = 0; j < configurationCount; j ++)
		{
			settings.setArrayIndex(j);
			compiler->addConfiguration(settings.value("Name").toString(),
			                           settings.value("CompilerArguments").toString(),
			                           settings.value("InterpreterArguments").toString());
		}

		QStringList values = settings.value("EnvironmentVariables").toStringList();
		QProcessEnvironment environment;

		for(int i = 0; i < values.size(); i ++)
		{
			int tmp = values[i].indexOf('=');
			QString variable = values[i].mid(0, tmp);
			QString value = values[i].mid(tmp + 1);
			environment.insert(variable, value);
		}

		compiler->setEnvironment(environment);
		settings.endArray();
		addCompiler(compiler);
	}

	settings.endArray();

	int listCount = settings.beginReadArray("v1.2/RecentContest");

	for(int i = 0; i < listCount; i ++)
	{
		settings.setArrayIndex(i);
		recentContest.append(settings.value("Location").toString());
	}

	settings.endArray();

#ifdef Q_OS_WIN32
	diffPath = QDir::toNativeSeparators(QDir::currentPath()) + QDir::separator() + "diff.exe";
#endif
#ifdef Q_OS_LINUX
	diffPath = "/usr/bin/diff";
#endif
}

int Settings::upperBoundForFullScore()
{
	return 2147483647;
}

int Settings::upperBoundForTimeLimit()
{
	return 1000 * 60 * 60 * 24;
}

int Settings::upperBoundForMemoryLimit()
{
	return 1048576;
}

int Settings::upperBoundForFileSizeLimit()
{
	return 10 * 1024;
}

int Settings::upperBoundForRejudgeTimes()
{
	return 10;
}

QString Settings::dataPath()
{
	return QString("data") + QDir::separator();
}

QString Settings::sourcePath()
{
	return QString("source") + QDir::separator();
}

QString Settings::temporaryPath()
{
	return QString("temp") + QDir::separator();
}

QString Settings::selfTestPath()
{
	return QString("selftest") + QDir::separator();
}
