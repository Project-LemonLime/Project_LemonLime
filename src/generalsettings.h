/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QWidget>
#include <QtCore>
#include <QtGui>

namespace Ui
{
	class GeneralSettings;
}

class Settings;

class GeneralSettings : public QWidget
{
	Q_OBJECT

  public:
	explicit GeneralSettings(QWidget *parent = nullptr);
	~GeneralSettings();
	void resetEditSettings(Settings *);
	bool checkValid();

  private:
	Ui::GeneralSettings *ui;
	Settings *editSettings{};

  private slots:
	void defaultFullScoreChanged(const QString &);
	void defaultTimeLimitChanged(const QString &);
	void defaultExtraTimeRatioChanged(const QString &);
	void defaultMemoryLimitChanged(const QString &);
	void compileTimeLimitChanged(const QString &);
	void specialJudgeTimeLimitChanged(const QString &);
	void fileSizeLimitChanged(const QString &);
	void rejudgeTimesChanged(const QString &);
	void inputFileExtensionsChanged(const QString &);
	void outputFileExtensionsChanged(const QString &);
	void onLanguageComboBoxChanged(const QString &);
};
