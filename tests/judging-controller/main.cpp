/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "base/LemonLog.hpp"
#include "base/settings.h"
#include "core/contestant.h"
#include "core/judgingcontroller.h"
#include "core/task.h"
#include "core/taskjudger.h"
#include "core/testcase.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QTemporaryDir>
#include <QtTest>
#include <array>
#include <atomic>
#include <memory>

namespace {

	bool writeFile(const QString &name, const QByteArray &contents) {
		QFile file(name);
		return file.open(QIODevice::WriteOnly) && file.write(contents) == contents.size();
	}

	// Lemon SPJ arguments: input, submitted answer, expected answer, full score, score file, message file.
	int runHelper(const QStringList &arguments) {
		QFile file(arguments[2]);
		if (! file.open(QIODevice::ReadOnly))
			return 2;
		const auto configuration = QJsonDocument::fromJson(file.readAll()).object();
		const QString prefix = configuration["prefix"].toString();
		QLockFile lock(prefix + ".lock");
		lock.setStaleLockTime(0);
		if (! lock.tryLock() || ! writeFile(prefix + ".started", "started"))
			return 3;
		QElapsedTimer timer;
		timer.start();
		while (! QFileInfo::exists(prefix + ".release")) {
			if (timer.elapsed() > 10000)
				return 4;
			QThread::msleep(10);
		}
		return writeFile(arguments[5], arguments[4].toUtf8()) && writeFile(arguments[6], "helper finished")
		           ? 0
		           : 5;
	}

	void configureSettings(Settings &settings) {
		settings.setMaxJudgingThreads(2);
		settings.setPreventSleepWhileJudging(false);
		settings.setFileSizeLimit(50);
		settings.setCompileTimeLimit(10000);
		settings.setSpecialJudgeTimeLimit(10000);
		settings.setRejudgeTimes(0);
		settings.setDefaultExtraTimeRatio(0.2);
	}

	void configureTask(Task &task, TestCase &testCase) {
		task.setProblemTitle("answer");
		task.setSourceFileName("program");
		task.setAnswerFileExtension("out");
		task.setStandardInputCheck(true);
		task.setStandardOutputCheck(true);
		testCase.setFullScore(100);
		testCase.setTimeLimit(10000);
		testCase.setMemoryLimit(512);
		testCase.addSingleCase("input.txt", "output.txt");
		task.addTestCase(&testCase);
	}

} // namespace

class JudgingControllerTests : public QObject {
	Q_OBJECT
	QString previousDirectory;
	std::unique_ptr<QTemporaryDir> directory;

  private slots:
	void initTestCase() { Lemon::base::logger = spdlog::stdout_color_mt("judging-controller-tests"); }

	void init() {
		previousDirectory = QDir::currentPath();
		directory = std::make_unique<QTemporaryDir>(QCoreApplication::applicationDirPath() +
		                                            "/judging-controller-XXXXXX");
		QVERIFY(directory->isValid());
		QVERIFY(QDir::setCurrent(directory->path()));
		QVERIFY(QDir().mkpath("data"));
		QVERIFY(QDir().mkpath("control"));
		QVERIFY(writeFile("data/input.txt", ""));
		QVERIFY(writeFile("data/output.txt", "42\n"));
	}

	void cleanup() {
		QVERIFY(QDir::setCurrent(previousDirectory));
		directory.reset();
	}

	void emptyBatches() {
		Settings settings;
		configureSettings(settings);
		JudgingController controller(&settings);
		QSignalSpy finished(&controller, &JudgingController::judgeFinished);
		QSignalSpy taskJudged(&controller, &JudgingController::taskJudged);
		for (int batch = 1; batch <= 2; ++batch) {
			controller.start();
			QCOMPARE(finished.size(), batch);
		}
		QCOMPARE(taskJudged.size(), 0);
	}

	void answersOnlyRepeatedBatches() {
		QVERIFY(QDir().mkpath("source/user"));
		Settings settings;
		configureSettings(settings);
		Task task(nullptr, Task::AnswersOnly);
		TestCase testCase;
		configureTask(task, testCase);
		Contestant contestant;
		contestant.setContestantName("user");
		contestant.addTask();
		std::atomic_bool signalsOnMainThread = true;
		bool completeAtSignal = true;
		ResultState expected = CorrectAnswer;
		JudgingController controller(&settings);
		QSignalSpy finished(&controller, &JudgingController::judgeFinished);
		QSignalSpy taskJudged(&controller, &JudgingController::taskJudged);
		connect(
		    &controller, &JudgingController::taskJudged, this,
		    [&] {
			    if (QThread::currentThread() != thread()) {
				    signalsOnMainThread = false;
				    return;
			    }
			    completeAtSignal &=
			        contestant.getCheckJudged(0) &&
			        contestant.getResult(0) == QList<QList<ResultState>>{{expected}} &&
			        contestant.getScore(0) == QList<QList<int>>{{expected == CorrectAnswer ? 100 : 0}};
		    },
		    Qt::DirectConnection);
		for (int batch = 1; batch <= 2; ++batch) {
			expected = batch == 1 ? CorrectAnswer : WrongAnswer;
			QVERIFY(writeFile("source/user/input.out", batch == 1 ? "42\n" : "17\n"));
			auto *judger = new TaskJudger;
			judger->setSettings(&settings);
			judger->setTask(&task);
			judger->setContestantName(contestant.getContestantName());
			controller.addTask(judger, &contestant, 0);
			controller.start();
			QTRY_COMPARE_WITH_TIMEOUT(finished.size(), batch, 5000);
			QCOMPARE(taskJudged.size(), batch);
			QVERIFY(completeAtSignal);
			QCOMPARE(contestant.getResult(0), QList<QList<ResultState>>{{expected}});
		}
		QVERIFY(signalsOnMainThread);
	}

