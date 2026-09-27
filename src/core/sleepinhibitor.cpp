/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "sleepinhibitor.h"

#include "base/LemonLog.hpp"

#ifdef Q_OS_WIN
#include "windowsprocessutils.h"
#elif defined(Q_OS_MACOS)
#include <IOKit/pwr_mgt/IOPMLib.h>
#elif defined(Q_OS_LINUX)
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>
#endif

#define LEMON_MODULE_NAME "SleepInhibitor"

struct SleepInhibitor::State {
#ifdef Q_OS_WIN
	Lemon::Windows::Handle request;
	~State() {
		if (request && ! PowerClearRequest(request.get(), PowerRequestSystemRequired))
			WARN("Failed to release the system sleep request:", GetLastError());
	}
#elif defined(Q_OS_MACOS)
	IOPMAssertionID assertion = kIOPMNullAssertionID;
	~State() {
		if (assertion != kIOPMNullAssertionID) {
			const IOReturn result = IOPMAssertionRelease(assertion);
			if (result != kIOReturnSuccess)
				WARN("Failed to release the system sleep assertion:", result);
		}
	}
#elif defined(Q_OS_LINUX)
	QDBusUnixFileDescriptor descriptor;
#endif
};

SleepInhibitor::SleepInhibitor(const QString &reason) : state(std::make_unique<State>()) {
#ifdef Q_OS_WIN
	std::wstring description = reason.toStdWString();
	REASON_CONTEXT context{};
	context.Version = POWER_REQUEST_CONTEXT_VERSION;
	context.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
	context.Reason.SimpleReasonString = description.data();
	state->request.reset(PowerCreateRequest(&context));
	if (! state->request) {
		WARN("Failed to create the system sleep request:", GetLastError());
		return;
	}
	if (! PowerSetRequest(state->request.get(), PowerRequestSystemRequired)) {
		const DWORD error = GetLastError();
		state->request.reset();
		WARN("Failed to prevent system sleep:", error);
	}
#elif defined(Q_OS_MACOS)
	CFStringRef description = reason.toCFString();
	IOPMAssertionID assertion = kIOPMNullAssertionID;
	const IOReturn result = IOPMAssertionCreateWithName(kIOPMAssertPreventUserIdleSystemSleep,
	                                                    kIOPMAssertionLevelOn, description, &assertion);
	CFRelease(description);
	if (result == kIOReturnSuccess)
		state->assertion = assertion;
	else
		WARN("Failed to prevent system sleep:", result);
#elif defined(Q_OS_LINUX)
	QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1", "/org/freedesktop/login1",
	                                                      "org.freedesktop.login1.Manager", "Inhibit");
	message.setArguments(
	    {QStringLiteral("sleep"), QStringLiteral("LemonLime"), reason, QStringLiteral("block")});
	const QDBusReply<QDBusUnixFileDescriptor> reply =
	    QDBusConnection::systemBus().call(message, QDBus::Block, 1000);
	if (reply.isValid() && reply.value().isValid())
		state->descriptor = reply.value();
	else
		WARN("Failed to prevent system sleep:", reply.error().message());
#else
	Q_UNUSED(reason)
	WARN("Preventing system sleep is unsupported on this platform");
#endif
}

SleepInhibitor::~SleepInhibitor() = default;
