/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QCompleter>
#include <QLineEdit>
#include <QtCore>
#include <QtWidgets>

class FileLineEdit : public QLineEdit {
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
