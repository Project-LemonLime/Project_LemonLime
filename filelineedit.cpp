/***************************************************************************
    This file is part of Project Lemon
    Copyright (C) 2011 Zhipeng Jia

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/
/**
 * filelineedit.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "filelineedit.h"
#include "settings.h"

FileLineEdit::FileLineEdit(QWidget *parent) :
	QLineEdit(parent)
{
	completer = nullptr;
}

void FileLineEdit::getFiles(const QString &curDir, const QString &prefix, QStringList &files)
{
	QDir dir(curDir);

	if (! nameFilters.isEmpty())
	{
		dir.setNameFilters(nameFilters);
	}

	QStringList list = dir.entryList(filters);

	for (int i = 0; i < list.size(); i ++)
	{
		list[i] = prefix + list[i];
	}

	files.append(list);
	list = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (int i = 0; i < list.size(); i ++)
	{
		getFiles(curDir + list[i] + QDir::separator(),
		         prefix + list[i] + QDir::separator(), files);
	}
}

void FileLineEdit::setFilters(QDir::Filters _filters)
{
	filters = _filters;
}

void FileLineEdit::setFileExtensions(const QStringList &extensions)
{
	nameFilters.clear();

	for (int i = 0; i < extensions.size(); i ++)
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
