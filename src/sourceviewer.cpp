/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "sourceviewer.h"
//
#include "base/settings.h"
#include "core/contest.h"
#include "core/contestant.h"
#include "core/task.h"
//
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QUrl>

namespace {

// Mirror TaskJudger::traditionalTaskPrepare()'s layout: the contestant's
// source directory gains the task's source-file-name sub-folder only when
// SubFolderCheck is enabled.
QString contestantSourceDir(Contest *contest, Contestant *contestant, int taskIndex) {
	const Task *task = contest->getTask(taskIndex);
	QString dir = Settings::sourcePath() + contestant->getContestantName();

	if (task->getSubFolderCheck())
		dir += QDir::separator() + task->getSourceFileName();

	return dir;
}

} // namespace

void SourceViewer::viewSource(QWidget *parent, Contest *contest, Contestant *contestant, int taskIndex) {
	const Task *task = contest->getTask(taskIndex);

	if (task == nullptr || task->getTaskType() == Task::AnswersOnly) {
		QMessageBox::information(parent, tr("View Source"), tr("Cannot find the source file of this task."));
		return;
	}

	const QString dir = contestantSourceDir(contest, contestant, taskIndex);

	// Communication tasks: getSourceFile() holds the compiled names (e.g.
	// send.cpp), while getSourceFilesPath() holds the actual submitted files.
	// Ordinary tasks: getSourceFile() is one complete file name, which must not
	// be split on spaces.
	QStringList fileNames;

	if (task->getTaskType() == Task::Communication || task->getTaskType() == Task::CommunicationExec)
		fileNames = task->getSourceFilesPath();
	else
		fileNames = {contestant->getSourceFile(taskIndex)};

	QStringList filePaths;

	for (const QString &fileName : fileNames) {
		if (! fileName.isEmpty())
			filePaths.append(dir + QDir::separator() + fileName);
	}

	if (filePaths.isEmpty()) {
		QMessageBox::information(parent, tr("View Source"), tr("Cannot find the source file of this task."));
		return;
	}

	for (const QString &path : filePaths) {
		if (! QFileInfo::exists(path)) {
			QMessageBox::information(parent, tr("View Source"),
			                         tr("Source file %1 does not exist.").arg(QDir::toNativeSeparators(path)));
			return;
		}
	}

	for (const QString &path : filePaths)
		QDesktopServices::openUrl(QUrl::fromLocalFile(QDir(path).absolutePath()));
}
