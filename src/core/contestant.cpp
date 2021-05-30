/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "contestant.h"

#include "base/LemonUtils.hpp"
#include "core/contest.h"
#include <utility>

Contestant::Contestant(QObject *parent) : QObject(parent) {}

auto Contestant::getContestantName() const -> const QString & { return contestantName; }

auto Contestant::getCheckJudged(int index) const -> bool { return checkJudged[index]; }

auto Contestant::getCompileState(int index) const -> CompileState { return compileState[index]; }

auto Contestant::getSourceFile(int index) const -> const QString & { return sourceFile[index]; }

auto Contestant::getCompileMessage(int index) const -> const QString & { return compileMesaage[index]; }

auto Contestant::getInputFiles(int index) const -> const QList<QStringList> & { return inputFiles[index]; }

auto Contestant::getResult(int index) const -> const QList<QList<ResultState>> & { return result[index]; }

auto Contestant::getMessage(int index) const -> const QList<QStringList> & { return message[index]; }

auto Contestant::getScore(int index) const -> const QList<QList<int>> & { return score[index]; }

auto Contestant::getTimeUsed(int index) const -> const QList<QList<int>> & { return timeUsed[index]; }

auto Contestant::getMemoryUsed(int index) const -> const QList<QList<int>> & { return memoryUsed[index]; }

auto Contestant::getJudingTime() const -> QDateTime { return judgingTime; }

void Contestant::setContestantName(const QString &name) { contestantName = name; }

void Contestant::setCheckJudged(int index, bool check) { checkJudged[index] = check; }

void Contestant::setCompileState(int index, CompileState state) { compileState[index] = state; }

void Contestant::setSourceFile(int index, const QString &fileName) { sourceFile[index] = fileName; }

void Contestant::setCompileMessage(int index, const QString &text) { compileMesaage[index] = text; }

void Contestant::setInputFiles(int index, const QList<QStringList> &files) { inputFiles[index] = files; }

void Contestant::setResult(int index, const QList<QList<ResultState>> &_result) { result[index] = _result; }

void Contestant::setMessage(int index, const QList<QStringList> &_message) { message[index] = _message; }

void Contestant::setScore(int index, const QList<QList<int>> &_score) { score[index] = _score; }

void Contestant::setTimeUsed(int index, const QList<QList<int>> &_timeUsed) { timeUsed[index] = _timeUsed; }

void Contestant::setMemoryUsed(int index, const QList<QList<int>> &_memoryUsed) {
	memoryUsed[index] = _memoryUsed;
}

void Contestant::setJudgingTime(QDateTime time) { judgingTime = std::move(time); }

void Contestant::addTask() {
	checkJudged.append(false);
	compileState.append(NoValidSourceFile);
	sourceFile.append("");
	compileMesaage.append("");
	inputFiles.append(QList<QStringList>());
	result.append(QList<QList<ResultState>>());
	message.append(QList<QStringList>());
	score.append(QList<QList<int>>());
	timeUsed.append(QList<QList<int>>());
	memoryUsed.append(QList<QList<int>>());
}

