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
 * compiler.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef COMPILER_H
#define COMPILER_H

#include <QtCore>
#include <QObject>

class Compiler : public QObject
{
	Q_OBJECT
public:
	enum CompilerType { Typical, InterpretiveWithByteCode, InterpretiveWithoutByteCode };

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

#endif // COMPILER_H
