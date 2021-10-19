/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "lemon.h"
#include "ui_lemon.h"
//
#include "addcompilerwizard.h"
#include "addtaskdialog.h"
#include "base/LemonBase.hpp"
#include "base/LemonLog.hpp"
#include "base/LemonTranslator.hpp"
#include "base/compiler.h"
#include "base/settings.h"
#include "component/exportutil/exportutil.h"
#include "core/contest.h"
#include "core/contestant.h"
#include "core/task.h"
#include "core/testcase.h"
#include "detaildialog.h"
#include "newcontestdialog.h"
#include "opencontestdialog.h"
#include "optionsdialog.h"
#include "statisticsbrowser.h"
#include "welcomedialog.h"
//
#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QProgressDialog>
#include <QStatusBar>
#include <QTextBrowser>
#include <QUrl>
#include <algorithm>
#include <chrono>
//
#define LEMON_MODULE_NAME "Lemon"

LemonLime::LemonLime(QWidget *parent) : QMainWindow(parent), ui(new Ui::LemonLime) {
	ui->setupUi(this);
	curContest = nullptr;
	settings = new Settings();
	ui->tabWidget->setVisible(false);
	ui->closeAction->setEnabled(false);
	ui->saveAction->setEnabled(false);
	ui->openFolderAction->setEnabled(false);
	ui->actionChangeContestName->setEnabled(false);
	dataDirWatcher = nullptr;
	settings->loadSettings();
	TaskMenu = new QMenu();
	signalMapper = new QSignalMapper();
	ui->summary->setSettings(settings);
	ui->taskEdit->setSettings(settings);
	ui->testCaseEdit->setSettings(settings);
	connect(this, &LemonLime::dataPathChanged, ui->taskEdit, &TaskEditWidget::dataPathChanged);
	connect(this, &LemonLime::dataPathChanged, ui->testCaseEdit, &TestCaseEditWidget::dataPathChanged);
	connect(ui->summary, &SummaryTree::currentItemChanged, this, &LemonLime::summarySelectionChanged);
	connect(ui->optionsAction, &QAction::triggered, this, &LemonLime::showOptionsDialog);
	connect(ui->cleanupButton, &QPushButton::clicked, this, &LemonLime::cleanupButtonClicked);
	connect(ui->refreshButton, &QPushButton::clicked, this, &LemonLime::refreshButtonClicked);
	connect(ui->judgeButton, &QPushButton::clicked, ui->resultViewer, &ResultViewer::judgeSelected);
	connect(ui->judgeAllButton, &QPushButton::clicked, ui->resultViewer, &ResultViewer::judgeAll);
	connect(ui->judgeUnjudgedButton, &QPushButton::clicked, ui->resultViewer, &ResultViewer::judgeUnjudged);
	connect(ui->judgeAction, &QAction::triggered, ui->resultViewer, &ResultViewer::judgeSelected);
	connect(ui->judgeAllAction, &QAction::triggered, ui->resultViewer, &ResultViewer::judgeAll);
	connect(ui->judgeUnjudgedAction, &QAction::triggered, ui->resultViewer, &ResultViewer::judgeUnjudged);
	connect(ui->cleanupAction, &QAction::triggered, this, &LemonLime::cleanupButtonClicked);
	connect(ui->refreshAction, &QAction::triggered, this, &LemonLime::refreshButtonClicked);
	connect(ui->judgeGreyAction, &QAction::triggered, ui->resultViewer, &ResultViewer::judgeGrey);
	connect(ui->judgeMagentaAction, &QAction::triggered, ui->resultViewer, &ResultViewer::judgeMagenta);
	connect(ui->tabWidget, &QTabWidget::currentChanged, this, &LemonLime::tabIndexChanged);
	connect(ui->moveUpButton, &QToolButton::clicked, this, &LemonLime::moveUpTask);
	connect(ui->moveDownButton, &QToolButton::clicked, this, &LemonLime::moveDownTask);
	connect(ui->resultViewer, &ResultViewer::itemSelectionChanged, this, &LemonLime::viewerSelectionChanged);
	connect(ui->resultViewer, &ResultViewer::contestantDeleted, this, &LemonLime::contestantDeleted);
	connect(ui->newAction, &QAction::triggered, this, &LemonLime::newAction);
	connect(ui->openAction, &QAction::triggered, this, &LemonLime::loadAction);
	connect(ui->saveAction, &QAction::triggered, this, &LemonLime::saveAction);
	connect(ui->openFolderAction, &QAction::triggered, this, &LemonLime::openFolderAction);
	connect(ui->closeAction, &QAction::triggered, this, &LemonLime::closeAction);
	connect(ui->addTasksAction, &QAction::triggered, this, &LemonLime::addTasksAction);
	connect(ui->exportAction, &QAction::triggered, this, &LemonLime::exportResult);
	connect(ui->actionExportStatistics, &QAction::triggered, this, &LemonLime::exportStatstics);
	connect(ui->aboutAction, &QAction::triggered, this, &LemonLime::aboutLemon);
	connect(ui->actionManual, &QAction::triggered, this, &LemonLime::actionManual);
	connect(ui->actionMore, &QAction::triggered, this, &LemonLime::actionMore);
	connect(ui->actionChangeContestName, &QAction::triggered, this, &LemonLime::changeContestName);
	connect(ui->exitAction, &QAction::triggered, this, &LemonLime::close);

	QSettings settings("LemonLime", "lemon");
	QSize _size = settings.value("WindowSize", size()).toSize();
	resize(_size);

	autoSaveTimer.callOnTimeout([this]() {
		if (curContest)
			saveAction();
	});
	using namespace std::chrono_literals;
	autoSaveTimer.start(30s);
}

