/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "lemon.h"
#include "addcompilerwizard.h"
#include "addtaskdialog.h"
#include "base/LemonBase.hpp"
#include "base/LemonLog.hpp"
#include "component/exportutil/exportutil.h"
#include "contest.h"
#include "core/compiler.h"
#include "core/contestant.h"
#include "core/settings.h"
#include "core/task.h"
#include "core/testcase.h"
#include "detaildialog.h"
#include "newcontestdialog.h"
#include "opencontestdialog.h"
#include "optionsdialog.h"
#include "statisticsbrowser.h"
#include "ui_lemon.h"
#include "welcomedialog.h"
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

LemonLime::LemonLime(QWidget *parent) : QMainWindow(parent), ui(new Ui::LemonLime)
{
	ui->setupUi(this);
	curContest = nullptr;
	settings = new Settings(this);
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
	connect(this, SIGNAL(dataPathChanged()), ui->taskEdit, SIGNAL(dataPathChanged()));
	connect(this, SIGNAL(dataPathChanged()), ui->testCaseEdit, SIGNAL(dataPathChanged()));
	connect(ui->summary, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this,
	        SLOT(summarySelectionChanged()));
	connect(ui->optionsAction, SIGNAL(triggered()), this, SLOT(showOptionsDialog()));
	connect(ui->cleanupButton, SIGNAL(clicked()), this, SLOT(cleanupButtonClicked()));
	connect(ui->refreshButton, SIGNAL(clicked()), this, SLOT(refreshButtonClicked()));
	connect(ui->judgeButton, SIGNAL(clicked()), ui->resultViewer, SLOT(judgeSelected()));
	connect(ui->judgeAllButton, SIGNAL(clicked()), ui->resultViewer, SLOT(judgeAll()));
	connect(ui->judgeUnjudgedButton, SIGNAL(clicked()), ui->resultViewer, SLOT(judgeUnjudged()));
	connect(ui->judgeAction, SIGNAL(triggered()), ui->resultViewer, SLOT(judgeSelected()));
	connect(ui->judgeAllAction, SIGNAL(triggered()), ui->resultViewer, SLOT(judgeAll()));
	connect(ui->judgeUnjudgedAction, SIGNAL(triggered()), ui->resultViewer, SLOT(judgeUnjudged()));
	connect(ui->cleanupAction, SIGNAL(triggered()), this, SLOT(cleanupButtonClicked()));
	connect(ui->refreshAction, SIGNAL(triggered()), this, SLOT(refreshButtonClicked()));
	connect(ui->judgeGreyAction, SIGNAL(triggered()), ui->resultViewer, SLOT(judgeGrey()));
	connect(ui->judgeMagentaAction, SIGNAL(triggered()), ui->resultViewer, SLOT(judgeMagenta()));
	connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabIndexChanged(int)));
	connect(ui->moveUpButton, SIGNAL(clicked()), this, SLOT(moveUpTask()));
	connect(ui->moveDownButton, SIGNAL(clicked()), this, SLOT(moveDownTask()));
	connect(ui->resultViewer, SIGNAL(itemSelectionChanged()), this, SLOT(viewerSelectionChanged()));
	connect(ui->resultViewer, SIGNAL(contestantDeleted()), this, SLOT(contestantDeleted()));
	connect(ui->newAction, SIGNAL(triggered()), this, SLOT(newAction()));
	connect(ui->openAction, SIGNAL(triggered()), this, SLOT(loadAction()));
	connect(ui->saveAction, SIGNAL(triggered()), this, SLOT(saveAction()));
	connect(ui->openFolderAction, SIGNAL(triggered()), this, SLOT(openFolderAction()));
	connect(ui->closeAction, SIGNAL(triggered()), this, SLOT(closeAction()));
	connect(ui->addTasksAction, SIGNAL(triggered()), this, SLOT(addTasksAction()));
	connect(ui->exportAction, SIGNAL(triggered()), this, SLOT(exportResult()));
	connect(ui->actionExportStatistics, SIGNAL(triggered()), this, SLOT(exportStatstics()));
	connect(ui->aboutAction, SIGNAL(triggered()), this, SLOT(aboutLemon()));
	connect(ui->actionManual, SIGNAL(triggered()), this, SLOT(actionManual()));
	connect(ui->actionMore, SIGNAL(triggered()), this, SLOT(actionMore()));
	connect(ui->actionChangeContestName, SIGNAL(triggered()), this, SLOT(changeContestName()));
	connect(ui->exitAction, SIGNAL(triggered()), this, SLOT(close()));

	LOG("LemonLime " LEMON_VERSION_STRING " on " + QSysInfo::prettyProductName() + " " +
	    QSysInfo::currentCpuArchitecture())
	DEBUG("LemonLime Start Time: " + QString::number(QTime::currentTime().msecsSinceStartOfDay()))
	DEBUG(LEMON_BUILD_INFO)
	DEBUG(LEMON_BUILD_EXTRA_INFO)
	DEBUG(QString::number(LEMON_VERSION_BUILD))

	appTranslator = new QTranslator(this);
	QApplication::installTranslator(appTranslator);
	QStringList fileList = QDir(":/translation").entryList(QStringList() << "*.qm", QDir::Files);
	DEBUG("Found Language: " + fileList.join(' '));

	for (int i = 0; i < fileList.size(); i++)
	{
		appTranslator->load(QString(":/translation/%1").arg(fileList[i]));
		auto *newLanguage = new QAction(appTranslator->translate("Lemon", "English"), this);
		newLanguage->setCheckable(true);
		QString language = QFileInfo(fileList[i]).baseName();
		// language.remove(0, language.indexOf('_') + 1);
		newLanguage->setData(language);
		connect(newLanguage, SIGNAL(triggered()), this, SLOT(setUiLanguage()));
		languageActions.append(newLanguage);
	}

	ui->languageMenu->addActions(languageActions);
	ui->setEnglishAction->setData("en");
	ui->setEnglishAction->setCheckable(true);
	connect(ui->setEnglishAction, SIGNAL(triggered()), this, SLOT(setUiLanguage()));
	loadUiLanguage();
	QSettings settings("LemonLime", "lemon");
	QSize _size = settings.value("WindowSize", size()).toSize();
	resize(_size);
}

