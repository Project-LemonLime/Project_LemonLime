/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "base/LemonLog.hpp"
#include "base/settings.h"
#include "component/exportutil/exportutil.h"
#include "core/contest.h"
#include "core/contestant.h"
#include "core/task.h"
#include "core/testcase.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <QCoreApplication>
#include <QFile>
#include <QJsonArray>
#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTextStream>
#include <QtTest>
#include <memory>

namespace {

	const QString specialText = " <img src=x onerror=alert(1)> & \" '";
	const QString escapedText = " &lt;img src=x onerror=alert(1)&gt; &amp; &quot; '";

} // namespace

class ExportUtilTests : public QObject {
	Q_OBJECT
	Settings settings;
	ColorTheme theme;
	std::unique_ptr<Contest> contest;
	std::unique_ptr<TestCase> testCase;
	Contestant *contestant = nullptr;

	static QString render(Contest &contest, bool smaller) {
		QString html;
		QTextStream out(&html);
		if (smaller)
			ExportUtil::writeSmallerHtml(out, &contest);
		else
			ExportUtil::writeHtml(out, &contest);
		out.flush();
		return html;
	}

	static void addFormats() {
		QTest::addColumn<bool>("smaller");
		QTest::newRow("html") << false;
		QTest::newRow("smaller-html") << true;
	}

	static void loadContestants(Contest &contest, const QStringList &names) {
		QJsonArray contestants;
		for (const auto &name : names) {
			Contestant saved;
			saved.setContestantName(name);
			saved.setJudgingTime(QDateTime(QDate(2026, 1, 1), QTime(0, 0)));
			QJsonObject row;
			QCOMPARE(saved.writeToJson(row), 0);
			contestants.append(row);
		}
		QCOMPARE(contest.readFromJson({{"version", "1.0"},
		                               {"contestTitle", "contest" + specialText},
		                               {"tasks", QJsonArray{}},
		                               {"contestants", contestants}}),
		         0);
		for (auto *entry : contest.getContestantList())
			entry->setParent(&contest);
	}

  private slots:
	void initTestCase() {
		Lemon::base::logger = spdlog::stdout_color_mt("export-util-tests");
		theme.setColor({120, 100, 85}, {0, 100, 85}, {0, 0, 85}, {60, 100, 85}, {}, {1, 1, 1});
		settings.addColorTheme(&theme);
	}

	void init() {
		contest = std::make_unique<Contest>();
		contest->setSettings(&settings);
		loadContestants(*contest, {"contestant" + specialText});
		contestant = contest->getContestant("contestant" + specialText);
		QVERIFY(contestant);

		auto *task = new Task(contest.get());
		task->setProblemTitle("task" + specialText);
		testCase = std::make_unique<TestCase>();
		testCase->setFullScore(100);
		testCase->addSingleCase("input" + specialText + ".in", "output.txt");
		task->addTestCase(testCase.get());
		contest->addTask(task);
		contestant->setCheckJudged(0, true);
		contestant->setCompileState(0, CompileSuccessfully);
		contestant->setSourceFile(0, "source" + specialText + ".cpp");
		contestant->setInputFiles(0, {{"input" + specialText + ".in"}});
		contestant->setResult(0, {{CorrectAnswer}});
		contestant->setMessage(0, {{""}});
		contestant->setScore(0, {{100}});
		contestant->setTimeUsed(0, {{1000}});
		contestant->setMemoryUsed(0, {{1024}});
	}

	void cleanup() {
		contest.reset();
		testCase.reset();
		contestant = nullptr;
	}

	void compileMessages_data() {
		QTest::addColumn<bool>("smaller");
		QTest::addColumn<QString>("message");
		QTest::addColumn<QString>("expected");
		for (bool smaller : {false, true}) {
			const QByteArray prefix = smaller ? "smaller-" : "html-";
			QTest::newRow((prefix + "script").constData())
			    << smaller << QString("<script>document.body.textContent = \"changed\";</script>")
			    << QString("&lt;script&gt;document.body.textContent = &quot;changed&quot;;&lt;/script&gt;");
			QTest::newRow((prefix + "special-characters").constData())
			    << smaller << QString("<>&\"' &lt;tag&gt; <br>")
			    << QString("&lt;&gt;&amp;&quot;' &amp;lt;tag&amp;gt; &lt;br&gt;");
			QTest::newRow((prefix + "line-endings").constData())
			    << smaller << QString("first\r\nsecond\nthird\rfourth\r\n")
			    << QString("first<br>second<br>third<br>fourth");
			QTest::newRow((prefix + "empty").constData()) << smaller << QString() << QString();
		}
	}

	void compileMessages() {
		QFETCH(bool, smaller);
		QFETCH(QString, message);
		QFETCH(QString, expected);
		contestant->setCompileState(0, CompileError);
		contestant->setCompileMessage(0, message);
		const QString html = render(*contest, smaller);
		const auto match = QRegularExpression("<code>(.*?)</code>").match(html);
		QCOMPARE(match.hasMatch(), ! message.isEmpty());
		QCOMPARE(match.captured(1), expected);
		QCOMPARE(contestant->getCompileMessage(0), message);
	}