LemonLime::~LemonLime() {
	delete TaskMenu;
	delete ui;
}

void LemonLime::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		ui->retranslateUi(this);
		ui->resultViewer->refreshViewer();
		ui->statisticsBrowser->refresh();
	}
}

void LemonLime::closeEvent(QCloseEvent * /*event*/) {
	if (curContest)
		saveContest(curFile);

	settings->saveSettings();
	QSettings settings("LemonLime", "lemon");
	settings.setValue("WindowSize", size());
}

auto LemonLime::getSplashTime() -> int { return settings->getSplashTime(); }

void LemonLime::welcome() {
	if (settings->getCompilerList().empty()) {
		auto *wizard = new AddCompilerWizard(this);

		if (wizard->exec() == QDialog::Accepted) {
			QList<Compiler *> compilerList = wizard->getCompilerList();

			for (auto &i : compilerList)
				settings->addCompiler(i);
		}

		delete wizard;
	}

	auto *dialog = new WelcomeDialog(this);
	dialog->setRecentContest(settings->getRecentContest());

	if (dialog->exec() == QDialog::Accepted) {
		settings->setRecentContest(dialog->getRecentContest());

		if (dialog->getCurrentTab() == 0) {
			loadContest(dialog->getSelectedContest());
		} else {
			newContest(dialog->getContestTitle(), dialog->getSavingName(), dialog->getContestPath());
		}
	} else {
		settings->setRecentContest(dialog->getRecentContest());
	}

	delete dialog;
}

void LemonLime::insertWatchPath(const QString &curDir, QFileSystemWatcher *watcher) {
	watcher->addPath(curDir);
	QDir dir(curDir);
	QStringList list = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (int i = 0; i < list.size(); i++) {
		insertWatchPath(curDir + list[i] + QDir::separator(), watcher);
	}
}

void LemonLime::resetDataWatcher() {
	delete dataDirWatcher;
	dataDirWatcher = new QFileSystemWatcher(this);
	insertWatchPath(Settings::dataPath(), dataDirWatcher);
	connect(dataDirWatcher, &QFileSystemWatcher::directoryChanged, this, &LemonLime::resetDataWatcher);
	connect(dataDirWatcher, &QFileSystemWatcher::fileChanged, this, &LemonLime::dataPathChanged);
	connect(dataDirWatcher, &QFileSystemWatcher::directoryChanged, this, &LemonLime::dataPathChanged);
	emit dataPathChanged();
}

void LemonLime::refreshSummary() {
	if (! ui->summary->isEnabled())
		return;

	ui->summary->setContest(curContest);
}

void LemonLime::summarySelectionChanged() {
	if (! ui->summary->isEnabled())
		return;

	QTreeWidgetItem *curItem = ui->summary->currentItem();

	if (! curItem) {
		ui->taskEdit->setEditTask(nullptr);
		ui->editWidget->setCurrentIndex(0);
		return;
	}

	int index = ui->summary->indexOfTopLevelItem(curItem);

	if (index != -1) {
		ui->taskEdit->setEditTask(curContest->getTask(index));
		ui->editWidget->setCurrentIndex(1);
	} else {
		QTreeWidgetItem *parentItem = curItem->parent();
		int taskIndex = ui->summary->indexOfTopLevelItem(parentItem);
		int testCaseIndex = parentItem->indexOfChild(curItem);
		Task *curTask = curContest->getTask(taskIndex);
		TestCase *curTestCase = curTask->getTestCase(testCaseIndex);
		ui->testCaseEdit->setEditTestCase(curTestCase, curTask->getTaskType() == Task::Traditional ||
		                                                   curTask->getTaskType() == Task::Interaction ||
		                                                   curTask->getTaskType() == Task::Communication ||
		                                                   curTask->getTaskType() == Task::CommunicationExec);
		ui->editWidget->setCurrentIndex(2);
	}
}

