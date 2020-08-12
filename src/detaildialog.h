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
 * detaildialog.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef DETAILDIALOG_H
#define DETAILDIALOG_H

#include <QDialog>
#include <QtCore>
#include <QtGui>

class Contestant;
class Contest;

namespace Ui
{
	class DetailDialog;
}

class DetailDialog : public QDialog
{
	Q_OBJECT

  public:
	explicit DetailDialog(QWidget *parent = nullptr);
	~DetailDialog();
	void refreshViewer(Contest *, Contestant *);
	void showDialog();

  private:
	Ui::DetailDialog *ui;
	Contest *contest{};
	Contestant *contestant{};

  private slots:
	void anchorClicked(const QUrl &);

  signals:
	void rejudgeSignal();
};

#endif // DETAILDIALOG_H
