/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "base/LemonLog.hpp"
#include "base/compiler.h"
#include "base/settings.h"
#include "core/contestant.h"
#include "core/processrunner.h"
#include "core/task.h"
#include "core/taskjudger.h"
#include "core/testcase.h"
#include "core/windowsprocessutils.h"
#include "core/windowssandbox.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <aclapi.h>
#include <sddl.h>

#include <Psapi.h>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QProcess>
#include <QRegularExpression>
#include <QScopeGuard>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QtTest>
#include <thread>
#include <winsock2.h>

using namespace Lemon::Windows;

namespace {
	QString quoteArgument(const QString &argument) {
		QString result = "\"";
		int slashes = 0;
		for (QChar c : argument) {
			if (c == '\\') {
				++slashes;
				continue;
			}
			result += QString(c == '"' ? slashes * 2 + 1 : slashes, '\\');
			result += c;
			slashes = 0;
		}
		return result + QString(slashes * 2, '\\') + '"';
	}

	QString currentUserSid() {
		Handle token;
		if (! OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, token.put()))
			qFatal("Cannot open test process token");
		DWORD size = 0;
		GetTokenInformation(token.get(), TokenUser, nullptr, 0, &size);
		QByteArray information(size, Qt::Uninitialized);
		if (! GetTokenInformation(token.get(), TokenUser, information.data(), size, &size))
			qFatal("Cannot read test process user");
		LocalMemory<LPWSTR> sid;
		if (! ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER *>(information.data())->User.Sid, sid.put()))
			qFatal("Cannot serialize test process user");
		return QString::fromWCharArray(sid.get());
	}

	bool setPermissions(const QString &path, const QString &sddl, bool protectedAcl = true) {
		LocalMemory<PSECURITY_DESCRIPTOR> descriptor;
		if (! ConvertStringSecurityDescriptorToSecurityDescriptorW(wide(sddl), SDDL_REVISION_1,
		                                                           descriptor.put(), nullptr)) {
			return false;
		}
		PACL acl = nullptr;
		BOOL present = FALSE, defaulted = FALSE;
		if (! GetSecurityDescriptorDacl(descriptor.get(), &present, &acl, &defaulted) || ! present)
			return false;
		Handle file(CreateFileW(wide(QDir::toNativeSeparators(path)), MAXIMUM_ALLOWED,
		                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
		                        OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr));
		return file && SetSecurityInfo(file.get(), SE_FILE_OBJECT,
		                               DACL_SECURITY_INFORMATION |
		                                   (protectedAcl ? PROTECTED_DACL_SECURITY_INFORMATION
		                                                 : UNPROTECTED_DACL_SECURITY_INFORMATION),
		                               nullptr, nullptr, acl, nullptr) == ERROR_SUCCESS;
	}
} // namespace

class WindowsSandboxTests : public QObject {
	Q_OBJECT
	QTemporaryDir root{QDir::currentPath() + "/sandbox-tests-XXXXXX"};
	int sequence = 0;
	QString python;
	QString java;
	QString javac;

	static bool write(const QString &path, const QByteArray &bytes) {
		QFile file(path);
		return file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size();
	}

	static QByteArray read(const QString &path) {
		QFile file(path);
		if (! file.open(QIODevice::ReadOnly))
			return {};
		return file.readAll();
	}

	static QString permissions(const QString &path) {
		LocalMemory<PSECURITY_DESCRIPTOR> descriptor;
		const auto native = QDir::toNativeSeparators(path);
		if (GetNamedSecurityInfoW(const_cast<wchar_t *>(wide(native)), SE_FILE_OBJECT,
		                          DACL_SECURITY_INFORMATION, nullptr, nullptr, nullptr, nullptr,
		                          descriptor.put()) != ERROR_SUCCESS) {
			qFatal("Cannot read test ACL");
		}
		// Windows may mark a rewritten DACL as auto-inherited. Compare its ACEs and protection state.
		SetSecurityDescriptorControl(descriptor.get(), SE_DACL_AUTO_INHERITED, 0);
		LocalMemory<LPWSTR> text;
		if (! ConvertSecurityDescriptorToStringSecurityDescriptorW(
		        descriptor.get(), SDDL_REVISION_1, DACL_SECURITY_INFORMATION, text.put(), nullptr)) {
			qFatal("Cannot serialize test ACL");
		}
		return QString::fromWCharArray(text.get());
	}

	static bool createJunction(const QString &link, const QString &target) {
		auto environment = QProcessEnvironment::systemEnvironment();
		environment.insert("LEMON_TEST_LINK", QDir::toNativeSeparators(link));
		environment.insert("LEMON_TEST_TARGET", QDir::toNativeSeparators(target));
		QProcess process;
		process.setProcessEnvironment(environment);
		process.setProgram(environment.value("SystemRoot") + "/System32/cmd.exe");
		process.setNativeArguments("/d /v:off /c mklink /J \"%LEMON_TEST_LINK%\" \"%LEMON_TEST_TARGET%\"");
		process.start();
		return process.waitForFinished(5000) && process.exitCode() == 0;
	}

	ProcessRunnerConfig config(const QString &arguments) {
		ProcessRunnerConfig cfg;
		cfg.workingDirectory = root.path() + QString("/case-%1/").arg(++sequence);
		QDir().mkpath(cfg.workingDirectory);
		cfg.executableFile = cfg.workingDirectory + "helper.exe";
		if (! QFile::copy(QDir::currentPath() + "/sandbox-helper.exe", cfg.executableFile))
			qFatal("Cannot copy sandbox helper");
		cfg.arguments = arguments;
		cfg.standardInputCheck = cfg.standardOutputCheck = true;
		cfg.inputFile = root.path() + "/input.txt";
		cfg.environment = QProcessEnvironment::systemEnvironment();
		cfg.sandboxSettings.enabled = true;
		cfg.timeLimit = cfg.rawTimeLimit = 3000;
		cfg.memoryLimit = cfg.rawMemoryLimit = 256;
		cfg.extraTimeRatio = 0.2;
		return cfg;
	}

	ProcessRunnerResult run(ProcessRunnerConfig cfg) {
		std::atomic<bool> stop{false};
		return ProcessRunner::create(std::move(cfg), stop)->run();
	}

