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
 * exportutil.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef EXPORTUTIL_H
#define EXPORTUTIL_H

#include "globaltype.h"
#include <QObject>
#include <QtCore>
#include <QtGui>

#ifdef Q_OS_WIN32
#include <QAxObject>
#endif

class Contest;
class Contestant;

class ExportUtil : public QObject
{
	Q_OBJECT
  public:
	explicit ExportUtil(QObject *parent = nullptr);
	static void exportResult(QWidget *, Contest *);

  private:
	static QString getContestantHtmlCode(Contest *, Contestant *, int);
	static QString getSmallerContestantHtmlCode(Contest *, Contestant *);
	static void exportHtml(QWidget *, Contest *, const QString &, int);
	static void exportSmallerHtml(QWidget *, Contest *, const QString &);
	static void exportCsv(QWidget *, Contest *, const QString &);
	static void exportXls(QWidget *, Contest *, const QString &);

  signals:

  public slots:
};

#endif // EXPORTUTIL_H
