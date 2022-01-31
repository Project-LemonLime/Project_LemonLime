/*
 * SPDX-FileCopyrightText: 2021-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//
#include <QList>
#include <QObject>
#include <QString>

namespace Lemon {
	struct LemonStartupArguments {
		enum Argument {
			NORMAL = 0,
			EXIT = 1,
			LEMON_LINK = 2 // Maybe support URL...
		};
		QList<Argument> arguments;
		QString version;
		int buildVersion;
		QString data;
		QList<QString> links;
		QList<QString> fullArgs;
		//
		bool debugLog;
		bool exitLemon;
	};

	class LemonApplicationInterface {

	  public:
		explicit LemonApplicationInterface();
		~LemonApplicationInterface();

		Lemon::LemonStartupArguments StartupArguments;
	};

	inline LemonApplicationInterface *LemonCoreApplication = nullptr;
} // namespace Lemon
