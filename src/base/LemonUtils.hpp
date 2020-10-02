/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//
#include <QDir>

namespace Lemon::common
{

	QStringList GetFileList(const QDir &dir);
	bool FileExistsIn(const QDir &dir, const QString &fileName);

} // namespace Lemon::common

using namespace Lemon::common;
