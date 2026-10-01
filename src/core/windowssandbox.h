/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include "processrunner.h"
#ifdef Q_OS_WIN
#include <windows.h>

class WindowsSandbox {
  public:
	static std::shared_ptr<WindowsSandboxSession> createSession();
	explicit WindowsSandbox(const ProcessRunnerConfig &config);
	~WindowsSandbox();
	WindowsSandbox(const WindowsSandbox &) = delete;
	WindowsSandbox &operator=(const WindowsSandbox &) = delete;
	bool prepare(QString &error);
	bool prepareProcess(STARTUPINFOEXW &startup, QString &error);
	const QProcessEnvironment &environment() const;
	int aclUpdates() const;
	bool cacheHit() const;

  private:
	struct Data;
	std::unique_ptr<Data> data;
};
#endif
