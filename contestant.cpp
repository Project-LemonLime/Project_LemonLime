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
 * contestant.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "contestant.h"
#include "contest.h"

Contestant::Contestant(QObject *parent) :
	QObject(parent)
{
}

const QString& Contestant::getContestantName() const
{
	return contestantName;
}

bool Contestant::getCheckJudged(int index) const
{
	return checkJudged[index];
}

CompileState Contestant::getCompileState(int index) const
{
	return compileState[index];
}

const QString& Contestant::getSourceFile(int index) const
{
	return sourceFile[index];
}

const QString& Contestant::getCompileMessage(int index) const
{
	return compileMesaage[index];
}

const QList<QStringList>& Contestant::getInputFiles(int index) const
{
	return inputFiles[index];
}

const QList< QList<ResultState> >& Contestant::getResult(int index) const
{
	return result[index];
}

const QList<QStringList>& Contestant::getMessage(int index) const
{
	return message[index];
}

const QList< QList<int> >& Contestant::getSocre(int index) const
{
	return score[index];
}

const QList< QList<int> >& Contestant::getTimeUsed(int index) const
{
	return timeUsed[index];
}

const QList< QList<int> >& Contestant::getMemoryUsed(int index) const
{
	return memoryUsed[index];
}

QDateTime Contestant::getJudingTime() const
{
	return judgingTime;
}

void Contestant::setContestantName(const QString &name)
{
	contestantName = name;
}

void Contestant::setCheckJudged(int index, bool check)
{
	checkJudged[index] = check;
}

void Contestant::setCompileState(int index, CompileState state)
{
	compileState[index] = state;
}

void Contestant::setSourceFile(int index, const QString &fileName)
{
	sourceFile[index] = fileName;
}

void Contestant::setCompileMessage(int index, const QString &text)
{
	compileMesaage[index] = text;
}

void Contestant::setInputFiles(int index, const QList<QStringList> &files)
{
	inputFiles[index] = files;
}

void Contestant::setResult(int index, const QList< QList<ResultState> > &_result)
{
	result[index] = _result;
}

void Contestant::setMessage(int index, const QList<QStringList>&_message)
{
	message[index] = _message;
}

void Contestant::setScore(int index, const QList< QList<int> > &_score)
{
	score[index] = _score;
}

void Contestant::setTimeUsed(int index, const QList< QList<int> > &_timeUsed)
{
	timeUsed[index] = _timeUsed;
}

void Contestant::setMemoryUsed(int index, const QList< QList<int> > &_memoryUsed)
{
	memoryUsed[index] = _memoryUsed;
}

void Contestant::setJudgingTime(QDateTime time)
{
	judgingTime = time;
}

void Contestant::addTask()
{
	checkJudged.append(false);
	compileState.append(NoValidSourceFile);
	sourceFile.append("");
	compileMesaage.append("");
	inputFiles.append(QList<QStringList>());
	result.append(QList< QList<ResultState> >());
	message.append(QList<QStringList>());
	score.append(QList< QList<int> >());
	timeUsed.append(QList< QList<int> >());
	memoryUsed.append(QList< QList<int> >());
}

void Contestant::deleteTask(int index)
{
	checkJudged.removeAt(index);
	compileState.removeAt(index);
	sourceFile.removeAt(index);
	compileMesaage.removeAt(index);
	inputFiles.removeAt(index);
	result.removeAt(index);
	message.removeAt(index);
	score.removeAt(index);
	timeUsed.removeAt(index);
	memoryUsed.removeAt(index);
}

int Contestant::getTaskScore(int index) const
{
	if(0 > index || index >= checkJudged.size()) return -1;

	if(! checkJudged[index]) return -1;

	int total = 0;

	for(int i = 0; i < score[index].size(); i ++)
	{
		int minv = 1000000000;

		for(int j = 0; j < score[index][i].size(); j ++)
		{
			if(score[index][i][j] < minv) minv = score[index][i][j];
		}

		if(minv == 1000000000) minv = 0;

		total += minv;
	}

	return total;
}

int Contestant::getTotalScore() const
{
	if(checkJudged.size() == 0) return -1;

	for(int i = 0; i < checkJudged.size(); i ++)
	{
		if(! checkJudged[i]) return -1;
	}

	int total = 0;

	for(int i = 0; i < score.size(); i ++)
	{
		total += getTaskScore(i);
	}

	return total;
}

int Contestant::getTotalUsedTime() const
{
	if(checkJudged.size() == 0) return -1;

	for(int i = 0; i < checkJudged.size(); i ++)
	{
		if(! checkJudged[i]) return -1;
	}

	int total = 0;

	for(int i = 0; i < timeUsed.size(); i ++)
	{
		for(int j = 0; j < timeUsed[i].size(); j ++)
		{
			for(int k = 0; k < timeUsed[i][j].size(); k ++)
			{
				if(timeUsed[i][j][k] >= 0) total += timeUsed[i][j][k];
			}
		}
	}

	return total;
}

void Contestant::writeToStream(QDataStream &out)
{
	out << contestantName;
	out << checkJudged;
	out << sourceFile;
	out << compileMesaage;
	out << inputFiles;
	out << message;
	out << score;
	out << timeUsed;
	out << memoryUsed;
	out << static_cast<quint32>(judgingTime.date().toJulianDay());
	out << static_cast<quint32>(judgingTime.time().msecsSinceStartOfDay());
	out << static_cast<quint8>(judgingTime.timeSpec());
	out << compileState.size();

	for(int i = 0; i < compileState.size(); i ++)
	{
		out << int(compileState[i]);
	}

	out << result.size();

	for(int i = 0; i < result.size(); i ++)
	{
		out << result[i].size();

		for(int j = 0; j < result[i].size(); j ++)
		{
			out << result[i][j].size();

			for(int k = 0; k < result[i][j].size(); k ++)
			{
				out << int(result[i][j][k]);
			}
		}
	}
}

void Contestant::readFromStream(QDataStream &in)
{
	in >> contestantName;
	in >> checkJudged;
	in >> sourceFile;
	in >> compileMesaage;
	in >> inputFiles;
	in >> message;
	in >> score;
	in >> timeUsed;
	in >> memoryUsed;
	quint32      judgingTime_date;
	quint32      judgingTime_time;
	quint8       judgingTime_timespec;
	in >> judgingTime_date;
	in >> judgingTime_time;
	in >> judgingTime_timespec;
	judgingTime = QDateTime(QDate::fromJulianDay(judgingTime_date), QTime::fromMSecsSinceStartOfDay(judgingTime_time), Qt::TimeSpec(judgingTime_timespec));
	int count, _count, __count, tmp;
	in >> count;

	for(int i = 0; i < count; i ++)
	{
		in >> tmp;
		compileState.append(CompileState(tmp));
	}

	in >> count;

	for(int i = 0; i < count; i ++)
	{
		result.append(QList< QList<ResultState> >());
		in >> _count;

		for(int j = 0; j < _count; j ++)
		{
			result[i].append(QList<ResultState>());
			in >> __count;

			for(int k = 0; k < __count; k ++)
			{
				in >> tmp;
				result[i][j].append(ResultState(tmp));
			}
		}
	}
}
