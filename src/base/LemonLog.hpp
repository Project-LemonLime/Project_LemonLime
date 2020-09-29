/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include <QString>

namespace Lemon::base
{
	void __LEMON_LOG_FUNC__(int type, const std::string &func, int line, const QString &log);
	QString readLastLog();
} // namespace Lemon::base

#define NEWLINE "\r\n"

#define LEMON_LOG_NORMAL 0
#define LEMON_LOG_DEBUG 1

#define __LOG_IMPL(LEVEL, MSG) ::Lemon::base::__LEMON_LOG_FUNC__(LEVEL, Q_FUNC_INFO, __LINE__, MSG);

#define LOG(MSG) __LOG_IMPL(LEMON_LOG_NORMAL, (MSG));
#define DEBUG(MSG) __LOG_IMPL(LEMON_LOG_DEBUG, (MSG));
