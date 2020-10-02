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
#define makeAbs(p) QDir(p).absolutePath()
		// Configuration Path
		QStringList list;

		// This is the default behavior on Windows
		list << makeAbs(QCoreApplication::applicationDirPath() + "/" + dirName);
#ifdef LEMON_CONFIG_DIR
		list << makeAbs(LEMON_CONFIG_DIR + dirName);
#endif
		list << ":/" + dirName;
		//
		list << QStandardPaths::locateAll(QStandardPaths::AppDataLocation, dirName,
		                                  QStandardPaths::LocateDirectory);
		list << QStandardPaths::locateAll(QStandardPaths::AppConfigLocation, dirName,
		                                  QStandardPaths::LocateDirectory);

#ifdef Q_OS_LINUX
		// Linux platform directories.
		list << makeAbs("/lib/lemon-lime/" + dirName);
		list << makeAbs("/usr/lib/lemon-lime/" + dirName);
		list << makeAbs("/usr/local/lib/lemon-lime/" + dirName);
		//
		list << makeAbs("/usr/share/lemon-lime/" + dirName);
		list << makeAbs("/usr/local/share/lemon-lime/" + dirName);
		// For AppImage?
		list << makeAbs(QCoreApplication::applicationDirPath() + "/../share/lemon-lime/" + dirName);
		// For Snap
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
		if (qEnvironmentVariableIsSet("SNAP"))
		{
			list << makeAbs(qEnvironmentVariable("SNAP") + "/usr/share/lemon-lime/" + dirName);
		}
		if (qEnvironmentVariableIsSet("APPIMAGE"))
			list << makeAbs(QCoreApplication::applicationDirPath() + "/../share/lemon-lime/" + dirName);
#endif
#elif defined(Q_OS_MAC)
		// macOS platform directories.
		list << QDir(QCoreApplication::applicationDirPath() + "/../Resources/" + dirName).absolutePath();
#endif
		list.removeDuplicates();
		return list;
#undef makeAbs
	}

} // namespace Lemon
