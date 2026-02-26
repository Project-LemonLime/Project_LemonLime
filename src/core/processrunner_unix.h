/*
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#ifndef Q_OS_WIN32

#include "processrunner.h"

class UnixProcessRunner : public ProcessRunner {
  public:
	using ProcessRunner::ProcessRunner;
	ProcessRunnerResult run() override;
};

#endif
