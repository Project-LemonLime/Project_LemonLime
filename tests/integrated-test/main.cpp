/*
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "base/LemonLog.hpp"
#include "base/compiler.h"
#include "base/settings.h"
#include "core/contest.h"
#include "core/contestant.h"
#include "core/task.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtTest>

class IntegratedTest : public QObject {
	Q_OBJECT

  private:
	QJsonObject contestJson;

  private slots:
	void initTestCase() {
		Lemon::base::logger = spdlog::stdout_color_mt("integrated-test");

		QFile file(QString(TEST_DATA_DIR) + "/TestContest1.cdf");
		QVERIFY2(file.open(QFile::ReadOnly), qPrintable(file.errorString()));
		QJsonParseError error;
		const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
		QCOMPARE(error.error, QJsonParseError::NoError);
		QVERIFY(document.isObject());
		contestJson = document.object();
	}

	void cdfRoundTrip() {
		Settings settings;
		Contest original;
		original.setSettings(&settings);
		QCOMPARE(original.readFromJson(contestJson), 0);

		QJsonObject saved;
		original.writeToJson(saved);
		const QJsonDocument document = QJsonDocument::fromJson(QJsonDocument(saved).toJson());
		Contest restored;
		restored.setSettings(&settings);
		QCOMPARE(restored.readFromJson(document.object()), 0);

		// Fixed expectations also catch a reader and writer that both omit the same field.
		for (const Contest *contest : {&original, &restored}) {
			QCOMPARE(contest->getContestTitle(), QString("TestContest1"));
			QStringList taskNames;
			QList<qsizetype> testCaseCounts;
			for (const Task *task : contest->getTaskList()) {
				taskNames.append(task->getProblemTitle());
				testCaseCounts.append(task->getTestCaseList().size());
			}
			QCOMPARE(taskNames, QStringList({"aplusb", "helloworld"}));
			QCOMPARE(testCaseCounts, QList<qsizetype>({1, 6}));
			QStringList contestantNames;
			for (const Contestant *contestant : contest->getContestantList())
				contestantNames.append(contestant->getContestantName());
			QCOMPARE(contestantNames, QStringList({"user1", "user2"}));
		}
	}

	void compilerConfiguration_data() {
		QTest::addColumn<QString>("savedConfiguration");
		QTest::addColumn<QString>("expectedConfiguration");

		QTest::newRow("missing-compiler") << QString() << QString("default");
		QTest::newRow("missing-configuration") << QString("remote-only") << QString("default");
		QTest::newRow("empty-configuration") << QString("") << QString("default");
		QTest::newRow("custom-configuration") << QString("custom") << QString("custom");
		QTest::newRow("disabled-compiler") << QString("disable") << QString("disable");
	}

	void compilerConfiguration() {
		QFETCH(QString, savedConfiguration);
		QFETCH(QString, expectedConfiguration);

		QJsonObject configuration{{"obsolete-compiler", "default"}};
		if (! savedConfiguration.isNull())
			configuration.insert("g++", savedConfiguration);
		QJsonObject input = contestJson;
		QJsonArray tasks = input["tasks"].toArray();
		for (qsizetype i = 0; i < tasks.size(); ++i) {
			QJsonObject task = tasks[i].toObject();
			task.insert("compilerConfiguration", configuration);
			tasks[i] = task;
		}
		input["tasks"] = tasks;

		Compiler compiler;
		compiler.setCompilerName("g++");
		compiler.addConfiguration("default", "%s.* -o %s", "");
		compiler.addConfiguration("custom", "%s.* -o %s -O2", "");
		Settings settings;
		settings.addCompiler(&compiler);
		Contest contest;
		contest.setSettings(&settings);
		QCOMPARE(contest.readFromJson(input), 0);
		QCOMPARE(contest.getTaskList().size(), tasks.size());
		for (const Task *task : contest.getTaskList()) {
			QCOMPARE(task->getCompilerConfiguration("g++"), expectedConfiguration);
			QVERIFY(task->getCompilerConfiguration("obsolete-compiler").isEmpty());
		}
	}

	void judgeAll() {
		const QString cppPath = QStandardPaths::findExecutable("g++");
		QVERIFY2(! cppPath.isEmpty(), "g++ must be available on PATH");
		QString pythonPath = QStandardPaths::findExecutable("python3");
		if (pythonPath.isEmpty())
			pythonPath = QStandardPaths::findExecutable("python");
		QVERIFY2(! pythonPath.isEmpty(), "python3 or python must be available on PATH");

		Compiler cpp;
		cpp.setCompilerName("g++");
		cpp.setSourceExtensions("cpp;cc;cxx");
		cpp.setCompilerLocation(cppPath);
		cpp.addConfiguration("default", "%s.* -o %s -O2 -lm", "");
		Compiler python;
		python.setCompilerType(Compiler::InterpretiveWithoutByteCode);
		python.setCompilerName("python");
		python.setSourceExtensions("py");
		python.setInterpreterLocation(pythonPath);
		python.setTimeLimitRatio(2.0);
		python.addConfiguration("default", "", "%s.py");

		// Keep the fixture independent of the user's saved application settings.
		Settings settings;
		settings.setCompileTimeLimit(20000);
		settings.setSpecialJudgeTimeLimit(10000);
		settings.setFileSizeLimit(50);
		settings.setRejudgeTimes(1);
		settings.setMaxJudgingThreads(1);
		settings.setDefaultExtraTimeRatio(0.2);
		settings.setPreventSleepWhileJudging(false);
		settings.addCompiler(&cpp);
		settings.addCompiler(&python);

		QTemporaryDir workingDirectory;
		QVERIFY(workingDirectory.isValid());
		const QDir fixtureDirectory(TEST_DATA_DIR);
		QDirIterator files(fixtureDirectory.absolutePath(), QDir::Files, QDirIterator::Subdirectories);
		while (files.hasNext()) {
			const QString source = files.next();
			const QString destination = workingDirectory.filePath(fixtureDirectory.relativeFilePath(source));
			QVERIFY2(QDir().mkpath(QFileInfo(destination).absolutePath()), qPrintable(destination));
			QVERIFY2(QFile::copy(source, destination), qPrintable(source));
		}
		const auto restoreDirectory =
		    qScopeGuard([previousDirectory = QDir::currentPath()] { QDir::setCurrent(previousDirectory); });
		QVERIFY(QDir::setCurrent(workingDirectory.path()));

		Contest contest;
		contest.setSettings(&settings);
		QCOMPARE(contest.readFromJson(contestJson), 0);
		contest.refreshContestantList();
		QCOMPARE(contest.getTaskList().size(), 2);
		QCOMPARE(contest.getTask(0)->getProblemTitle(), QString("aplusb"));
		QCOMPARE(contest.getTask(1)->getProblemTitle(), QString("helloworld"));
		QCOMPARE(contest.getContestantList().size(), 2);
		contest.judgeAll();

		struct Expected {
			const char *contestant;
			int task;
			CompileState compilation;
			QList<ResultState> verdicts;
		};
		const QList<Expected> expected = {
		    {"user1", 0, CompileSuccessfully, {CorrectAnswer}},
		    {"user2", 0, NoValidSourceFile, {}},
		    {"user1",
		     1,
		     CompileSuccessfully,
		     {CorrectAnswer, TimeLimitExceeded, MemoryLimitExceeded, WrongAnswer, RunTimeError, WrongAnswer}},
		    {"user2",
		     1,
		     CompileSuccessfully,
		     {CorrectAnswer, CorrectAnswer, CorrectAnswer, CorrectAnswer, CorrectAnswer, CorrectAnswer}},
		};
		for (const Expected &row : expected) {
			const Contestant *contestant = contest.getContestant(row.contestant);
			QVERIFY2(contestant != nullptr, row.contestant);
			QVERIFY2(contestant->getCheckJudged(row.task), row.contestant);
			QCOMPARE(contestant->getCompileState(row.task), row.compilation);
			const auto &results = contestant->getResult(row.task);
			QCOMPARE(results.size(), row.verdicts.size());
			for (qsizetype i = 0; i < results.size(); ++i) {
				QCOMPARE(results[i].size(), 1);
				const ResultState actual = results[i][0];
				const ResultState wanted = row.verdicts[i];
				// Allocation failures may be reported as MLE or RE depending on the platform.
				const bool allocationFailure = wanted == MemoryLimitExceeded && actual == RunTimeError;
				QVERIFY2(actual == wanted || allocationFailure,
				         qPrintable(QString("%1 task %2 case %3: expected %4, got %5")
				                        .arg(row.contestant)
				                        .arg(row.task)
				                        .arg(i)
				                        .arg(wanted)
				                        .arg(actual)));
			}
		}
	}
};

QTEST_GUILESS_MAIN(IntegratedTest)
#include "main.moc"
