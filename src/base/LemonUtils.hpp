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
#include <type_traits>

/*
 * Use a lot of SFAINE tricks to support varied data and container types
 */
namespace Lemon::detail {
	inline int jsonReadHelper(QString &val, const QJsonValue &jval) {
		if (jval.isString()) {
			val = jval.toString();
			return 0;
		} else
			return -1;
	}
	inline int jsonReadHelper(int &val, const QJsonValue &jval) {
		if (jval.isDouble()) {
			val = jval.toInt();
			return 0;
		} else
			return -1;
	}
	inline int jsonReadHelper(bool &val, const QJsonValue &jval) {
		if (jval.isBool()) {
			val = jval.toBool();
			return 0;
		} else
			return -1;
	}
	inline int jsonReadHelper(double &val, const QJsonValue &jval) {
		if (jval.isDouble()) {
			val = jval.toDouble();
			return 0;
		} else
			return -1;
	}
	inline int jsonReadHelper(QJsonObject &val, const QJsonValue &jval) {
		if (jval.isObject()) {
			val = jval.toObject();
			return 0;
		} else
			return -1;
	}
	inline int jsonReadHelper(QJsonArray &val, const QJsonValue &jval) {
		if (jval.isArray()) {
			val = jval.toArray();
			QJsonArray arr;
			return 0;
		} else
			return -1;
	}
	inline int jsonReadHelper(ResultState &val, const QJsonValue &jval) {
		int x;
		if (jsonReadHelper(x, jval) == -1)
			return -1;
		val = static_cast<ResultState>(x);
		return 0;
	}
	inline int jsonReadHelper(CompileState &val, const QJsonValue &jval) {
		int x;
		if (jsonReadHelper(x, jval) == -1)
			return -1;
		val = static_cast<CompileState>(x);
		return 0;
	}
	template <typename T> int jsonReadHelper(QList<T> &val, const QJsonArray &jval);
	template <typename T> int jsonReadHelper(QList<T> &val, const QJsonValue &jval) {
		QJsonArray arr;
		return jsonReadHelper(arr, jval) == -1 || jsonReadHelper(val, arr) == -1 ? -1 : 0;
	}
	inline int jsonReadHelper(QStringList &val, const QJsonArray &jval) {
		QList<QString> s;
		if (jsonReadHelper<QString>(s, jval) == -1)
			return -1;
		val = s;
		return 0;
	}
	template <typename T> int jsonReadHelper(QList<T> &val, const QJsonArray &jval) {
		val.clear();
		for (auto i : jval) {
			T x;
			if (jsonReadHelper(x, i) == -1)
				return -1;
			val.append(x);
		}
		return 0;
	}
	template <typename T> int jsonReadHelper(T &val, const QString &name, const QJsonObject &json) {
		if (json.contains(name))
			return jsonReadHelper(val, json[name]);
		else
			return -1;
	}

	template <typename T>
	std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T> ||
	                     std::is_same_v<std::remove_cv_t<T>, QString> ||
	                     std::is_same_v<std::remove_cv_t<T>, QJsonArray> ||
	                     std::is_same_v<std::remove_cv_t<T>, QJsonValue> ||
	                     std::is_same_v<std::remove_cv_t<T>, QJsonObject>,
	                 void>
	jsonWriteHelper(const T &val, QJsonValue &jval) {
		jval = val;
	}
	template <typename T>
	std::enable_if_t<std::is_enum_v<T>, void> jsonWriteHelper(const T &val, QJsonValue &jval) {
		jval = static_cast<int>(val);
	}
	template <typename T> void jsonWriteHelper(const QList<T> &val, QJsonValue &jval) {
		QJsonArray arr;
		for (const T &i : val) {
			QJsonValue x;
			jsonWriteHelper(i, x);
			arr.append(x);
		}
		jval = arr;
	}
	inline void jsonWriteHelper(const QStringList &val, QJsonValue &jval) {
		QList<QString> s = val;
		jsonWriteHelper<QString>(s, jval);
	}
	template <typename T> void jsonWriteHelper(const T &val, const QString &name, QJsonObject &json) {
		QJsonValue jval;
		jsonWriteHelper(val, jval);
		json[name] = jval;
	}
} // namespace Lemon::detail

namespace Lemon {
	template <typename T> int readJson(T &x, const QString &name, const QJsonObject &json) {
		return detail::jsonReadHelper(x, name, json);
	}
	template <typename T> void writeJson(const T &x, const QString &name, QJsonObject &json) {
		detail::jsonWriteHelper(x, name, json);
	}
} // namespace Lemon
#define READ_JSON(json, x) Lemon::readJson(x, #x, json)
#define WRITE_JSON(json, x) Lemon::writeJson(x, #x, json)
namespace Lemon::common {

	QStringList GetFileList(const QDir &dir);

	bool FileExistsIn(const QDir &dir, const QString &fileName);

} // namespace Lemon::common