	void namesAndFiles_data() {
		QTest::addColumn<bool>("smaller");
		QTest::addColumn<int>("compileState");
		for (bool smaller : {false, true}) {
			const QByteArray prefix = smaller ? "smaller-" : "html-";
			QTest::newRow((prefix + "compile-error").constData()) << smaller << int(CompileError);
			QTest::newRow((prefix + "compile-timeout").constData())
			    << smaller << int(CompileTimeLimitExceeded);
			QTest::newRow((prefix + "success").constData()) << smaller << int(CompileSuccessfully);
		}
	}

	void namesAndFiles() {
		QFETCH(bool, smaller);
		QFETCH(int, compileState);
		contestant->setCompileState(0, static_cast<CompileState>(compileState));
		const QString html = render(*contest, smaller);
		QVERIFY(html.contains("task" + escapedText));
		QVERIFY(html.contains("source" + escapedText + ".cpp"));
		QVERIFY(! html.contains("<img"));
		if (compileState == CompileSuccessfully) {
			QVERIFY(html.contains("<td>input" + escapedText + ".in</td>"));
			QVERIFY(html.contains("1.000 s"));
			QVERIFY(! html.contains("data-message="));
		}
	}

	void resultMessages_data() { addFormats(); }

	void resultMessages() {
		QFETCH(bool, smaller);
		const QString message = "quote: \"; slash: \\; entity: &quot;);alert(1);// percent: %22 <img src=x>"
		                        "\r\nnext\nlast\r";
		const QString expected =
		    "quote: &quot;; slash: \\; entity: &amp;quot;);alert(1);// percent: %22 &lt;img src=x&gt;"
		    "\r\nnext\nlast\r";
		contestant->setMessage(0, {{message}});
		const QString html = render(*contest, smaller);
		QVERIFY(html.contains("data-message=\"" + expected + "\""));
		QVERIFY(html.contains("onclick=\"alert(this.dataset.message); return false;\""));
		QVERIFY(html.contains("> (...)</a>"));
		QVERIFY(! html.contains("javascript:"));
		QVERIFY(! html.contains("<img"));
		QCOMPARE(contestant->getMessage(0)[0][0], message);
	}

	void rankings_data() { addFormats(); }

	void rankings() {
		QFETCH(bool, smaller);
		Contest rankedContest;
		rankedContest.setSettings(&settings);
		loadContestants(rankedContest, {"Eve", "Cal", "Bea", "Dee", "Amy"});
		Task task;
		task.addTestCase(testCase.get());
		rankedContest.addTask(&task);
		for (const QString &name : QStringList{"Amy", "Bea", "Cal"}) {
			auto *entry = rankedContest.getContestant(name);
			QVERIFY(entry);
			entry->setCheckJudged(0, true);
			entry->setScore(0, {{name == "Amy" ? 0 : 100}});
		}

		const QString html = render(rankedContest, smaller);
		const QRegularExpression rankAndName(
		    "<tr><td[^>]*>(\\d+)</td><td[^>]*><a href=\"#c(\\d+)\"[^>]*>([^<]+)</a>");
		QList<QStringList> rows;
		for (const auto &match : rankAndName.globalMatch(html))
			rows.append(match.capturedTexts().mid(1));
		// Rank, original detail index, and name in displayed order.
		const QList<QStringList> expected{
		    {"1", "1", "Bea"}, {"1", "2", "Cal"}, {"3", "0", "Amy"}, {"4", "3", "Dee"}, {"4", "4", "Eve"}};
		QCOMPARE(rows, expected);
		if (! smaller) {
			QVERIFY(html.contains("href=\"#c1p0\" class=\"a-0\">100</a>"));
			QVERIFY(html.contains("Contestant: Bea</span><div id=\"c1p0\">"));
		}
	}

	void fullExport_data() { addFormats(); }

	void fullExport() {
		QFETCH(bool, smaller);
		QTemporaryDir directory;
		QVERIFY(directory.isValid());
		QFile file(directory.filePath("result.html"));
		QVERIFY2(file.open(QIODevice::WriteOnly), qPrintable(file.errorString()));
		{
			QTextStream out(&file);
			if (smaller)
				ExportUtil::writeSmallerHtml(out, contest.get());
			else
				ExportUtil::writeHtml(out, contest.get());
		}
		file.close();
		QVERIFY2(file.open(QIODevice::ReadOnly), qPrintable(file.errorString()));
		const QString html = QString::fromUtf8(file.readAll());
		QCOMPARE(html.count("contest" + escapedText), 2);
		QCOMPARE(html.count("contestant" + escapedText), 2);
		QCOMPARE(html.count("task" + escapedText), 2);
		QVERIFY(! html.contains("<img"));
		QVERIFY(html.contains("href=\"#c0\""));
	}
};

QTEST_GUILESS_MAIN(ExportUtilTests)
#include "main.moc"
