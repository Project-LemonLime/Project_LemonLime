/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "LemonBaseApplication.hpp"
//
#include "base/LemonLog.hpp"
#include "base/LemonTranslator.hpp"
#include "base/settings.h" // TODO: Config Refactor
//
#include <QCoreApplication>
//
#define LEMON_MODULE_NAME "LemonBaseApplication"

using namespace Lemon;

auto LemonBaseApplication::Initialize() -> bool {
	QString errorMessage;
	bool canContinue;
	const auto hasError = parseCommandLine(&canContinue, &errorMessage);
	if (hasError) {
		// LOG("Command line: " + A(errorMessage));
		if (! canContinue) {
			LOG("Fatal Error, LemonLime cannot continue.");
			return false;
		} else {
			LOG("Non-fatal error, LemonLime will continue starting up.");
		}
	}

	// Load Translations
	Settings *settings = new Settings;
	settings->loadSettings();
	LemonLimeTranslator = std::make_unique<LemonTranslator>();
	const auto allTranslations = LemonLimeTranslator->GetAvailableLanguages();
	const auto osLanguage = QLocale::system().name();
	if (! allTranslations.contains(settings->getUiLanguage())) {
		// If we need to reset the language.
		if (allTranslations.contains(osLanguage)) {
			settings->setUiLanguage(osLanguage);
		} else if (! allTranslations.isEmpty()) {
			settings->setUiLanguage(allTranslations.first());
		}
	}
	LemonLimeTranslator->InstallTranslation(settings->getUiLanguage());

	return true;
}

auto LemonBaseApplication::parseCommandLine(bool *canContinue, QString *errorMessage) -> bool {
	*canContinue = true;
	QStringList filteredArgs;
	for (const auto &arg : arguments()) {
#ifdef Q_OS_MACOS
		if (arg.contains("-psn"))
			continue;
#endif
		filteredArgs << arg;
	}
	QCommandLineParser parser;
	//
	QCommandLineOption debugLogOption("debug", QObject::tr("Enable debug output"));
	//
	parser.setApplicationDescription(QObject::tr("LemonLime - A tiny judging environment for OI contest."));
	parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
	//
	parser.addOption(debugLogOption);
	//
	const auto helpOption = parser.addHelpOption();
	const auto versionOption = parser.addVersionOption();

	if (! parser.parse(filteredArgs)) {
		*canContinue = true;
		*errorMessage = parser.errorText();
		return false;
	}

	if (parser.isSet(versionOption)) {
		parser.showVersion();
		return true;
	}

	if (parser.isSet(helpOption)) {
		parser.showHelp();
		return true;
	}

	/*
#define ProcessExtraStartupOptions(option)                                                                   \
	DEBUG("Startup Options:" + A(parser.isSet(option##Option)));                                             \
	StartupArguments.option = parser.isSet(option##Option);

	ProcessExtraStartupOptions(debugLog);
	*/
	StartupArguments.debugLog = parser.isSet(debugLogOption);
	return true;
}
