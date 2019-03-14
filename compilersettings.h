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
 * compilersettings.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef COMPILERSETTINGS_H
#define COMPILERSETTINGS_H

#include <QtCore>
#include <QtGui>
#include <QWidget>

namespace Ui {
    class CompilerSettings;
}

class Settings;
class Compiler;

class CompilerSettings : public QWidget
{
    Q_OBJECT

public:
    explicit CompilerSettings(QWidget *parent = 0);
    ~CompilerSettings();
    void resetEditSettings(Settings*);
    bool checkValid();

private:
    Ui::CompilerSettings *ui;
    Settings *editSettings;
    Compiler *curCompiler;
    QAction *deleteCompilerKeyAction;
    void setCurrentCompiler(Compiler*);
    void refreshItemState();

private slots:
    void moveUpCompiler();
    void moveDownCompiler();
    void addCompiler();
    void deleteCompiler();
    void compilerNameChanged(const QString&);
    void sourceExtensionsChanged(const QString&);
    void compilerListCurrentRowChanged();
    void advancedButtonClicked();
};

#endif // COMPILERSETTINGS_H
