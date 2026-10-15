/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "base/LemonLog.hpp"
#include "base/cputopology.h"
#include "base/cputopology_p.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QtTest>

namespace {

	class LinuxSysfsFixture {
	  public:
		bool isValid() const { return directory.isValid(); }

		bool writeFile(const QString &relativeFileName, const QByteArray &contents) {
			const QString fileName = directory.filePath(relativeFileName);
			if (! QDir().mkpath(QFileInfo(fileName).absolutePath()))
				return false;
			QFile file(fileName);
			return file.open(QIODevice::WriteOnly) && file.write(contents) == contents.size();
		}

		bool addCpu(int cpu, const QByteArray &siblings, int online = 1,
		            const QString &attribute = QStringLiteral("thread_siblings_list")) {
			const QString prefix = QStringLiteral("cpu/cpu%1/").arg(cpu);
			if (! writeFile(prefix + QStringLiteral("topology/") + attribute, siblings))
				return false;
			return online < 0 ||
			       writeFile(prefix + QStringLiteral("online"), QByteArray::number(online) + '\n');
		}

		bool setCapacity(int cpu, int capacity) {
			return writeFile(QStringLiteral("cpu/cpu%1/cpu_capacity").arg(cpu),
			                 QByteArray::number(capacity) + '\n');
		}

		Lemon::CpuTopology getTopology(bool useCpuCapacityClasses = false) const {
			return Lemon::detail::detectLinuxCpuTopology(directory.filePath(QStringLiteral("cpu")),
			                                             directory.filePath(QStringLiteral("events")),
			                                             useCpuCapacityClasses);
		}

	  private:
		QTemporaryDir directory;
	};

} // namespace

class CpuTopologyTest : public QObject {
	Q_OBJECT

  private slots:
	void initTestCase() {
		Lemon::base::logger = spdlog::stdout_color_mt("cpu-topology-test");
		Lemon::base::logger->set_level(spdlog::level::debug);
	}

	void defaultThreadCount_data() {
		QTest::addColumn<int>("physicalCores");
		QTest::addColumn<int>("performanceCores");
		QTest::addColumn<int>("fallbackProcessorCount");
		QTest::addColumn<int>("expected");

		QTest::newRow("hybrid-eight-p-four-e") << 12 << 8 << 1 << 8;
		QTest::newRow("hybrid-two-p-six-e") << 8 << 2 << 1 << 2;
		QTest::newRow("n100-four-e") << 4 << 0 << 1 << 3;
		QTest::newRow("eight-identical-p") << 8 << 8 << 1 << 6;
		QTest::newRow("eight-unclassified-cores") << 8 << 0 << 1 << 6;
		QTest::newRow("round-down-six-cores") << 6 << 0 << 1 << 4;
		QTest::newRow("round-down-three-cores") << 3 << 0 << 1 << 2;
		QTest::newRow("single-core") << 1 << 0 << 1 << 1;
		QTest::newRow("unknown-topology") << 0 << 0 << 8 << 6;
		QTest::newRow("invalid-topology") << -1 << 0 << 3 << 2;
		QTest::newRow("zero-fallback") << 0 << 0 << 0 << 1;
		QTest::newRow("negative-fallback") << 0 << 0 << -1 << 1;
		QTest::newRow("inconsistent-p-count") << 4 << 5 << 1 << 3;
	}

	void defaultThreadCount() {
		QFETCH(int, physicalCores);
		QFETCH(int, performanceCores);
		QFETCH(int, fallbackProcessorCount);
		QFETCH(int, expected);

		const Lemon::CpuTopology topology{physicalCores, performanceCores};
		QCOMPARE(Lemon::getDefaultJudgingThreadCount(topology, fallbackProcessorCount), expected);
	}

#ifdef Q_OS_WIN
	void nativeWindowsTopology() {
		const auto topology = Lemon::detectCpuTopology();
		QVERIFY(topology.physicalCores > 0);
		QVERIFY(topology.performanceCores > 0);
		QVERIFY(topology.performanceCores <= topology.physicalCores);
		qInfo() << "Physical cores:" << topology.physicalCores
		        << "highest performance class cores:" << topology.performanceCores
		        << "default judging threads:" << Lemon::getDefaultJudgingThreadCount();
	}
#endif

	void linuxHybridSmt_data() {
		QTest::addColumn<QString>("attribute");
		QTest::newRow("thread-siblings") << QStringLiteral("thread_siblings_list");
		QTest::newRow("core-cpus") << QStringLiteral("core_cpus_list");
	}

