/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "compiler.h"
//
#include "base/LemonUtils.hpp"

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define QT_SkipEmptyParts Qt::SkipEmptyParts
#else
#define QT_SkipEmptyParts QString::SkipEmptyParts
#endif

Compiler::Compiler(QObject *parent) : QObject(parent)
{
	compilerType = Typical;
	timeLimitRatio = 1;
	memoryLimitRatio = 1;
	disableMemoryLimitCheck = false;
}

auto Compiler::getCompilerType() const -> Compiler::CompilerType { return compilerType; }

auto Compiler::getCompilerName() const -> const QString & { return compilerName; }

auto Compiler::getSourceExtensions() const -> const QStringList & { return sourceExtensions; }

auto Compiler::getCompilerLocation() const -> const QString & { return compilerLocation; }

auto Compiler::getInterpreterLocation() const -> const QString & { return interpreterLocation; }

auto Compiler::getBytecodeExtensions() const -> const QStringList & { return bytecodeExtensions; }

auto Compiler::getConfigurationNames() const -> const QStringList & { return configurationNames; }

auto Compiler::getCompilerArguments() const -> const QStringList & { return compilerArguments; }

auto Compiler::getInterpreterArguments() const -> const QStringList & { return interpreterArguments; }

auto Compiler::getEnvironment() const -> const QProcessEnvironment & { return environment; }

auto Compiler::getTimeLimitRatio() const -> double { return timeLimitRatio; }

auto Compiler::getMemoryLimitRatio() const -> double { return memoryLimitRatio; }

auto Compiler::getDisableMemoryLimitCheck() const -> bool { return disableMemoryLimitCheck; }

void Compiler::setCompilerType(Compiler::CompilerType type) { compilerType = type; }

void Compiler::setCompilerName(const QString &name) { compilerName = name; }

void Compiler::setSourceExtensions(const QString &extensions)
{
	sourceExtensions = extensions.split(";", QT_SkipEmptyParts);
}

void Compiler::setCompilerLocation(const QString &location) { compilerLocation = location; }

void Compiler::setInterpreterLocation(const QString &location) { interpreterLocation = location; }

void Compiler::setBytecodeExtensions(const QString &extensions)
{
	bytecodeExtensions = extensions.split(";", QT_SkipEmptyParts);
}

void Compiler::setEnvironment(const QProcessEnvironment &env) { environment = env; }

void Compiler::setTimeLimitRatio(double ratio) { timeLimitRatio = ratio; }

void Compiler::setMemoryLimitRatio(double ratio) { memoryLimitRatio = ratio; }

void Compiler::setDisableMemoryLimitCheck(bool check) { disableMemoryLimitCheck = check; }

void Compiler::addConfiguration(const QString &name, const QString &arguments1, const QString &arguments2)
{
	configurationNames.append(name);
	compilerArguments.append(arguments1);
	interpreterArguments.append(arguments2);
}

void Compiler::setConfigName(int index, const QString &name)
{
	if (0 <= index && index < configurationNames.size())
	{
		configurationNames[index] = name;
	}
}

void Compiler::setCompilerArguments(int index, const QString &arguments)
{
	if (0 <= index && index < compilerArguments.size())
	{
		compilerArguments[index] = arguments;
	}
}

void Compiler::setInterpreterArguments(int index, const QString &arguments)
{
	if (0 <= index && index < interpreterArguments.size())
	{
		interpreterArguments[index] = arguments;
	}
}

void Compiler::deleteConfiguration(int index)
{
	if (0 <= index && index < configurationNames.size())
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

void Compiler::read(const QJsonObject &json) {
	
	if (json.contains("compilerType") && json["compilerType"].isDouble())
        compilerType = CompilerType(json["compilerType"].toInt());
	
	READ_JSON_STR(compilerName)
	READ_JSON_STR(compilerLocation)
	READ_JSON_STR(interpreterLocation)
	
	READ_JSON_STRLIST(sourceExtensions)
	READ_JSON_STRLIST(bytecodeExtensions)
	READ_JSON_STRLIST(configurationNames)
	READ_JSON_STRLIST(compilerArguments)
	READ_JSON_STRLIST(interpreterArguments)
	
	QStringList _environment;
	if (json.contains("environment") && json["environment"].isString()) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
		_environment = json["environment"].toString().split(QLatin1Char(';'), Qt::SkipEmptyParts);
#else
		_environment = json["environment"].toString().split(QLatin1Char(';'), QString::SkipEmptyParts);
#endif
	}

	for (auto &i : _environment)
	{
		int tmp = i.indexOf('=');
		QString variable = i.mid(0, tmp);
		QString value = i.mid(tmp + 1);
		environment.insert(variable, value);
	}
	
	READ_JSON_DOUBLE(timeLimitRatio)
	READ_JSON_DOUBLE(memoryLimitRatio)
	READ_JSON_BOOL(disableMemoryLimitCheck)
}

void Compiler::write(QJsonObject &json) const {
	WRITE_JSON(compilerType)
	WRITE_JSON(compilerName)
	WRITE_JSON(compilerLocation)
	WRITE_JSON(interpreterLocation)
	
	WRITE_JSON_STRLIST(sourceExtensions)
	WRITE_JSON_STRLIST(bytecodeExtensions)
	WRITE_JSON_STRLIST(configurationNames)
	WRITE_JSON_STRLIST(compilerArguments)
	WRITE_JSON_STRLIST(interpreterArguments)
	
	WRITE_JSON_STRLIST(environment.toStringList())
	
	WRITE_JSON(timeLimitRatio)  // double
	WRITE_JSON(memoryLimitRatio)  // double
	WRITE_JSON(disableMemoryLimitCheck)  // bool
}
