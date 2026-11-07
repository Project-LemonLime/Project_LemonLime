// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/memorylimiter.h"
#include <sys/types.h>
Lemon::Execution::Bytes sampleMemory(pid_t pid);
#ifdef __APPLE__
Lemon::Execution::Bytes maxRSSBytes(long value);
#endif