void LemonLime::showOptionsDialog() {
	auto *dialog = new OptionsDialog(this);
	dialog->resetEditSettings(settings);

	if (dialog->exec() == QDialog::Accepted) {
		settings->copyFrom(dialog->getEditSettings());
		LemonLimeTranslator->InstallTranslation(settings->getUiLanguage());
		ui->testCaseEdit->setSettings(settings);

		if (curContest) {
			const QList<Task *> &taskList = curContest->getTaskList();

			for (auto *i : taskList)
				i->refreshCompilerConfiguration(settings);
		}
	}

	ui->resultViewer->refreshViewer();
	ui->statisticsBrowser->refresh();
	delete dialog;
}

void LemonLime::judgeExtButtonFlip(bool stat) {
	ui->judgeAllButton->setEnabled(stat);
	ui->judgeAllAction->setEnabled(stat);
	ui->judgeUnjudgedButton->setEnabled(stat);
	ui->judgeUnjudgedAction->setEnabled(stat);
	ui->judgeGreyAction->setEnabled(stat);
	ui->judgeMagentaAction->setEnabled(stat);
}

void LemonLime::refreshButtonClicked() {
	curContest->refreshContestantList();
	ui->resultViewer->refreshViewer();
	ui->statisticsBrowser->refresh();
	judgeExtButtonFlip(ui->resultViewer->rowCount() > 0);
	ui->cleanupAction->setEnabled(true);
	ui->refreshAction->setEnabled(true);
}

void removePath(const QString &path) {
	if (path.isEmpty())
		return;

	QDir dir(path);

	if (! dir.exists())
		return;

	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);

	for (const auto &fi : dir.entryInfoList()) {
		if (fi.isFile() || fi.isSymLink())
			fi.dir().remove(fi.fileName());
		else
			removePath(fi.absoluteFilePath());
	}

	dir.rmpath(dir.absolutePath());
}

void copyPath(const QString &fromPath, const QString &toPath) {
	QDir dir(fromPath);

	if (! dir.exists())
		return;

	QString fpath = fromPath + QDir::separator();
	QString tpath = toPath + QDir::separator();
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);

	for (const auto &fi : dir.entryInfoList()) {
		QString fn = fpath + fi.fileName();
		QString tn = tpath + fi.fileName();

		if (fi.isFile() || fi.isSymLink())
			QFile::copy(fn, tn);
		else {
			QDir toDir(toPath);
			toDir.mkpath(fi.fileName());
			copyPath(fn, tn);
		}
	}
}

