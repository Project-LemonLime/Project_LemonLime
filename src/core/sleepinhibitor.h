/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include <memory>

class QString;

class SleepInhibitor {
  public:
	explicit SleepInhibitor(const QString &reason);
	~SleepInhibitor();

	SleepInhibitor(const SleepInhibitor &) = delete;
	SleepInhibitor &operator=(const SleepInhibitor &) = delete;

  private:
	struct State;
	std::unique_ptr<State> state;
};
