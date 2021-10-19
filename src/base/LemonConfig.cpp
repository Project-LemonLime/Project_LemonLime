/*
 * SPDX-FileCopyrightText: 2020-2021 Project LemonLime
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
		READ_JSON(json, maxJudgingThreads);

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
			for (int i = 0; i < _compilerList.size(); ++i) {
				QJsonObject compilerObject = _compilerList[i].toObject();
				Compiler *compiler = new Compiler;
				if (compiler->read(compilerObject) == -1)
					return -1;
				compilerList.append(compiler);
			}
		} else
			return -1;
		return 0;
	}

	void LemonConfigJudge::write(QJsonObject &json) const {
		WRITE_JSON(json, defaultFullScore);
		WRITE_JSON(json, defaultTimeLimit);
		WRITE_JSON(json, defaultMemoryLimit);
		WRITE_JSON(json, compileTimeLimit);
		WRITE_JSON(json, specialJudgeTimeLimit);
		WRITE_JSON(json, fileSizeLimit);
		WRITE_JSON(json, rejudgeTimes);
		WRITE_JSON(json, maxJudgingThreads);

		WRITE_JSON(json, defaultInputFileExtension);
		WRITE_JSON(json, defaultOutputFileExtension);
		WRITE_JSON(json, diffPath);

		WRITE_JSON(json, inputFileExtensions);
		WRITE_JSON(json, outputFileExtensions);
		WRITE_JSON(json, recentContest);

		QJsonArray compilerList;
		for (const auto compiler : this->compilerList) {
			QJsonObject obj;
			compiler->write(obj);
			compilerList.append(obj);
		}
		WRITE_JSON(json, compilerList);
	}

} // namespace Lemon::base::config