void LemonLime::cleanupButtonClicked() {
	QString text;
	text += tr("Are you sure to Clean up Files?") + "<br>";
	text += tr("Reading guide are recommended.") + "<br>";
	QMessageBox::StandardButton res =
	    QMessageBox::warning(this, tr("Clean up Files"), text,
	                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort, QMessageBox::No);

	if (res == QMessageBox::Yes) {
		QDir basDir(Settings::sourcePath());
		basDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
		QFileInfoList basDirLis = basDir.entryInfoList();
		int tarcnt = basDirLis.size();
		QString backupFolder = "source_bak_%1";
		int backupNum = 0;
		QDir tempBackupLoca;

		while (tempBackupLoca.exists(backupFolder.arg(backupNum)))
			backupNum++;

		backupFolder = backupFolder.arg(backupNum);
		text = tr("Making backup files to dir <br> `%1'?").arg(backupFolder) + "<br>";
		QMessageBox::StandardButton doBackup = QMessageBox::information(
		    this, tr("Clean up Files"), text, QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort,
		    QMessageBox::Yes);

		if (doBackup == QMessageBox::Abort) {
			QMessageBox::information(this, tr("Clean up Files"), tr("Aborted."));
			return;
		}

		if (doBackup == QMessageBox::Yes) {
			QDir bkLoca;

			if (bkLoca.exists(backupFolder)) {
				QMessageBox::information(this, tr("Clean up Files"),
				                         tr("Aborted: `%1' already exist.").arg(backupFolder));
				return;
			}

			if (! bkLoca.mkpath(backupFolder)) {
				QMessageBox::information(this, tr("Clean up Files"),
				                         tr("Aborted: Cannot make dir `%1'.").arg(backupFolder));
				return;
			}

			bkLoca = QDir(backupFolder);
			auto *bkProcess = new QProgressDialog(tr("Making Backup..."), "", 0, 0, this);
			bkProcess->setWindowModality(Qt::WindowModal);
			bkProcess->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
			bkProcess->setMinimumDuration(0);
			bkProcess->setCancelButton(nullptr);
			bkProcess->setRange(0, tarcnt);
			bkProcess->setValue(0);
			QCoreApplication::processEvents();
			basDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

			for (const auto &conDirWho : basDir.entryInfoList()) {
				bkLoca.mkpath(conDirWho.fileName());
				copyPath(conDirWho.path() + QDir::separator() + conDirWho.fileName(),
				         bkLoca.path() + QDir::separator() + conDirWho.fileName());
				bkProcess->setValue(bkProcess->value() + 1);
				QCoreApplication::processEvents();
			}

			delete bkProcess;
		}

		auto *process = new QProgressDialog(tr("Cleaning"), "", 0, 0, this);
		process->setWindowModality(Qt::WindowModal);
		process->setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
		process->setMinimumDuration(0);
		process->setCancelButton(nullptr);
		process->setRange(0, 5);
		process->setValue(0);
		process->setLabelText(tr("Working on it..."));
		QCoreApplication::processEvents();
		process->setRange(0, tarcnt + 5);
		process->setValue(0);
		process->setModal(true);
		process->setLabelText(tr("Fetching Data..."));
		QCoreApplication::processEvents();
		QSet<QString> tarNameSet;
		QSet<QString> nameSet;
		QMap<QString, int> typeSet;
		QMap<QString, QString> origSet;
		QList<Task *> taskList = curContest->getTaskList();
		process->setValue(1);
		process->setLabelText(tr("Initing..."));
		QCoreApplication::processEvents();

		for (int i = 0; i < taskList.size(); i++) {
			QString taskName = taskList[i]->getSourceFileName();
			typeSet[taskName] = i;
			nameSet.insert(taskName);

			if (taskList[i]->getTaskType() == Task::AnswersOnly) {
				for (auto *j : taskList[i]->getTestCaseList()) {
					for (const auto &k : j->getInputFiles()) {
						QString temp = QFileInfo(k).completeBaseName();
						tarNameSet.insert(temp);
						origSet[temp] = taskName;
					}
				}
			} else if (taskList[i]->getTaskType() == Task::Communication ||
			           taskList[i]->getTaskType() == Task::CommunicationExec) {
				QStringList sourcePaths = taskList[i]->getSourceFilesPath();

				for (const auto &j : sourcePaths) {
					QString temp = QFileInfo(j).completeBaseName();
					tarNameSet.insert(temp);
					origSet[temp] = taskName;
				}
			} else {
				tarNameSet.insert(taskName);
				origSet[taskName] = taskName;
			}
		}

		process->setValue(5);
		process->setLabelText(tr("Now Cleaning..."));
		QCoreApplication::processEvents();
		basDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

		for (const auto &conDirWho : basDir.entryInfoList()) {
			QDir conDir(conDirWho.filePath());
			conDir.setFilter(QDir::Files | QDir::Hidden);

			for (const auto &proFilWho : conDir.entryInfoList()) {
				if (proFilWho.suffix().length() <= 0 || proFilWho.suffix().toUpper() == "EXE")
					QFile::remove(proFilWho.absoluteFilePath());
			}

			conDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

			for (const auto &proDirWho : conDir.entryInfoList()) {
				if (nameSet.contains(proDirWho.fileName())) {
					QDir proDir(proDirWho.filePath());
					proDir.setFilter(QDir::Files | QDir::Hidden);

					for (const auto &sorFilWho : proDir.entryInfoList()) {
						if (sorFilWho.suffix().length() > 0 && sorFilWho.suffix().toUpper() != "EXE")
							QFile::copy(sorFilWho.filePath(),
							            conDirWho.filePath() + QDir::separator() + sorFilWho.fileName());
					}
				}

				removePath(proDirWho.absoluteFilePath());
			}

			for (const auto &proName : nameSet) {
				conDir.mkpath(proName);
			}

			conDir.setFilter(QDir::Files | QDir::Hidden);

			for (const auto &proFilWho : conDir.entryInfoList()) {
				QString proFilName = proFilWho.fileName();
				QString proName = proFilName;
				proName.truncate(proName.lastIndexOf("."));

				if (tarNameSet.contains(proName)) {
					QString taskName = origSet[proName];
					int who = typeSet[taskName];
					int types = taskList[who]->getTaskType();

					if (types == Task::Traditional || types == Task::Interaction ||
					    types == Task::Communication || types == Task::CommunicationExec) {
						if (proFilName != taskList[who]->getInputFileName() &&
						    proFilName != taskList[who]->getOutputFileName())
							QFile::copy(proFilWho.filePath(), conDirWho.filePath() + QDir::separator() +
							                                      taskName + QDir::separator() + proFilName);
					} else if (types == Task::AnswersOnly) {
						if (proFilWho.suffix() == taskList[who]->getAnswerFileExtension())
							QFile::copy(proFilWho.filePath(), conDirWho.filePath() + QDir::separator() +
							                                      taskName + QDir::separator() + proFilName);
					}
				}

				QFile::remove(proFilWho.absoluteFilePath());
			}

			conDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

			for (const auto &proDirWho : conDir.entryInfoList()) {
				QDir proDir(proDirWho.filePath());
				proDir.setFilter(QDir::Files | QDir::Hidden);

				for (const auto &sorFilWho : proDir.entryInfoList())
					QFile::copy(sorFilWho.filePath(),
					            conDirWho.filePath() + QDir::separator() + sorFilWho.fileName());
			}

			process->setValue(process->value() + 1);
			QCoreApplication::processEvents();
		}

		delete process;
		text = tr("Finished.") + "<br>";
		QMessageBox::information(this, tr("Clean up Files"), text);
	} else {
		QMessageBox::information(this, tr("Clean up Files"), tr("Aborted"));
	}
}

