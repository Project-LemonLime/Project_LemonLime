/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include "base/LemonType.hpp"
#include <QObject>

#ifdef ENABLE_XLS_EXPORT
#include <QAxObject>
#endif

class Contest;
class Contestant;

class ExportUtil : public QObject {
	Q_OBJECT
  public:
	explicit ExportUtil(QObject *parent = nullptr);
	static void exportResult(QWidget *, Contest *);

  private:
	static QString getContestantHtmlCode(Contest *, Contestant *, int);
	static QString getSmallerContestantHtmlCode(Contest *, Contestant *);
	static void exportHtml(QWidget *, Contest *, const QString &);
	static void exportSmallerHtml(QWidget *, Contest *, const QString &);
	static void exportCsv(QWidget *, Contest *, const QString &);
#ifdef ENABLE_XLS_EXPORT
	static void exportXls(QWidget *, Contest *, const QString &);
#endif
  signals:

  public slots:
};