LemonLime::~LemonLime()
{
	delete TaskMenu;
	delete ui;
}

void LemonLime::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		ui->retranslateUi(this);
		ui->resultViewer->refreshViewer();
		ui->statisticsBrowser->refresh();
	}
}

void LemonLime::closeEvent(QCloseEvent * /*event*/)
{
	if (curContest)
		saveContest(curFile);

	settings->saveSettings();
	QSettings settings("LemonLime", "lemon");
	settings.setValue("WindowSize", size());
}

auto LemonLime::getSplashTime() -> int { return settings->getSplashTime(); }

void LemonLime::welcome()
{
	if (settings->getCompilerList().empty())
	{
		auto *wizard = new AddCompilerWizard(this);

		if (wizard->exec() == QDialog::Accepted)
		{
			QList<Compiler *> compilerList = wizard->getCompilerList();

			for (auto &i : compilerList)
				settings->addCompiler(i);
		}

		delete wizard;
	}

	auto *dialog = new WelcomeDialog(this);
	dialog->setRecentContest(settings->getRecentContest());

	if (dialog->exec() == QDialog::Accepted)
	{
		settings->setRecentContest(dialog->getRecentContest());

		if (dialog->getCurrentTab() == 0)
		{
			loadContest(dialog->getSelectedContest());
		}
		else
		{
			newContest(dialog->getContestTitle(), dialog->getSavingName(), dialog->getContestPath());
		}
	}
	else
	{
		settings->setRecentContest(dialog->getRecentContest());
	}

	delete dialog;
}

void LemonLime::loadUiLanguage()
{
	ui->setEnglishAction->setChecked(false);

	for (auto &languageAction : languageActions)
	{
		languageAction->setChecked(false);
	}

	for (auto &languageAction : languageActions)
	{
		if (languageAction->data().toString() == settings->getUiLanguage())
		{
			languageAction->setChecked(true);
			appTranslator->load(QString(":/translation/%1.qm").arg(settings->getUiLanguage()));
			return;
		}
	}

	settings->setUiLanguage("en");
	appTranslator->load("");
	ui->setEnglishAction->setChecked(true);
}

void LemonLime::insertWatchPath(const QString &curDir, QFileSystemWatcher *watcher)
{
	watcher->addPath(curDir);
	QDir dir(curDir);
	QStringList list = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (int i = 0; i < list.size(); i++)
	{
		insertWatchPath(curDir + list[i] + QDir::separator(), watcher);
	}
}

