#
#    Project Lemon - A tiny judging environment for OI contest
#    Copyright (C) 2011 Zhipeng Jia
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

QT       += core gui network
QT       += core gui widgets
QMAKE_CXXFLAGS += -std=c++11
unix:QMAKE_LFLAGS += -no-pie

TARGET = lemon
TEMPLATE = app

SOURCES += main.cpp \
    exttestcasemodifier.cpp \
    exttestcasemodifierdialog.cpp \
    exttestcasetable.cpp \
    exttestcaseupdater.cpp \
    visualsettings.cpp \
    lemon.cpp \
    contest.cpp \
    task.cpp \
    testcase.cpp \
    settings.cpp \
    compiler.cpp \
    filelineedit.cpp \
    summarytree.cpp \
    taskeditwidget.cpp \
    testcaseeditwidget.cpp \
    generalsettings.cpp \
    compilersettings.cpp \
    addtestcaseswizard.cpp \
    contestant.cpp \
    judgingdialog.cpp \
    judgingthread.cpp \
    optionsdialog.cpp \
    resultviewer.cpp \
    assignmentthread.cpp \
    detaildialog.cpp \
    newcontestwidget.cpp \
    opencontestwidget.cpp \
    newcontestdialog.cpp \
    opencontestdialog.cpp \
    welcomedialog.cpp \
    addtaskdialog.cpp \
    qtlockedfile/qtlockedfile.cpp \
    qtsingleapplication/qtsinglecoreapplication.cpp \
    qtsingleapplication/qtsingleapplication.cpp \
    qtsingleapplication/qtlocalpeer.cpp \
    advancedcompilersettingsdialog.cpp \
    environmentvariablesdialog.cpp \
    editvariabledialog.cpp \
    addcompilerwizard.cpp \
    exportutil.cpp \
    statisticsbrowser.cpp

win32:SOURCES += qtlockedfile/qtlockedfile_win.cpp
unix:SOURCES += qtlockedfile/qtlockedfile_unix.cpp

HEADERS  += lemon.h \
    exttestcasemodifier.h \
    contest.h \
    exttestcasemodifierdialog.h \
    exttestcasetable.h \
    exttestcaseupdater.h \
    visualsettings.h \
    task.h \
    testcase.h \
    settings.h \
    compiler.h \
    filelineedit.h \
    summarytree.h \
    taskeditwidget.h \
    testcaseeditwidget.h \
    generalsettings.h \
    compilersettings.h \
    addtestcaseswizard.h \
    contestant.h \
    judgingdialog.h \
    judgingthread.h \
    optionsdialog.h \
    resultviewer.h \
    assignmentthread.h \
    globaltype.h \
    detaildialog.h \
    newcontestwidget.h \
    opencontestwidget.h \
    newcontestdialog.h \
    opencontestdialog.h \
    welcomedialog.h \
    addtaskdialog.h \
    qtlockedfile/qtlockedfile.h \
    qtsingleapplication/qtsinglecoreapplication.h \
    qtsingleapplication/qtsingleapplication.h \
    qtsingleapplication/qtlocalpeer.h \
    advancedcompilersettingsdialog.h \
    environmentvariablesdialog.h \
    editvariabledialog.h \
    addcompilerwizard.h \
    exportutil.h \
    statisticsbrowser.h \
    judgesharedvariables.h


FORMS += forms/lemon.ui \
    forms/exttestcaseupdater.ui \
    forms/exttestcasemodifierdialog.ui \
    forms/exttestcasemodifier.ui \
    forms/taskeditwidget.ui \
    forms/testcaseeditwidget.ui \
    forms/generalsettings.ui \
    forms/compilersettings.ui \
    forms/addtestcaseswizard.ui \
    forms/judgingdialog.ui \
    forms/optionsdialog.ui \
    forms/detaildialog.ui \
    forms/newcontestwidget.ui \
    forms/opencontestwidget.ui \
    forms/newcontestdialog.ui \
    forms/opencontestdialog.ui \
    forms/visualsettings.ui \
    forms/welcomedialog.ui \
    forms/addtaskdialog.ui \
    forms/advancedcompilersettingsdialog.ui \
    forms/environmentvariablesdialog.ui \
    forms/editvariabledialog.ui \
    forms/addcompilerwizard.ui \
    forms/statisticsbrowser.ui

TRANSLATIONS += lemon_zh_CN.ts

win32:RC_FILE = lemon.rc

win32:LIBS += -lpsapi

win32:QT += axcontainer

RESOURCES += resource.qrc
