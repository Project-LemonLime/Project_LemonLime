// SPDX-License-Identifier: GPL-3.0-or-later
#include "../../src/core/windowsprocessutils.h"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <psapi.h>

// Define after dependency headers so their macro cleanup cannot hide the regression.
#ifdef max
#undef max
#endif
#define max(a, b) windows_max_macro_must_not_expand(a, b)
#include "../../src/core/windowsmemory.h"
#undef max