void LemonLime::tabIndexChanged(int index) {
	if (index != 1) {
		judgeExtButtonFlip(false);
		ui->judgeAction->setEnabled(false);
		ui->judgeButton->setEnabled(false);
		ui->cleanupAction->setEnabled(false);
		ui->refreshAction->setEnabled(false);

		if (index == 2) {
			ui->statisticsBrowser->refresh();
		}
	} else {
		QList<QTableWidgetSelectionRange> selectionRange = ui->resultViewer->selectedRanges();

		if (! selectionRange.empty()) {
			ui->judgeAction->setEnabled(true);
			ui->judgeButton->setEnabled(true);
		} else {
			ui->judgeAction->setEnabled(false);
			ui->judgeButton->setEnabled(false);
		}

		judgeExtButtonFlip(ui->resultViewer->rowCount() > 0);
		ui->cleanupAction->setEnabled(true);
		ui->refreshAction->setEnabled(true);
	}
}

void LemonLime::moveUpTask() {
	QTreeWidgetItem *curItem = ui->summary->currentItem();

	if (! curItem)
		return;

	int index = ui->summary->indexOfTopLevelItem(curItem);
	curContest->swapTask(index - 1, index);
	ui->summary->setContest(curContest);
	ui->resultViewer->refreshViewer();
	ui->statisticsBrowser->refresh();
	curItem = ui->summary->topLevelItem(index - 1);

	if (! curItem)
		curItem = ui->summary->topLevelItem(index);

	if (curItem)
		ui->summary->setCurrentItem(curItem);
}

void LemonLime::moveDownTask() {
	QTreeWidgetItem *curItem = ui->summary->currentItem();

	if (! curItem)
		return;

	int index = ui->summary->indexOfTopLevelItem(curItem);
	curContest->swapTask(index + 1, index);
	ui->summary->setContest(curContest);
	ui->resultViewer->refreshViewer();
	ui->statisticsBrowser->refresh();
	curItem = ui->summary->topLevelItem(index + 1);

	if (! curItem)
		curItem = ui->summary->topLevelItem(index);

	if (curItem)
		ui->summary->setCurrentItem(curItem);
}

void LemonLime::viewerSelectionChanged() {
	QList<QTableWidgetSelectionRange> selectionRange = ui->resultViewer->selectedRanges();

	if (! selectionRange.empty()) {
		ui->judgeButton->setEnabled(true);
		ui->judgeAction->setEnabled(true);
	} else {
		ui->judgeButton->setEnabled(false);
		ui->judgeAction->setEnabled(false);
	}
}

void LemonLime::contestantDeleted() {
	judgeExtButtonFlip(ui->resultViewer->rowCount() > 0);
	ui->cleanupAction->setEnabled(true);
	ui->refreshAction->setEnabled(true);
}

