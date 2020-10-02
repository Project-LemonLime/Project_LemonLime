/*
 * SPDX-FileCopyrightText: 2020  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//
#include <QString>
#include <QTranslator>
#include <memory>
#include <optional>

namespace Lemon::common
{
	class LemonTranslator
	{
	  public:
		explicit LemonTranslator();

	  public:
		/**
		 * @brief get the available languages.
		 * @return (if available) languages (zh_CN, en_US, ...)
		 */
		const inline QStringList GetAvailableLanguages() const { return languages; }
		/**
		 * @brief reload the translation from file
		 * @param code eg: zh_CN, ...
		 */
		bool InstallTranslation(const QString &);

	  private:
		QStringList languages;
		QStringList searchPaths;
		std::unique_ptr<QTranslator> pTranslator;
		void refreshTranslations();
	};
	inline std::unique_ptr<common::LemonTranslator> LemonLimeTranslator;
} // namespace Lemon::common

using namespace Lemon::common;
