/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "compiler.h"
//
#include "base/LemonUtils.hpp"

Compiler::Compiler(QObject *parent) : QObject(parent) {
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

void Compiler::setSourceExtensions(const QString &extensions) {
	sourceExtensions = extensions.split(";", Qt::SkipEmptyParts);
}

void Compiler::setCompilerLocation(const QString &location) { compilerLocation = location; }

void Compiler::setInterpreterLocation(const QString &location) { interpreterLocation = location; }

void Compiler::setBytecodeExtensions(const QString &extensions) {
	bytecodeExtensions = extensions.split(";", Qt::SkipEmptyParts);
}

void Compiler::setEnvironment(const QProcessEnvironment &env) { environment = env; }

void Compiler::setTimeLimitRatio(double ratio) { timeLimitRatio = ratio; }

void Compiler::setMemoryLimitRatio(double ratio) { memoryLimitRatio = ratio; }

void Compiler::setDisableMemoryLimitCheck(bool check) { disableMemoryLimitCheck = check; }

void Compiler::addConfiguration(const QString &name, const QString &arguments1, const QString &arguments2) {
	configurationNames.append(name);
	compilerArguments.append(arguments1);
	interpreterArguments.append(arguments2);
}

void Compiler::setConfigName(int index, const QString &name) {
	if (0 <= index && index < configurationNames.size()) {
		configurationNames[index] = name;
	}
}

void Compiler::setCompilerArguments(int index, const QString &arguments) {
	if (0 <= index && index < compilerArguments.size()) {
		compilerArguments[index] = arguments;
	}
}

void Compiler::setInterpreterArguments(int index, const QString &arguments) {
	if (0 <= index && index < interpreterArguments.size()) {
		interpreterArguments[index] = arguments;
	}
}

void Compiler::deleteConfiguration(int index) {
	if (0 <= index && index < configurationNames.size()) {
		configurationNames.removeAt(index);
		compilerArguments.removeAt(index);
		interpreterArguments.removeAt(index);
	}
}

void Compiler::copyFrom(Compiler *other) {
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

int Compiler::read(const QJsonObject &json) {

	if (json.contains("compilerType") && json["compilerType"].isDouble())
		compilerType = CompilerType(json["compilerType"].toInt());

	READ_JSON(json, compilerName);
	READ_JSON(json, compilerLocation);
	READ_JSON(json, interpreterLocation);

	READ_JSON(json, sourceExtensions);
	READ_JSON(json, bytecodeExtensions);
	READ_JSON(json, configurationNames);
	READ_JSON(json, compilerArguments);
	READ_JSON(json, interpreterArguments);

	QStringList environment;
	READ_JSON(json, environment);

	for (auto &i : environment) {
		int tmp = i.indexOf('=');
		QString variable = i.mid(0, tmp);
		QString value = i.mid(tmp + 1);
		this->environment.insert(variable, value);
	}

	READ_JSON(json, timeLimitRatio);
	READ_JSON(json, memoryLimitRatio);
	READ_JSON(json, disableMemoryLimitCheck);
	return 0;
}

void Compiler::write(QJsonObject &json) const {
	WRITE_JSON(json, compilerType);
	WRITE_JSON(json, compilerName);
	WRITE_JSON(json, compilerLocation);
	WRITE_JSON(json, interpreterLocation);

	WRITE_JSON(json, sourceExtensions);
	WRITE_JSON(json, bytecodeExtensions);
	WRITE_JSON(json, configurationNames);
	WRITE_JSON(json, compilerArguments);
	WRITE_JSON(json, interpreterArguments);

	WRITE_JSON(json, environment.toStringList());

	WRITE_JSON(json, timeLimitRatio);          // double
	WRITE_JSON(json, memoryLimitRatio);        // double
	WRITE_JSON(json, disableMemoryLimitCheck); // bool
}
