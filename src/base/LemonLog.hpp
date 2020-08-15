/***************************************************************************
    This file is part of Project LemonLime
    Copyright (C) 2020 Coelacanthus

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

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
