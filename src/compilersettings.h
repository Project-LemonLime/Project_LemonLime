/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QWidget>
#include <QtCore>
#include <QtGui>

namespace Ui
{
	class CompilerSettings;
}

class Settings;
class Compiler;

class CompilerSettings : public QWidget
{
	Q_OBJECT

  public:
	explicit CompilerSettings(QWidget *parent = nullptr);
	~CompilerSettings();
	void resetEditSettings(Settings *);
	bool checkValid();

  private:
	Ui::CompilerSettings *ui;
	Settings *editSettings{};
	Compiler *curCompiler{};
	QAction *deleteCompilerKeyAction;
	void setCurrentCompiler(Compiler *);
	void refreshItemState();

  private slots:
	void moveUpCompiler();
	void moveDownCompiler();
	void addCompiler();
	void deleteCompiler();
	void compilerNameChanged(const QString &);
	void sourceExtensionsChanged(const QString &);
	void compilerListCurrentRowChanged();
	void advancedButtonClicked();
};
