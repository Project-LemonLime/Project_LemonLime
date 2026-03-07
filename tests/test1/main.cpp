/*
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "base/compiler.h"
#include "base/settings.h"
#include "core/contest.h"
#include "core/contestant.h"
#include "core/task.h"
#include "core/testcase.h"

#include "base/LemonLog.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QTemporaryDir>
#include <QTextStream>
#include <QtTest>

// ---------------------------------------------------------------------------
// Settings factory: creates a Settings with g++ and python configured
// for use in judging tests.  Fails loudly if either is not found.
// ---------------------------------------------------------------------------
static Settings *createSettings(QObject *parent = nullptr) {
	auto *s = new Settings();
	s->setDefaultFullScore(100);
	s->setDefaultTimeLimit(1000);
	s->setDefaultMemoryLimit(512);
	s->setCompileTimeLimit(10000);
	s->setSpecialJudgeTimeLimit(10000);
	s->setFileSizeLimit(50);
	s->setRejudgeTimes(1);
	s->setMaxJudgingThreads(1);
	s->setDefaultInputFileExtension("in");
	s->setDefaultOutputFileExtension("out");
	s->setInputFileExtensions("in");
	s->setOutputFileExtensions("out;ans");
	// Don't call loadSettings() — it reads from system QSettings which is not
	// set up in test environments and would clobber our intentional configuration.

	// --- g++ (Typical compiler) ---
	QString gppPath = QStandardPaths::findExecutable("g++");
	if (gppPath.isEmpty())
		qFatal("g++ not found on PATH – cannot run judging tests.");

	auto *gpp = new Compiler();
	gpp->setCompilerType(Compiler::Typical);
	gpp->setCompilerName("g++");
	gpp->setSourceExtensions("cpp;cc;cxx");
	gpp->setCompilerLocation(gppPath);
	gpp->setTimeLimitRatio(1.0);
	gpp->setMemoryLimitRatio(1.0);
	gpp->setDisableMemoryLimitCheck(true); // memory check needs watcher on Unix
	gpp->setInterpreterAsWatcher(false);
	gpp->addConfiguration("default", "%s.* -o %s -O2 -lm", "");
	s->addCompiler(gpp);

	// --- python (InterpretiveWithoutByteCode interpreter) ---
	// Try both "python3" and "python"
	QString pythonPath = QStandardPaths::findExecutable("python3");
	if (pythonPath.isEmpty())
		pythonPath = QStandardPaths::findExecutable("python");
	if (pythonPath.isEmpty())
		qFatal("python/python3 not found on PATH – cannot run judging tests.");

	auto *py = new Compiler();
	py->setCompilerType(Compiler::InterpretiveWithoutByteCode);
	py->setCompilerName("python");
	py->setSourceExtensions("py");
	py->setInterpreterLocation(pythonPath);
	py->setTimeLimitRatio(5.0); // give python more time
	py->setMemoryLimitRatio(1.0);
	py->setDisableMemoryLimitCheck(true);
	py->setInterpreterAsWatcher(false);
	py->addConfiguration("default", "", "%s.py");
	s->addCompiler(py);

	Q_UNUSED(parent)
	return s;
}

// ---------------------------------------------------------------------------
// Helper: load a Contest from the copied test data dir.
// Sets QDir::current to the contest root so that relative paths work.
// ---------------------------------------------------------------------------
static Contest *loadContest(const QString &contestDir, Settings *settings, QObject *parent = nullptr) {
	const QString cdfPath = contestDir + "/TestContest1.cdf";
	QFile file(cdfPath);
	if (! file.open(QFile::ReadOnly))
		return nullptr;

	QJsonParseError parseError;
	QJsonObject inObj = QJsonDocument::fromJson(file.readAll(), &parseError).object();
	if (parseError.error != QJsonParseError::NoError)
		return nullptr;

	QDir::setCurrent(contestDir);

	auto *contest = new Contest(parent);
	contest->setSettings(settings);
	if (contest->readFromJson(inObj) == -1) {
		delete contest;
		return nullptr;
	}
	return contest;
}

// ===========================================================================
// Test class
// ===========================================================================
class TestContest : public QObject {
	Q_OBJECT

  private:
	// Path to the copied TestContest1 directory (inside cmake build dir)
	QString m_contestDir;
	// A working copy dir for judging (we judge inside a temp copy)
	QTemporaryDir m_tempWorkDir;

  private slots:
	// Called once before all tests
	void initTestCase() {
		// Initialize the spdlog logger that all Lemon LOG/DEBUG macros use.
		// Without this, the inline shared_ptr `Lemon::base::logger` is null
		// and any Settings setter that calls DEBUG() will crash.
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::warn);
		Lemon::base::logger = std::make_shared<spdlog::logger>(spdlog::logger("test", {console_sink}));

		m_contestDir = QString(TEST_DATA_DIR) + "/TestContest1";
		QVERIFY2(QDir(m_contestDir).exists(),
		         qPrintable(QString("Test data dir not found: %1").arg(m_contestDir)));

		// Create a working copy of the contest data for judging tests
		// (judging writes compiled binaries, so we don't want to pollute the original)
		QVERIFY(m_tempWorkDir.isValid());
		const QString dst = m_tempWorkDir.path() + "/TestContest1";
		QDir().mkpath(dst);

		auto copyDir = [](const QString &src, const QString &dst_, auto &selfRef) -> void {
			QDir srcDir(src);
			QDir().mkpath(dst_);
			for (const QFileInfo &fi :
			     srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
				const QString dn = dst_ + "/" + fi.fileName();
				if (fi.isDir())
					selfRef(fi.absoluteFilePath(), dn, selfRef);
				else
					QFile::copy(fi.absoluteFilePath(), dn);
			}
		};
		copyDir(m_contestDir, dst, copyDir);
	}

	// ------------------------------------------------------------------
	// Test 1: read CDF file
	// ------------------------------------------------------------------
	void testReadCdf() {
		Settings settings;
		// Do not call loadSettings() in tests

		Contest *contest = loadContest(m_contestDir, &settings, this);
		QVERIFY2(contest != nullptr, "Failed to load TestContest1.cdf");

		// Verify contest title
		QCOMPARE(contest->getContestTitle(), QString("TestContest1"));

		// Verify task count and names
		const QList<Task *> &tasks = contest->getTaskList();
		QCOMPARE(tasks.size(), 2);

		QStringList taskNames;
		for (const Task *t : tasks)
			taskNames << t->getProblemTitle();
		QVERIFY2(taskNames.contains("aplusb"), "Task 'aplusb' not found");
		QVERIFY2(taskNames.contains("helloworld"), "Task 'helloworld' not found");

		// Verify contestant count and names
		const QList<Contestant *> contestants = contest->getContestantList();
		QCOMPARE(contestants.size(), 2);

		QStringList names;
		for (const Contestant *c : contestants)
			names << c->getContestantName();
		QVERIFY2(names.contains("user1"), "Contestant 'user1' not found");
		QVERIFY2(names.contains("user2"), "Contestant 'user2' not found");

		delete contest;
	}

	// ------------------------------------------------------------------
	// Test 2: get compiler path
	// ------------------------------------------------------------------
	void testGetCompilerPath() {
		Settings *settings = createSettings(this);

		const QList<Compiler *> &compilers = settings->getCompilerList();
		QVERIFY2(compilers.size() >= 2, "Expected at least 2 compilers (g++ and python)");

		bool foundGpp = false;
		bool foundPython = false;
		for (const Compiler *c : compilers) {
			if (c->getCompilerName() == "g++") {
				QVERIFY2(! c->getCompilerLocation().isEmpty(), "g++ location is empty");
				QVERIFY2(QFileInfo(c->getCompilerLocation()).exists(),
				         qPrintable("g++ not found at: " + c->getCompilerLocation()));
				foundGpp = true;
			}
			if (c->getCompilerName() == "python") {
				QVERIFY2(! c->getInterpreterLocation().isEmpty(), "python location is empty");
				QVERIFY2(QFileInfo(c->getInterpreterLocation()).exists(),
				         qPrintable("python not found at: " + c->getInterpreterLocation()));
				foundPython = true;
			}
		}
		QVERIFY2(foundGpp, "g++ compiler not found in settings");
		QVERIFY2(foundPython, "python compiler not found in settings");

		delete settings;
	}

	// ------------------------------------------------------------------
	// Test 3 & 4: judge all test cases and check expected output.
	// Combined into one slot to avoid running the judger twice on the
	// same working-copy directory (stale binaries cause error 87 on Windows).
	// ------------------------------------------------------------------
	void testJudgeAllAndCheckOutput() {
		Settings *settings = createSettings(this);

		const QString workingContestDir = m_tempWorkDir.path() + "/TestContest1";
		Contest *contest = loadContest(workingContestDir, settings, this);
		QVERIFY2(contest != nullptr, "Failed to load contest from working copy");

		contest->refreshContestantList();

		// judgeAll() blocks internally via QEventLoop
		contest->judgeAll();

		// ---- All contestants must have been judged ----
		const QList<Contestant *> contestants = contest->getContestantList();
		QCOMPARE(contestants.size(), 2);
		for (const Contestant *c : contestants) {
			for (int taskIdx = 0; taskIdx < contest->getTaskList().size(); taskIdx++) {
				QVERIFY2(c->getCheckJudged(taskIdx),
				         qPrintable(QString("Contestant '%1' task %2 was not judged")
				                        .arg(c->getContestantName())
				                        .arg(taskIdx)));
			}
		}

		// ---- Verify expected scores for helloworld ----
		const QList<Task *> &tasks = contest->getTaskList();
		int helloworldIdx = -1;
		for (int i = 0; i < tasks.size(); i++) {
			if (tasks[i]->getProblemTitle() == "helloworld")
				helloworldIdx = i;
		}
		QVERIFY2(helloworldIdx >= 0, "Task 'helloworld' not found");

		// user2: always outputs "AC". Case 1 (input=1, expected="AC") passes (17 pts).
		Contestant *user2 = contest->getContestant("user2");
		QVERIFY2(user2 != nullptr, "Contestant 'user2' not found");
		QCOMPARE(user2->getTaskScore(helloworldIdx), 100);

		// user1: case 1 -> "AC" (17 pts), cases 2-6 fail for various reasons (WA/TLE/MLE).
		// Expected score: 17.
		Contestant *user1 = contest->getContestant("user1");
		QVERIFY2(user1 != nullptr, "Contestant 'user1' not found");
		QCOMPARE(user1->getTaskScore(helloworldIdx), 17);

		delete contest;
		delete settings;
	}

	// ------------------------------------------------------------------
	// Test 5: write CDF file to a new path (round-trip serialization)
	// ------------------------------------------------------------------
	void testWriteCdf() {
		Settings settings;
		// Do not call loadSettings() in tests

		Contest *contest = loadContest(m_contestDir, &settings, this);
		QVERIFY2(contest != nullptr, "Failed to load TestContest1.cdf");

		// Serialize to a temp file
		QTemporaryDir outDir;
		QVERIFY(outDir.isValid());
		const QString outPath = outDir.path() + "/out.cdf";

		QFile outFile(outPath);
		QVERIFY2(outFile.open(QFile::WriteOnly), "Cannot open output CDF file for writing");

		QJsonObject outObj;
		contest->writeToJson(outObj);
		outFile.write(QJsonDocument(outObj).toJson(QJsonDocument::Compact));
		outFile.close();

		// Re-read the written CDF
		QFile inFile(outPath);
		QVERIFY2(inFile.open(QFile::ReadOnly), "Cannot open output CDF file for reading");
		QJsonParseError parseError;
		QJsonObject inObj = QJsonDocument::fromJson(inFile.readAll(), &parseError).object();
		QVERIFY2(parseError.error == QJsonParseError::NoError,
		         qPrintable("JSON parse error: " + parseError.errorString()));

		// Load into a fresh Contest and verify round-trip
		auto *contest2 = new Contest(this);
		contest2->setSettings(&settings);
		QVERIFY2(contest2->readFromJson(inObj) == 0, "Re-reading written CDF failed");

		QCOMPARE(contest2->getContestTitle(), contest->getContestTitle());
		QCOMPARE(contest2->getTaskList().size(), contest->getTaskList().size());
		QCOMPARE(contest2->getContestantList().size(), contest->getContestantList().size());

		for (int i = 0; i < contest->getTaskList().size(); i++) {
			QCOMPARE(contest2->getTask(i)->getProblemTitle(), contest->getTask(i)->getProblemTitle());
		}

		delete contest;
		delete contest2;
	}
};

QTEST_MAIN(TestContest)
#include "main.moc"