	void configureRuntimeFixture(ProcessRunnerConfig &cfg, SandboxSettings::Runtime runtime) {
		cfg.sandboxSettings.runtime = runtime;
		if (runtime != SandboxSettings::Java && runtime != SandboxSettings::Python)
			return;
		const auto directory =
		    root.path() + (runtime == SandboxSettings::Java ? "/fake-java" : "/fake-python");
		if (runtime == SandboxSettings::Java) {
			QDir().mkpath(directory + "/bin");
			QDir().mkpath(directory + "/lib");
			cfg.runtimeExecutable = directory + "/bin/helper.exe";
		} else {
			QDir().mkpath(directory);
			cfg.runtimeExecutable = directory + "/python.exe";
		}
		if (! QFileInfo::exists(cfg.runtimeExecutable) &&
		    ! QFile::copy(QDir::currentPath() + "/sandbox-helper.exe", cfg.runtimeExecutable)) {
			qFatal("Cannot copy runtime discovery fixture");
		}
	}

	QString basePythonExecutable() const {
		// Resolve the installed interpreter through a trusted host-side probe, including launcher entries.
		QProcess probe;
		probe.start(python, {"-I", "-S", "-c", "import sys,json;print(json.dumps([sys._base_executable]))"});
		if (! probe.waitForFinished(10000) || probe.exitStatus() != QProcess::NormalExit ||
		    probe.exitCode() != 0) {
			qWarning() << "Cannot locate the base Python interpreter:" << probe.readAllStandardError();
			return {};
		}
		const auto values = QJsonDocument::fromJson(probe.readAllStandardOutput()).array();
		return values.size() == 1 ? QFileInfo(values[0].toString()).canonicalFilePath() : QString{};
	}

  private slots:
	void initTestCase() {
		Lemon::base::logger = spdlog::stdout_color_mt("sandbox-tests");
		QVERIFY(root.isValid());
		QVERIFY(write(root.path() + "/input.txt", "12 30\n"));
		QVERIFY(write(root.path() + "/secret.txt", "secret answer\n"));
		python = QStandardPaths::findExecutable("python");
		java = QStandardPaths::findExecutable("java");
		javac = QStandardPaths::findExecutable("javac");
	}

	void disabledByDefault() {
		auto cfg = config("host " + quoteArgument(root.path() + "/secret.txt"));
		cfg.sandboxSettings = SandboxSettings{};
		QVERIFY(! cfg.sandboxSettings.enabled);
		cfg.runtimeExecutable = root.path() + "/missing-runtime.exe";
		cfg.sandboxSettings.readOnlyDirectories = {root.path()};
		cfg.environment.insert("LEMON_SANDBOX_TEST", "preserved");

		const auto result = run(cfg);
		QVERIFY2(result.result == CorrectAnswer, qPrintable(result.message));
		QCOMPARE(read(cfg.workingDirectory + "_tmpout").trimmed(), QByteArray("host"));
		QCOMPARE(result.preparationTime, 0);
		QCOMPARE(result.runtimeAclUpdates, 0);
		QVERIFY(! cfg.sandboxSession);
		QVERIFY(! QFileInfo::exists(cfg.workingDirectory + ".sandbox-home"));
		cfg.arguments = "spawn 100";
		const auto child = run(cfg);
		QVERIFY2(child.result == CorrectAnswer, qPrintable(child.message));
	}

	void standardIo() {
		auto cfg = config("sum");

		const auto result = run(cfg);
		QVERIFY2(result.result == CorrectAnswer, qPrintable(result.message));
		QCOMPARE(read(cfg.workingDirectory + "_tmpout").trimmed(), QByteArray("42"));
		QVERIFY(result.memoryUsed > 0);
	}

	void testcaseDirectoryCreationFailure_data() {
		QTest::addColumn<bool>("enabled");
		QTest::newRow("ordinary") << false;
		QTest::newRow("sandbox") << true;
	}

	void testcaseDirectoryCreationFailure() {
		QFETCH(bool, enabled);
		const auto contestDirectory = root.path() + QString("/directory-failure-%1").arg(++sequence);
		QVERIFY(QDir().mkpath(contestDirectory + "/source/_0.0"));
		QVERIFY(QDir().mkpath(contestDirectory + "/data"));
		QVERIFY(QDir().mkpath(contestDirectory + "/runtime"));
		QVERIFY(write(contestDirectory + "/source/_0.0/program.dummy", "source"));
		QVERIFY(write(contestDirectory + "/data/input.txt", ""));
		QVERIFY(write(contestDirectory + "/data/output.txt", ""));
		const auto helper = contestDirectory + "/runtime/helper.exe";
		QVERIFY(QFile::copy(QDir::currentPath() + "/sandbox-helper.exe", helper));
		const auto originalDirectory = QDir::currentPath();
		const auto restoreDirectory = qScopeGuard([&] { QDir::setCurrent(originalDirectory); });
		QVERIFY(QDir::setCurrent(contestDirectory));

		Compiler compiler;
		compiler.setCompilerName("helper");
		compiler.setCompilerType(Compiler::InterpretiveWithoutByteCode);
		compiler.setSourceExtensions("dummy");
		compiler.setInterpreterLocation(helper);
		compiler.addConfiguration("default", "", "work-files");
		SandboxSettings sandbox;
		sandbox.enabled = enabled;
		sandbox.runtime = SandboxSettings::Native;
		compiler.setSandboxSettings(sandbox);
		Settings settings;
		settings.setFileSizeLimit(50);
		settings.setDefaultExtraTimeRatio(0.2);
		settings.setSpecialJudgeTimeLimit(10000);
		settings.setRejudgeTimes(0);
		settings.addCompiler(&compiler);

		Task task;
		task.setSourceFileName("program");
		task.setCompilerConfiguration("helper", "default");
		task.setStandardInputCheck(true);
		task.setStandardOutputCheck(true);
		TestCase failed, dependent, independent;
		for (auto *testCase : {&failed, &dependent, &independent}) {
			testCase->setFullScore(100);
			testCase->setTimeLimit(3000);
			testCase->setMemoryLimit(256);
			testCase->addSingleCase("input.txt", "output.txt");
			task.addTestCase(testCase);
		}
		failed.addSingleCase("input.txt", "output.txt");
		dependent.setDependenceSubtask(QList<int>{1});
		Contestant contestant;
		// Source preparation occupies the first testcase directory before mkdir() is called.
		contestant.setContestantName("_0.0");
		contestant.addTask();
		TaskJudger judger;
		judger.setSettings(&settings);
		judger.setTask(&task);
		judger.setTaskId(0);
		judger.setContestant(&contestant);
		QSignalSpy finished(&judger, &TaskJudger::singleCaseFinished);
		judger.judgeIt();

		QVERIFY(contestant.getCheckJudged(0));
		QCOMPARE(contestant.getCompileState(0), CompileSuccessfully);
		QCOMPARE(contestant.getResult(0)[0], QList<ResultState>({FileError, Skipped}));
		QCOMPARE(contestant.getResult(0)[1][0], Skipped);
		QCOMPARE(contestant.getResult(0)[2][0], CorrectAnswer);
		QCOMPARE(contestant.getScore(0)[0][0], 0);
		QCOMPARE(contestant.getTimeUsed(0)[0][0], -1);
		QCOMPARE(contestant.getMemoryUsed(0)[0][0], -1);
		QCOMPARE(finished.size(), 3);
		QCOMPARE(finished[0][4].toInt(), int(FileError));
		const QString prefix = "Cannot create testcase working directory: ";
		const auto error = contestant.getMessage(0)[0][0];
		QVERIFY2(error.startsWith(prefix), qPrintable(error));
		const QDir workingDirectory(error.mid(prefix.size()));
		QVERIFY(workingDirectory.exists("program.dummy"));
		QVERIFY(! workingDirectory.exists("created"));
		QCOMPARE(read(workingDirectory.filePath("../_2.0/created/nested/data.txt")), QByteArray("created"));
	}

