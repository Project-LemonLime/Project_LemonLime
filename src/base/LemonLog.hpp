/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include <QPair>
#include <QString>
#include <QTextStream>
#include <iostream>

#define NEWLINE "\r\n"
#define ___LOG_EXPAND(___x)                                                                                  \
	, QPair                                                                                                  \
	{                                                                                                        \
#___x, [&] { return ___x; }()                                                                        \
	}
#define A(...) FOREACH_CALL_FUNC(___LOG_EXPAND, __VA_ARGS__)

#ifdef QT_DEBUG
#define LEMON_IS_DEBUG true
// __FILE__ ":" QT_STRINGIFY(__LINE__),
#define LEMON_LOG_PREPEND_CONTENT Q_FUNC_INFO,
#else
#define LEMON_IS_DEBUG false
#define LEMON_LOG_PREPEND_CONTENT
#endif

#define _LOG_ARG_(...) LEMON_LOG_PREPEND_CONTENT "[" LEMON_MODULE_NAME "]", __VA_ARGS__

#define LOG(...) Lemon::base::log_concat<LEMON_LOG_NORMAL>(_LOG_ARG_(__VA_ARGS__))
#define DEBUG(...) Lemon::base::log_concat<LEMON_LOG_DEBUG>(_LOG_ARG_(__VA_ARGS__))

enum LemonLogType
{
	LEMON_LOG_NORMAL = 0,
	LEMON_LOG_DEBUG = 1
};

Q_DECLARE_METATYPE(const char *)

namespace Lemon::base
{
	inline QString logBuffer;
	inline QString tempBuffer;
	inline QTextStream logStream{&logBuffer};
	inline QTextStream tempStream{&tempBuffer};

	inline QString ReadLog() { return logStream.readAll(); }

	template <LemonLogType t, typename... T> inline void log_concat(T... v)
	{
		((logStream << v << " "), ...);
		((tempStream << v << " "), ...);
		logStream << NEWLINE;
#ifndef QT_DEBUG
		// We only process DEBUG log in Release mode
		if (t == LEMON_LOG_DEBUG /* && !StartupOption.debugLog */)
		{
			// Discard debug log in non-debug Lemon version with
			// no-debugLog mode.
			return;
		}
#endif

		const auto logString = tempStream.readAll();

		std::cout << logString.toStdString() << std::endl;
	}
} // namespace Lemon::base

template <typename TKey, typename TVal>
QTextStream &operator<<(QTextStream &stream, const QPair<TKey, TVal> &pair)
{
	return stream << pair.first << ": " << pair.second;
}

inline QTextStream &operator<<(QTextStream &stream, const std::string &ss) { return stream << ss.data(); }

template <typename TKey, typename TVal>
QTextStream &operator<<(QTextStream &stream, const QMap<TKey, TVal> &map)
{
	stream << "{ ";
	for (const auto &[k, v] : map.toStdMap())
		stream << QPair(k, v) << "; ";
	stream << "}";
	return stream;
}

template <typename TVal>
QTextStream &operator<<(QTextStream &stream, const std::initializer_list<TVal> &init_list)
{
	for (const auto &x : init_list)
		stream << x;
	return stream;
}
