/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//
#include <SingleApplication>

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
} // namespace Lemon

class LemonBaseApplication : public SingleApplication {
	Q_OBJECT

  public:
	LemonBaseApplication(int &argc, char *argv[])
	    : SingleApplication(argc, argv, true, User | ExcludeAppPath | ExcludeAppVersion){};
	virtual ~LemonBaseApplication(){};

	Lemon::LemonStartupArguments StartupArguments;

	virtual bool Initialize() final;

  private:
	bool parseCommandLine(bool *canContinue, QString *errorMessage);
};
