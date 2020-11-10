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

namespace Lemon::base::config
{

	void LemonConfigJudge::read(const QJsonObject &json)
	{
		READ_JSON_INT(defaultFullScore)
		READ_JSON_INT(defaultTimeLimit)
		READ_JSON_INT(defaultMemoryLimit)
		READ_JSON_INT(compileTimeLimit)
		READ_JSON_INT(specialJudgeTimeLimit)
		READ_JSON_INT(fileSizeLimit)
		READ_JSON_INT(rejudgeTimes)
		
		READ_JSON_STR(defaultInputFileExtension)
		READ_JSON_STR(defaultOutputFileExtension)
		READ_JSON_STR(diffPath)
		
		READ_JSON_STRLIST(inputFileExtensions)
		READ_JSON_STRLIST(outputFileExtensions)
		READ_JSON_STRLIST(recentContest)
		
		// CompilerList
		if (json.contains("compilerList") && json["compilerList"].isArray()) {
			QJsonArray _compilerList = json["compilerList"].toArray();
			compilerList.clear();
			compilerList.reserve(_compilerList.size());
			for (int compilerIndex = 0; compilerIndex < _compilerList.size(); ++compilerIndex) {
				QJsonObject compilerObject = _compilerList[compilerIndex].toObject();
				Compiler *compiler = new Compiler();
				compiler->read(compilerObject);
				compilerList.append(compiler);
			}
		}
	}
	
	void LemonConfigJudge::write(QJsonObject &json) const {
		
	}

} // namespace Lemon::base::config
