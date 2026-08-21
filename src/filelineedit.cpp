/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "filelineedit.h"
//
#include "base/settings.h"
//
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

FileLineEdit::FileLineEdit(QWidget *parent) : QLineEdit(parent) {
	completer = nullptr;
	QAction *browseAction = addAction(QIcon(":/icon/document-open.svg"), QLineEdit::TrailingPosition);
	browseAction->setToolTip(tr("Browse..."));
	connect(browseAction, &QAction::triggered, this, &FileLineEdit::browse);
}

void FileLineEdit::getFiles(const QString &curDir, const QString &prefix, QStringList &files) {
	QDir dir(curDir);

	if (! nameFilters.isEmpty()) {
		dir.setNameFilters(nameFilters);
	}

	QStringList fileList = dir.entryList(filters);

	for (auto &f : fileList)
		f = prefix + f;

	files.append(fileList);

	// Get file in Child dir
	QStringList dirList = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (auto &&d : dirList) {
		getFiles(curDir + d + QDir::separator(), prefix + d + QDir::separator(), files);
	}
}

void FileLineEdit::setFilters(QDir::Filters _filters) { filters = _filters; }

void FileLineEdit::setFileExtensions(const QStringList &extensions) {
	nameFilters.clear();

	for (int i = 0; i < extensions.size(); i++) {
		nameFilters.append("*." + extensions[i]);
	}

	refreshFileList();
}

void FileLineEdit::refreshFileList() {
	QStringList files;
	getFiles(Settings::dataPath(), "", files);
	delete completer;
	completer = new QCompleter(files, this);
	setCompleter(completer);
}

void FileLineEdit::browse() {
	QDir dataDir(QDir::currentPath());
	dataDir.mkpath(Settings::dataPath());
	dataDir.cd(Settings::dataPath());
	QString filePath = QFileDialog::getOpenFileName(this, tr("Choose File"), dataDir.absolutePath());

	if (filePath.isEmpty())
		return;

	QString relPath = dataDir.relativeFilePath(filePath);

	if (relPath.startsWith("..") || QDir::isAbsolutePath(relPath)) {
		QMessageBox::warning(this, tr("Warning"), tr("Please select a file inside the data directory."),
		                     QMessageBox::Close);
		return;
	}

	setText(QDir::toNativeSeparators(relPath));
}