void LemonLime::saveContest(const QString &fileName) {
	QFile file(fileName);

	if (! file.open(QFile::WriteOnly)) {
		QMessageBox::warning(this, tr("Error"), tr("Cannot open file %1").arg(fileName), QMessageBox::Close);
		ui->statusBar->showMessage(tr("Save Failed"), 1000);
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	QJsonObject out;
	curContest->writeToJson(out);
	file.write(QJsonDocument(out).toJson(QJsonDocument::Compact));
	/* QByteArray data;
	QDataStream _out(&data, QIODevice::WriteOnly);
	curContest->writeToStream(_out);
	data = qCompress(data);
	QDataStream out(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	out << unsigned(MagicNumber) << qChecksum(QByteArrayView(data))
	    << static_cast<int>(data.length()); // Qt 6 changed the type fo data.length() to qsizetype
#else
	out << unsigned(MagicNumber) << qChecksum(data.data(), static_cast<uint>(data.length())) << data.length();
#endif
	out.writeRawData(data.data(), data.length()); */
	QApplication::restoreOverrideCursor();
	ui->statusBar->showMessage(tr("Saved"), 1000);
}

void LemonLime::loadContest(const QString &filePath) {
	if (curContest)
		closeAction();

	curContest = new Contest(this);

	QFile file(filePath);

	if (! file.open(QFile::ReadOnly)) {
		QMessageBox::warning(this, tr("Error"), tr("Cannot open file %1").arg(QFileInfo(filePath).fileName()),
		                     QMessageBox::Close);
		return;
	}
	char firstChar;
	file.peek(&firstChar, 1);
	// Don't support RFC 7159, but support RFC 4627
	if (firstChar == '[' || firstChar == '{') {
		QJsonParseError parseError;
		QJsonObject inObj(QJsonDocument::fromJson(file.readAll(), &parseError).object());
		if (parseError.error != 0) {
			QMessageBox::warning(this, tr("Error"),
			                     tr("File %1 is broken").arg(QFileInfo(filePath).fileName()) + "\n" +
			                         parseError.errorString() + "at position" +
			                         QString("%1").arg(parseError.offset),
			                     QMessageBox::Close);
			return;
		}
		QApplication::setOverrideCursor(Qt::WaitCursor);
		curContest->setSettings(settings);
		if (curContest->readFromJson(inObj) == -1) {
			QMessageBox::warning(this, tr("Error"),
			                     tr("File %1 is broken").arg(QFileInfo(filePath).fileName()),
			                     QMessageBox::Close);
			QApplication::restoreOverrideCursor();
			return;
		}
	} else {
		QDataStream _in(&file);
		unsigned checkNumber = 0;
		_in >> checkNumber;

		if (checkNumber != unsigned(MagicNumber)) {
			QMessageBox::warning(this, tr("Error"),
			                     tr("File %1 is broken").arg(QFileInfo(filePath).fileName()),
			                     QMessageBox::Close);
			return;
		}

		quint16 checksum = 0;
		int len = 0;
		_in >> checksum >> len;
		char *raw = new char[len];
		_in.readRawData(raw, len);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		if (qChecksum(QByteArrayView(raw, static_cast<uint>(len))) != checksum)
#else
		if (qChecksum(raw, static_cast<uint>(len)) != checksum)
#endif
		{
			QMessageBox::warning(this, tr("Error"),
			                     tr("File %1 is broken").arg(QFileInfo(filePath).fileName()),
			                     QMessageBox::Close);
			delete[] raw;
			return;
		}

		QByteArray data(raw, len);
		delete[] raw;
		data = qUncompress(data);
		QDataStream in(data);
		QApplication::setOverrideCursor(Qt::WaitCursor);
		curContest->setSettings(settings);
		curContest->readFromStream(in);
	}
	curFile = QFileInfo(filePath).fileName();
	QDir::setCurrent(QFileInfo(filePath).path());
	QDir().mkdir(Settings::dataPath());
	QDir().mkdir(Settings::sourcePath());
	ui->summary->setContest(curContest);
	ui->resultViewer->setContest(curContest);
	ui->resultViewer->refreshViewer();
	ui->statisticsBrowser->setContest(curContest);
	ui->statisticsBrowser->refresh();
	ui->tabWidget->setVisible(true);
	resetDataWatcher();
	ui->closeAction->setEnabled(true);
	ui->openFolderAction->setEnabled(true);
	ui->saveAction->setEnabled(true);
	ui->addTasksAction->setEnabled(true);
	ui->exportAction->setEnabled(true);
	ui->actionExportStatistics->setEnabled(true);
	ui->actionChangeContestName->setEnabled(true);
	ui->cleanupAction->setEnabled(false);
	ui->refreshAction->setEnabled(false);
	setWindowTitle(tr("LemonLime - %1").arg(curContest->getContestTitle()));
	ui->tabWidget->setCurrentIndex(0);
	QApplication::restoreOverrideCursor();
}

void LemonLime::newContest(const QString &title, const QString &savingName, const QString &path) {
	if (! QDir(path).exists() && ! QDir().mkpath(path)) {
		QMessageBox::warning(this, tr("Error"), tr("Cannot make contest path"), QMessageBox::Close);
		return;
	}

	if (curContest)
		closeAction();

	curContest = new Contest(this);
	curContest->setSettings(settings);
	curContest->setContestTitle(title);
	setWindowTitle(tr("LemonLime - %1").arg(title));
	QDir::setCurrent(path);
	QDir().mkdir(Settings::dataPath());
	QDir().mkdir(Settings::sourcePath());
	curFile = savingName + ".cdf";
	saveContest(curFile);
	ui->summary->setContest(curContest);
	ui->resultViewer->setContest(curContest);
	ui->resultViewer->refreshViewer();
	ui->statisticsBrowser->setContest(curContest);
	ui->statisticsBrowser->refresh();
	ui->tabWidget->setVisible(true);
	resetDataWatcher();
	ui->closeAction->setEnabled(true);
	ui->openFolderAction->setEnabled(true);
	ui->saveAction->setEnabled(true);
	ui->addTasksAction->setEnabled(true);
	ui->exportAction->setEnabled(true);
	ui->actionExportStatistics->setEnabled(true);
	ui->actionChangeContestName->setEnabled(true);
	ui->cleanupAction->setEnabled(false);
	ui->refreshAction->setEnabled(false);
	QStringList recentContest = settings->getRecentContest();
	recentContest.append(QDir::toNativeSeparators((QDir().absoluteFilePath(curFile))));
	settings->setRecentContest(recentContest);
	ui->tabWidget->setCurrentIndex(0);
}

void LemonLime::newAction() {
	auto *dialog = new NewContestDialog(this);

	if (dialog->exec() == QDialog::Accepted) {
		newContest(dialog->getContestTitle(), dialog->getSavingName(), dialog->getContestPath());
	}

	delete dialog;
}

void LemonLime::closeAction() {
	saveContest(curFile);
	ui->summary->setContest(nullptr);
	ui->taskEdit->setEditTask(nullptr);
	ui->resultViewer->setContest(nullptr);
	ui->statisticsBrowser->setContest(nullptr);
	delete curContest;
	curContest = nullptr;
	ui->tabWidget->setCurrentIndex(0);
	ui->tabWidget->setVisible(false);
	ui->closeAction->setEnabled(false);
	ui->openFolderAction->setEnabled(false);
	ui->saveAction->setEnabled(false);
	ui->addTasksAction->setEnabled(false);
	ui->exportAction->setEnabled(false);
	ui->actionExportStatistics->setEnabled(false);
	ui->actionChangeContestName->setEnabled(false);
	ui->cleanupAction->setEnabled(false);
	ui->refreshAction->setEnabled(false);
	setWindowTitle(tr("LemonLime"));
}

void LemonLime::saveAction() { saveContest(curFile); }

void LemonLime::openFolderAction() { QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::currentPath())); }

