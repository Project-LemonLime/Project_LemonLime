/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <base/LemonBase.hpp>
#include <core/compiler.h>
//

namespace Lemon::base::config
{

	struct LemonConfigJudge
	{
		QList<Compiler *> compilerList;
		int defaultFullScore{};
		int defaultTimeLimit{};
		int defaultMemoryLimit{};
		int compileTimeLimit{};
		int specialJudgeTimeLimit{};
		int fileSizeLimit{};
		int rejudgeTimes{};
		QString defaultInputFileExtension;
		QString defaultOutputFileExtension;
		QStringList inputFileExtensions;
		QStringList outputFileExtensions;
		QStringList recentContest;
		QString diffPath;
	};

	struct LemonConfigUI
	{
		QString language = "en_US";
		// Prepare for theme setting
		// QString theme = ;
	};

} // namespace Lemon::base::config
