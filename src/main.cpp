/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "lemon.h"
//
#include "base/LemonBase.hpp"
#include "base/LemonBaseApplication.hpp"
#include "base/LemonLog.hpp"
//
#include <QApplication>
#include <QPixmap>
#include <QSplashScreen>

#define LEMON_MODULE_NAME "Main"

int main(int argc, char *argv[]) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
#ifndef LEMON_QT6
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // High DPI supported
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif
#endif

	LemonBaseApplication app(argc, argv);

	LOG("LemonLime " LEMON_VERSION_STRING " on " + QSysInfo::prettyProductName() + " " +
	    QSysInfo::currentCpuArchitecture());
	DEBUG("LemonLime Start Time: " + QString::number(QTime::currentTime().msecsSinceStartOfDay()));
	DEBUG(LEMON_BUILD_INFO);
	DEBUG(LEMON_BUILD_EXTRA_INFO);

	app.Initialize();

	if (app.sendMessage("")) {
		app.activeWindow();
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

	if (splashTime > 0) {
		screen.show();

		do {
			SingleApplication::processEvents();
		} while (QDateTime::currentMSecsSinceEpoch() - startTime <= splashTime);

		screen.finish(&w);
	}

	app.setActiveWindow(&w);
	w.show();
	w.welcome();
	return app.exec();
}
