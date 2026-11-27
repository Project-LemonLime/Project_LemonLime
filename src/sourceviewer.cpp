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

// Resolve one submitted source file, mirroring TaskJudger's layout (including
// the sub-folder layout used by tasks with SubFolderCheck enabled).
QString contestantSourceFilePath(Contest *contest, Contestant *contestant, int taskIndex, int fileIndex) {
	const Task *task = contest->getTask(taskIndex);

	if (task == nullptr || task->getTaskType() == Task::AnswersOnly)
		return {};

	// Communication tasks store a space-separated list of submitted files
	const QStringList files = contestant->getSourceFile(taskIndex).split(' ', Qt::SkipEmptyParts);

	if (fileIndex < 0 || fileIndex >= files.size())
		return {};

	const QString &fileName = files[fileIndex];

	if (fileName.isEmpty())
		return {};

	QString path = Settings::sourcePath() + contestant->getContestantName();

	if (task->getSubFolderCheck())
		path += QDir::separator() + task->getSourceFileName();

	path += QDir::separator() + fileName;
	return path;
}

} // namespace

void SourceViewer::viewSource(QWidget *parent, Contest *contest, Contestant *contestant, int taskIndex) {
	QStringList filePaths;

	for (int i = 0;; i++) {
		const QString path = contestantSourceFilePath(contest, contestant, taskIndex, i);

		if (path.isEmpty())
			break;

		filePaths.append(path);
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
