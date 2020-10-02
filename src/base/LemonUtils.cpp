/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "LemonUtils.hpp"
//

namespace Lemon::common
{

	QStringList GetFileList(const QDir &dir)
	{
		return dir.entryList(QStringList{"*", "*.*"}, QDir::Hidden | QDir::Files);
	}

	bool FileExistsIn(const QDir &dir, const QString &fileName)
	{
		return GetFileList(dir).contains(fileName);
	}

} // namespace Lemon::common
