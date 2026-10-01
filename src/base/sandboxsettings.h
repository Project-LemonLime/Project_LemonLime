/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

struct SandboxSettings {
	enum Runtime { Automatic, Native, Java, Python };
	bool enabled = false;
	Runtime runtime = Automatic;
	QStringList readOnlyDirectories;
	int preparationTimeLimit = 15000;

	void read(const QJsonObject &json) {
		enabled = json.value("enabled").toBool(false);
		const int value = json.value("runtime").toInt();
		runtime = value >= Automatic && value <= Python ? Runtime(value) : Automatic;
		preparationTimeLimit = qBound(1000, json.value("preparationTimeLimit").toInt(15000), 120000);
		readOnlyDirectories.clear();
		for (const auto &directory : json.value("readOnlyDirectories").toArray()) {
			if (directory.isString() && ! directory.toString().trimmed().isEmpty())
				readOnlyDirectories.append(directory.toString().trimmed());
		}
	}

	void write(QJsonObject &json) const {
		json["enabled"] = enabled;
		json["runtime"] = int(runtime);
		json["preparationTimeLimit"] = preparationTimeLimit;
		json["readOnlyDirectories"] = QJsonArray::fromStringList(readOnlyDirectories);
	}
};
