// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/memorylimiter.h"
#include <sys/types.h>
Lemon::Execution::MemorySample sampleMemory(pid_t pid);
Lemon::Execution::Bytes maxRSSBytes(long value);
