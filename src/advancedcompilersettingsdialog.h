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

#include <QDialog>

class Compiler;

namespace Ui {
	class AdvancedCompilerSettingsDialog;
}

class AdvancedCompilerSettingsDialog : public QDialog {
	Q_OBJECT

  public:
	explicit AdvancedCompilerSettingsDialog(QWidget *parent = nullptr);
	~AdvancedCompilerSettingsDialog();
	void resetEditCompiler(Compiler *);
	Compiler *getEditCompiler() const;

  private:
	Ui::AdvancedCompilerSettingsDialog *ui;
	Compiler *editCompiler;
	int configCount{};

  private slots:
	void okayButtonClicked();
	void compilerTypeChanged();
	void compilerLocationChanged();
	void interpreterLocationChanged();
	void selectCompilerLocation();
	void selectInterpreterLocation();
	void bytecodeExtensionsChanged();
	void timeLimitRatioChanged();
	void memoryLimitRatioChanged();
	void disableMemoryLimitCheckChanged();
	void configurationIndexChanged();
	void configurationTextChanged();
	void deleteConfiguration();
	void compilerArgumentsChanged();
	void interpreterArgumentsChanged();
	void environmentVariablesButtonClicked();
};
