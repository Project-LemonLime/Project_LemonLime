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

TARGET = lemon
TEMPLATE = app

SOURCES += main.cpp \
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
    exportutil.cpp

win32:SOURCES += qtlockedfile/qtlockedfile_win.cpp
unix:SOURCES += qtlockedfile/qtlockedfile_unix.cpp

HEADERS  += lemon.h \
    contest.h \
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
    exportutil.h

win32:FORMS += forms_win32/lemon.ui \
    forms_win32/taskeditwidget.ui \
    forms_win32/testcaseeditwidget.ui \
    forms_win32/generalsettings.ui \
    forms_win32/compilersettings.ui \
    forms_win32/addtestcaseswizard.ui \
    forms_win32/judgingdialog.ui \
    forms_win32/optionsdialog.ui \
    forms_win32/detaildialog.ui \
    forms_win32/newcontestwidget.ui \
    forms_win32/opencontestwidget.ui \
    forms_win32/newcontestdialog.ui \
    forms_win32/opencontestdialog.ui \
    forms_win32/welcomedialog.ui \
    forms_win32/addtaskdialog.ui \
    forms_win32/advancedcompilersettingsdialog.ui \
    forms_win32/environmentvariablesdialog.ui \
    forms_win32/editvariabledialog.ui \
    forms_win32/addcompilerwizard.ui

unix:FORMS += forms_unix/lemon.ui \
    forms_unix/taskeditwidget.ui \
    forms_unix/testcaseeditwidget.ui \
    forms_unix/generalsettings.ui \
    forms_unix/compilersettings.ui \
    forms_unix/addtestcaseswizard.ui \
    forms_unix/judgingdialog.ui \
    forms_unix/optionsdialog.ui \
    forms_unix/detaildialog.ui \
    forms_unix/newcontestwidget.ui \
    forms_unix/opencontestwidget.ui \
    forms_unix/newcontestdialog.ui \
    forms_unix/opencontestdialog.ui \
    forms_unix/welcomedialog.ui \
    forms_unix/addtaskdialog.ui \
    forms_unix/advancedcompilersettingsdialog.ui \
    forms_unix/environmentvariablesdialog.ui \
    forms_unix/editvariabledialog.ui \
    forms_unix/addcompilerwizard.ui

TRANSLATIONS += lemon_zh_CN.ts

win32:RC_FILE = lemon.rc

win32:LIBS += -lpsapi

win32:QT += axcontainer

RESOURCES += resource.qrc