	void commandLineArguments_data() {
		QTest::addColumn<bool>("enabled");
		QTest::newRow("ordinary") << false;
		QTest::newRow("sandbox") << true;
	}

	void commandLineArguments() {
		QFETCH(bool, enabled);
		const QStringList arguments = {"",
		                               "with spaces",
		                               "embedded\"quote",
		                               "trailing space\\",
		                               "backslash\\\"quote",
		                               "two trailing\\\\"};
		QString command = "arguments";
		for (const auto &argument : arguments)
			command += ' ' + quoteArgument(argument);
		auto cfg = config(command);
		cfg.sandboxSettings.enabled = enabled;
		const auto executable = cfg.workingDirectory + "helper with spaces.exe";
		QVERIFY(QFile::rename(cfg.executableFile, executable));
		cfg.executableFile = executable;

		const auto result = run(cfg);
		QVERIFY2(result.result == CorrectAnswer, qPrintable(result.message));
		QCOMPARE(read(cfg.workingDirectory + "_tmpout").replace("\r\n", "\n"),
		         arguments.join('\n').toUtf8() + '\n');
	}

	void runtimeAceOrder_data() {
		QTest::addColumn<QString>("entries");
		QTest::addColumn<int>("updates");
		QTest::newRow("allow-before-inherited-deny") << "(A;;FRFX;;;S-1-15-2-1)(D;ID;FRFX;;;S-1-15-2-1)" << 0;
		QTest::newRow("partial-allow-before-deny-of-granted-rights")
		    << "(A;;FR;;;S-1-15-2-1)(D;ID;FR;;;S-1-15-2-1)(A;ID;FX;;;S-1-15-2-1)" << 0;
		QTest::newRow("deny-of-remaining-rights")
		    << "(A;;FR;;;S-1-15-2-1)(D;ID;FX;;;S-1-15-2-1)(A;ID;FX;;;S-1-15-2-1)" << 1;
	}

	void runtimeAceOrder() {
		QFETCH(QString, entries);
		QFETCH(int, updates);
		auto cfg = config("sum");
		const auto directory = root.path() + QString("/ace-order-%1").arg(++sequence);
		const auto file = directory + "/resource.txt";
		QVERIFY(QDir().mkpath(directory));
		QVERIFY(write(file, "runtime data"));
		const auto base = QString("D:P(A;;FA;;;SY)(A;;FA;;;BA)(A;;FA;;;%1)").arg(currentUserSid());
		QVERIFY(setPermissions(directory, base + "(A;;FRFX;;;S-1-15-2-1)"));
		QVERIFY(setPermissions(file, base + entries));
		const auto before = permissions(file);
		cfg.sandboxSettings.readOnlyDirectories = {directory};
		{
			WindowsSandbox sandbox(cfg);
			QString error;
			QVERIFY2(sandbox.prepare(error), qPrintable(error));
			QCOMPARE(sandbox.aclUpdates(), updates);
			if (updates == 0)
				QCOMPARE(permissions(file), before);
			else
				QVERIFY(permissions(file) != before);
		}
		QCOMPARE(permissions(file), before);
	}

	void nestedFileIo_data() {
		QTest::addColumn<bool>("enabled");
		QTest::addColumn<bool>("standardInput");
		QTest::addColumn<bool>("standardOutput");
		for (bool enabled : {false, true}) {
			for (bool input : {false, true}) {
				for (bool output : {false, true}) {
					const auto name = QString("%1-input%2-output%3")
					                      .arg(enabled ? "sandbox" : "ordinary")
					                      .arg(input)
					                      .arg(output);
					QTest::newRow(qPrintable(name)) << enabled << input << output;
				}
			}
		}
	}

	void nestedFileIo() {
		QFETCH(bool, enabled);
		QFETCH(bool, standardInput);
		QFETCH(bool, standardOutput);
		auto cfg = config(
		    QString("file-io files/input.txt files/output.txt %1 %2").arg(standardInput).arg(standardOutput));
		cfg.sandboxSettings.enabled = enabled;
		cfg.standardInputCheck = standardInput;
		cfg.standardOutputCheck = standardOutput;
		cfg.inputFileName = "files/input.txt";
		cfg.outputFileName = "files/output.txt";
		QVERIFY(QDir(cfg.workingDirectory).mkdir("files"));
		QVERIFY(QFile::copy(cfg.inputFile, cfg.workingDirectory + cfg.inputFileName));

		const auto result = run(cfg);
		QVERIFY2(result.result == CorrectAnswer, qPrintable(result.message));
		const auto output = cfg.standardOutputCheck ? QString("_tmpout") : cfg.outputFileName;
		QCOMPARE(read(cfg.workingDirectory + output).trimmed(), QByteArray("42"));
	}

	void isolationAndHandles() {
		SECURITY_ATTRIBUTES attributes{sizeof(attributes), nullptr, TRUE};
		Handle secret(CreateFileW(wide(root.path() + "/secret.txt"), GENERIC_READ, FILE_SHARE_READ,
		                          &attributes, OPEN_EXISTING, 0, nullptr));
		QVERIFY(bool(secret));
		auto cfg = config("isolation " + quoteArgument(root.path() + "/secret.txt") + ' ' +
		                  QString::number(quintptr(secret.get())));

		const auto result = run(cfg);
		QVERIFY2(result.result == CorrectAnswer, qPrintable(result.message));
		QCOMPARE(read(cfg.workingDirectory + "_tmpout").trimmed(), QByteArray("isolated"));
		QVERIFY(QFileInfo::exists(cfg.workingDirectory + "own.txt"));
	}

