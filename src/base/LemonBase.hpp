/*
 * SPDX-FileCopyrightText: 2020-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//
#include <QCoreApplication>
#include <QDir>
#include <QFont>
#include <QFontDatabase>
#include <QMap>
#include <QStandardPaths>

#include <algorithm>
#include <ctime>
#include <iostream>
#include <optional>
#include <vector>

#define LEMON_BUILD_INFO QString(LEMON_BUILD_INFO_STR)
#define LEMON_BUILD_EXTRA_INFO QString(LEMON_BUILD_EXTRA_INFO_STR)

namespace Lemon {
	// The UI font follows the platform/user preference. No family is hard-coded:
	// the system font fallback (Fontconfig on Linux, DirectWrite on Windows,
	// Core Text on macOS) resolves the user's chosen font and supplies CJK
	// glyphs automatically.
	inline QFont uiFont() {
		QFont font;
		font.setStyleHint(QFont::System);
		return font;
	}

	// Fixed-pitch font for program text. QFontDatabase::FixedFont asks the
	// platform font database (Fontconfig on Linux, DirectWrite on Windows,
	// Core Text on macOS) for the user's preferred monospaced font, without
	// assuming any specific family name. CJK glyphs are handled by the
	// platform's font fallback mechanism.
	inline QFont codeFont() {
		return QFontDatabase::systemFont(QFontDatabase::FixedFont);
	}

	inline QStringList LemonAssetsPaths(const QString &dirName) {
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
		// For Snap
		if (qEnvironmentVariableIsSet("SNAP")) {
			list << makeAbs(qEnvironmentVariable("SNAP") + "/usr/share/lemon-lime/" + dirName);
		}
		if (qEnvironmentVariableIsSet("APPIMAGE"))
			list << makeAbs(QCoreApplication::applicationDirPath() + "/../share/lemon-lime/" + dirName);
#elif defined(Q_OS_MAC)
		// macOS platform directories.
		list << QDir(QCoreApplication::applicationDirPath() + "/../Resources/" + dirName).absolutePath();
#endif
		list.removeDuplicates();
		return list;
#undef makeAbs
	}

} // namespace Lemon
