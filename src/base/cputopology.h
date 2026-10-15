/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

namespace Lemon {

	struct CpuTopology {
		int physicalCores = 0;
		int performanceCores = 0;
	};

	CpuTopology detectCpuTopology();
	int getDefaultJudgingThreadCount(const CpuTopology &topology, int fallbackProcessorCount = 1);
	int getDefaultJudgingThreadCount();

} // namespace Lemon