	void runtimeCache() {
		auto session = WindowsSandbox::createSession();
		const auto directory = root.path() + "/runtime";
		QDir().mkpath(directory);
		for (int i = 0; i < 128; ++i)
			QVERIFY(write(directory + QString("/file-%1.txt").arg(i), "runtime data"));
		auto first = config("runtime " + quoteArgument(directory + "/file-0.txt"));
		first.sandboxSession = session;
		first.sandboxSettings.readOnlyDirectories = {directory};
		const auto cold = run(first);
		QVERIFY2(cold.result == CorrectAnswer, qPrintable(cold.message));
		QVERIFY(cold.runtimeAclUpdates > 0);
		auto second = config(first.arguments);
		second.sandboxSession = session;
		second.sandboxSettings = first.sandboxSettings;
		const auto warm = run(second);
		QVERIFY2(warm.result == CorrectAnswer, qPrintable(warm.message));
		QVERIFY(warm.runtimeCacheHit);
		QCOMPARE(warm.runtimeAclUpdates, 0);
		qInfo() << "Runtime preparation cold/warm ms:" << cold.preparationTime << warm.preparationTime;
	}

	void permissionsAreRestored() {
		const auto directory = root.path() + "/temporary-runtime";
		QVERIFY(QDir().mkpath(directory));
		QVERIFY(write(directory + "/resource.txt", "runtime data"));
		const auto beforeDirectory = permissions(directory);
		const auto beforeFile = permissions(directory + "/resource.txt");
		auto cfg = config("runtime " + quoteArgument(directory + "/resource.txt"));
		cfg.sandboxSettings.readOnlyDirectories = {directory};

		const auto result = run(cfg);
		QVERIFY2(result.result == CorrectAnswer, qPrintable(result.message));
		QCOMPARE(permissions(directory), beforeDirectory);
		QCOMPARE(permissions(directory + "/resource.txt"), beforeFile);
		QVERIFY(! permissions(cfg.workingDirectory).contains("S-1-15-2-"));
		QVERIFY(! permissions(cfg.executableFile).contains("S-1-15-2-"));
		QVERIFY(! permissions(cfg.workingDirectory + "_tmpout").contains("S-1-15-2-"));
		cfg.sandboxSettings.readOnlyDirectories.append(directory + "/missing");
		QCOMPARE(run(cfg).result, CannotStartProgram);
		QCOMPARE(permissions(directory), beforeDirectory);
		QCOMPARE(permissions(directory + "/resource.txt"), beforeFile);
		QVERIFY(QDir(cfg.workingDirectory).removeRecursively());
	}

	void sharedPermissionsLifetime() {
		const auto directory = root.path() + "/shared-runtime";
		QVERIFY(QDir().mkpath(directory));
		QVERIFY(write(directory + "/resource.txt", "runtime data"));
		const auto before = permissions(directory + "/resource.txt");
		auto first = config("sum");
		auto second = config("sum");
		first.sandboxSettings.readOnlyDirectories = {directory};
		second.sandboxSettings = first.sandboxSettings;
		auto one = std::make_unique<WindowsSandbox>(first);
		auto two = std::make_unique<WindowsSandbox>(second);
		QString error;
		QVERIFY2(one->prepare(error), qPrintable(error));
		QVERIFY2(two->prepare(error), qPrintable(error));
		QVERIFY(two->cacheHit());
		const auto granted = permissions(directory + "/resource.txt");
		QVERIFY(granted != before);
		one.reset();
		QCOMPARE(permissions(directory + "/resource.txt"), granted);
		two.reset();
		QCOMPARE(permissions(directory + "/resource.txt"), before);
	}

	void workPermissionsArePreserved_data() {
		QTest::addColumn<bool>("protectedAcl");
		QTest::newRow("inherited") << false;
		QTest::newRow("protected") << true;
	}

	void workPermissionsArePreserved() {
		QFETCH(bool, protectedAcl);
		auto cfg = config("work-files");
		const auto sddl = QString("D:(A;OICI;FA;;;%1)(A;OICI;FR;;;S-1-15-2-123456789)").arg(currentUserSid());
		QVERIFY(setPermissions(cfg.workingDirectory, sddl, protectedAcl));
		QVERIFY(QDir(cfg.workingDirectory).mkdir("existing"));
		QVERIFY(write(cfg.workingDirectory + "existing/data.txt", "existing data"));
		const QStringList existing{cfg.workingDirectory, cfg.executableFile,
		                           cfg.workingDirectory + "existing",
		                           cfg.workingDirectory + "existing/data.txt"};
		QStringList before;
		for (const auto &path : existing)
			before.append(permissions(path));

		const auto result = run(cfg);
		QVERIFY2(result.result == CorrectAnswer, qPrintable(result.message));
		for (int i = 0; i < existing.size(); ++i)
			QCOMPARE(permissions(existing[i]), before[i]);
		QCOMPARE(read(cfg.workingDirectory + "created/nested/data.txt"), QByteArray("created"));
		for (const auto &name :
		     {"created", "created/nested", "created/nested/data.txt", "_tmpout", "_tmperr"}) {
			const auto acl = permissions(cfg.workingDirectory + name);
			QVERIFY(acl.contains("S-1-15-2-123456789"));
			QCOMPARE(acl.count("S-1-15-2-"), 1);
		}
		QVERIFY(QDir(cfg.workingDirectory).removeRecursively());
	}

	void workFileCleanupPreservesRuntimeGrants() {
		auto session = WindowsSandbox::createSession();
		auto first = config("sum");
		first.sandboxSession = session;
		const auto resource = first.workingDirectory + "resource.txt";
		QVERIFY(write(resource, "runtime data"));
		const auto before = permissions(resource);
		auto one = std::make_unique<WindowsSandbox>(first);
		QString error;
		QVERIFY2(one->prepare(error), qPrintable(error));

		auto second = config("runtime " + quoteArgument(resource));
		second.sandboxSession = session;
		second.sandboxSettings.readOnlyDirectories = {first.workingDirectory};
		auto two = std::make_unique<WindowsSandbox>(second);
		QVERIFY2(two->prepare(error), qPrintable(error));
		one.reset();

		const auto result = run(second);
		QVERIFY2(result.result == CorrectAnswer, qPrintable(result.message));
		QVERIFY(result.runtimeCacheHit);
		QCOMPARE(result.runtimeAclUpdates, 0);
		QCOMPARE(read(second.workingDirectory + "_tmpout").trimmed(), QByteArray("readonly"));
		two.reset();
		first.sandboxSession.reset();
		second.sandboxSession.reset();
		session.reset();
		QCOMPARE(permissions(resource), before);
	}

