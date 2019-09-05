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
 * compiler.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "compiler.h"

Compiler::Compiler(QObject *parent) :
	QObject(parent)
{
	compilerType = Typical;
	timeLimitRatio = 1;
	memoryLimitRatio = 1;
	disableMemoryLimitCheck = false;
}

Compiler::CompilerType Compiler::getCompilerType() const
{
	return compilerType;
}

const QString &Compiler::getCompilerName() const
{
	return compilerName;
}

const QStringList &Compiler::getSourceExtensions() const
{
	return sourceExtensions;
}

const QString &Compiler::getCompilerLocation() const
{
	return compilerLocation;
}

const QString &Compiler::getInterpreterLocation() const
{
	return interpreterLocation;
}

const QStringList &Compiler::getBytecodeExtensions() const
{
	return bytecodeExtensions;
}

const QStringList &Compiler::getConfigurationNames() const
{
	return configurationNames;
}

const QStringList &Compiler::getCompilerArguments() const
{
	return compilerArguments;
}

const QStringList &Compiler::getInterpreterArguments() const
{
	return interpreterArguments;
}

const QProcessEnvironment &Compiler::getEnvironment() const
{
	return environment;
}

double Compiler::getTimeLimitRatio() const
{
	return timeLimitRatio;
}

double Compiler::getMemoryLimitRatio() const
{
	return memoryLimitRatio;
}

bool Compiler::getDisableMemoryLimitCheck() const
{
	return disableMemoryLimitCheck;
}

void Compiler::setCompilerType(Compiler::CompilerType type)
{
	compilerType = type;
}

void Compiler::setCompilerName(const QString &name)
{
	compilerName = name;
}

void Compiler::setSourceExtensions(const QString &extensions)
{
	sourceExtensions = extensions.split(";", QString::SkipEmptyParts);
}

void Compiler::setCompilerLocation(const QString &location)
{
	compilerLocation = location;
}

void Compiler::setInterpreterLocation(const QString &location)
{
	interpreterLocation = location;
}

void Compiler::setBytecodeExtensions(const QString &extensions)
{
	bytecodeExtensions = extensions.split(";", QString::SkipEmptyParts);
}

void Compiler::setEnvironment(const QProcessEnvironment &env)
{
	environment = env;
}

void Compiler::setTimeLimitRatio(double ratio)
{
	timeLimitRatio = ratio;
}

void Compiler::setMemoryLimitRatio(double ratio)
{
	memoryLimitRatio = ratio;
}

void Compiler::setDisableMemoryLimitCheck(bool check)
{
	disableMemoryLimitCheck = check;
}

void Compiler::addConfiguration(const QString &name, const QString &arguments1, const QString &arguments2)
{
	configurationNames.append(name);
	compilerArguments.append(arguments1);
	interpreterArguments.append(arguments2);
}

void Compiler::setConfigName(int index, const QString &name)
{
	if(0 <= index && index < configurationNames.size())
	{
		configurationNames[index] = name;
	}
}

void Compiler::setCompilerArguments(int index, const QString &arguments)
{
	if(0 <= index && index < compilerArguments.size())
	{
		compilerArguments[index] = arguments;
	}
}

void Compiler::setInterpreterArguments(int index, const QString &arguments)
{
	if(0 <= index && index < interpreterArguments.size())
	{
		interpreterArguments[index] = arguments;
	}
}

void Compiler::deleteConfiguration(int index)
{
	if(0 <= index && index < configurationNames.size())
	{
		configurationNames.removeAt(index);
		compilerArguments.removeAt(index);
		interpreterArguments.removeAt(index);
	}
}

void Compiler::copyFrom(Compiler *other)
{
	compilerType = other->getCompilerType();
	compilerName = other->getCompilerName();
	sourceExtensions = other->getSourceExtensions();
	compilerLocation = other->getCompilerLocation();
	interpreterLocation = other->getInterpreterLocation();
	bytecodeExtensions = other->getBytecodeExtensions();
	configurationNames = other->getConfigurationNames();
	compilerArguments = other->getCompilerArguments();
	interpreterArguments = other->getInterpreterArguments();
	environment = other->getEnvironment();
	timeLimitRatio = other->getTimeLimitRatio();
	memoryLimitRatio = other->getMemoryLimitRatio();
	disableMemoryLimitCheck = other->getDisableMemoryLimitCheck();
}
