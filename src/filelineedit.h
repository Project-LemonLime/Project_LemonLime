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
 * filelineedit.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef FILELINEEDIT_H
#define FILELINEEDIT_H

#include <QCompleter>
#include <QLineEdit>
#include <QtCore>
#include <QtGui>

class FileLineEdit : public QLineEdit
{
	Q_OBJECT
  public:
	explicit FileLineEdit(QWidget *parent = nullptr);
	void setFilters(QDir::Filters);
	void setFileExtensions(const QStringList &);
	void getFiles(const QString &, const QString &, QStringList &);

  private:
	QCompleter *completer;
	QStringList nameFilters;
	QDir::Filters filters;

  public slots:
	void refreshFileList();
};

#endif // FILELINEEDIT_H
