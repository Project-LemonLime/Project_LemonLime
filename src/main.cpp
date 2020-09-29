/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

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
