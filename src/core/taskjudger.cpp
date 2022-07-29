/*
 * SPDX-FileCopyrightText: 2021-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */
#include "core/taskjudger.h"
#include "base/LemonLog.hpp"
#include "base/LemonType.hpp"
#include "base/compiler.h"
#include "base/settings.h"
#include "core/contestant.h"
#include "core/judgesharedvariables.h"
#include "core/judgingthread.h"
#include "core/subtaskdependencelib.h"
#include "core/task.h"
#include "core/testcase.h"

#include <QSysInfo>
#include <QTimer>
#include <QtMath>

#include <algorithm>
#include <utility>

#define LEMON_MODULE_NAME "TaskJudger"

TaskJudger::TaskJudger(QObject *parent) : QObject(parent) { compileState = NoValidSourceFile; }

void TaskJudger::setSettings(Settings *_settings) { settings = _settings; }

void TaskJudger::setTask(Task *_task) { task = _task; }

void TaskJudger::setTaskId(int id) { taskId = id; }

void TaskJudger::setContestant(Contestant *contestant) { this->contestant = contestant; }

Contestant *TaskJudger::getContestant() const { return contestant; }

// Get executable file
auto TaskJudger::traditionalTaskPrepare() -> bool {
	makeDialogAlert(tr("Preparing..."));

	// Get the source code of contestant
	compileState = NoValidSourceFile;
	QString contestantName = contestant->getContestantName();
	QDir contestantDir;
	contestantDir =
	    ! task->getSubFolderCheck()
	        ? QDir(Settings::sourcePath() + contestantName)
	        : QDir(Settings::sourcePath() + contestantName + QDir::separator() + task->getSourceFileName());
	QString contestantDirName = contestantDir.path();
	QList<Compiler *> compilerList = settings->getCompilerList();

	for (auto &i : compilerList) {
		if (task->getCompilerConfiguration(i->getCompilerName()) == "disable")
			continue;

		QStringList filters;

		if (task->getTaskType() == Task::Communication || task->getTaskType() == Task::CommunicationExec) {
			filters = task->getSourceFilesPath();
		} else {
			filters = i->getSourceExtensions();

			for (int j = 0; j < filters.size(); j++) {
				filters[j] = task->getSourceFileName() + "." + filters[j];
			}
		}

		QStringList files = contestantDir.entryList(filters, QDir::Files);
		sourceFile = "";

		for (int j = 0; j < files.size(); j++) {
			qint64 fileSize = QFileInfo(contestantDirName + QDir::separator() + files[j]).size();
			// Refuse to compile if the source file is too large
			if (fileSize <= settings->getFileSizeLimit() * 1024) {
				if (task->getTaskType() == Task::Communication ||
				    task->getTaskType() == Task::CommunicationExec) {
					sourceFile = sourceFile + " " + files[j] + " ";
				} else {
					sourceFile = files[j];
					break;
				}
			}
		}

		QStringList sourcePaths;
		QStringList sourceNames;
		QStringList graderPaths;
		QStringList graderNames;
		QString mainGraderPath;
		QString mainGraderName;

		if (sourceFile.isEmpty())
			continue;
		QDir(QDir::toNativeSeparators(temporaryDir.path()) + QDir::separator()).mkdir(contestantName);
		// Copy files to temporary dir
		if (task->getTaskType() == Task::Communication || task->getTaskType() == Task::CommunicationExec) {
			sourceFile = "";
			sourcePaths = task->getSourceFilesPath();
			sourceNames = task->getSourceFilesName();

			for (int i = 0; i < sourcePaths.length(); i++) {
				QFile::copy(contestantDirName + QDir::separator() + sourcePaths[i],
				            QDir::toNativeSeparators(temporaryDir.path()) + QDir::separator() +
				                contestantName + QDir::separator() + sourceNames[i]);
				sourceFile = sourceFile + " " + sourceNames[i] + " ";
			}
		} else {
			QFile::copy(contestantDirName + QDir::separator() + sourceFile,
			            QDir::toNativeSeparators(temporaryDir.path()) + QDir::separator() + contestantName +
			                QDir::separator() + sourceFile);
		}

		QString extraFiles = "";

		if (task->getTaskType() == Task::Interaction) {
			QFile::copy(Settings::dataPath() + task->getInteractor(),
			            QDir::toNativeSeparators(temporaryDir.path()) + QDir::separator() + contestantName +
			                QDir::separator() + task->getInteractorName());
			QFile::copy(Settings::dataPath() + task->getGrader(),
			            QDir::toNativeSeparators(temporaryDir.path()) + QDir::separator() + contestantName +
			                QDir::separator() + "__grader.cpp");
		}

		if (task->getTaskType() == Task::Communication || task->getTaskType() == Task::CommunicationExec) {
			graderPaths = task->getGraderFilesPath();
			graderNames = task->getGraderFilesName();

			for (int i = 0; i < graderPaths.length(); i++) {
				QFile::copy(Settings::dataPath() + graderPaths[i],
				            QDir::toNativeSeparators(temporaryDir.path()) + QDir::separator() +
				                contestantName + QDir::separator() + graderNames[i]);
				extraFiles = extraFiles + " " + graderNames[i] + " ";
			}
		}

		// Get compiler configuration
		QStringList configurationNames = i->getConfigurationNames();
		QStringList compilerArguments = i->getCompilerArguments();
		QStringList interpreterArguments = i->getInterpreterArguments();
		QString currentConfiguration = task->getCompilerConfiguration(i->getCompilerName());

		int configurationIndex =
		    std::find(configurationNames.begin(), configurationNames.end(), currentConfiguration) -
		    configurationNames.begin();

		compilerTimeLimitRatio = i->getTimeLimitRatio();
		compilerMemoryLimitRatio = i->getMemoryLimitRatio();
		disableMemoryLimitCheck = i->getDisableMemoryLimitCheck();
		environment = i->getEnvironment();
		QStringList values = QProcessEnvironment::systemEnvironment().toStringList();

		for (int k = 0; k < values.size(); k++) {
			int tmp = values[k].indexOf("=");

			if (tmp == 0)
				continue;

			QString variable = values[k].mid(0, tmp);
			if (environment.contains(variable))
				// ';' for windows ':' for linux
				environment.insert(variable, environment.value(variable) +
#ifdef Q_OS_WIN32
				                                 ";"
#else
				                                 ":"
#endif
				                                 + QProcessEnvironment::systemEnvironment().value(variable));
			else
				environment.insert(variable, QProcessEnvironment::systemEnvironment().value(variable));
		}

		if (i->getCompilerType() == Compiler::Typical) {
			if (task->getTaskType() == Task::CommunicationExec)
				executableFile = commExecGrader;
			else
				executableFile = task->getSourceFileName();
#ifdef Q_OS_WIN32
			executableFile.append(".exe");
#endif
			interpreterFlag = false;
		} else {
			executableFile = i->getInterpreterLocation();
			arguments = interpreterArguments[configurationIndex];
			arguments.replace("%s.*", sourceFile + extraFiles);
			arguments.replace("%s", task->getSourceFileName());
			interpreterFlag = true;
		}

		if (i->getCompilerType() != Compiler::InterpretiveWithoutByteCode) {
			makeDialogAlert(tr("Compiling..."));

			QStringList arguments;
			arguments.append(compilerArguments[configurationIndex]);

			if (task->getTaskType() == Task::Interaction) {
				arguments[0].replace("%s.*", sourceFile + " __grader.cpp");
				arguments[0].replace("%s", task->getSourceFileName());
			} else if (task->getTaskType() == Task::Communication) {
				arguments[0].replace("%s.*", sourceFile + extraFiles);
				arguments[0].replace("%s", task->getSourceFileName());
			} else if (task->getTaskType() == Task::CommunicationExec) {
				for (int k = 1; k < sourceNames.size(); k++)
					arguments.append(compilerArguments[configurationIndex]);
				for (int k = 0; k < sourceNames.size(); k++) {
					arguments[k].replace("%s.*", sourceNames[k]);
					QString name = sourceNames[k];
					name.truncate(name.lastIndexOf('.'));
					arguments[k].replace("%s", name);
				}

				QStringList filters = i->getSourceExtensions();
				for (auto &k : filters)
					k = commExecGrader + "." + k;
				bool found = 0;
				for (auto k : graderPaths) {
					QString name = k.section(QDir::separator(), -1);
					if (filters.contains(name)) {
						mainGraderPath = k;
						mainGraderName = name;
						found = 1;
						break;
					}
				}
				if (! found) {
					compileState = NoValidGraderFile;
					compileMessage = tr("Main grader (grader.*) cannot be found");
					break;
				}
				// Why? It's unix Only
				auto graderArgument = compilerArguments[configurationIndex] + " -pthread";
				arguments.append(graderArgument);
				graderArgument.replace("%s.*", mainGraderName);
				graderArgument.replace("%s", commExecGrader);
				arguments.append(graderArgument);
			} else {
				arguments[0].replace("%s.*", sourceFile);
				arguments[0].replace("%s", task->getSourceFileName());
			}

			for (auto k : arguments) {
				QProcess compilerProcess;
				compilerProcess.setProcessChannelMode(QProcess::MergedChannels);
				compilerProcess.setProcessEnvironment(environment);
				compilerProcess.setWorkingDirectory(QDir::toNativeSeparators(temporaryDir.path()) +
				                                    QDir::separator() + contestantName);
				// TODO: 需要重构代码来处理含空格路径问题

				compilerProcess.start(i->getCompilerLocation(),
				                      k.split(QLatin1Char(' '), Qt::SkipEmptyParts));

				if (! compilerProcess.waitForStarted(-1)) {
					compileState = InvalidCompiler;
					break;
				}

				QElapsedTimer timer;
				timer.start();
				bool flag = false;

				while (timer.elapsed() < settings->getCompileTimeLimit()) {
					if (compilerProcess.state() != QProcess::Running) {
						flag = true;
						break;
					}

					QCoreApplication::processEvents();

					if (! isJudging) {
						compilerProcess.kill();
						return false;
					}
					QThread::msleep(10);
				}

				if (! flag) {
					compilerProcess.kill();
					compileState = CompileTimeLimitExceeded;
				} else if (compilerProcess.exitCode() != 0) {
					compileState = CompileError;
					compileMessage =
					    QString::fromLocal8Bit(compilerProcess.readAllStandardOutput().constData());
				} else {
					if (i->getCompilerType() == Compiler::Typical) {
						if (! QDir(QDir::toNativeSeparators(temporaryDir.path()) + QDir::separator() +
						           contestantName)
						          .exists(executableFile)) {
							compileState = InvalidCompiler;
						} else {
							compileState = CompileSuccessfully;
						}
					} else {
						QStringList filters = i->getBytecodeExtensions();

						for (int k = 0; k < filters.size(); k++) {
							filters[k] = QString("*.") + filters[k];
						}

						if (QDir(QDir::toNativeSeparators(temporaryDir.path()) + QDir::separator() +
						         contestantName)
						        .entryList(filters, QDir::Files)
						        .empty()) {
							compileState = InvalidCompiler;
						} else {
							compileState = CompileSuccessfully;
						}
					}
				}
			}

			makeDialogAlert(tr("Compiled Successfully"));
		}

		if (i->getCompilerType() == Compiler::InterpretiveWithoutByteCode)
			compileState = CompileSuccessfully;

		// One file compiled, skip all other files
		break;
	}

	if (compileState != CompileSuccessfully) {
		emit compileError(task->getTotalTimeLimit(), static_cast<int>(compileState));
		return false;
	}

	return true;
}

