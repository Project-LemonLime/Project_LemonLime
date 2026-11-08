// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>

namespace Lemon::Execution {
	using Bytes = std::int64_t;

	inline bool memoryBytes(std::int64_t mib, Bytes &bytes) {
		if (mib < 0) {
			bytes = -1;
			return true;
		}
		if (mib > (std::numeric_limits<Bytes>::max)() / (1024 * 1024))
			return false;
		bytes = mib * 1024 * 1024;
		return true;
	}

	// Tracks memory usage only; platform code owns process operations and exit statuses.
	class Limiter {
	  public:
		explicit Limiter(Bytes limit) : limit(limit) {}
		bool observe(Bytes declaredUsage) {
			peak = (std::max)(peak, declaredUsage);
			return isExceeded();
		}
		Bytes usage() const { return peak; }
		bool isExceeded() const { return limit >= 0 && peak > limit; }

	  private:
		Bytes limit;
		Bytes peak = -1;
	};
} // namespace Lemon::Execution