void Contestant::deleteTask(int index) {
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

void Contestant::swapTask(int a, int b) {
	if (a < 0 || a >= checkJudged.size())
		return;

	if (b < 0 || b >= checkJudged.size())
		return;

	checkJudged.swapItemsAt(a, b);
	compileState.swapItemsAt(a, b);
	sourceFile.swapItemsAt(a, b);
	compileMesaage.swapItemsAt(a, b);
	inputFiles.swapItemsAt(a, b);
	result.swapItemsAt(a, b);
	message.swapItemsAt(a, b);
	score.swapItemsAt(a, b);
	timeUsed.swapItemsAt(a, b);
	memoryUsed.swapItemsAt(a, b);
}

auto Contestant::getTaskScore(int index) const -> int {
	if (0 > index || index >= checkJudged.size())
		return -1;

	if (! checkJudged[index])
		return -1;

	int total = 0;

	for (const auto &i : score[index]) {
		int minv = 1000000000;

		for (int j : i) {
			if (j < minv && j >= 0)
				minv = j;
		}

		if (minv == 1000000000)
			minv = 0;

		total += minv;
	}

	return total;
}

auto Contestant::getTotalScore() const -> int {
	if (checkJudged.empty())
		return -1;

	for (bool i : checkJudged) {
		if (! i)
			return -1;
	}

	int total = 0;

	for (int i = 0; i < score.size(); i++) {
		total += getTaskScore(i);
	}

	return total;
}

auto Contestant::getTotalUsedTime() const -> int {
	if (checkJudged.empty())
		return -1;

	for (bool i : checkJudged) {
		if (! i)
			return -1;
	}

	int total = 0;

	for (const auto &i : timeUsed) {
		for (const auto &j : i) {
			for (int k : j) {
				if (k >= 0)
					total += k;
			}
		}
	}

	return total;
}
int Contestant::writeToJson(QJsonObject &out) {
	WRITE_JSON(out, contestantName);
	WRITE_JSON(out, checkJudged);
	WRITE_JSON(out, sourceFile);
	WRITE_JSON(out, compileMesaage);
	WRITE_JSON(out, inputFiles);
	WRITE_JSON(out, message);
	WRITE_JSON(out, score);
	WRITE_JSON(out, timeUsed);
	WRITE_JSON(out, memoryUsed);
	int judgingTime_date = judgingTime.date().toJulianDay();
	int judgingTime_time = judgingTime.time().msecsSinceStartOfDay();
	int judgingTime_timespec = judgingTime.timeSpec();
	WRITE_JSON(out, judgingTime_date);
	WRITE_JSON(out, judgingTime_time);
	WRITE_JSON(out, judgingTime_timespec);
	WRITE_JSON(out, compileState);
	WRITE_JSON(out, result);
	return 0;
}
void Contestant::writeToStream(QDataStream &out) {
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

	for (auto &i : compileState) {
		out << int(i);
	}

	out << result.size();

	for (auto &i : result) {
		out << i.size();

		for (auto &j : i) {
			out << j.size();

			for (auto &k : j) {
				out << int(k);
			}
		}
	}
}
int Contestant::readFromJson(const QJsonObject &in) {
	READ_JSON(in, contestantName);
	READ_JSON(in, checkJudged);
	READ_JSON(in, sourceFile);
	READ_JSON(in, compileMesaage);
	READ_JSON(in, inputFiles);
	READ_JSON(in, message);
	READ_JSON(in, score);
	READ_JSON(in, timeUsed);
	READ_JSON(in, memoryUsed);
	int judgingTime_date = 0;
	int judgingTime_time = 0;
	int judgingTime_timespec = 0;
	READ_JSON(in, judgingTime_date);
	READ_JSON(in, judgingTime_time);
	READ_JSON(in, judgingTime_timespec);
	judgingTime =
	    QDateTime(QDate::fromJulianDay(judgingTime_date), QTime::fromMSecsSinceStartOfDay(judgingTime_time),
	              Qt::TimeSpec(judgingTime_timespec));
	READ_JSON(in, compileState);
	READ_JSON(in, result);
	return 0;
}

void Contestant::readFromStream(QDataStream &in) {
	in >> contestantName;
	in >> checkJudged;
	in >> sourceFile;
	in >> compileMesaage;
	in >> inputFiles;
	in >> message;
	in >> score;
	in >> timeUsed;
	in >> memoryUsed;
	quint32 judgingTime_date = 0;
	quint32 judgingTime_time = 0;
	quint8 judgingTime_timespec = 0;
	in >> judgingTime_date;
	in >> judgingTime_time;
	in >> judgingTime_timespec;
	judgingTime = QDateTime(QDate::fromJulianDay(judgingTime_date),
	                        QTime::fromMSecsSinceStartOfDay(static_cast<int>(judgingTime_time)),
	                        Qt::TimeSpec(judgingTime_timespec));
	int count = 0;
	int _count = 0;
	int __count = 0;
	int tmp = 0;
	in >> count;

	for (int i = 0; i < count; i++) {
		in >> tmp;
		compileState.append(CompileState(tmp));
	}

	in >> count;

	for (int i = 0; i < count; i++) {
		result.append(QList<QList<ResultState>>());
		in >> _count;

		for (int j = 0; j < _count; j++) {
			result[i].append(QList<ResultState>());
			in >> __count;

			for (int k = 0; k < __count; k++) {
				in >> tmp;
				result[i][j].append(ResultState(tmp));
			}
		}
	}
}