void TaskJudger::judgeIt() {
	qDebug() << "Start Judging";
	emit judgingStarted(task->getProblemTitle());
	if (judge()) {
		contestant->setCheckJudged(taskId, true);
		contestant->setCompileMessage(taskId, compileMessage);
		contestant->setCompileState(taskId, compileState);
		contestant->setResult(taskId, result);
		contestant->setMessage(taskId, message);
		contestant->setTimeUsed(taskId, timeUsed);
		contestant->setMemoryUsed(taskId, memoryUsed);
		contestant->setScore(taskId, score);
		contestant->setInputFiles(taskId, inputFiles);
		contestant->setSourceFile(taskId, sourceFile);
	} else {
		contestant->setCheckJudged(taskId, false);
	}
	emit judgingFinished();
}

int TaskJudger::judge() {
	isJudging = true;
	QString contestantName = contestant->getContestantName();
	if (! temporaryDir.isValid())
		return 0;

	if (task->getTaskType() != Task::AnswersOnly)
		if (! traditionalTaskPrepare())
			return 1;

	for (int i = 0; i < task->getTestCaseList().size(); i++) {
		timeUsed.append(QList<int>());
		memoryUsed.append(QList<int>());
		score.append(QList<int>());
		result.append(QList<ResultState>());
		overallStatus.append(maxDependValue);
		message.append(QStringList());
		inputFiles.append(QStringList());
		testCaseScore.append(task->getTestCase(i)->getFullScore());

		for (int j = 0; j < task->getTestCase(i)->getInputFiles().size(); j++) {
			timeUsed[i].append(-1);
			memoryUsed[i].append(-1);
			score[i].append(0);
			result[i].append(Skipped);
			message[i].append("");
			inputFiles[i].append("");
		}
	}

	skipEnabled = false;
	for (int i = 0; i < task->getTestCaseList().size(); i++) {

		QCoreApplication::processEvents();
		if (! isJudging) {
			return 0;
		}

		auto curTestCase = task->getTestCase(i);
		const QList<int> &dependenceSubtask(curTestCase->getDependenceSubtask());

		overallStatus[i] = maxDependValue;

		bool isSkipped = false;

		for (int j = 0; j != dependenceSubtask.size(); ++j) {
			int status = overallStatus[dependenceSubtask[j] - 1];
			emit singleSubtaskDependenceFinished(i, dependenceSubtask[j], status);

			if (status < 0)
				isSkipped = true;

			overallStatus[i] = qMin(overallStatus[i], status);
		}

		if (! dependenceSubtask.empty())
			score[i].push_back(overallStatus[i]);

		if (isSkipped)
			continue;

		for (int j = 0; j < task->getTestCase(i)->getInputFiles().size(); j++) {
			inputFiles[i][j] = QFileInfo(curTestCase->getInputFiles().at(j)).fileName();

			testCaseScore[i] =
			    qMin(testCaseScore[i], statusToScore(overallStatus[i], curTestCase->getFullScore()));

			if (overallStatus[i] < 0 || isSkipped) {
				overallStatus[i] = -1;
				taskSkipped(std::make_pair(i, j++));
				break;
			}

			auto *thread = new JudgingThread();
			thread->setExtraTimeRatio(settings->getDefaultExtraTimeRatio());
			QString workingDirectory =
			    QDir::toNativeSeparators(QDir(QDir::toNativeSeparators(temporaryDir.path()) +
			                                  QDir::separator() + QString("_%1.%2").arg(i).arg(j))
			                                 .absolutePath()) +
			    QDir::separator();
			thread->setWorkingDirectory(workingDirectory);
			QDir(QDir::toNativeSeparators(temporaryDir.path()) + QDir::separator())
			    .mkdir(QString("_%1.%2").arg(i).arg(j));
			QStringList entryList =
			    QDir(QDir::toNativeSeparators(temporaryDir.path()) + QDir::separator() + contestantName)
			        .entryList(QDir::Files);

			for (int i = 0; i < entryList.size(); i++) {
				QFile::copy(QDir::toNativeSeparators(temporaryDir.path()) + QDir::separator() +
				                contestantName + QDir::separator() + entryList[i],
				            workingDirectory + entryList[i]);
			}

			thread->setSpecialJudgeTimeLimit(settings->getSpecialJudgeTimeLimit());
			thread->setDiffPath(settings->getDiffPath());

			if (task->getTaskType() == Task::Traditional || task->getTaskType() == Task::Interaction ||
			    task->getTaskType() == Task::Communication ||
			    task->getTaskType() == Task::CommunicationExec) {
				if (interpreterFlag) {
					thread->setExecutableFile(executableFile);
				} else {
					thread->setExecutableFile(workingDirectory + executableFile);
				}

				thread->setArguments(arguments);
			}

			if (task->getTaskType() == Task::AnswersOnly) {
				QString fileName;
				fileName = QFileInfo(curTestCase->getInputFiles().at(j)).completeBaseName();
				fileName += QString(".") + task->getAnswerFileExtension();

				if (! task->getSubFolderCheck())
					thread->setAnswerFile(Settings::sourcePath() + contestantName + QDir::separator() +
					                      fileName);
				else
					thread->setAnswerFile(Settings::sourcePath() + contestantName + QDir::separator() +
					                      task->getSourceFileName() + QDir::separator() + fileName);
			}

			thread->setTask(task);
			connect(this, &TaskJudger::stopJudgingSignal, thread, &JudgingThread::stopJudgingSlot);
			thread->setInputFile(Settings::dataPath() + curTestCase->getInputFiles().at(j));
			thread->setOutputFile(Settings::dataPath() + curTestCase->getOutputFiles().at(j));
			thread->setFullScore(curTestCase->getFullScore());

			if (task->getTaskType() != Task::AnswersOnly) {
				thread->setEnvironment(environment);
				thread->setTimeLimit(qCeil(curTestCase->getTimeLimit() * compilerTimeLimitRatio));

				if (disableMemoryLimitCheck) {
					thread->setMemoryLimit(-1);
				} else {
					thread->setMemoryLimit(qCeil(curTestCase->getMemoryLimit() * compilerMemoryLimitRatio));
				}
			}
			thread->start();
			thread->wait();

			QCoreApplication::processEvents();
			if (! isJudging) {
				delete thread;
				return 0;
			}

			while (thread->getNeedRejudge() && thread->getJudgeTimes() != settings->getRejudgeTimes() + 1 &&
			       isJudging) {
				thread->start();
				thread->wait();
				QCoreApplication::processEvents();
			}
			timeUsed[i][j] = thread->getTimeUsed();
			memoryUsed[i][j] = thread->getMemoryUsed();
			score[i][j] = thread->getScore();
			result[i][j] = thread->getResult();
			overallStatus[i] = qMin(overallStatus[i], stateToStatus(thread->getResult(), thread->getScore(),
			                                                        thread->getFullScore()));
			message[i][j] = thread->getMessage();
			delete thread;
			int nowScore = score[i][j];

			if (j + 1 == task->getTestCase(i)->getInputFiles().size()) {
				for (int k = 0; k < j; k++)
					nowScore = qMin(nowScore, score[i][k]);

				if (! task->getTestCase(i)->getDependenceSubtask().empty())
					nowScore =
					    qMin(nowScore, statusToScore(overallStatus[i], task->getTestCase(i)->getFullScore()));
			}

			emit singleCaseFinished(task->getTestCase(i)->getTimeLimit(), i, j, int(result[i][j]),
			                        (j + 1 == task->getTestCase(i)->getInputFiles().size() ? 1 : -1) *
			                            nowScore,
			                        timeUsed[i][j], memoryUsed[i][j]);

			if (score[i][j] < testCaseScore[i])
				testCaseScore[i] = score[i][j];
		}
	}

	return 1;
}

void TaskJudger::makeDialogAlert(QString msg) { emit dialogAlert(std::move(msg)); }
void TaskJudger::taskSkipped(const std::pair<int, int> &cur) {
	emit singleCaseFinished(task->getTestCase(cur.first)->getTimeLimit(), cur.first, cur.second,
	                        int(result[cur.first][cur.second]), 0, 0, 0);
}

void TaskJudger::stop() { isJudging = false; }
