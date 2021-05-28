/*
 * SPDX-FileCopyrightText: 2020-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <base/LemonBase.hpp>
//
#include <QJsonObject>

class Compiler;

namespace Lemon::base::config {

	class LemonConfigJudge {
	  private:
		QList<Compiler *> compilerList;
		int defaultFullScore{};
		int defaultTimeLimit{};
		int defaultMemoryLimit{};
		int compileTimeLimit{};
		int specialJudgeTimeLimit{};
		int fileSizeLimit{};
		int rejudgeTimes{};
        double defaultExtraTimeRatio{};
		QString defaultInputFileExtension;
		QString defaultOutputFileExtension;
		QStringList inputFileExtensions;
		QStringList outputFileExtensions;
		QStringList recentContest;
		QString diffPath;

	  public:
		int read(const QJsonObject &json);
		void write(QJsonObject &json) const;
	};

	class LemonConfigUI {
	  private:
		QString language = "en_US";
		// Prepare for theme setting
		// TODO: Theme support
		QString theme = "";

	  public:
		int read(const QJsonObject &json);
		void write(QJsonObject &json) const;
	};

	class LemonConfig {
	  private:
		LemonConfigJudge judgeConfig;
		LemonConfigUI uiConfig;
		int splashTime{};

	  public:
		enum SaveFormat { Json, Binary, Yaml };
		void read(const QJsonObject &json);
		void write(QJsonObject &json) const;
		bool loadConfig(SaveFormat saveFormat);
		bool saveConfig(SaveFormat saveFormat) const;
	};

} // namespace Lemon::base::config