void LemonLime::resetDataWatcher()
{
	delete dataDirWatcher;
	dataDirWatcher = new QFileSystemWatcher(this);
	insertWatchPath(Settings::dataPath(), dataDirWatcher);
	connect(dataDirWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(resetDataWatcher()));
	connect(dataDirWatcher, SIGNAL(fileChanged(QString)), this, SIGNAL(dataPathChanged()));
	connect(dataDirWatcher, SIGNAL(directoryChanged(QString)), this, SIGNAL(dataPathChanged()));
	emit dataPathChanged();
}

void LemonLime::refreshSummary()
{
	if (! ui->summary->isEnabled())
		return;

	ui->summary->setContest(curContest);
}

void LemonLime::summarySelectionChanged()
{
	if (! ui->summary->isEnabled())
		return;

	QTreeWidgetItem *curItem = ui->summary->currentItem();

	if (! curItem)
	{
		ui->taskEdit->setEditTask(nullptr);
		ui->editWidget->setCurrentIndex(0);
		return;
	}

	int index = ui->summary->indexOfTopLevelItem(curItem);

	if (index != -1)
	{
		ui->taskEdit->setEditTask(curContest->getTask(index));
		ui->editWidget->setCurrentIndex(1);
	}
	else
	{
		QTreeWidgetItem *parentItem = curItem->parent();
		int taskIndex = ui->summary->indexOfTopLevelItem(parentItem);
		int testCaseIndex = parentItem->indexOfChild(curItem);
		Task *curTask = curContest->getTask(taskIndex);
		TestCase *curTestCase = curTask->getTestCase(testCaseIndex);
		ui->testCaseEdit->setEditTestCase(curTestCase, curTask->getTaskType() == Task::Traditional ||
		                                                   curTask->getTaskType() == Task::Interaction ||
		                                                   curTask->getTaskType() == Task::Communication);
		ui->editWidget->setCurrentIndex(2);
	}
}

void LemonLime::showOptionsDialog()
{
	auto *dialog = new OptionsDialog(this);
	dialog->resetEditSettings(settings);

	if (dialog->exec() == QDialog::Accepted)
	{
		settings->copyFrom(dialog->getEditSettings());
		ui->testCaseEdit->setSettings(settings);

		if (curContest)
		{
			const QList<Task *> &taskList = curContest->getTaskList();

			for (auto i : taskList)
				i->refreshCompilerConfiguration(settings);
		}
	}

	ui->resultViewer->refreshViewer();
	ui->statisticsBrowser->refresh();
	delete dialog;
}

void LemonLime::judgeExtButtonFlip(bool stat)
{
	ui->judgeAllButton->setEnabled(stat);
	ui->judgeAllAction->setEnabled(stat);
	ui->judgeUnjudgedButton->setEnabled(stat);
	ui->judgeUnjudgedAction->setEnabled(stat);
	ui->judgeGreyAction->setEnabled(stat);
	ui->judgeMagentaAction->setEnabled(stat);
}

void LemonLime::refreshButtonClicked()
{
	curContest->refreshContestantList();
	ui->resultViewer->refreshViewer();
	ui->statisticsBrowser->refresh();
	judgeExtButtonFlip(ui->resultViewer->rowCount() > 0);
	ui->cleanupAction->setEnabled(true);
	ui->refreshAction->setEnabled(true);
}

void removePath(const QString &path)
{
	if (path.isEmpty())
		return;

	QDir dir(path);

	if (! dir.exists())
		return;

	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
	QFileInfoList fileList = dir.entryInfoList();

	foreach (QFileInfo fi, fileList)
	{
		if (fi.isFile() || fi.isSymLink())
			fi.dir().remove(fi.fileName());
		else
			removePath(fi.absoluteFilePath());
	}

	dir.rmpath(dir.absolutePath());
}

void copyPath(const QString &fromPath, const QString &toPath)
{
	QDir dir(fromPath);

	if (! dir.exists())
		return;

	QString fpath = fromPath + QDir::separator();
	QString tpath = toPath + QDir::separator();
	dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
	QFileInfoList fileList = dir.entryInfoList();

	foreach (QFileInfo fi, fileList)
	{
		QString fn = fpath + fi.fileName();
		QString tn = tpath + fi.fileName();

		if (fi.isFile() || fi.isSymLink())
			QFile::copy(fn, tn);
		else
		{
			QDir toDir(toPath);
			toDir.mkpath(fi.fileName());
			copyPath(fn, tn);
		}
	}
}

