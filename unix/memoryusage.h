// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "../src/core/memoryunits.h"
#include <sys/types.h>
Lemon::Execution::Bytes sampleMemory(pid_t pid);
bool applyMemoryLimit(Lemon::Execution::Bytes limit);
