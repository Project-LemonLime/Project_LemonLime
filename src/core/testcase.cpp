/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "testcase.h"
#include "base/LemonUtils.hpp"
#include "base/settings.h"

TestCase::TestCase() {}

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

int TestCase::writeToJson(QJsonObject &out) {
	WRITE_JSON(out, fullScore);
	WRITE_JSON(out, timeLimit);
	WRITE_JSON(out, memoryLimit);

	QStringList inputFiles = this->inputFiles;
	for (auto &filename : inputFiles) {
		filename.replace('/', QDir::separator());
	}
	for (int i : qAsConst(dependenceSubtask)) {
		inputFiles.push_back(QString("%1_lemon_SUbtaskDEPENDENCE_fLAg").arg(i));
	}

	WRITE_JSON(out, inputFiles);

	QStringList outputFiles = this->outputFiles;

	for (auto &filename : inputFiles) {
		filename.replace('/', QDir::separator());
	}

	WRITE_JSON(out, outputFiles);
	return 0;
}

int TestCase::readFromJson(const QJsonObject &in) {
	READ_JSON(in, fullScore);
	READ_JSON(in, timeLimit);
	READ_JSON(in, memoryLimit);

	QJsonArray inputFiles;
	this->inputFiles.clear();
	READ_JSON(in, inputFiles);
	for (const auto &i : inputFiles) {
		if (! i.isString())
			return -1;
		auto fileName = i.toString();
		if (fileName.endsWith("_lemon_SUbtaskDEPENDENCE_fLAg")) {
			int temp{0};
			for (auto c : fileName) {
				temp *= 10;
				temp += c.toLatin1() ^ '0';
			}
			dependenceSubtask.push_back(temp);
		} else {
			fileName.replace('/', QDir::separator());
			this->inputFiles.push_back(fileName);
		}
	}

	READ_JSON(in, outputFiles);
	return 0;
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