	void runtimeJunction_data() {
		QTest::addColumn<QString>("subdirectory");
		QTest::addColumn<bool>("chained");
		QTest::newRow("root") << QString() << false;
		QTest::newRow("ancestor") << QString("/bin") << false;
		QTest::newRow("chained") << QString("/bin") << true;
	}

	void runtimeJunction() {
		auto session = WindowsSandbox::createSession();
		QFETCH(QString, subdirectory);
		QFETCH(bool, chained);
		const auto installation = root.path() + QString("/runtime-%1 安装").arg(++sequence);
		const auto link = installation + " current";
		QVERIFY(QDir().mkpath(installation + subdirectory));
		QVERIFY(write(installation + subdirectory + "/resource.txt", "runtime data"));
		if (chained)
			QVERIFY(createJunction(installation + " alias", installation));
		QVERIFY(createJunction(link, chained ? installation + " alias" : installation));
		auto first = config("runtime " + quoteArgument(link + subdirectory + "/resource.txt"));
		first.sandboxSession = session;
		first.sandboxSettings.readOnlyDirectories = {link + subdirectory};
		const auto linked = run(first);
		QVERIFY2(linked.result == CorrectAnswer, qPrintable(linked.message));
		auto second = config(first.arguments);
		second.sandboxSession = session;
		second.sandboxSettings = first.sandboxSettings;
		const auto repeated = run(second);
		QVERIFY2(repeated.result == CorrectAnswer, qPrintable(repeated.message));
		QVERIFY(repeated.runtimeCacheHit);
		QCOMPARE(repeated.runtimeAclUpdates, 0);
	}

	void runtimeInsideWorkingDirectory() {
		auto cfg = config("sum");
		QVERIFY(QDir().mkpath(cfg.workingDirectory + "nested"));
		const auto link = root.path() + "/work-alias";
		QVERIFY(createJunction(link, cfg.workingDirectory));
		cfg.sandboxSettings.readOnlyDirectories = {link + "/nested"};
		QCOMPARE(run(cfg).result, CorrectAnswer);
	}

	void unavailableRuntime() {
		auto cfg = config("sum");
		cfg.runtimeExecutable = root.path() + "/missing.exe";
		QCOMPARE(run(cfg).result, CannotStartProgram);
	}

	void unavailableJunction_data() {
		QTest::addColumn<bool>("cycle");
		QTest::newRow("missing-target") << false;
		QTest::newRow("cycle") << true;
	}

	void unavailableJunction() {
		QFETCH(bool, cycle);
		auto cfg = config("sum");
		const auto link = root.path() + QString("/invalid-link-%1").arg(++sequence);
		const auto target = link + "-target";
		QVERIFY(createJunction(link, target));
		if (cycle)
			QVERIFY(createJunction(target, link));
		cfg.sandboxSettings.readOnlyDirectories = {link};
		QCOMPARE(run(cfg).result, CannotStartProgram);
		QVERIFY(QDir().rmdir(link));
		if (cycle)
			QVERIFY(QDir().rmdir(target));
	}

	void sharedCompilerCache() {
		auto session = WindowsSandbox::createSession();
		const auto directory = root.path() + "/compilers";
		QVERIFY(QDir().mkpath(directory));
		QVERIFY(write(directory + "/gcc.exe", "C compiler"));
		QVERIFY(write(directory + "/g++.exe", "C++ compiler"));
		auto first = config("sum");
		first.sandboxSession = session;
		first.runtimeExecutable = directory + "/gcc.exe";
		first.sandboxSettings.runtime = SandboxSettings::Native;
		auto second = config("sum");
		second.sandboxSession = session;
		second.runtimeExecutable = directory + "/g++.exe";
		second.sandboxSettings.runtime = SandboxSettings::Native;
		QCOMPARE(run(first).result, CorrectAnswer);
		QCOMPARE(run(second).result, CorrectAnswer);
		const auto reused = run(first);
		QCOMPARE(reused.result, CorrectAnswer);
		QVERIFY(reused.runtimeCacheHit);
		QCOMPARE(reused.runtimeAclUpdates, 0);
		QVERIFY(write(first.runtimeExecutable, "updated C compiler"));
		const auto updated = run(first);
		QCOMPARE(updated.result, CorrectAnswer);
		QVERIFY(! updated.runtimeCacheHit);
	}

	void parallelRuntimeCache() {
		auto session = WindowsSandbox::createSession();
		const auto directory = root.path() + "/parallel-runtime";
		QVERIFY(QDir().mkpath(directory));
		QVERIFY(write(directory + "/resource.txt", "shared runtime"));
		auto first = config("runtime " + quoteArgument(directory + "/resource.txt"));
		first.sandboxSession = session;
		auto second = config(first.arguments);
		second.sandboxSession = session;
		first.sandboxSettings.readOnlyDirectories = {directory};
		second.sandboxSettings = first.sandboxSettings;
		ProcessRunnerResult one, two;
		std::thread worker([&] { one = run(first); });
		two = run(second);
		worker.join();
		QVERIFY2(one.result == CorrectAnswer, qPrintable(one.message));
		QVERIFY2(two.result == CorrectAnswer, qPrintable(two.message));
		QVERIFY(one.runtimeCacheHit != two.runtimeCacheHit);
		QVERIFY((one.runtimeAclUpdates == 0) != (two.runtimeAclUpdates == 0));
	}

	void networkDenied() {
		WSADATA data{};
		QCOMPARE(WSAStartup(MAKEWORD(2, 2), &data), 0);
		SOCKET server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in address{};
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
		QCOMPARE(bind(server, reinterpret_cast<sockaddr *>(&address), sizeof(address)), 0);
		QCOMPARE(listen(server, 5), 0);
		int size = sizeof(address);
		getsockname(server, reinterpret_cast<sockaddr *>(&address), &size);
		SOCKET control = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		QCOMPARE(::connect(control, reinterpret_cast<sockaddr *>(&address), sizeof(address)), 0);
		SOCKET accepted = accept(server, nullptr, nullptr);
		closesocket(accepted);
		closesocket(control);

		const auto result = run(config("network " + QString::number(ntohs(address.sin_port))));
		closesocket(server);
		WSACleanup();
		QVERIFY2(result.result == CorrectAnswer, qPrintable(result.message));
	}