	void linuxHybridSmt() {
		QFETCH(QString, attribute);
		LinuxSysfsFixture sysfs;
		QVERIFY(sysfs.isValid());
		QVERIFY(sysfs.addCpu(0, "0,4\n", -1, attribute));
		QVERIFY(sysfs.addCpu(4, "0,4\n", 1, attribute));
		QVERIFY(sysfs.addCpu(1, "1,5\n", 1, attribute));
		QVERIFY(sysfs.addCpu(5, "1,5\n", 0, attribute));
		for (int cpu : {2, 3, 6})
			QVERIFY(sysfs.addCpu(cpu, QByteArray::number(cpu) + '\n', 1, attribute));
		QVERIFY(sysfs.addCpu(7, "7\n", 0, attribute));
		QVERIFY(sysfs.writeFile(QStringLiteral("events/cpu_core/cpus"), "0-1,4-5\n"));
		QVERIFY(sysfs.writeFile(QStringLiteral("events/cpu_atom/cpus"), "2-3,7\n"));
		QVERIFY(sysfs.writeFile(QStringLiteral("events/cpu_lowpower/cpus"), "6\n"));

		const auto topology = sysfs.getTopology();
		QCOMPARE(topology.physicalCores, 5);
		QCOMPARE(topology.performanceCores, 2);
		QCOMPARE(Lemon::getDefaultJudgingThreadCount(topology), 2);
	}

	void linuxHomogeneousSmt() {
		LinuxSysfsFixture sysfs;
		QVERIFY(sysfs.isValid());
		for (int core : {0, 1, 2, 3}) {
			const QByteArray siblings = QByteArray::number(core) + ',' + QByteArray::number(core + 4) + '\n';
			QVERIFY(sysfs.addCpu(core, siblings));
			QVERIFY(sysfs.addCpu(core + 4, siblings));
		}

		const auto topology = sysfs.getTopology();
		QCOMPARE(topology.physicalCores, 4);
		QCOMPARE(Lemon::getDefaultJudgingThreadCount(topology), 3);
	}

	void linuxEfficiencyOnly() {
		LinuxSysfsFixture sysfs;
		QVERIFY(sysfs.isValid());
		for (int cpu : {0, 1, 2, 3})
			QVERIFY(sysfs.addCpu(cpu, QByteArray::number(cpu) + '\n'));
		QVERIFY(sysfs.writeFile(QStringLiteral("events/cpu_atom/cpus"), "0-3\n"));

		const auto topology = sysfs.getTopology();
		QCOMPARE(topology.physicalCores, 4);
		QCOMPARE(topology.performanceCores, 0);
		QCOMPARE(Lemon::getDefaultJudgingThreadCount(topology), 3);
	}

	void linuxCapacityClasses_data() {
		QTest::addColumn<QList<int>>("capacities");
		QTest::addColumn<bool>("useCpuCapacityClasses");
		QTest::addColumn<int>("expectedThreads");

		QTest::newRow("arm-hybrid") << QList<int>{1024, 1024, 512, 512} << true << 2;
		QTest::newRow("arm-three-classes") << QList<int>{1024, 768, 512, 512} << true << 1;
		QTest::newRow("arm-homogeneous") << QList<int>{1024, 1024, 1024, 1024} << true << 3;
		QTest::newRow("arm-missing-capacities") << QList<int>{-1, -1, -1, -1} << true << 3;
		QTest::newRow("arm-partial-capacities") << QList<int>{-1, 512, 1024, 512} << true << 3;
		QTest::newRow("x86-frequency-differences") << QList<int>{1024, 768, 512, 512} << false << 3;
	}

	void linuxCapacityClasses() {
		QFETCH(QList<int>, capacities);
		QFETCH(bool, useCpuCapacityClasses);
		QFETCH(int, expectedThreads);
		LinuxSysfsFixture sysfs;
		QVERIFY(sysfs.isValid());
		int cpu = 0;
		for (int capacity : capacities) {
			QVERIFY(sysfs.addCpu(cpu, QByteArray::number(cpu) + '\n'));
			if (capacity > 0)
				QVERIFY(sysfs.setCapacity(cpu, capacity));
			++cpu;
		}

		const auto topology = sysfs.getTopology(useCpuCapacityClasses);
		QCOMPARE(topology.physicalCores, 4);
		QCOMPARE(Lemon::getDefaultJudgingThreadCount(topology), expectedThreads);
	}

	void linuxUnavailableTopology() {
		LinuxSysfsFixture sysfs;
		QVERIFY(sysfs.isValid());
		QCOMPARE(sysfs.getTopology().physicalCores, 0);
		QVERIFY(sysfs.addCpu(0, "0\n", 0));
		QVERIFY(sysfs.addCpu(1, "1\n", 0));
		const auto topology = sysfs.getTopology();
		QCOMPARE(topology.physicalCores, 0);
		QCOMPARE(topology.performanceCores, 0);
	}

	void linuxMissingSiblings() {
		LinuxSysfsFixture sysfs;
		QVERIFY(sysfs.isValid());
		QVERIFY(sysfs.addCpu(0, "0\n"));
		QVERIFY(sysfs.writeFile(QStringLiteral("cpu/cpu1/online"), "1\n"));

		const auto topology = sysfs.getTopology();
		QCOMPARE(topology.physicalCores, 0);
		QCOMPARE(topology.performanceCores, 0);
	}
};

QTEST_GUILESS_MAIN(CpuTopologyTest)

#include "main.moc"
