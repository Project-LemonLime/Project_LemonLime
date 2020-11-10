/*
 * SPDX-FileCopyrightText: 2019-2020  Qv2ray Development Group
 *                         2020       Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "LemonTranslator.hpp"
//
#include <base/LemonBase.hpp>
#include <base/LemonLog.hpp>
#include <base/LemonUtils.hpp>
//

#define LEMON_MODULE_NAME "Translator"

using namespace Lemon;

// path searching list.
auto getLanguageSearchPaths() -> QStringList {
	// Configuration Path
	QStringList list = LemonAssetsPaths("lang");
#ifdef LEMON_EMBED_TRANSLATIONS
	// If the translations have been embedded.
	list << QString(":/translation/");
#endif
#ifdef LEMON_TRANSLATION_PATH
	// Platform-specific dir, if specified.
	list << QString(LEMON_TRANSLATION_PATH);
#endif
	return list;
}

namespace Lemon::common {
	LemonTranslator::LemonTranslator() { refreshTranslations(); }

	void LemonTranslator::refreshTranslations() {
		searchPaths = getLanguageSearchPaths();
		languages.clear();
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
		for (const auto &path : searchPaths)
#else
		for (const auto &path : qAsConst(searchPaths))
#endif
		{
			languages << QDir(path).entryList({"*.qm"}, QDir::Hidden | QDir::Files);
		}
		std::transform(languages.begin(), languages.end(), languages.begin(),
		               [](QString &fileName) { return fileName.replace(".qm", ""); });
		languages.removeDuplicates();
		DEBUG("Found translations: " + languages.join(" "));
	}

	auto LemonTranslator::InstallTranslation(const QString &code) -> bool {
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
		for (const auto &path : searchPaths)
#else
		for (const auto &path : qAsConst(searchPaths))
#endif
		{
			if (FileExistsIn(QDir(path), code + ".qm")) {
				DEBUG("Found " + code + " in folder: " + path);
				QTranslator *translatorNew = new QTranslator();
				bool success = translatorNew->load(code + ".qm", path);
				if (! success) {
					LOG("Cannot load translation: " + code);
				}
				if (pTranslator) {
					LOG("Removed translations");
					qApp->removeTranslator(pTranslator.get());
				}
				this->pTranslator.reset(translatorNew);
				qApp->installTranslator(pTranslator.get());
				LOG("Successfully installed a translator for " + code);
				return true;
			}
		}
		return false;
	}
} // namespace Lemon::common