	void childProcessDenied_data() {
		QTest::addColumn<int>("runtime");
		QTest::newRow("automatic") << int(SandboxSettings::Automatic);
		QTest::newRow("native") << int(SandboxSettings::Native);
		QTest::newRow("java") << int(SandboxSettings::Java);
		QTest::newRow("python") << int(SandboxSettings::Python);
	}

	void childProcessDenied() {
		QFETCH(int, runtime);
		auto cfg = config("deny-child");
		configureRuntimeFixture(cfg, SandboxSettings::Runtime(runtime));

		const auto result = run(cfg);
		QVERIFY2(result.result == CorrectAnswer, qPrintable(result.message));
		QVERIFY(read(cfg.workingDirectory + "_tmpout").startsWith("child-denied="));
	}

	void primaryProcessAccounting_data() {
		QTest::addColumn<bool>("enabled");
		QTest::newRow("ordinary") << false;
		QTest::newRow("sandbox") << true;
	}

	void primaryProcessAccounting() {
		QFETCH(bool, enabled);
		auto cfg = config("metrics 600");
		cfg.sandboxSettings.enabled = enabled;
		ProcessRunnerResult result;
		std::thread worker([&] { result = run(cfg); });
		Handle primary;
		QElapsedTimer timer;
		timer.start();
		while (! primary && timer.elapsed() < 5000) {
			const auto match = QRegularExpression("process=(\\d+)")
			                       .match(QString::fromUtf8(read(cfg.workingDirectory + "_tmpout")));
			if (match.hasMatch()) {
				primary.reset(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE, FALSE,
				                          match.captured(1).toULong()));
			}
			if (! primary)
				QThread::msleep(10);
		}
		worker.join();
		QVERIFY2(result.result == CorrectAnswer, qPrintable(result.message));
		QVERIFY(bool(primary));
		FILETIME created{}, exited{}, kernel{}, user{};
		QVERIFY(GetProcessTimes(primary.get(), &created, &exited, &kernel, &user));
		const quint64 userTicks = (quint64(user.dwHighDateTime) << 32) | user.dwLowDateTime;
		const quint64 kernelTicks = (quint64(kernel.dwHighDateTime) << 32) | kernel.dwLowDateTime;
		QVERIFY(userTicks > 0);
		QVERIFY(kernelTicks > 0);
		QCOMPARE(result.timeUsed, int(userTicks / 10000));
		PROCESS_MEMORY_COUNTERS memory{};
		memory.cb = sizeof(memory);
		QVERIFY(GetProcessMemoryInfo(primary.get(), &memory, sizeof(memory)));
		QCOMPARE(result.memoryUsed, qint64(memory.PeakWorkingSetSize));
		QVERIFY(result.memoryUsed < 64ll * 1024 * 1024);
	}

	void runtimeErrorDetails_data() { primaryProcessAccounting_data(); }
	void runtimeErrorDetails() {
		QFETCH(bool, enabled);
		auto cfg = config("stderr");
		cfg.sandboxSettings.enabled = enabled;

		const auto result = run(cfg);
		QCOMPARE(result.result, RunTimeError);
		QCOMPARE(result.timeUsed, -1);
		QCOMPARE(result.memoryUsed, -1);
		QCOMPARE(result.message, QString(1020, 'a') + "tail");
	}

	void limits() {
		auto cpu = config("burn 3000");
		cpu.timeLimit = 100;
		QCOMPARE(run(cpu).result, TimeLimitExceeded);
		auto memory = config("memory");
		memory.memoryLimit = 24;
		QCOMPARE(run(memory).result, MemoryLimitExceeded);
	}

	void cancellation() {
		auto cfg = config("burn 10000");
		const auto directory = root.path() + "/cancellation-runtime";
		QVERIFY(QDir().mkpath(directory));
		const auto resource = directory + "/resource.txt";
		QVERIFY(write(resource, "runtime resource"));
		cfg.sandboxSettings.readOnlyDirectories = {directory};
		const auto beforeRuntime = permissions(resource);
		std::atomic<bool> stop{false};
		Handle primary;
		std::thread cancel([&] {
			QElapsedTimer timer;
			timer.start();
			while (! primary && timer.elapsed() < 5000) {
				const auto match = QRegularExpression("process=(\\d+)")
				                       .match(QString::fromUtf8(read(cfg.workingDirectory + "_tmpout")));
				if (match.hasMatch())
					primary.reset(OpenProcess(SYNCHRONIZE, FALSE, match.captured(1).toULong()));
				if (! primary)
					QThread::msleep(10);
			}
			stop = true;
		});
		const auto result = ProcessRunner::create(cfg, stop)->run();
		cancel.join();
		QVERIFY(bool(primary));
		QCOMPARE(WaitForSingleObject(primary.get(), 0), DWORD(WAIT_OBJECT_0));
		QCOMPARE(permissions(resource), beforeRuntime);
		QVERIFY(! permissions(cfg.workingDirectory).contains("S-1-15-2-"));
		QCOMPARE(result.result, CorrectAnswer);
		QCOMPARE(result.timeUsed, -1);
		QCOMPARE(result.memoryUsed, -1);
	}

	void runtimeDirectoryWithInput() {
		auto cfg = config("sum");
		const auto directory = root.path() + "/runtime-with-input";
		QVERIFY(QDir().mkpath(directory));
		QVERIFY(QFile::copy(cfg.inputFile, directory + "/input.txt"));
		cfg.inputFile = directory + "/input.txt";
		cfg.sandboxSettings.readOnlyDirectories = {directory};
		QCOMPARE(run(cfg).result, CorrectAnswer);
		QCOMPARE(read(cfg.workingDirectory + "_tmpout").trimmed(), QByteArray("42"));
	}

	void rejectHardLinkedWorkFile() {
		auto cfg = config("sum");
		const auto target = root.path() + "/secret.txt";
		QVERIFY(CreateHardLinkW(wide(cfg.workingDirectory + "linked.txt"), wide(target), nullptr));
		const auto beforeDirectory = permissions(cfg.workingDirectory);
		const auto beforeExecutable = permissions(cfg.executableFile);

		const auto result = run(cfg);
		QCOMPARE(result.result, CannotStartProgram);
		QCOMPARE(read(target), QByteArray("secret answer\n"));
		QCOMPARE(permissions(cfg.workingDirectory), beforeDirectory);
		QCOMPARE(permissions(cfg.executableFile), beforeExecutable);
	}

	void preparationDeadlineIgnoresStopFlag() {
		auto cfg = config("sum");
		cfg.runtimeExecutable = root.path() + "/slow-python.exe";
		QVERIFY(QFile::copy(QDir::currentPath() + "/sandbox-helper.exe", cfg.runtimeExecutable));
		cfg.sandboxSettings.runtime = SandboxSettings::Python;
		cfg.sandboxSettings.preparationTimeLimit = 250;
		const auto before = permissions(cfg.runtimeExecutable);
		const auto limited = run(cfg);
		QCOMPARE(limited.result, CannotStartProgram);
		QVERIFY(limited.message.contains("preparation exceeded"));
		const auto marker = cfg.runtimeExecutable + ".started";
		QVERIFY(! QFileInfo::exists(marker) || QFile::remove(marker));
		std::atomic<bool> stop{false};
		cfg.sandboxSettings.preparationTimeLimit = 1000;
		DWORD probeId = 0;
		std::thread cancel([&] {
			QElapsedTimer timer;
			timer.start();
			while (! probeId && timer.elapsed() < 5000) {
				probeId = read(marker).trimmed().toULong();
				if (! probeId)
					QThread::msleep(10);
			}
			stop = true;
		});
		const auto limitedWithStop = ProcessRunner::create(cfg, stop)->run();
		cancel.join();
		QVERIFY(probeId != 0);
		QCOMPARE(limitedWithStop.result, CannotStartProgram);
		QVERIFY(limitedWithStop.message.contains("preparation exceeded"));
		QVERIFY(limitedWithStop.preparationTime >= cfg.sandboxSettings.preparationTimeLimit);
		QCOMPARE(limitedWithStop.timeUsed, -1);
		QCOMPARE(limitedWithStop.runtimeAclUpdates, 0);
		QCOMPARE(permissions(cfg.runtimeExecutable), before);
		Handle probe(OpenProcess(SYNCHRONIZE, FALSE, probeId));
		QVERIFY(! probe || WaitForSingleObject(probe.get(), 0) == WAIT_OBJECT_0);
	}

	void parallelIsolation() {
		auto first = config({});
		auto second = config({});
		QVERIFY(write(first.workingDirectory + "private.txt", "first secret"));
		QVERIFY(write(second.workingDirectory + "private.txt", "second secret"));
		first.arguments = "isolation " + quoteArgument(second.workingDirectory + "private.txt");
		second.arguments = "isolation " + quoteArgument(first.workingDirectory + "private.txt");
		ProcessRunnerResult one, two;
		std::thread worker([&] { one = run(first); });
		two = run(second);
		worker.join();
		QVERIFY2(one.result == CorrectAnswer, qPrintable(one.message));
		QVERIFY2(two.result == CorrectAnswer, qPrintable(two.message));
		QCOMPARE(read(first.workingDirectory + "_tmpout").trimmed(), QByteArray("isolated"));
		QCOMPARE(read(second.workingDirectory + "_tmpout").trimmed(), QByteArray("isolated"));
	}

	void nativeCompiler_data() {
		QTest::addColumn<QString>("compiler");
		QTest::addColumn<QString>("extension");
		QTest::newRow("C") << "gcc" << "c";
		QTest::newRow("C++") << "g++" << "cpp";
	}

	void nativeCompiler() {
		QFETCH(QString, compiler);
		QFETCH(QString, extension);
		const auto tool = QStandardPaths::findExecutable(compiler);
		if (tool.isEmpty())
			QSKIP("Native compiler not installed");
		auto cfg = config({});
		cfg.runtimeExecutable = tool;
		cfg.executableFile = cfg.workingDirectory + "native.exe";
		const auto source = cfg.workingDirectory + "main." + extension;
		QVERIFY(write(
		    source,
		    extension == "c"
		        ? "#include <stdio.h>\nint main(void){int a,b;scanf(\"%d%d\",&a,&b);printf(\"%d\\n\",a+b);}"
		        : "#include <iostream>\nint main(){int a,b;std::cin>>a>>b;std::cout<<a+b<<'\\n';}"));
		QProcess compile;
		compile.start(tool, {source, "-O2", "-o", cfg.executableFile});
		QVERIFY(compile.waitForFinished(20000));
		QVERIFY2(compile.exitCode() == 0, compile.readAllStandardError().constData());

		const auto result = run(cfg);
		QVERIFY2(result.result == CorrectAnswer, qPrintable(result.message));
		QCOMPARE(read(cfg.workingDirectory + "_tmpout").trimmed(), QByteArray("42"));
	}

	void settingsRoundTrip() {
		SandboxSettings settings;
		QVERIFY(! settings.enabled);
		settings.enabled = true;
		settings.runtime = SandboxSettings::Python;
		settings.readOnlyDirectories = {"C:/runtime"};
		settings.preparationTimeLimit = 30000;
		QJsonObject json;
		settings.write(json);
		SandboxSettings restored;
		restored.read(json);
		QVERIFY(restored.enabled);
		QCOMPARE(restored.runtime, settings.runtime);
		QCOMPARE(restored.readOnlyDirectories, settings.readOnlyDirectories);
		QCOMPARE(restored.preparationTimeLimit, settings.preparationTimeLimit);
		restored.read({{"runtime", -1}, {"preparationTimeLimit", 0}});
		QVERIFY(! restored.enabled);
		QCOMPARE(restored.runtime, SandboxSettings::Automatic);
		QCOMPARE(restored.preparationTimeLimit, 1000);
	}

	void pythonRuntime() {
		auto session = WindowsSandbox::createSession();
		if (python.isEmpty())
			QSKIP("Python not installed");
		const auto interpreter = basePythonExecutable();
		QVERIFY2(! interpreter.isEmpty(), "Cannot locate the base Python interpreter");
		auto cfg = config({});
		cfg.sandboxSession = session;
		cfg.executableFile = cfg.runtimeExecutable = interpreter;
		cfg.arguments = "answer.py";
		cfg.sandboxSettings.runtime = SandboxSettings::Python;
		cfg.runtimeEnvironment.insert("PYTHONIOENCODING", "cp1252");
		QVERIFY(write(cfg.workingDirectory + "answer.py",
		              "import sys,math,json,tempfile,ctypes\n"
		              "assert not sys.dont_write_bytecode and sys.flags.no_user_site == 0\n"
		              "assert ctypes.windll.kernel32.GetCurrentProcessId() > 0\n"
		              "with tempfile.TemporaryFile() as f: f.write(b'private')\n"
		              "print(sum(map(int,sys.stdin.read().split())), end=' ')\n"
		              "print('\\u00e9', end='')\n"));

		const auto first = run(cfg);
		QVERIFY2(first.result == CorrectAnswer, qPrintable(first.message));
		QCOMPARE(read(cfg.workingDirectory + "_tmpout"), QByteArray("42 \xe9"));

		const auto second = run(cfg);
		QVERIFY2(second.result == CorrectAnswer, qPrintable(second.message));
		QVERIFY(second.runtimeCacheHit);
		QCOMPARE(second.runtimeAclUpdates, 0);
		qInfo() << "Python preparation cold/warm ms:" << first.preparationTime << second.preparationTime;
	}

	void pythonChildDenied() {
		if (python.isEmpty())
			QSKIP("Python not installed");
		const auto interpreter = basePythonExecutable();
		QVERIFY2(! interpreter.isEmpty(), "Cannot locate the base Python interpreter");
		auto cfg = config("child.py");
		cfg.executableFile = cfg.runtimeExecutable = interpreter;
		cfg.sandboxSettings.runtime = SandboxSettings::Python;
		QVERIFY(write(cfg.workingDirectory + "child.py",
		              "import subprocess,sys\n"
		              "try:\n"
		              "    child = subprocess.Popen([sys.executable, '-c', 'pass'])\n"
		              "except OSError:\n"
		              "    print('denied')\n"
		              "else:\n"
		              "    assert child.wait(timeout=5) == 0\n"
		              "    print('allowed')\n"));
		cfg.sandboxSettings.enabled = false;

		const auto ordinary = run(cfg);
		QVERIFY2(ordinary.result == CorrectAnswer, qPrintable(ordinary.message));
		QCOMPARE(read(cfg.workingDirectory + "_tmpout").trimmed(), QByteArray("allowed"));
		cfg.sandboxSettings.enabled = true;

		const auto restricted = run(cfg);
		QVERIFY2(restricted.result == CorrectAnswer, qPrintable(restricted.message));
		QCOMPARE(read(cfg.workingDirectory + "_tmpout").trimmed(), QByteArray("denied"));
	}

	void pythonVenv() {
		if (python.isEmpty())
			QSKIP("Python not installed");
		const auto directory = root.path() + "/venv";
		QProcess create;
		create.start(python, {"-m", "venv", "--without-pip", directory});
		QVERIFY(create.waitForFinished(20000));
		QCOMPARE(create.exitCode(), 0);
		auto cfg = config("answer.py");
		cfg.executableFile = cfg.runtimeExecutable = directory + "/Scripts/python.exe";
		cfg.sandboxSettings.runtime = SandboxSettings::Python;
		QVERIFY(write(cfg.workingDirectory + "answer.py",
		              "import sys\nassert sys.prefix != sys.base_prefix\nprint(42)\n"));
		cfg.sandboxSettings.enabled = false;

		const auto ordinary = run(cfg);
		QVERIFY2(ordinary.result == CorrectAnswer, qPrintable(ordinary.message));
		QCOMPARE(read(cfg.workingDirectory + "_tmpout").trimmed(), QByteArray("42"));
		cfg.sandboxSettings.enabled = true;

		const auto restricted = run(cfg);
		QCOMPARE(restricted.result, RunTimeError);
		QVERIFY2(restricted.message.contains("Unable to create process", Qt::CaseInsensitive),
		         qPrintable(restricted.message));
		QVERIFY(read(cfg.workingDirectory + "_tmpout").isEmpty());
	}

	void javaRuntime() {
		auto session = WindowsSandbox::createSession();
		if (java.isEmpty() || javac.isEmpty())
			QSKIP("JDK not installed");
		auto cfg = config("-Xmx64m -XX:-UsePerfData Main");
		cfg.sandboxSession = session;
		cfg.executableFile = cfg.runtimeExecutable = java;
		cfg.sandboxSettings.runtime = SandboxSettings::Java;
		cfg.memoryLimit = 512;
		QVERIFY(write(cfg.workingDirectory + "Main.java",
		              "import java.util.*; class Main { public static void main(String[] args) { "
		              "Scanner s = new Scanner(System.in); System.out.println(s.nextInt()+s.nextInt()); }}"));
		QProcess compile;
		compile.start(javac, {cfg.workingDirectory + "Main.java"});
		QVERIFY(compile.waitForFinished(20000));
		QCOMPARE(compile.exitCode(), 0);

		const auto first = run(cfg);
		QVERIFY2(first.result == CorrectAnswer, qPrintable(first.message));
		QCOMPARE(read(cfg.workingDirectory + "_tmpout").trimmed(), QByteArray("42"));

		const auto second = run(cfg);
		QVERIFY2(second.result == CorrectAnswer, qPrintable(second.message));
		QCOMPARE(second.runtimeAclUpdates, 0);
		qInfo() << "Java preparation cold/warm ms:" << first.preparationTime << second.preparationTime;
	}

	void javaChildDenied() {
		if (java.isEmpty() || javac.isEmpty())
			QSKIP("JDK not installed");
		auto cfg = config("-Xmx64m -XX:-UsePerfData Main");
		cfg.executableFile = cfg.runtimeExecutable = java;
		cfg.sandboxSettings.runtime = SandboxSettings::Java;
		cfg.memoryLimit = 512;
		QVERIFY(
		    write(cfg.workingDirectory + "Main.java",
		          "import java.io.*; class Main { public static void main(String[] args) throws Exception { "
		          "Process child; try { child = new ProcessBuilder("
		          "new File(\"helper.exe\").getAbsolutePath(), \"burn\", \"100\").start(); } "
		          "catch (IOException e) { System.out.println(\"denied\"); return; } "
		          "if (child.waitFor() != 0) throw new AssertionError(\"Child failed\"); "
		          "System.out.println(\"allowed\"); }}"));
		QProcess compile;
		compile.start(javac, {cfg.workingDirectory + "Main.java"});
		QVERIFY(compile.waitForFinished(20000));
		QCOMPARE(compile.exitCode(), 0);
		cfg.sandboxSettings.enabled = false;

		const auto ordinary = run(cfg);
		QVERIFY2(ordinary.result == CorrectAnswer, qPrintable(ordinary.message));
		QCOMPARE(read(cfg.workingDirectory + "_tmpout").trimmed(), QByteArray("allowed"));
		cfg.sandboxSettings.enabled = true;

		const auto restricted = run(cfg);
		QVERIFY2(restricted.result == CorrectAnswer, qPrintable(restricted.message));
		QCOMPARE(read(cfg.workingDirectory + "_tmpout").trimmed(), QByteArray("denied"));
	}
};

QTEST_GUILESS_MAIN(WindowsSandboxTests)
#include "main.moc"
