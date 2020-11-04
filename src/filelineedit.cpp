/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "filelineedit.h"
//
#include "core/settings.h"

FileLineEdit::FileLineEdit(QWidget *parent) : QLineEdit(parent) { completer = nullptr; }

void FileLineEdit::getFiles(const QString &curDir, const QString &prefix, QStringList &files)
{
	QDir dir(curDir);

	if (! nameFilters.isEmpty())
	{
		dir.setNameFilters(nameFilters);
	}

	QStringList list = dir.entryList(filters);

	for (int i = 0; i < list.size(); i++)
	{
		list[i] = prefix + list[i];
	}

	files.append(list);
	list = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (int i = 0; i < list.size(); i++)
	{
		getFiles(curDir + list[i] + QDir::separator(), prefix + list[i] + QDir::separator(), files);
	}
}

void FileLineEdit::setFilters(QDir::Filters _filters) { filters = _filters; }

void FileLineEdit::setFileExtensions(const QStringList &extensions)
{
	nameFilters.clear();

	for (int i = 0; i < extensions.size(); i++)
	{
		nameFilters.append("*." + extensions[i]);
	}

	refreshFileList();
}

void FileLineEdit::refreshFileList()
{
	QStringList files;
	getFiles(Settings::dataPath(), "", files);
	delete completer;
	completer = new QCompleter(files, this);
	setCompleter(completer);
}