void LemonLime::loadAction() {
	auto *dialog = new OpenContestDialog(this);
	dialog->setRecentContest(settings->getRecentContest());
	QStringList recentContest = dialog->getRecentContest();

	if (dialog->exec() == QDialog::Accepted) {
		QString selectedContest = dialog->getSelectedContest();

		for (int i = 0; i < recentContest.size(); i++) {
			if (recentContest[i] == selectedContest) {
				recentContest.removeAt(i);
				break;
			}
		}

		recentContest.prepend(selectedContest);
		loadContest(selectedContest);
	}

	settings->setRecentContest(recentContest);
	delete dialog;
}

void LemonLime::getFiles(const QString &path, const QStringList &filters, QMap<QString, QString> &files) {
	QDir dir(path);

	if (! filters.isEmpty())
		dir.setNameFilters(filters);

	QFileInfoList list = dir.entryInfoList(QDir::Files);

	for (auto &i : list) {
		files.insert(i.completeBaseName(), i.fileName());
	}
}

void LemonLime::addTask(const QString &title, const QList<std::pair<QString, QString>> &testCases,
                        int fullScore, int timeLimit, int memoryLimit) {
	Task *newTask = new Task;
	newTask->setProblemTitle(title);
	newTask->setSourceFileName(title);
	newTask->setInputFileName(title + ".in");
	newTask->setOutputFileName(title + ".out");
	newTask->refreshCompilerConfiguration(settings);
	newTask->setAnswerFileExtension(settings->getDefaultOutputFileExtension());
	curContest->addTask(newTask);

	for (const auto &testCase : testCases) {
		auto *newTestCase = new TestCase;
		newTestCase->setFullScore(fullScore);
		newTestCase->setTimeLimit(timeLimit);
		newTestCase->setMemoryLimit(memoryLimit);
		newTestCase->addSingleCase(title + QDir::separator() + testCase.first,
		                           title + QDir::separator() + testCase.second);
		newTask->addTestCase(newTestCase);
	}
}

void LemonLime::addTaskWithScoreScale(const QString &title,
                                      const QList<std::pair<QString, QString>> &testCases, int sumScore,
                                      int timeLimit, int memoryLimit) {
	Task *newTask = new Task;
	newTask->setProblemTitle(title);
	newTask->setSourceFileName(title);
	newTask->setInputFileName(title + ".in");
	newTask->setOutputFileName(title + ".out");
	newTask->refreshCompilerConfiguration(settings);
	newTask->setAnswerFileExtension(settings->getDefaultOutputFileExtension());
	curContest->addTask(newTask);
	int scorePer = sumScore / testCases.size();
	int scoreLos = sumScore - scorePer * testCases.size();

	for (int i = 0; i < testCases.size(); i++) {
		auto *newTestCase = new TestCase;
		newTestCase->setFullScore(scorePer + static_cast<int>(i < scoreLos));
		newTestCase->setTimeLimit(timeLimit);
		newTestCase->setMemoryLimit(memoryLimit);
		newTestCase->addSingleCase(title + QDir::separator() + testCases[i].first,
		                           title + QDir::separator() + testCases[i].second);
		newTask->addTestCase(newTestCase);
	}
}

auto LemonLime::compareFileName(const std::pair<QString, QString> &a, const std::pair<QString, QString> &b)
    -> bool {
	return (a.first.length() < b.first.length()) ||
	       (a.first.length() == b.first.length() && QString::localeAwareCompare(a.first, b.first) < 0);
}

