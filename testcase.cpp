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
 * testcase.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "testcase.h"
#include "settings.h"

TestCase::TestCase(QObject *parent) :
	QObject(parent)
{
}

int TestCase::getFullScore() const
{
	return fullScore;
}

int TestCase::getTimeLimit() const
{
	return timeLimit;
}

int TestCase::getMemoryLimit() const
{
	return memoryLimit;
}

const QStringList &TestCase::getInputFiles() const
{
	return inputFiles;
}

const QStringList &TestCase::getOutputFiles() const
{
	return outputFiles;
}

const QList<int> &TestCase::getDependenceSubtask() const
{
	return dependenceSubtask;
}

void TestCase::setIndex(int ind)
{
	index = ind;
}

void TestCase::setFullScore(int score)
{
	fullScore = score;
}

void TestCase::setTimeLimit(int limit)
{
	timeLimit = limit;
}

void TestCase::setMemoryLimit(int limit)
{
	memoryLimit = limit;
}

void TestCase::setInputFiles(int index, const QString &fileName)
{
	if(0 <= index && index < inputFiles.size())
	{
		inputFiles[index] = fileName;
	}
}

void TestCase::setOutputFiles(int index, const QString &fileName)
{
	if(0 <= index && index < outputFiles.size())
	{
		outputFiles[index] = fileName;
	}
}

void TestCase::setDependenceSubtask(const QStringList &list)
{
	dependenceSubtask.clear();

	for(int i = 0; i != list.size(); ++i)
		dependenceSubtask.push_back(list[i].toInt());
}

bool TestCase::checkDependenceSubtask(const QStringList &list)
{
	QList<int> temp;

	for(int i = 0; i != list.size(); ++i)
	{
		int     t;
		t = list[i].toInt();

		if(t <= 0 || t >= index)
			return false;

		for(int j = 0; j != temp.size(); ++j)
			if(temp[j] == t)
				return false;

		temp.push_back(t);
	}

	return true;
}

void TestCase::addSingleCase(const QString &inputFile, const QString &outputFile)
{
	inputFiles.append(inputFile);
	outputFiles.append(outputFile);
}

void TestCase::deleteSingleCase(int index)
{
	inputFiles.removeAt(index);
	outputFiles.removeAt(index);
}

void TestCase::writeToStream(QDataStream &out)
{
	out << fullScore;
	out << timeLimit;
	out << memoryLimit;
	QStringList _inputFiles(inputFiles);

	for(int i = 0; i < _inputFiles.size(); i ++)
	{
		_inputFiles[i].replace(QDir::separator(), '/');
	}

	for(int i = 0; i < dependenceSubtask.size(); ++i)
	{
		_inputFiles.push_back(QString("%1_lemon_SUbtaskDEPENDENCE_fLAg").arg(dependenceSubtask[i]));
	}

	QStringList _outputFiles(outputFiles);

	for(int i = 0; i < _outputFiles.size(); i ++)
	{
		_outputFiles[i].replace(QDir::separator(), '/');
	}

	out << _inputFiles;
	out << _outputFiles;
}

void TestCase::readFromStream(QDataStream &in)
{
	in >> fullScore;
	in >> timeLimit;
	in >> memoryLimit;
	QStringList _inputFiles;
	in >> _inputFiles;
	in >> outputFiles;
	inputFiles.clear();
	dependenceSubtask.clear();

	for(int i = 0; i < _inputFiles.size(); i ++)
	{
		if(_inputFiles[i].endsWith("_lemon_SUbtaskDEPENDENCE_fLAg"))
		{
			int temp(0);

			for(QString::iterator itr = _inputFiles[i].begin(); *itr != '_'; ++itr)
				(temp *= 10) += itr->toLatin1() ^ '0';

			dependenceSubtask.push_back(temp);
		}
		else
		{
			inputFiles.push_back(_inputFiles[i]);
			inputFiles.back().replace('/', QDir::separator());
		}
	}

	for(int i = 0; i < outputFiles.size(); i ++)
	{
		outputFiles[i].replace('/', QDir::separator());
	}
}

void TestCase::clearDependenceSubtask()
{
	dependenceSubtask.clear();
}
