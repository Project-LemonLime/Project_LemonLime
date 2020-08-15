/***************************************************************************
    This file is part of Project LemonLime
    Copyright (C) 2020 Coelacanthus

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

#pragma once
//
#include <QMap>
#include <QtCore>
#include <algorithm>
#include <ctime>
#include <iostream>
#include <optional>
#include <vector>


#define LEMON_BUILD_INFO QString(LEMON_BUILD_INFO_STR)
#define LEMON_BUILD_EXTRA_INFO QString(LEMON_BUILD_EXTRA_INFO_STR)


namespace Lemon
{
    inline QStringList LemonAssetsPaths(const QString &dirName)
    {
        // Configuration Path
        QStringList list;
#ifdef LEMON_CONFIG_DIR
        list << LEMON_CONFIG_DIR + dirName;
#endif
        list << ":/" + dirName;
        //
#ifdef Q_OS_LINUX
        // Linux platform directories.
        list << QString("/lib/lemon-lime/" + dirName);
        list << QString("/usr/lib/lemon-lime/" + dirName);
        list << QString("/usr/local/lib/lemon-lime/" + dirName);
        //
        list << QString("/usr/share/lemon-lime/" + dirName);
        list << QString("/usr/local/share/lemon-lime/" + dirName);
        // For AppImage?
        list << QString(QDir(QCoreApplication::applicationDirPath() + "/../share/lemon-lime/" + dirName).absolutePath());
        // For Snap
        if (qEnvironmentVariableIsSet("SNAP"))
        {
            list << QString(qEnvironmentVariable("SNAP") + "/usr/share/lemon-lime/" + dirName);
        }
#elif defined(Q_OS_MAC)
        // macOS platform directories.
        list << QDir(QCoreApplication::applicationDirPath() + "/../Resources/" + dirName).absolutePath();
#endif
        list << QStandardPaths::locateAll(QStandardPaths::AppDataLocation, dirName, QStandardPaths::LocateDirectory);
        list << QStandardPaths::locateAll(QStandardPaths::AppConfigLocation, dirName, QStandardPaths::LocateDirectory);
        // This is the default behavior on Windows
        list << QCoreApplication::applicationDirPath() + "/" + dirName;
        list.removeDuplicates();
        return list;
    }

} // namespace Lemon
