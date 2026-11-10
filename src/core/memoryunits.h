// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <cstdint>
#include <limits>

namespace Lemon::Execution {
	using Bytes = std::int64_t;
	inline constexpr Bytes KiB = 1024;
	inline constexpr Bytes MiB = KiB * 1024;

	inline bool memoryBytes(std::int64_t mib, Bytes &bytes) {
		if (mib < 0) {
			bytes = -1;
			return true;
		}
		if (mib > (std::numeric_limits<Bytes>::max)() / MiB)
			return false;
		bytes = mib * MiB;
		return true;
	}
} // namespace Lemon::Execution
