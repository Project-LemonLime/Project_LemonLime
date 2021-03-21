/*
 * SPDX-FileCopyrightText: 2020-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//
#include <QDir>

#define READ_JSON_INT(___x)                                                                                  \
	if (json.contains("___x") && json["___x"].isDouble())                                                    \
		___x = json["___x"].toInt();
#define READ_JSON_BOOL(___x)                                                                                 \
	if (json.contains("___x") && json["___x"].isBool())                                                      \
		___x = json["___x"].toBool();
#define READ_JSON_DOUBLE(___x)                                                                               \
	if (json.contains("___x") && json["___x"].isDouble())                                                    \
		___x = json["___x"].toDouble();
#define READ_JSON_STR(___x)                                                                                  \
	if (json.contains("___x") && json["___x"].isString())                                                    \
		___x = json["___x"].toString();
#define READ_JSON_STRLIST(___x)                                                                              \
	if (json.contains("___x") && json["___x"].isString())                                                    \
		___x = json["___x"].toString().split(QLatin1Char(';'), Qt::SkipEmptyParts);

#define WRITE_JSON(___x) json["___x"] = ___x;
#define WRITE_JSON_STRLIST(___x) json["___x"] = ___x.join(QLatin1Char(';'));

namespace Lemon::common {

	QStringList GetFileList(const QDir &dir);
	bool FileExistsIn(const QDir &dir, const QString &fileName);

} // namespace Lemon::common

using namespace Lemon::common;
