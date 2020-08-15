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

// Refer to Qv2ray project log module

#include "LemonLog.hpp"

#include "LemonBase.hpp"

#include <iostream>

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
        //if (type == LEMON_LOG_DEBUG || StartupOption.debugLog)
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
