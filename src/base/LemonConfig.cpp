/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "LemonConfig.hpp"
//
#include "base/LemonUtils.hpp"
#include "base/compiler.h"

namespace Lemon::base::config {

	int LemonConfigJudge::read(const QJsonObject &json) {
		READ_JSON(json, defaultFullScore);
		READ_JSON(json, defaultTimeLimit);
		READ_JSON(json, defaultMemoryLimit);
		READ_JSON(json, compileTimeLimit);
		READ_JSON(json, specialJudgeTimeLimit);
		READ_JSON(json, fileSizeLimit);
		READ_JSON(json, rejudgeTimes);

		READ_JSON(json, defaultInputFileExtension);
		READ_JSON(json, defaultOutputFileExtension);
		READ_JSON(json, diffPath);

		READ_JSON(json, inputFileExtensions);
		READ_JSON(json, outputFileExtensions);
		READ_JSON(json, recentContest);

		// CompilerList
		if (json.contains("compilerList") && json["compilerList"].isArray()) {
			QJsonArray _compilerList = json["compilerList"].toArray();
			compilerList.clear();
			compilerList.reserve(_compilerList.size());
			for (int compilerIndex = 0; compilerIndex < _compilerList.size(); ++compilerIndex) {
				QJsonObject compilerObject = _compilerList[compilerIndex].toObject();
				Compiler *compiler = new Compiler;
				if (compiler->read(compilerObject) == -1)
					return -1;
				compilerList.append(compiler);
			}
		}
		return 0;
	}

	void LemonConfigJudge::write(QJsonObject &json) const {
		// TODO: Write Config
	}

} // namespace Lemon::base::config
