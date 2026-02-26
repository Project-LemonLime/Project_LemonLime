/*
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include "base/LemonType.hpp"
#include <QProcessEnvironment>
#include <QString>
#include <QtGlobal>
#include <atomic>
#include <memory>

struct ProcessRunnerConfig {
	QString executableFile;
	QString arguments;
	QString workingDirectory;
	QString inputFile;
	QProcessEnvironment environment;
	int timeLimit{};
	int rawTimeLimit{};
	int memoryLimit{};
	int rawMemoryLimit{};
	double extraTimeRatio{};
	bool standardInputCheck{};
	bool standardOutputCheck{};
	QString inputFileName;
	QString outputFileName;
	bool interpreterAsWatcher{};
};

struct ProcessRunnerResult {
	ResultState result = CorrectAnswer;
	int score = 0;
	int timeUsed = -1;
	int memoryUsed = -1;
	QString message;
};

class ProcessRunner {
  public:
	ProcessRunner(ProcessRunnerConfig config, const std::atomic<bool> &stopFlag,
	              const std::atomic<bool> &skipFlag);
	virtual ~ProcessRunner() = default;

	virtual ProcessRunnerResult run() = 0;

	static std::unique_ptr<ProcessRunner>
	create(ProcessRunnerConfig config, const std::atomic<bool> &stopFlag, const std::atomic<bool> &skipFlag);

  protected:
	ProcessRunnerConfig config;
	const std::atomic<bool> &stopFlag;
	const std::atomic<bool> &skipFlag;
};
