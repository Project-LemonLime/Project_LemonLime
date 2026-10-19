/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once

#include "base/LemonType.hpp"

#include <QList>
#include <QString>
#include <QStringList>

struct TaskResult {
	bool judged = false;
	CompileState compileState = NoValidSourceFile;
	QString compileMessage;
	QString sourceFile;
	QList<QStringList> inputFiles;
	QList<QList<ResultState>> result;
	QList<QStringList> message;
	QList<QList<int>> score;
	QList<QList<int>> timeUsed;
	QList<QList<qint64>> memoryUsed;
};
