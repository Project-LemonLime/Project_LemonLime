/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

// Standalone diagnostic equivalent of cputopology.cpp's Linux implementation.
// Uses the standard library and Linux APIs so the executable can be linked statically.

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <map>
#include <optional>
#include <sched.h>
#include <set>
#include <string>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <vector>

namespace {

	struct FileValue {
		bool readable = false;
		std::string text;
		std::string error;
	};

	std::string trim(const std::string &text) {
		const auto first = text.find_first_not_of(" \t\r\n\f\v");
		if (first == std::string::npos)
			return {};
		return text.substr(first, text.find_last_not_of(" \t\r\n\f\v") - first + 1);
	}

	FileValue readFile(const std::string &name) {
		FILE *file = std::fopen(name.c_str(), "r");
		if (! file)
			return {false, {}, std::strerror(errno)};
		FileValue result{true, {}, {}};
		char buffer[4096];
		while (const size_t count = std::fread(buffer, 1, sizeof(buffer), file))
			result.text.append(buffer, count);
		if (std::ferror(file)) {
			result.readable = false;
			result.error = std::strerror(errno);
		}
		std::fclose(file);
		result.text = trim(result.text);
		return result;
	}

	std::string describe(const FileValue &value) {
		if (! value.readable)
			return "<unavailable: " + value.error + ">";
		return value.text.empty() ? "<empty>" : value.text;
	}

	std::vector<std::string> split(const std::string &text, char delimiter) {
		std::vector<std::string> parts;
		size_t first = 0;
		for (;;) {
			const size_t last = text.find(delimiter, first);
			parts.push_back(text.substr(first, last - first));
			if (last == std::string::npos)
				return parts;
			first = last + 1;
		}
	}

	std::optional<int> parseInt(const std::string &value) {
		const std::string text = trim(value);
		const char *first = text.data();
		const char *last = first + text.size();
		if (first != last && *first == '+')
			++first;
		int number = 0;
		const auto result = std::from_chars(first, last, number);
		if (result.ec != std::errc() || result.ptr != last)
			return std::nullopt;
		return number;
	}

	std::optional<std::set<int>> parseCpuList(const FileValue &value) {
		if (! value.readable)
			return std::nullopt;
		std::set<int> processors;
		if (value.text.empty())
			return processors;
		for (const auto &part : split(value.text, ',')) {
			const auto bounds = split(trim(part), '-');
			const int first = std::atoi(bounds.front().c_str());
			const int last = std::atoi(bounds.back().c_str());
			for (int processor = first; processor <= last; ++processor)
				processors.insert(processor);
		}
		return processors;
	}

	std::string cpuList(const std::set<int> &processors) {
		std::string text;
		for (int processor : processors) {
			if (! text.empty())
				text += ',';
			text += std::to_string(processor);
		}
		return text.empty() ? "<empty>" : text;
	}

	std::string cpuModel() {
		for (const auto &line : split(readFile("/proc/cpuinfo").text, '\n')) {
			const size_t colon = line.find(':');
			if (colon == std::string::npos)
				continue;
			const std::string key = trim(line.substr(0, colon));
			if (key == "model name" || key == "Hardware")
				return trim(line.substr(colon + 1));
		}
		return "<unavailable>";
	}

	struct Core {
		std::set<int> logicalProcessors;
		int capacity = 0;
	};

	struct Topology {
		int physicalCores = 0;
		int performanceCores = 0;
	};

