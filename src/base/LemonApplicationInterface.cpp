/*
 * SPDX-FileCopyrightText: 2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "LemonApplicationInterface.hpp"
//
#include "base/LemonBase.hpp"
#include "base/LemonLog.hpp"
#include "base/LemonTranslator.hpp"
#include "base/settings.h" // TODO: Config Refactor
//
#include <QCoreApplication>
//
#define LEMON_MODULE_NAME "LemonApplicationInterface"

using namespace Lemon;

LemonApplicationInterface::LemonApplicationInterface() {
	// ConfigObject = new LemonConfigObject;
	LemonCoreApplication = this;
	LOG("LemonLime", LEMON_VERSION_STRING, "on", QSysInfo::prettyProductName(),
	    QSysInfo::currentCpuArchitecture());
	DEBUG("LemonLime Start Time: ", QTime::currentTime().msecsSinceStartOfDay());
	DEBUG(LEMON_BUILD_INFO);
	DEBUG(LEMON_BUILD_EXTRA_INFO);
}

LemonApplicationInterface::~LemonApplicationInterface() {
	// delete ConfigObject;
	LemonCoreApplication = nullptr;
}
