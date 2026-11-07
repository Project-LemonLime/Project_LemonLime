#include <algorithm>
#include <cstdint>
#include <limits>
// Reproduce the Windows SDK function-like macro, including with a PCH.
#define max(a, b) windows_max_macro_must_not_expand(a, b)
#include "../../src/core/memorylimiter.h"
#undef max
#include <cstdlib>
using namespace Lemon::Execution;
void check(bool value) {
	if (! value)
		std::abort();
}
int main() {
	Limiter limited(100);
	check(! limited.observe({100, 10}));
	check(! limited.observe({20, 90}));
	check(limited.peaks().declared == 100 && limited.peaks().actual == 90);
	check(limited.observe({-1, 101}));
	check(limited.observe({0, 0}) && limited.usage() == 101);
	check(limited.finish(ExitKind::RuntimeError).reason == RunReason::MemoryLimit);
	check(limited.finish(ExitKind::StartError).reason == RunReason::InternalError);
	Limiter unlimited(-1);
	check(unlimited.finish(ExitKind::RuntimeError).reason == RunReason::RuntimeError);
	check(unlimited.finish(ExitKind::Normal).reason == RunReason::Accepted);
	check(unlimited.finish(ExitKind::TimeLimit).reason == RunReason::TimeLimit);
	check(unlimited.usage() == -1);
	check(! unlimited.observe({1000, 2000}));
	Bytes bytes;
	check(memoryBytes(-1, bytes) && bytes == -1);
	check(memoryBytes(4096, bytes) && bytes == 4294967296LL);
	check(! memoryBytes(std::numeric_limits<Bytes>::max(), bytes));
}
