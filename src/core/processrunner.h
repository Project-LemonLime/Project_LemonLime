/*
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include "base/LemonType.hpp"
#include "base/sandboxsettings.h"
#include <QProcessEnvironment>
#include <QString>
#include <QtGlobal>
#include <atomic>
#include <memory>

class WindowsSandboxSession;

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
	SandboxSettings sandboxSettings;
	// The trusted compiler/interpreter selected by the user, never a submission.
	QString runtimeExecutable;
	QProcessEnvironment runtimeEnvironment;
	std::shared_ptr<WindowsSandboxSession> sandboxSession;
};

struct ProcessRunnerResult {
	ResultState result = CorrectAnswer;
	int score = 0;
	int timeUsed = -1;
	qint64 memoryUsed = -1;
	QString message;
	qint64 preparationTime = 0;
	int runtimeAclUpdates = 0;
	bool runtimeCacheHit = false;
};

class ProcessRunner {
  public:
	ProcessRunner(ProcessRunnerConfig config, const std::atomic<bool> &stopFlag);
	virtual ~ProcessRunner() = default;

	virtual ProcessRunnerResult run() = 0;

	static std::unique_ptr<ProcessRunner> create(ProcessRunnerConfig config,
	                                             const std::atomic<bool> &stopFlag);

  protected:
	ProcessRunnerConfig config;
	const std::atomic<bool> &stopFlag;
};
