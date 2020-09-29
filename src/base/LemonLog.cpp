/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "LemonLog.hpp"

#include "LemonBase.hpp"

#include <iostream>
#include <memory>

namespace Lemon::base
{

	static auto __loggerBuffer = std::make_unique<QStringList>();
	static auto __purgerBuffer = std::make_unique<QStringList>();
	static QMutex __loggerMutex;
	static QMutex __purgerMutex;

	void __LEMON_LOG_FUNC__(int type, const std::string &func, int line, const QString &log)
	{
		auto logString = QString(log);
		auto funcPrepend = QString::fromStdString(func + ":" + std::to_string(line) + " ");

#ifdef QT_DEBUG
		// if (type == LEMON_LOG_DEBUG || StartupOption.debugLog)
		if (type == LEMON_LOG_DEBUG)
		{
			logString.prepend(funcPrepend);
		}
#else
		// We only process DEBUG log in Release mode
		if (type == LEMON_LOG_DEBUG /* && StartupOption.debugLog */)
		{
			logString.prepend(funcPrepend);
		}
		else
		{
			return;
		}
#endif
		std::cout << logString.toStdString() << std::endl;
		{
			QMutexLocker _(&__loggerMutex);
			__loggerBuffer->append(logString + NEWLINE);
		}
	} // namespace Lemon::base

	QString readLastLog()
	{
		QMutexLocker _(&__purgerMutex);
		{
			QMutexLocker locker(&__loggerMutex);
			__loggerBuffer.swap(__purgerBuffer);
		}
		auto result = __purgerBuffer->join("");
		__purgerBuffer->clear();
		return result;
	}
} // namespace Lemon::base
