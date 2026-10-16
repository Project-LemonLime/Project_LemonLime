/*
 * SPDX-FileCopyrightText: 2020-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "LemonUtils.hpp"
//

namespace Lemon::common {

	auto GetFileList(const QDir &dir) -> QStringList {
		return dir.entryList(QStringList{"*", "*.*"}, QDir::Hidden | QDir::Files);
	}

	auto FileExistsIn(const QDir &dir, const QString &fileName) -> bool {
		return GetFileList(dir).contains(fileName);
	}

	bool lessFileName(const QString &a, const QString &b) {
		return a.length() < b.length() || (a.length() == b.length() && QString::localeAwareCompare(a, b) < 0);
	}

} // namespace Lemon::common
