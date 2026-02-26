/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef Q_OS_WIN32

#include "processrunner_unix.h"
#include "base/LemonLog.hpp"

#include <QCoreApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>
#include <QTextStream>
#include <QThread>
#include <QUuid>
#include <QtMath>

#define LEMON_MODULE_NAME "ProcessRunner"

ProcessRunnerResult UnixProcessRunner::run() {
	ProcessRunnerResult res;
	res.result = CorrectAnswer;
	int extraTime = qCeil(qMax(2000, config.timeLimit * 2) * config.extraTimeRatio);

	if (skipFlag) {
		res.result = TimeLimitExceeded;
		res.score = 0;
		return res;
	}

#ifdef Q_OS_LINUX
	// TODO: rewrite with cgroup
	QFile watcher(config.workingDirectory + QUuid::createUuid().toString(QUuid::Id128));

	if (config.interpreterAsWatcher) {
		QFile::copy(config.executableFile, watcher.fileName());
	} else {
		QFile::copy(":/watcher/watcher_unix", watcher.fileName());
	}

	watcher.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
	auto *runner = new QProcess();
	QStringList argumentsList;

	argumentsList << "--dev" << "/dev";
	argumentsList << "--proc" << "/proc";
	argumentsList << "--ro-bind" << "/usr" << "/usr";
	argumentsList << "--symlink" << "/usr/lib" << "/lib";
	argumentsList << "--symlink" << "/usr/lib64" << "/lib64";
	argumentsList << "--symlink" << "/usr/bin" << "/bin";
	argumentsList << "--symlink" << "/usr/sbin" << "/sbin";
	argumentsList << "--tmpfs" << "/tmp";

	argumentsList << "--unshare-all" << "--die-with-parent";

	argumentsList << "--chdir" << config.workingDirectory;

	argumentsList << "--bind" << config.workingDirectory << config.workingDirectory;

	if (config.standardInputCheck) {
		argumentsList << "--ro-bind" << QFileInfo(config.inputFile).absoluteFilePath()
		              << QFileInfo(config.inputFile).absoluteFilePath();
	}

	argumentsList << watcher.fileName();

	argumentsList << config.executableFile;
	argumentsList << config.arguments;

	if (config.standardInputCheck) {
		argumentsList << QFileInfo(config.inputFile).absoluteFilePath();
	} else {
		argumentsList << "";
	}

	if (config.standardOutputCheck) {
		argumentsList << "_tmpout";
	} else {
		argumentsList << "";
	}

	argumentsList << "_tmperr";
	argumentsList << QString("%1").arg(config.timeLimit);
	argumentsList << QString("%1").arg(config.memoryLimit);
	argumentsList << QString("%1").arg(config.rawTimeLimit);
	argumentsList << QString("%1").arg(config.rawMemoryLimit);

	if (config.standardInputCheck) {
		argumentsList << "";
	} else {
		argumentsList << config.inputFileName;
	}

	if (config.standardOutputCheck) {
		argumentsList << "";
	} else {
		argumentsList << config.outputFileName;
	}

	qDebug() << argumentsList;

	QString bwrapPath = QStandardPaths::findExecutable("bwrap");
	if (bwrapPath.isEmpty()) {
		res.score = 0;
		res.result = CannotStartProgram;
		res.message = QObject::tr("bwrap not found. Please install bubblewrap.");
		delete runner;
		return res;
	}

	runner->setProcessEnvironment(config.environment);
	runner->setWorkingDirectory(config.workingDirectory);
	runner->start(bwrapPath, argumentsList);

#else

	QFile watcher(config.workingDirectory + QUuid::createUuid().toString(QUuid::Id128));

	if (config.interpreterAsWatcher) {
		QFile::copy(config.executableFile, watcher.fileName());
	} else {
		QFile::copy(":/watcher/watcher_unix", watcher.fileName());
	}

	watcher.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
	auto *runner = new QProcess();
	QStringList argumentsList;

	argumentsList << config.executableFile;
	argumentsList << config.arguments;

	if (config.standardInputCheck) {
		argumentsList << QFileInfo(config.inputFile).absoluteFilePath();
	} else {
		argumentsList << "";
	}

	if (config.standardOutputCheck) {
		argumentsList << "_tmpout";
	} else {
		argumentsList << "";
	}

	argumentsList << "_tmperr";
	argumentsList << QString("%1").arg(config.timeLimit);
	argumentsList << QString("%1").arg(config.memoryLimit);
	argumentsList << QString("%1").arg(config.rawTimeLimit);
	argumentsList << QString("%1").arg(config.rawMemoryLimit);

	if (config.standardInputCheck) {
		argumentsList << "";
	} else {
		argumentsList << config.inputFileName;
	}

	if (config.standardOutputCheck) {
		argumentsList << "";
	} else {
		argumentsList << config.outputFileName;
	}

	qDebug() << argumentsList;

	runner->setProcessEnvironment(config.environment);
	runner->setWorkingDirectory(config.workingDirectory);
	runner->start(watcher.fileName(), argumentsList);

#endif

	if (! runner->waitForStarted(-1)) {
		delete runner;
		res.score = 0;
		res.result = CannotStartProgram;
		res.message = "Start runner failed";
		return res;
	}

	bool isProgramFinishedInExtraTimeLimit = false;
	QElapsedTimer timer;
	timer.start();

	// Using rlimit to limit CPU time can only be accurate to seconds,
	// so here it is rounded up to an integer second.
	long long killTimeLimit = (config.timeLimit + 999) / 1000 * 1000 + extraTime;
	while (timer.elapsed() <= killTimeLimit) {
		if (runner->state() != QProcess::Running) {
			isProgramFinishedInExtraTimeLimit = true;
			break;
		}

		QCoreApplication::processEvents();

		if (stopFlag || skipFlag) {
			runner->terminate();
			runner->waitForFinished(-1);
			delete runner;

			if (skipFlag) {
				res.score = 0;
				res.result = TimeLimitExceeded;
				res.timeUsed = res.memoryUsed = -1;
			}

			return res;
		}

		QThread::msleep(10);
	}

	if (! isProgramFinishedInExtraTimeLimit) {
		runner->terminate();
		runner->waitForFinished(-1);
		delete runner;
		res.score = 0;
		res.timeUsed = res.memoryUsed = -1;
		// Watcher usually needs to handle the situation of program timeout and kill it. Therefore, it is
		// abnormal for watcher to timeout itself, and report FAIL instead of TLE.
		res.result = CannotStartProgram;
		res.message = "Watcher time limit exceeded";
		return res;
	}

	{
		QString out = QString::fromLocal8Bit(runner->readAllStandardOutput().constData());
		QTextStream stream(&out, QIODevice::ReadOnly);
		stream >> res.timeUsed >> res.memoryUsed;
	}

	res.message = QString::fromLocal8Bit(runner->readAllStandardError().constData());

	enum : int {
		RS_AC = 0,
		RS_FAIL = 1,
		RS_RE = 2,
		RS_TLE = 3,
		RS_MLE = 4,
	};

	int code = runner->exitCode();

	switch (code) {
		case RS_RE:
			res.result = RunTimeError;
			res.score = 0;
			[[fallthrough]];
		case RS_AC: {
			QFile file(config.workingDirectory + "_tmperr");
			if (file.open(QFile::ReadOnly)) {
				res.message += file.readAll().right(1024);
				file.close();
			}
		} break;
		case RS_FAIL:
			res.result = CannotStartProgram;
			res.score = 0;
			break;
		case RS_TLE:
			res.result = TimeLimitExceeded;
			res.score = 0;
			break;
		case RS_MLE:
			res.result = MemoryLimitExceeded;
			res.score = 0;
			break;
		default:
			res.result = CannotStartProgram;
			res.score = 0;
			res.message = QString("Watcher reported an invalid result: %1").arg(code);
			break;
	}

	delete runner;

	return res;
}

#endif
