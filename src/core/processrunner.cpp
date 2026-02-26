/*
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "processrunner.h"

#ifdef Q_OS_WIN32
#include "processrunner_win.h"
#else
#include "processrunner_unix.h"
#endif

ProcessRunner::ProcessRunner(ProcessRunnerConfig cfg, const std::atomic<bool> &stop,
                             const std::atomic<bool> &skip)
    : config(std::move(cfg)), stopFlag(stop), skipFlag(skip) {}

auto ProcessRunner::create(ProcessRunnerConfig config, const std::atomic<bool> &stopFlag,
                           const std::atomic<bool> &skipFlag) -> std::unique_ptr<ProcessRunner> {
#ifdef Q_OS_WIN32
	return std::make_unique<WinProcessRunner>(std::move(config), stopFlag, skipFlag);
#else
	return std::make_unique<UnixProcessRunner>(std::move(config), stopFlag, skipFlag);
#endif
}