void LemonLime::addTasksAction() {
	QStringList list = QDir(Settings::dataPath()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	QSet<QString> nameSet;
	QList<Task *> taskList = curContest->getTaskList();

	for (auto &i : taskList) {
		nameSet.insert(i->getSourceFileName());
	}

	QStringList nameList;
	QList<QList<std::pair<QString, QString>>> testCases;

	for (int i = 0; i < list.size(); i++) {
		if (! nameSet.contains(list[i])) {
			QStringList filters;
			filters = settings->getInputFileExtensions();

			if (filters.isEmpty())
				filters << "in";

			for (int j = 0; j < filters.size(); j++) {
				filters[j] = QString("*.") + filters[j];
			}

			QMap<QString, QString> inputFiles;
			getFiles(Settings::dataPath() + list[i], filters, inputFiles);
			filters = settings->getOutputFileExtensions();

			if (filters.isEmpty())
				filters << "out"
				        << "ans";

			for (int j = 0; j < filters.size(); j++) {
				filters[j] = QString("*.") + filters[j];
			}

			QMap<QString, QString> outputFiles;
			getFiles(Settings::dataPath() + list[i], filters, outputFiles);
			QList<std::pair<QString, QString>> cases;
			QStringList baseNameList = inputFiles.keys();

			for (int j = 0; j < baseNameList.size(); j++) {
				if (outputFiles.contains(baseNameList[j])) {
					cases.append(std::make_pair(inputFiles[baseNameList[j]], outputFiles[baseNameList[j]]));
				}
			}

			std::sort(cases.begin(), cases.end(), compareFileName);

			if (! cases.isEmpty()) {
				nameList.append(list[i]);
				testCases.append(cases);
			}
		}
	}

	if (nameList.isEmpty()) {
		QMessageBox::warning(this, tr("LemonLime"), tr("No task found"), QMessageBox::Ok);
		return;
	}

	auto *dialog = new AddTaskDialog(this);
	dialog->resize(dialog->sizeHint());
	dialog->setMaximumSize(dialog->sizeHint());
	dialog->setMinimumSize(dialog->sizeHint());

	for (int i = 0; i < nameList.size(); i++) {
		dialog->addTask(nameList[i], qMax(100, testCases[i].size()), settings->getDefaultTimeLimit(),
		                settings->getDefaultMemoryLimit());
	}

	if (dialog->exec() == QDialog::Accepted) {
		for (int i = 0; i < nameList.size(); i++) {
			addTaskWithScoreScale(nameList[i], testCases[i], dialog->getFullScore(i), dialog->getTimeLimit(i),
			                      dialog->getMemoryLimit(i));
		}
	}

	ui->summary->setContest(curContest);
}

void LemonLime::exportResult() { ExportUtil::exportResult(this, curContest); }

void LemonLime::exportStatstics() { StatisticsBrowser::exportStatstics(this, curContest); }

void LemonLime::changeContestName() {
	if (! curContest) {
		QMessageBox::warning(this, tr("Rename Contest"), tr("No Contest Yet"));
	}

	bool confirmed = false;
	QString newName = QInputDialog::getText(this, tr("Rename Contest"), tr("Write the name you want."),
	                                        QLineEdit::Normal, tr("New Name"), &confirmed);

	if (! confirmed) {
		QMessageBox::warning(this, tr("Rename Contest"), tr("The name did not changes."));
		return;
	}

	curContest->setContestTitle(newName);
	setWindowTitle(tr("LemonLime - %1").arg(curContest->getContestTitle()));
	ui->resultViewer->refreshViewer();
	ui->statisticsBrowser->refresh();
	saveContest(curFile);
}

void LemonLime::aboutLemon() {
	QString text;
	text += "<h2>Project LemonLime</h2>";
	text += "<h3>" +
	        tr("Version: %1")
	            .arg(QString(LEMON_VERSION_STRING) + QString(":") + QString::number(LEMON_VERSION_BUILD)) +
	        "</h3>";
	text += tr("This is a tiny judging environment for OI contest based on Project LemonPlus.") + "<br>";
	text += tr("Based on Project Lemon version 1.2 Beta by Zhipeng Jia, 2011") + "<br>";
	text += tr("Based on Project LemonPlus by Dust1404, 2019") + "<br>";
	text += tr("Update by iotang and Coelacanthus") + "<br><br>";
	text += tr("Build Info: %1").arg(QString(LEMON_BUILD_INFO_STR)) + "<br>";
	text += tr("Build Extra Info: %1").arg(QString(LEMON_BUILD_EXTRA_INFO_STR)) + "<br>";
	text += tr("Build Date: %1").arg(QString(__DATE__) + QString(", ") + QString(__TIME__)) + "<br>";
	text += tr("This program is under the <a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">GPLv3</a> "
	           "license") +
	        "<br>";
	QMessageBox::about(this, tr("About LemonLime"), text);
}

void LemonLime::actionManual() {
#ifndef LEMON_EMBED_DOCS
	QString fileName("/usr/share/doc/lemon-lime/llmanual.pdf");
#else
	QString fileName = QFileDialog::getSaveFileName(this, tr("Manual"), "llmanual.pdf");

	if (fileName.isEmpty())
		return;

	QFile::copy(":/manual/llmanual.pdf", fileName);
#endif
	QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

void LemonLime::actionMore() {
	QDesktopServices::openUrl(QUrl(QString("https://github.com/Project-LemonLime/Project_LemonLime")));
}
