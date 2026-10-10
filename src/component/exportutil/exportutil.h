/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include "base/LemonType.hpp"
#include <QJsonObject>
#include <QObject>

class Contest;
class Contestant;

class ExportUtil : public QObject {
	Q_OBJECT
  public:
	explicit ExportUtil(QObject *parent = nullptr);
	static void exportResult(QWidget *, Contest *);

  private:
	static QJsonObject buildExportJson(Contest *);
	static void exportHtml(QWidget *, Contest *, const QString &);
	static void exportCsv(QWidget *, Contest *, const QString &);
  signals:

  public slots:
};