	Topology detectLinuxCpuTopology(const std::string &sysfsRoot, bool useCpuCapacityClasses) {
		const std::string cpuRoot = sysfsRoot + "/devices/system/cpu";
		const std::string eventRoot = sysfsRoot + "/bus/event_source/devices";
		std::printf("\n[sysfs]\nonline=%s\npresent=%s\n", describe(readFile(cpuRoot + "/online")).c_str(),
		            describe(readFile(cpuRoot + "/present")).c_str());

		std::vector<std::string> names;
		DIR *directory = opendir(cpuRoot.c_str());
		if (! directory) {
			std::printf("topology_error=cannot open %s: %s\n", cpuRoot.c_str(), std::strerror(errno));
			return {};
		}
		while (const auto *entry = readdir(directory)) {
			const std::string name = entry->d_name;
			if (name.size() > 3 && name.compare(0, 3, "cpu") == 0 &&
			    std::isdigit(static_cast<unsigned char>(name[3]))) {
				struct stat status{};
				if (stat((cpuRoot + '/' + name).c_str(), &status) == 0 && S_ISDIR(status.st_mode))
					names.push_back(name);
			}
		}
		closedir(directory);
		std::sort(names.begin(), names.end());

		std::map<int, Core> cores;
		bool validTopology = true;
		bool completeCapacities = useCpuCapacityClasses;
		std::printf("\n[logical_cpus]\n");
		for (const auto &name : names) {
			const int processor = parseInt(name.substr(3)).value_or(0);
			const std::string root = cpuRoot + '/' + name;
			const auto online = readFile(root + "/online");
			const auto modernSiblings = readFile(root + "/topology/core_cpus_list");
			const auto legacySiblings = readFile(root + "/topology/thread_siblings_list");
			const auto capacityFile = readFile(root + "/cpu_capacity");
			const bool active = online.text.empty() || online.text == "1";
			auto siblings = parseCpuList(modernSiblings);
			const char *selected = "core_cpus_list";
			if (! siblings || siblings->empty()) {
				siblings = parseCpuList(legacySiblings);
				selected = "thread_siblings_list";
			}
			std::printf("cpu=%d active=%d online=%s package=%s core=%s core_cpus=%s "
			            "thread_siblings=%s selected=%s capacity=%s\n",
			            processor, active, describe(online).c_str(),
			            describe(readFile(root + "/topology/physical_package_id")).c_str(),
			            describe(readFile(root + "/topology/core_id")).c_str(),
			            describe(modernSiblings).c_str(), describe(legacySiblings).c_str(), selected,
			            describe(capacityFile).c_str());
			if (! active)
				continue;
			if (! siblings || ! siblings->count(processor)) {
				std::printf("topology_error=cpu%d has no valid same-core CPU list\n", processor);
				validTopology = false;
				continue;
			}
			auto &core = cores[*siblings->begin()];
			core.logicalProcessors.insert(processor);
			if (useCpuCapacityClasses) {
				const int capacity = parseInt(capacityFile.text).value_or(0);
				if (capacity <= 0 || (core.capacity > 0 && core.capacity != capacity))
					completeCapacities = false;
				core.capacity = capacity;
			}
		}

		std::printf("\n[physical_cores]\n");
		for (const auto &[id, core] : cores)
			std::printf("key=%d online_cpus=%s capacity=%d\n", id, cpuList(core.logicalProcessors).c_str(),
			            core.capacity);

		std::printf("\n[pmu]\n");
		const auto performanceFile = readFile(eventRoot + "/cpu_core/cpus");
		const auto atomFile = readFile(eventRoot + "/cpu_atom/cpus");
		const auto lowPowerFile = readFile(eventRoot + "/cpu_lowpower/cpus");
		const auto performanceProcessors = parseCpuList(performanceFile);
		const auto atomProcessors = parseCpuList(atomFile);
		const auto lowPowerProcessors = parseCpuList(lowPowerFile);
		std::printf("cpu_core.cpus=%s parsed=%d\ncpu_atom.cpus=%s parsed=%d\n"
		            "cpu_lowpower.cpus=%s parsed=%d\n",
		            describe(performanceFile).c_str(), bool(performanceProcessors),
		            describe(atomFile).c_str(), bool(atomProcessors), describe(lowPowerFile).c_str(),
		            bool(lowPowerProcessors));

		if (! validTopology) {
			std::printf("classification_source=unavailable_topology\n");
			return {};
		}
		Topology topology{static_cast<int>(cores.size()), 0};
		if (performanceProcessors) {
			for (const auto &[id, core] : cores) {
				if (std::any_of(core.logicalProcessors.begin(), core.logicalProcessors.end(),
				                [&](int cpu) { return performanceProcessors->count(cpu); }))
					++topology.performanceCores;
			}
			std::printf("classification_source=cpu_core_pmu\n");
		} else if (completeCapacities) {
			int highestCapacity = 0;
			for (const auto &[id, core] : cores) {
				if (core.capacity > highestCapacity) {
					highestCapacity = core.capacity;
					topology.performanceCores = 1;
				} else if (core.capacity == highestCapacity) {
					++topology.performanceCores;
				}
			}
			std::printf("classification_source=cpu_capacity\n");
		} else {
			std::printf("classification_source=unclassified\n");
		}
		return topology;
	}

} // namespace

int main(int argc, char **argv) {
	std::string sysfsRoot = "/sys";
	if (argc == 3 && std::strcmp(argv[1], "--sysfs-root") == 0) {
		sysfsRoot = argv[2];
	} else if (argc != 1) {
		std::printf("Usage: %s [--sysfs-root DIRECTORY]\n", argv[0]);
		return argc == 2 && std::strcmp(argv[1], "--help") == 0 ? 0 : 2;
	}

	utsname system{};
	uname(&system);
#if defined(__arm__) || defined(__aarch64__)
	constexpr bool useCpuCapacityClasses = true;
#else
	constexpr bool useCpuCapacityClasses = false;
#endif
	cpu_set_t affinity;
	int fallbackProcessors = 0;
	const char *fallbackSource = "sched_getaffinity";
	if (sched_getaffinity(0, sizeof(affinity), &affinity) == 0) {
		fallbackProcessors = CPU_COUNT(&affinity);
	} else {
		fallbackSource = "sysconf(_SC_NPROCESSORS_ONLN)";
		fallbackProcessors = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));
	}

	std::printf("LemonLime Linux CPU probe 1\ncompiler=GCC %s\narchitecture=%s\nkernel=%s\n"
	            "cpu_model=%s\nsysfs_root=%s\nuse_cpu_capacity_classes=%d\n"
	            "fallback_logical_cpus=%d\nfallback_source=%s\n",
	            __VERSION__, system.machine, system.release, cpuModel().c_str(), sysfsRoot.c_str(),
	            useCpuCapacityClasses, fallbackProcessors, fallbackSource);
	const Topology topology = detectLinuxCpuTopology(sysfsRoot, useCpuCapacityClasses);
	const bool hybrid = topology.performanceCores > 0 && topology.performanceCores < topology.physicalCores;
	const int cores = topology.physicalCores > 0 ? topology.physicalCores : fallbackProcessors;
	const int threads = hybrid ? topology.performanceCores : std::max(1, cores * 3 / 4);
	std::printf("\n[result]\nphysical_cores=%d\nperformance_cores=%d\nhybrid=%d\n"
	            "default_judging_threads=%d\n",
	            topology.physicalCores, topology.performanceCores, hybrid, threads);
	return 0;
}
