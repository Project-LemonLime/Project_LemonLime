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

	enum class ExitKind { Normal, RuntimeError, TimeLimit, StartError };
	enum class RunReason { Accepted, RuntimeError, TimeLimit, MemoryLimit, InternalError };
	struct RunOutcome {
		RunReason reason;
		Bytes usage;
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
		RunOutcome finish(ExitKind exit) const {
			RunReason reason = RunReason::Accepted;
			if (exit == ExitKind::StartError)
				reason = RunReason::InternalError;
			else if (exceeded)
				reason = RunReason::MemoryLimit;
			else if (exit == ExitKind::TimeLimit)
				reason = RunReason::TimeLimit;
			else if (exit == ExitKind::RuntimeError)
				reason = RunReason::RuntimeError;
			return {reason, usage()};
		}
		Bytes usage() const { return peak; }
		bool isExceeded() const { return exceeded; }

	  private:
		Bytes limit;
		Bytes peak = -1;
		bool exceeded = false;
	};
} // namespace Lemon::Execution
