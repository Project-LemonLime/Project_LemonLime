/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "cputopology.h"
#include "base/LemonLog.hpp"
#include "cputopology_p.h"

#include <QDir>
#include <QFile>
#include <QMap>
#include <QSet>
#include <QThread>

#include <algorithm>
#include <optional>

#ifdef Q_OS_WIN
#include <qt_windows.h>

#include <cstddef>
#include <vector>
#elif defined(Q_OS_MACOS)
#include <sys/sysctl.h>

#include <cerrno>
#endif

#define LEMON_MODULE_NAME "CpuTopology"

namespace {

	QByteArray readSysfsFile(const QString &fileName) {
		QFile file(fileName);
		if (! file.open(QIODevice::ReadOnly))
			return {};
		return file.readAll().trimmed();
	}

	std::optional<QSet<int>> readCpuList(const QString &fileName) {
		QFile file(fileName);
		if (! file.open(QIODevice::ReadOnly))
			return std::nullopt;
		const QByteArray contents = file.readAll().trimmed();
		QSet<int> processors;
		if (contents.isEmpty())
			return processors;
		for (const QByteArray &part : contents.split(',')) {
			const QList<QByteArray> bounds = part.trimmed().split('-');
			const int first = bounds.first().toInt();
			const int last = bounds.last().toInt();
			for (int processor = first; processor <= last; ++processor)
				processors.insert(processor);
		}
		return processors;
	}

	struct LinuxCore {
		QSet<int> logicalProcessors;
		int capacity = 0;
	};

#ifdef Q_OS_WIN
	Lemon::CpuTopology detectWindowsCpuTopology() {
		DWORD size = 0;
		GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &size);
		const DWORD error = GetLastError();
		if (error != ERROR_INSUFFICIENT_BUFFER || size == 0) {
			DEBUG("Windows CPU topology size query failed; error:", error, "buffer size:", size);
			return {};
		}
		std::vector<unsigned char> buffer(size);
		if (! GetLogicalProcessorInformationEx(
		        RelationProcessorCore,
		        reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()), &size)) {
			DEBUG("Windows CPU topology query failed; error:", GetLastError());
			return {};
		}

		Lemon::CpuTopology topology;
		int highestClass = -1;
		for (size_t offset = 0; offset < size;) {
			const auto *record =
			    reinterpret_cast<const SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *>(buffer.data() + offset);
			if (record->Relationship == RelationProcessorCore) {
				++topology.physicalCores;
				const int performanceClass = record->Processor.EfficiencyClass;
				if (performanceClass > highestClass) {
					highestClass = performanceClass;
					topology.performanceCores = 1;
				} else if (performanceClass == highestClass) {
					++topology.performanceCores;
				}
			}
			offset += record->Size;
		}
		DEBUG("Windows CPU topology; physical cores:", topology.physicalCores,
		      "highest efficiency class:", highestClass,
		      "cores in highest class:", topology.performanceCores);
		return topology;
	}
#elif defined(Q_OS_MACOS)
	int readSysctlInt(const char *name) {
		int value = 0;
		size_t size = sizeof(value);
		if (sysctlbyname(name, &value, &size, nullptr, 0) != 0) {
			DEBUG("CPU topology sysctl query failed:", name, "errno:", errno);
			return 0;
		}
		if (size != sizeof(value)) {
			DEBUG("CPU topology sysctl returned an unexpected size:", name, "size:", size);
			return 0;
		}
		DEBUG("CPU topology sysctl:", name, "value:", value);
		return value;
	}
#endif

} // namespace

namespace Lemon {

