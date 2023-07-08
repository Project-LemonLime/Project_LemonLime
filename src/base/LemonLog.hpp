/*
 * SPDX-FileCopyrightText: 2019-2020  Qv2ray Development Group
 *                         2020-2022  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include <QPair>
#include <QString>
#include <QTextStream>
#include <iostream>
//
#include <base/LemonBaseApplication.hpp>
#include <base/LemonMacro.hpp>

#include "spdlog/spdlog.h"

#define NEWLINE "\r\n"
#define ___LOG_EXPAND(___x) , QPair<std::string, decltype(___x)>(std::string(#___x), [&] { return ___x; }())

// Generate pairs for each variable passed as args to use in the log.
// with line 73: QTextStream &operator<<(QTextStream &stream, const std::pair<TKey, TVal> &pair)
#define GEN_PAIR(...) FOREACH_CALL_FUNC(___LOG_EXPAND, __VA_ARGS__)

#ifdef QT_DEBUG
#define LEMON_IS_DEBUG true
// __FILE__ ":" QT_STRINGIFY(__LINE__),
// https://doc.qt.io/qt-5/qtglobal.html#Q_FUNC_INFO
#define LEMON_LOG_PREPEND_CONTENT Q_FUNC_INFO,
#else
#define LEMON_IS_DEBUG false
#define LEMON_LOG_PREPEND_CONTENT
#endif

// prepend module name in log
#define _LOG_ARG_(...) "[" LEMON_MODULE_NAME "]", LEMON_LOG_PREPEND_CONTENT __VA_ARGS__

#define WARN(...) Lemon::base::log_concat<LEMON_LOG_WARN>(_LOG_ARG_(__VA_ARGS__))
#define LOG(...) Lemon::base::log_concat<LEMON_LOG_NORMAL>(_LOG_ARG_(__VA_ARGS__))
#define DEBUG(...) Lemon::base::log_concat<LEMON_LOG_DEBUG>(_LOG_ARG_(__VA_ARGS__))

enum LemonLogType { LEMON_LOG_WARN, LEMON_LOG_NORMAL, LEMON_LOG_DEBUG };

Q_DECLARE_METATYPE(const char *)

namespace Lemon::base {
	inline std::shared_ptr<spdlog::logger> logger;
	inline QString tempBuffer;
	inline QTextStream tempStream{&tempBuffer};

	template <LemonLogType t, typename... T> inline void log_concat(T... v) {
		((tempStream << v << " "), ...);

		const auto logString = tempStream.readAll().toStdString();

		if constexpr (t == LEMON_LOG_DEBUG) {
			if (LemonCoreApplication && ! LemonCoreApplication->StartupArguments.debugLog)
				return;
			logger->debug(logString);
		} else if constexpr (t == LEMON_LOG_NORMAL) {
			logger->info(logString);
		} else if constexpr (t == LEMON_LOG_WARN) {
			logger->warn(logString);
		}
	}
} // namespace Lemon::base

template <typename TKey, typename TVal>
QTextStream &operator<<(QTextStream &stream, const QPair<TKey, TVal> &pair) {
	return stream << pair.first << ": " << pair.second;
}

inline QTextStream &operator<<(QTextStream &stream, const std::string &ss) { return stream << ss.data(); }

template <typename TKey, typename TVal>
QTextStream &operator<<(QTextStream &stream, const QMap<TKey, TVal> &map) {
	stream << "{ ";
	for (const auto &[k, v] : map.toStdMap())
		stream << QPair<TKey, TVal>(k, v) << "; ";
	stream << "}";
	return stream;
}

template <typename TVal>
QTextStream &operator<<(QTextStream &stream, const std::initializer_list<TVal> &init_list) {
	for (const auto &x : init_list)
		stream << x;
	return stream;
}