	void parallelTasks_data() {
		QTest::addColumn<bool>("cancel");
		QTest::newRow("complete") << false;
		QTest::newRow("cancel-running-and-queued") << true;
	}

	void parallelTasks() {
		QFETCH(bool, cancel);
		Settings settings;
		configureSettings(settings);
		// SPJ processes use the host environment on every platform, including Linux with bubblewrap.
		Task task(nullptr, Task::AnswersOnly, Task::LemonSpecialJudgeMode);
		TestCase testCase;
		configureTask(task, testCase);
		task.setSpecialJudge(
		    QDir(directory->filePath("data")).relativeFilePath(QCoreApplication::applicationFilePath()));
		std::array<Contestant, 4> contestants;
		std::atomic_int started = 0;
		std::atomic_int active = 0;
		std::atomic_int peak = 0;
		std::atomic_bool workerThreadsCorrect = true;
		std::atomic_bool signalsOnMainThread = true;
		JudgingController controller(&settings);
		QSignalSpy finished(&controller, &JudgingController::judgeFinished);
		QSignalSpy taskJudged(&controller, &JudgingController::taskJudged);
		connect(
		    &controller, &JudgingController::taskJudged, this,
		    [&] {
			    if (QThread::currentThread() != thread())
				    signalsOnMainThread = false;
		    },
		    Qt::DirectConnection);
		connect(
		    &controller, &JudgingController::judgeFinished, this,
		    [&] {
			    if (QThread::currentThread() != thread())
				    signalsOnMainThread = false;
		    },
		    Qt::DirectConnection);
		for (int index = 0; index < int(contestants.size()); ++index) {
			auto &contestant = contestants[index];
			contestant.setContestantName(QString("user%1").arg(index));
			contestant.addTask();
			contestant.setCheckJudged(0, true);
			contestant.setResult(0, {{WrongAnswer}});
			contestant.setScore(0, {{7}});
			const QString sourceDirectory = "source/" + contestant.getContestantName();
			QVERIFY(QDir().mkpath(sourceDirectory));
			const QString prefix = directory->filePath(QString("control/%1").arg(index));
			QVERIFY(writeFile(sourceDirectory + "/input.out",
			                  QJsonDocument(QJsonObject{{"prefix", prefix}}).toJson()));
			auto *judger = new TaskJudger;
			judger->setSettings(&settings);
			judger->setTask(&task);
			judger->setContestantName(contestant.getContestantName());
			connect(
			    judger, &TaskJudger::judgingStarted, this,
			    [&, judger] {
				    ++started;
				    const int count = ++active;
				    int previousPeak = peak.load();
				    while (previousPeak < count && ! peak.compare_exchange_weak(previousPeak, count)) {
				    }
				    if (QThread::currentThread() == thread() || judger->thread() != thread())
					    workerThreadsCorrect = false;
			    },
			    Qt::DirectConnection);
			connect(judger, &TaskJudger::singleCaseFinished, this, [&] { --active; }, Qt::DirectConnection);
			controller.addTask(judger, &contestant, 0);
		}
		controller.start();
		QTRY_COMPARE_WITH_TIMEOUT(QDir("control").entryList({"*.started"}, QDir::Files).size(), 2, 5000);
		QTest::qWait(100);
		QCOMPARE(started.load(), 2);
		QCOMPARE(peak.load(), 2);
		if (cancel) {
			controller.stop();
			QCOMPARE(finished.size(), 0);
		} else {
			for (int index = 0; index < int(contestants.size()); ++index)
				QVERIFY(writeFile(QString("control/%1.release").arg(index), "release"));
		}
		QTRY_COMPARE_WITH_TIMEOUT(finished.size(), 1, 10000);
		QCOMPARE(taskJudged.size(), cancel ? 2 : 4);
		QCOMPARE(started.load(), cancel ? 2 : 4);
		QCOMPARE(peak.load(), 2);
		QVERIFY(workerThreadsCorrect);
		QVERIFY(signalsOnMainThread);
		for (int index = 0; index < int(contestants.size()); ++index) {
			const auto &contestant = contestants[index];
			const QString prefix = QString("control/%1").arg(index);
			const bool didStart = QFileInfo::exists(prefix + ".started");
			QLockFile processLock(prefix + ".lock");
			processLock.setStaleLockTime(0);
			QVERIFY2(processLock.tryLock(), "judgeFinished must follow helper process exit");
			if (cancel) {
				QCOMPARE(contestant.getCheckJudged(0), ! didStart);
				QCOMPARE(contestant.getResult(0), QList<QList<ResultState>>{{WrongAnswer}});
				QCOMPARE(contestant.getScore(0), QList<QList<int>>{{7}});
			} else {
				QVERIFY(didStart);
				QVERIFY(contestant.getCheckJudged(0));
				QCOMPARE(contestant.getResult(0), QList<QList<ResultState>>{{CorrectAnswer}});
				QCOMPARE(contestant.getScore(0), QList<QList<int>>{{100}});
			}
		}
	}
};

int main(int argc, char **argv) {
	QCoreApplication application(argc, argv);
	const QStringList arguments = application.arguments();
	if (arguments.size() == 7 && arguments[5].endsWith("_score") && arguments[6].endsWith("_message"))
		return runHelper(arguments);
	JudgingControllerTests tests;
	return QTest::qExec(&tests, argc, argv);
}

#include "main.moc"
