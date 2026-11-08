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

	class UsageProvider {
	  public:
		virtual ~UsageProvider() = default;
		// DeclaredUsage in bytes; negative means unavailable.
		virtual Bytes sample() = 0;
	};
	class Prelimit {
	  public:
		virtual ~Prelimit() = default;
		virtual bool apply(Bytes limit) = 0;
	};
	class Killer {
	  public:
		virtual ~Killer() = default;
		// Must terminate the execution unit and perform bounded reaping.
		virtual bool killAndReap() = 0;
	};

	// Pure policy: OS operations and process exit classification belong to adapters.
	class Limiter {
	  public:
		explicit Limiter(Bytes limit) : limit(limit) {}
		bool observe(Bytes declaredUsage) {
			peak = (std::max)(peak, declaredUsage);
			exceeded = exceeded || (limit >= 0 && usage() > limit);
			return exceeded;
		}
		Bytes usage() const { return peak; }
		bool isExceeded() const { return exceeded; }

	  private:
		Bytes limit;
		Bytes peak = -1;
		bool exceeded = false;
	};
} // namespace Lemon::Execution