	CpuTopology detail::detectLinuxCpuTopology(const QString &cpuRoot, const QString &eventSourceRoot,
	                                           bool useCpuCapacityClasses) {
		const QDir directory(cpuRoot);
		QMap<int, LinuxCore> cores;
		bool completeCapacities = useCpuCapacityClasses;
		for (const QString &name : directory.entryList({"cpu[0-9]*"}, QDir::Dirs | QDir::NoDotAndDotDot)) {
			const int processor = name.mid(3).toInt();
			const QString processorRoot = directory.filePath(name);
			const QByteArray online = readSysfsFile(processorRoot + "/online");
			if (! online.isEmpty() && online != "1")
				continue;

			auto siblings = readCpuList(processorRoot + "/topology/core_cpus_list");
			if (! siblings || siblings->isEmpty())
				siblings = readCpuList(processorRoot + "/topology/thread_siblings_list");
			if (! siblings || ! siblings->contains(processor)) {
				DEBUG("Linux CPU topology unavailable; missing or inconsistent sibling list:", processorRoot);
				return {};
			}
			const int coreId = *std::min_element(siblings->cbegin(), siblings->cend());
			auto &core = cores[coreId];
			core.logicalProcessors.insert(processor);
			if (useCpuCapacityClasses) {
				bool valid = false;
				const int capacity = readSysfsFile(processorRoot + "/cpu_capacity").toInt(&valid);
				if (! valid || capacity <= 0 || (core.capacity > 0 && core.capacity != capacity))
					completeCapacities = false;
				core.capacity = capacity;
			}
		}

		CpuTopology topology;
		topology.physicalCores = static_cast<int>(cores.size());
		const QDir eventSources(eventSourceRoot);
		const auto performanceProcessors = readCpuList(eventSources.filePath("cpu_core/cpus"));
		if (performanceProcessors) {
			for (const auto &core : cores) {
				if (core.logicalProcessors.intersects(*performanceProcessors))
					++topology.performanceCores;
			}
			DEBUG("Linux CPU topology; classification: cpu_core/cpus; physical cores:",
			      topology.physicalCores, "performance cores:", topology.performanceCores);
			return topology;
		}
		// On ARM, the scheduler's maximum capacity identifies the highest performance class.
		// Do not infer x86 core types from per-core frequency or preferred-core differences.
		if (completeCapacities) {
			int highestCapacity = 0;
			for (const auto &core : cores) {
				if (core.capacity > highestCapacity) {
					highestCapacity = core.capacity;
					topology.performanceCores = 1;
				} else if (core.capacity == highestCapacity) {
					++topology.performanceCores;
				}
			}
			DEBUG("Linux CPU topology; classification: cpu_capacity; physical cores:", topology.physicalCores,
			      "highest capacity:", highestCapacity, "cores in highest class:", topology.performanceCores);
		} else {
			DEBUG("Linux CPU topology; physical cores:", topology.physicalCores,
			      "core classification unavailable; capacity classification enabled:", useCpuCapacityClasses);
		}
		return topology;
	}

	CpuTopology detectCpuTopology() {
#ifdef Q_OS_WIN
		return detectWindowsCpuTopology();
#elif defined(Q_OS_MACOS)
		CpuTopology topology;
		topology.physicalCores = readSysctlInt("hw.physicalcpu");
		if (readSysctlInt("hw.nperflevels") > 1)
			topology.performanceCores = readSysctlInt("hw.perflevel0.physicalcpu");
		return topology;
#elif defined(Q_OS_LINUX)
#ifdef Q_PROCESSOR_ARM
		constexpr bool useCpuCapacityClasses = true;
#else
		constexpr bool useCpuCapacityClasses = false;
#endif
		return detail::detectLinuxCpuTopology("/sys/devices/system/cpu", "/sys/bus/event_source/devices",
		                                      useCpuCapacityClasses);
#else
		DEBUG("CPU topology detection is unavailable on this platform");
		return {};
#endif
	}

	int getDefaultJudgingThreadCount(const CpuTopology &topology, int fallbackProcessorCount) {
		if (topology.performanceCores > 0 && topology.performanceCores < topology.physicalCores) {
			DEBUG("Default judging threads:", topology.performanceCores,
			      "policy: highest performance class on a heterogeneous CPU; physical cores:",
			      topology.physicalCores);
			return topology.performanceCores;
		}
		const int cores = topology.physicalCores > 0 ? topology.physicalCores : fallbackProcessorCount;
		const int threads = qMax(1, cores * 3 / 4);
		DEBUG("Default judging threads:", threads, "policy: three quarters; processor count:", cores,
		      "count source:", topology.physicalCores > 0 ? "physical cores" : "logical processor fallback");
		return threads;
	}

	int getDefaultJudgingThreadCount() {
		return getDefaultJudgingThreadCount(detectCpuTopology(), QThread::idealThreadCount());
	}

} // namespace Lemon
