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

#include <QDialog>
#include <QtCore>
#include <QtWidgets>

class Contestant;
class Contest;

namespace Ui {
	class DetailDialog;
}

class DetailDialog : public QDialog {
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
