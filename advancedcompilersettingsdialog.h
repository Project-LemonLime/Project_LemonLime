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
 * advancedcompilersettingsdialog.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef ADVANCEDCOMPILERSETTINGSDIALOG_H
#define ADVANCEDCOMPILERSETTINGSDIALOG_H

#include <QtCore>
#include <QtGui>
#include <QDialog>

class Compiler;

namespace Ui {
    class AdvancedCompilerSettingsDialog;
}

class AdvancedCompilerSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdvancedCompilerSettingsDialog(QWidget *parent = 0);
    ~AdvancedCompilerSettingsDialog();
    void resetEditCompiler(Compiler*);
    Compiler* getEditCompiler() const;

private:
    Ui::AdvancedCompilerSettingsDialog *ui;
    Compiler *editCompiler;
    int configCount;

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

#endif // ADVANCEDCOMPILERSETTINGSDIALOG_H
