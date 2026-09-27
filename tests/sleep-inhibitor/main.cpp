/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "base/LemonLog.hpp"
#include "core/sleepinhibitor.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <QtTest>
#include <windows.h>

class SleepInhibitorTest : public QObject {
	Q_OBJECT

  private:
	static DWORD handleCount() {
		DWORD count = 0;
		if (! GetProcessHandleCount(GetCurrentProcess(), &count))
			return 0;
		return count;
	}

  private slots:
	void initTestCase() {
		Lemon::base::logger = spdlog::stdout_color_mt("sleep-inhibitor-test");
		SleepInhibitor warmup(QStringLiteral("LemonLime sleep inhibitor test"));
	}

	void nestedRequests() {
		const DWORD before = handleCount();
		QVERIFY(before > 0);
		{
			SleepInhibitor first(QStringLiteral("LemonLime first judging session"));
			QCOMPARE(handleCount(), before + 1);
			{
				SleepInhibitor second(QStringLiteral("LemonLime second judging session"));
				QCOMPARE(handleCount(), before + 2);
			}
			QCOMPARE(handleCount(), before + 1);
		}
		QCOMPARE(handleCount(), before);
	}

	void repeatedRequests() {
		const DWORD before = handleCount();
		QVERIFY(before > 0);
		for (int i = 0; i < 100; ++i) {
			SleepInhibitor inhibitor(QStringLiteral("LemonLime repeated judging session"));
			QCOMPARE(handleCount(), before + 1);
		}
		QCOMPARE(handleCount(), before);
	}
};

QTEST_GUILESS_MAIN(SleepInhibitorTest)

#include "main.moc"
