/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QObject>
#include <QProcessEnvironment>

class Compiler : public QObject {
	Q_OBJECT
  public:
	enum CompilerType { Typical, InterpretiveWithByteCode, InterpretiveWithoutByteCode };
	Q_ENUM(CompilerType)

	explicit Compiler(QObject *parent = nullptr);

	CompilerType getCompilerType() const;
	const QString &getCompilerName() const;
	const QStringList &getSourceExtensions() const;
	const QString &getCompilerLocation() const;
	const QString &getInterpreterLocation() const;
	const QStringList &getBytecodeExtensions() const;
	const QStringList &getConfigurationNames() const;
	const QStringList &getCompilerArguments() const;
	const QStringList &getInterpreterArguments() const;
	const QProcessEnvironment &getEnvironment() const;
	double getTimeLimitRatio() const;
	double getMemoryLimitRatio() const;
	bool getDisableMemoryLimitCheck() const;

	void setCompilerType(CompilerType);
	void setCompilerName(const QString &);
	void setSourceExtensions(const QString &);
	void setCompilerLocation(const QString &);
	void setInterpreterLocation(const QString &);
	void setBytecodeExtensions(const QString &);
	void setEnvironment(const QProcessEnvironment &);
	void setTimeLimitRatio(double);
	void setMemoryLimitRatio(double);
	void setDisableMemoryLimitCheck(bool);

	void addConfiguration(const QString &, const QString &, const QString &);
	void setConfigName(int, const QString &);
	void setCompilerArguments(int, const QString &);
	void setInterpreterArguments(int, const QString &);
	void deleteConfiguration(int);

	void copyFrom(Compiler *);

	int read(const QJsonObject &json);
	void write(QJsonObject &json) const;

  private:
	CompilerType compilerType;
	QString compilerName;
	QStringList sourceExtensions;
	QString compilerLocation;
	QString interpreterLocation;
	QStringList bytecodeExtensions;
	QStringList configurationNames;
	QStringList compilerArguments;
	QStringList interpreterArguments;
	QProcessEnvironment environment;
	double timeLimitRatio;
	double memoryLimitRatio;
	bool disableMemoryLimitCheck;
};
