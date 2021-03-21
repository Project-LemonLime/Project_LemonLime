/*
 * SPDX-FileCopyrightText: 2020-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
#include <LemonType.hpp>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>

namespace Lemon::detail {
	inline int JsonReadHelper(QString &val, const QJsonValue &jval) {
		if (jval.isString()) {
			val = jval.toString();
			return 0;
		} else
			return -1;
	}
	inline int JsonReadHelper(int &val, const QJsonValue &jval) {
		if (jval.isDouble()) {
			val = jval.toInt();
			return 0;
		} else
			return -1;
	}
	inline int JsonReadHelper(bool &val, const QJsonValue &jval) {
		if (jval.isBool()) {
			val = jval.toBool();
			return 0;
		} else
			return -1;
	}
	inline int JsonReadHelper(double &val, const QJsonValue &jval) {
		if (jval.isDouble()) {
			val = jval.toDouble();
			return 0;
		} else
			return -1;
	}
	inline int JsonReadHelper(QJsonObject &val, const QJsonValue &jval) {
		if (jval.isObject()) {
			val = jval.toObject();
			return 0;
		} else
			return -1;
	}
	inline int JsonReadHelper(QJsonArray &val, const QJsonValue &jval) {
		if (jval.isArray()) {
			val = jval.toArray();
			QJsonArray arr;
			return 0;
		} else
			return -1;
	}
	inline int JsonReadHelper(ResultState &val, const QJsonValue &jval) {
		int x;
		if (JsonReadHelper(x, jval) == -1)
			return -1;
		val = ResultState(x);
		return 0;
	}
	inline int JsonReadHelper(CompileState &val, const QJsonValue &jval) {
		int x;
		if (JsonReadHelper(x, jval) == -1)
			return -1;
		val = CompileState(x);
		return 0;
	}
	template <typename T> int JsonReadHelper(QList<T> &val, const QJsonValue &jval) {
		QJsonArray arr;
		return JsonReadHelper(arr, jval) == -1 || JsonReadHelper(val, arr) == -1 ? -1 : 0;
	}
	inline int JsonReadHelper(QStringList &val, const QJsonArray &jval) {
		QList<QString> s;
		if (JsonReadHelper(s, jval) == -1)
			return -1;
		val = s;
		return 0;
	}
	template <typename T> int JsonReadHelper(QList<T> &val, const QJsonArray &jval) {
		val.clear();
		for (auto i : jval) {
			T x;
			if (JsonReadHelper(x, i) == -1)
				return -1;
			val.append(x);
		}
		return 0;
	}
	template <typename T> int JsonReadHelper(T &val, const QString &name, const QJsonObject &json);
	template <typename T> int JsonReadHelper(T &val, const QString &name, const QJsonObject &json) {
		if (json.contains(name))
			return JsonReadHelper(val, json[name]);
		else
			return -1;
	}
} // namespace Lemon::detail
#define WRITE_JSON(json, ___x) json[#___x] = ___x;
#define WRITE_JSON_STRLIST(json, ___x) json[#___x] = ___x.join(QLatin1Char(';'));
namespace Lemon {
	template <typename T> int readJson(T &x, const QString &name, const QJsonObject &json) {
		return detail::JsonReadHelper(x, name, json);
	}
} // namespace Lemon
#define READ_JSON(json, x) Lemon::readJson(x, #x, json)
namespace Lemon::common {

	QStringList GetFileList(const QDir &dir);

	bool FileExistsIn(const QDir &dir, const QString &fileName);

} // namespace Lemon::common
