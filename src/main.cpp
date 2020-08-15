/***************************************************************************
    This file is part of Project Lemon
    Copyright (C) 2011 Zhipeng Jia

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
/**
 * main.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * main.cpp @Project LemonLime
 * Update 2019 iotang
 **/
/**
 * main.cpp @Project LemonLime
 * Update 2020 Coelacanthus
 **/

#include "lemon.h"
#include <QApplication>
#include <QPixmap>
#include <QSplashScreen>
#include <SingleApplication>
#include <iostream>

auto main(int argc, char *argv[]) -> int
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // High DPI supported
#endif
	SingleApplication a(argc, argv);

	if (a.sendMessage(""))
	{
		a.activeWindow();
		return 0;
	}

#ifdef Q_OS_LINUX
	// fonts.setFamily("Noto Sans CJK SC");
#endif
#ifdef Q_OS_WIN32
	QFont fonts;
	fonts.setFamily("Microsoft YaHei");
	SingleApplication::setFont(fonts);
#endif
#ifdef Q_OS_MAC
	// fonts.setFamily("PingFangSC-Regular");
#endif
	Q_INIT_RESOURCE(resource);
	QPixmap pixmap(":/logo/splash2.png");
	QSplashScreen screen(pixmap.scaled(450, 191, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	LemonLime w;
	qint64 startTime = QDateTime::currentMSecsSinceEpoch();
	int splashTime = w.getSplashTime();

	if (splashTime > 0)
	{
		screen.show();

		do
		{
			SingleApplication::processEvents();
		} while (QDateTime::currentMSecsSinceEpoch() - startTime <= splashTime);

		screen.finish(&w);
	}

	a.setActiveWindow(&w);
	w.show();
	w.welcome();
	return a.exec();
}
