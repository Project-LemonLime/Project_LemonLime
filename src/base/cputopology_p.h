/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include "cputopology.h"

#include <QString>

namespace Lemon::detail {

	// Kept independent of the host OS so sysfs snapshots can be tested on every platform.
	CpuTopology detectLinuxCpuTopology(const QString &cpuRoot, const QString &eventSourceRoot,
	                                   bool useCpuCapacityClasses = false);

} // namespace Lemon::detail