void LemonLime::cleanupButtonClicked()
{
	QString text;
	text += tr("Are you sure to Clean up Files?") + "<br>";
	text += tr("Reading guide are recommended.") + "<br>";
	QMessageBox::StandardButton res =
	    QMessageBox::warning(this, tr("Clean up Files"), text,
	                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Abort, QMessageBox::No);

	if (res == QMessageBox::Yes)
	{
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

		if (doBackup == QMessageBox::Abort)
		{
			QMessageBox::information(this, tr("Clean up Files"), tr("Aborted."));
			return;
		}

		if (doBackup == QMessageBox::Yes)
		{
			QDir bkLoca;

			if (bkLoca.exists(backupFolder))
			{
				QMessageBox::information(this, tr("Clean up Files"),
				                         tr("Aborted: `%1' already exist.").arg(backupFolder));
				return;
			}

			if (! bkLoca.mkpath(backupFolder))
			{
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
			basDirLis = basDir.entryInfoList();

			foreach (QFileInfo conDirWho, basDirLis)
			{
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

		for (int i = 0; i < taskList.size(); i++)
		{
			QString taskName = taskList[i]->getSourceFileName();
			typeSet[taskName] = i;
			nameSet.insert(taskName);

			if (taskList[i]->getTaskType() == Task::AnswersOnly)
			{
				for (auto j : taskList[i]->getTestCaseList())
				{
					for (const auto &k : j->getInputFiles())
					{
						QString temp = QFileInfo(k).completeBaseName();
						tarNameSet.insert(temp);
						origSet[temp] = taskName;
					}
				}
			}
			else if (taskList[i]->getTaskType() == Task::Communication)
			{
				QStringList sourcePaths = taskList[i]->getSourceFilesPath();

				for (const auto &j : sourcePaths)
				{
					QString temp = QFileInfo(j).completeBaseName();
					tarNameSet.insert(temp);
					origSet[temp] = taskName;
				}
			}
			else
			{
				tarNameSet.insert(taskName);
				origSet[taskName] = taskName;
			}
		}

		process->setValue(5);
		process->setLabelText(tr("Now Cleaning..."));
		QCoreApplication::processEvents();
		basDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
		basDirLis = basDir.entryInfoList();

		foreach (QFileInfo conDirWho, basDirLis)
		{
			QDir conDir(conDirWho.filePath());
			conDir.setFilter(QDir::Files | QDir::Hidden);
			QFileInfoList conDirLis = conDir.entryInfoList();

			foreach (QFileInfo proFilWho, conDirLis)
			{
				if (proFilWho.suffix().length() <= 0 || proFilWho.suffix().toUpper() == "EXE")
					QFile::remove(proFilWho.absoluteFilePath());
			}

			conDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
			conDirLis = conDir.entryInfoList();

			foreach (QFileInfo proDirWho, conDirLis)
			{
				if (nameSet.contains(proDirWho.fileName()))
				{
					QDir proDir(proDirWho.filePath());
					proDir.setFilter(QDir::Files | QDir::Hidden);

					foreach (QFileInfo sorFilWho, proDir.entryInfoList())
					{
						if (sorFilWho.suffix().length() > 0 && sorFilWho.suffix().toUpper() != "EXE")
							QFile::copy(sorFilWho.filePath(),
							            conDirWho.filePath() + QDir::separator() + sorFilWho.fileName());
					}
				}

				removePath(proDirWho.absoluteFilePath());
			}

			for (const auto &proName : nameSet)
			{
				conDir.mkpath(proName);
			}

			conDir.setFilter(QDir::Files | QDir::Hidden);
			conDirLis = conDir.entryInfoList();

			foreach (QFileInfo proFilWho, conDirLis)
			{
				QString proFilName = proFilWho.fileName();
				QString proName = proFilName;
				proName.truncate(proName.lastIndexOf("."));

				if (tarNameSet.contains(proName))
				{
					QString taskName = origSet[proName];
					int who = typeSet[taskName];
					int types = taskList[who]->getTaskType();

					if (types == Task::Traditional || types == Task::Interaction ||
					    types == Task::Communication)
					{
						if (proFilName != taskList[who]->getInputFileName() &&
						    proFilName != taskList[who]->getOutputFileName())
							QFile::copy(proFilWho.filePath(), conDirWho.filePath() + QDir::separator() +
							                                      taskName + QDir::separator() + proFilName);
					}
					else if (types == Task::AnswersOnly)
					{
						if (proFilWho.suffix() == taskList[who]->getAnswerFileExtension())
							QFile::copy(proFilWho.filePath(), conDirWho.filePath() + QDir::separator() +
							                                      taskName + QDir::separator() + proFilName);
					}
				}

				QFile::remove(proFilWho.absoluteFilePath());
			}

			conDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
			conDirLis = conDir.entryInfoList();

			foreach (QFileInfo proDirWho, conDirLis)
			{
				QDir proDir(proDirWho.filePath());
				proDir.setFilter(QDir::Files | QDir::Hidden);

				foreach (QFileInfo sorFilWho, proDir.entryInfoList())
					QFile::copy(sorFilWho.filePath(),
					            conDirWho.filePath() + QDir::separator() + sorFilWho.fileName());
			}

			process->setValue(process->value() + 1);
			QCoreApplication::processEvents();
		}

		delete process;
		text = tr("Finished.") + "<br>";
		QMessageBox::information(this, tr("Clean up Files"), text);
	}
	else
	{
		QMessageBox::information(this, tr("Clean up Files"), tr("Aborted"));
	}
}

void LemonLime::tabIndexChanged(int index)
{
	if (index != 1)
	{
		judgeExtButtonFlip(false);
		ui->judgeAction->setEnabled(false);
		ui->judgeButton->setEnabled(false);
		ui->cleanupAction->setEnabled(false);
		ui->refreshAction->setEnabled(false);

		if (index == 2)
		{
			ui->statisticsBrowser->refresh();
		}
	}
	else
	{
		QList<QTableWidgetSelectionRange> selectionRange = ui->resultViewer->selectedRanges();

		if (! selectionRange.empty())
		{
			ui->judgeAction->setEnabled(true);
			ui->judgeButton->setEnabled(true);
		}
		else
		{
			ui->judgeAction->setEnabled(false);
			ui->judgeButton->setEnabled(false);
		}

		judgeExtButtonFlip(ui->resultViewer->rowCount() > 0);
		ui->cleanupAction->setEnabled(true);
		ui->refreshAction->setEnabled(true);
	}
}

void LemonLime::moveUpTask()
{
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

void LemonLime::moveDownTask()
{
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

void LemonLime::viewerSelectionChanged()
{
	QList<QTableWidgetSelectionRange> selectionRange = ui->resultViewer->selectedRanges();

	if (! selectionRange.empty())
	{
		ui->judgeButton->setEnabled(true);
		ui->judgeAction->setEnabled(true);
	}
	else
	{
		ui->judgeButton->setEnabled(false);
		ui->judgeAction->setEnabled(false);
	}
}

void LemonLime::contestantDeleted()
{
	judgeExtButtonFlip(ui->resultViewer->rowCount() > 0);
	ui->cleanupAction->setEnabled(true);
	ui->refreshAction->setEnabled(true);
}

void LemonLime::saveContest(const QString &fileName)
{
	QFile file(fileName);

	if (! file.open(QFile::WriteOnly))
	{
		QMessageBox::warning(this, tr("Error"), tr("Cannot open file %1").arg(fileName), QMessageBox::Close);
		ui->statusBar->showMessage(tr("Save Failed"), 1000);
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);
	QByteArray data;
	QDataStream _out(&data, QIODevice::WriteOnly);
	curContest->writeToStream(_out);
	data = qCompress(data);
	QDataStream out(&file);
	out << unsigned(MagicNumber) << qChecksum(data.data(), static_cast<uint>(data.length())) << data.length();
	out.writeRawData(data.data(), data.length());
	QApplication::restoreOverrideCursor();
	ui->statusBar->showMessage(tr("Saved"), 1000);
}

void LemonLime::loadContest(const QString &filePath)
{
	if (curContest)
		closeAction();

	QFile file(filePath);

	if (! file.open(QFile::ReadOnly))
	{
		QMessageBox::warning(this, tr("Error"), tr("Cannot open file %1").arg(QFileInfo(filePath).fileName()),
		                     QMessageBox::Close);
		return;
	}

	QDataStream _in(&file);
	unsigned checkNumber = 0;
	_in >> checkNumber;

	if (checkNumber != unsigned(MagicNumber))
	{
		QMessageBox::warning(this, tr("Error"), tr("File %1 is broken").arg(QFileInfo(filePath).fileName()),
		                     QMessageBox::Close);
		return;
	}

	quint16 checksum = 0;
	int len = 0;
	_in >> checksum >> len;
	char *raw = new char[len];
	_in.readRawData(raw, len);

	if (qChecksum(raw, static_cast<uint>(len)) != checksum)
	{
		QMessageBox::warning(this, tr("Error"), tr("File %1 is broken").arg(QFileInfo(filePath).fileName()),
		                     QMessageBox::Close);
		delete[] raw;
		return;
	}

	QByteArray data(raw, len);
	delete[] raw;
	data = qUncompress(data);
	QDataStream in(data);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	curContest = new Contest(this);
	curContest->setSettings(settings);
	curContest->readFromStream(in);
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
	QApplication::restoreOverrideCursor();
	ui->tabWidget->setCurrentIndex(0);
}

void LemonLime::newContest(const QString &title, const QString &savingName, const QString &path)
{
	if (! QDir(path).exists() && ! QDir().mkpath(path))
	{
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

void LemonLime::newAction()
{
	auto *dialog = new NewContestDialog(this);

	if (dialog->exec() == QDialog::Accepted)
	{
		newContest(dialog->getContestTitle(), dialog->getSavingName(), dialog->getContestPath());
	}

	delete dialog;
}

void LemonLime::closeAction()
{
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

void LemonLime::loadAction()
{
	auto *dialog = new OpenContestDialog(this);
	dialog->setRecentContest(settings->getRecentContest());
	QStringList recentContest = dialog->getRecentContest();

	if (dialog->exec() == QDialog::Accepted)
	{
		QString selectedContest = dialog->getSelectedContest();

		for (int i = 0; i < recentContest.size(); i++)
		{
			if (recentContest[i] == selectedContest)
			{
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

void LemonLime::getFiles(const QString &path, const QStringList &filters, QMap<QString, QString> &files)
{
	QDir dir(path);

	if (! filters.isEmpty())
		dir.setNameFilters(filters);

	QFileInfoList list = dir.entryInfoList(QDir::Files);

	for (auto &i : list)
	{
		files.insert(i.completeBaseName(), i.fileName());
	}
}

void LemonLime::addTask(const QString &title, const QList<QPair<QString, QString>> &testCases, int fullScore,
                        int timeLimit, int memoryLimit)
{
	Task *newTask = new Task;
	newTask->setProblemTitle(title);
	newTask->setSourceFileName(title);
	newTask->setInputFileName(title + ".in");
	newTask->setOutputFileName(title + ".out");
	newTask->refreshCompilerConfiguration(settings);
	newTask->setAnswerFileExtension(settings->getDefaultOutputFileExtension());
	curContest->addTask(newTask);

	for (const auto &testCase : testCases)
	{
		auto *newTestCase = new TestCase;
		newTestCase->setFullScore(fullScore);
		newTestCase->setTimeLimit(timeLimit);
		newTestCase->setMemoryLimit(memoryLimit);
		newTestCase->addSingleCase(title + QDir::separator() + testCase.first,
		                           title + QDir::separator() + testCase.second);
		newTask->addTestCase(newTestCase);
	}
}

void LemonLime::addTaskWithScoreScale(const QString &title, const QList<QPair<QString, QString>> &testCases,
                                      int sumScore, int timeLimit, int memoryLimit)
{
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

	for (int i = 0; i < testCases.size(); i++)
	{
		auto *newTestCase = new TestCase;
		newTestCase->setFullScore(scorePer + static_cast<int>(i < scoreLos));
		newTestCase->setTimeLimit(timeLimit);
		newTestCase->setMemoryLimit(memoryLimit);
		newTestCase->addSingleCase(title + QDir::separator() + testCases[i].first,
		                           title + QDir::separator() + testCases[i].second);
		newTask->addTestCase(newTestCase);
	}
}

auto LemonLime::compareFileName(const QPair<QString, QString> &a, const QPair<QString, QString> &b) -> bool
{
	return (a.first.length() < b.first.length()) ||
	       (a.first.length() == b.first.length() && QString::localeAwareCompare(a.first, b.first) < 0);
}

void LemonLime::addTasksAction()
{
	QStringList list = QDir(Settings::dataPath()).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	QSet<QString> nameSet;
	QList<Task *> taskList = curContest->getTaskList();

	for (auto &i : taskList)
	{
		nameSet.insert(i->getSourceFileName());
	}

	QStringList nameList;
	QList<QList<QPair<QString, QString>>> testCases;

	for (int i = 0; i < list.size(); i++)
	{
		if (! nameSet.contains(list[i]))
		{
			QStringList filters;
			filters = settings->getInputFileExtensions();

			if (filters.isEmpty())
				filters << "in";

			for (int j = 0; j < filters.size(); j++)
			{
				filters[j] = QString("*.") + filters[j];
			}

			QMap<QString, QString> inputFiles;
			getFiles(Settings::dataPath() + list[i], filters, inputFiles);
			filters = settings->getOutputFileExtensions();

			if (filters.isEmpty())
				filters << "out"
				        << "ans";

			for (int j = 0; j < filters.size(); j++)
			{
				filters[j] = QString("*.") + filters[j];
			}

			QMap<QString, QString> outputFiles;
			getFiles(Settings::dataPath() + list[i], filters, outputFiles);
			QList<QPair<QString, QString>> cases;
			QStringList baseNameList = inputFiles.keys();

			for (int j = 0; j < baseNameList.size(); j++)
			{
				if (outputFiles.contains(baseNameList[j]))
				{
					cases.append(qMakePair(inputFiles[baseNameList[j]], outputFiles[baseNameList[j]]));
				}
			}

			std::sort(cases.begin(), cases.end(), compareFileName);

			if (! cases.isEmpty())
			{
				nameList.append(list[i]);
				testCases.append(cases);
			}
		}
	}

	if (nameList.isEmpty())
	{
		QMessageBox::warning(this, tr("LemonLime"), tr("No task found"), QMessageBox::Ok);
		return;
	}

	auto *dialog = new AddTaskDialog(this);
	dialog->resize(dialog->sizeHint());
	dialog->setMaximumSize(dialog->sizeHint());
	dialog->setMinimumSize(dialog->sizeHint());

	for (int i = 0; i < nameList.size(); i++)
	{
		dialog->addTask(nameList[i], qMax(100, testCases[i].size()), settings->getDefaultTimeLimit(),
		                settings->getDefaultMemoryLimit());
	}

	if (dialog->exec() == QDialog::Accepted)
	{
		for (int i = 0; i < nameList.size(); i++)
		{
			addTaskWithScoreScale(nameList[i], testCases[i], dialog->getFullScore(i), dialog->getTimeLimit(i),
			                      dialog->getMemoryLimit(i));
		}
	}

	ui->summary->setContest(curContest);
}

void LemonLime::exportResult() { ExportUtil::exportResult(this, curContest); }

void LemonLime::exportStatstics() { StatisticsBrowser::exportStatstics(this, curContest); }

void LemonLime::changeContestName()
{
	if (! curContest)
	{
		QMessageBox::warning(this, tr("Rename Contest"), tr("No Contest Yet"));
	}

	bool confirmed = false;
	QString newName = QInputDialog::getText(this, tr("Rename Contest"), tr("Write the name you want."),
	                                        QLineEdit::Normal, tr("New Name"), &confirmed);

	if (! confirmed)
	{
		QMessageBox::warning(this, tr("Rename Contest"), tr("The name did not changes."));
		return;
	}

	curContest->setContestTitle(newName);
	setWindowTitle(tr("LemonLime - %1").arg(curContest->getContestTitle()));
	ui->resultViewer->refreshViewer();
	ui->statisticsBrowser->refresh();
	saveContest(curFile);
}

void LemonLime::aboutLemon()
{
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

void LemonLime::actionManual()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Manual"), "llmanual.pdf");

	if (fileName.isEmpty())
		return;

	QFile::copy(":/manual/llmanual.pdf", fileName);
	QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

void LemonLime::actionMore()
{
	QDesktopServices::openUrl(QUrl(QString("https://github.com/Project-LemonLime/Project_LemonLime")));
}

void LemonLime::setUiLanguage()
{
	auto *language = dynamic_cast<QAction *>(sender());
	settings->setUiLanguage(language->data().toString());
	loadUiLanguage();
}
