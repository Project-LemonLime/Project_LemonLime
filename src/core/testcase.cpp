/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "testcase.h"
//
#include "base/settings.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#define qAsConst
#endif

TestCase::TestCase(QObject *parent) : QObject(parent) {}

void TestCase::copyTo(TestCase *to) {
	QByteArray data;
	QDataStream tmpin(&data, QIODevice::WriteOnly);
	writeToStream(tmpin);
	QDataStream tmpout(&data, QIODevice::ReadOnly);
	to->readFromStream(tmpout);
}

auto TestCase::getFullScore() const -> int { return fullScore; }

auto TestCase::getTimeLimit() const -> int { return timeLimit; }

auto TestCase::getMemoryLimit() const -> int { return memoryLimit; }

auto TestCase::getInputFiles() const -> const QStringList & { return inputFiles; }

auto TestCase::getOutputFiles() const -> const QStringList & { return outputFiles; }

auto TestCase::getDependenceSubtask() const -> const QList<int> & { return dependenceSubtask; }

void TestCase::setIndex(int ind) { index = ind; }

void TestCase::setFullScore(int score) { fullScore = score; }

void TestCase::setTimeLimit(int limit) { timeLimit = limit; }

void TestCase::setMemoryLimit(int limit) { memoryLimit = limit; }

void TestCase::setInputFiles(int index, const QString &fileName) {
	if (0 <= index && index < inputFiles.size()) {
		inputFiles[index] = fileName;
	}
}

void TestCase::setOutputFiles(int index, const QString &fileName) {
	if (0 <= index && index < outputFiles.size()) {
		outputFiles[index] = fileName;
	}
}

void TestCase::setDependenceSubtask(const QStringList &list) {
	dependenceSubtask.clear();

	for (int i = 0; i != list.size(); ++i)
		dependenceSubtask.push_back(list[i].toInt());

	std::sort(dependenceSubtask.begin(), dependenceSubtask.end());
}

void TestCase::setDependenceSubtask(const QList<int> &list) {
	dependenceSubtask = list;

	std::sort(dependenceSubtask.begin(), dependenceSubtask.end());
}

void TestCase::setDependenceSubtask(const QSet<int> &list) {
	dependenceSubtask.clear();

	for (auto i : list)
		dependenceSubtask.push_back(i);

	std::sort(dependenceSubtask.begin(), dependenceSubtask.end());
}

auto TestCase::checkDependenceSubtask(const QStringList &list) const -> bool {
	QList<int> temp;

	for (int i = 0; i != list.size(); ++i) {
		int t = 0;
		t = list[i].toInt();

		if (t <= 0 || t >= index)
			return false;

		for (int j = 0; j != temp.size(); ++j)
			if (temp[j] == t)
				return false;

		temp.push_back(t);
	}

	return true;
}

void TestCase::addSingleCase(const QString &inputFile, const QString &outputFile) {
	inputFiles.append(inputFile);
	outputFiles.append(outputFile);
}

void TestCase::deleteSingleCase(int index) {
	inputFiles.removeAt(index);
	outputFiles.removeAt(index);
}

void TestCase::writeToStream(QDataStream &out) {
	out << fullScore;
	out << timeLimit;
	out << memoryLimit;
	QStringList _inputFiles(inputFiles);

	for (auto &i : _inputFiles) {
		i.replace(QDir::separator(), '/');
	}

	for (int i : qAsConst(dependenceSubtask)) {
		_inputFiles.push_back(QString("%1_lemon_SUbtaskDEPENDENCE_fLAg").arg(i));
	}

	QStringList _outputFiles(outputFiles);

	for (auto &i : _outputFiles) {
		i.replace(QDir::separator(), '/');
	}

	out << _inputFiles;
	out << _outputFiles;
}

void TestCase::readFromStream(QDataStream &in) {
	in >> fullScore;
	in >> timeLimit;
	in >> memoryLimit;
	QStringList _inputFiles;
	in >> _inputFiles;
	in >> outputFiles;
	inputFiles.clear();
	dependenceSubtask.clear();

	for (auto &i : _inputFiles) {
		if (i.endsWith("_lemon_SUbtaskDEPENDENCE_fLAg")) {
			int temp(0);

			for (auto *itr = i.constBegin(); *itr != '_'; ++itr)
				(temp *= 10) += itr->toLatin1() ^ '0';

			dependenceSubtask.push_back(temp);
		} else {
			inputFiles.push_back(i);
			inputFiles.back().replace('/', QDir::separator());
		}
	}

	for (auto &i : outputFiles) {
		i.replace('/', QDir::separator());
	}
}

void TestCase::clearDependenceSubtask() { dependenceSubtask.clear(); }

void TestCase::swapFiles(int a, int b) {
	if (a < 0 || a >= inputFiles.size())
		return;

	if (b < 0 || b >= inputFiles.size())
		return;

	qSwap(inputFiles[a], inputFiles[b]);
	qSwap(outputFiles[a], outputFiles[b]);
}
