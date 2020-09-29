/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

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
		list << QString(
		    QDir(QCoreApplication::applicationDirPath() + "/../share/lemon-lime/" + dirName).absolutePath());
		// For Snap
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
		if (qEnvironmentVariableIsSet("SNAP"))
		{
			list << QString(qEnvironmentVariable("SNAP") + "/usr/share/lemon-lime/" + dirName);
		}
#endif
#elif defined(Q_OS_MAC)
		// macOS platform directories.
		list << QDir(QCoreApplication::applicationDirPath() + "/../Resources/" + dirName).absolutePath();
#endif
		list << QStandardPaths::locateAll(QStandardPaths::AppDataLocation, dirName,
		                                  QStandardPaths::LocateDirectory);
		list << QStandardPaths::locateAll(QStandardPaths::AppConfigLocation, dirName,
		                                  QStandardPaths::LocateDirectory);
		// This is the default behavior on Windows
		list << QCoreApplication::applicationDirPath() + "/" + dirName;
		list.removeDuplicates();
		return list;
	}

} // namespace Lemon
