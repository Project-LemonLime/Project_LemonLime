/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include "base/LemonType.hpp"
#include <QColor>
#include <QObject>
#include <QtCore>

class Compiler;

struct hslTuple {
	int h;
	double s;
	double l;
	hslTuple(int _h = 0, double _s = 0.00, double _l = 0.00) : h(_h), s(_s), l(_l) {}
	QColor toHsl() { return QColor::fromHslF(h / 360.0, s / 100.0, l / 100.0); }
};
Q_DECLARE_METATYPE(hslTuple)

struct dddTuple {
	double h;
	double s;
	double l;
	dddTuple(double _h = 0, double _s = 0.00, double _l = 0.00) : h(_h), s(_s), l(_l) {}
};
Q_DECLARE_METATYPE(dddTuple)

class ColorTheme : public QObject {
	Q_OBJECT
  public:
	explicit ColorTheme(QObject *parent = nullptr);

	void setName(QString);
	void setColor(hslTuple, hslTuple, hslTuple, hslTuple, dddTuple, dddTuple);
	void copyFrom(ColorTheme *);

	QString getName() const;
	hslTuple getMxColor() const;
	hslTuple getMiColor() const;
	hslTuple getNfColor() const;
	hslTuple getCeColor() const;
	dddTuple getGrandComp() const;
	dddTuple getGrandRate() const;

	QColor getColorNf() const;
	QColor getColorCe() const;
	QColor getColorPer(double) const;
	QColor getColorGrand(double) const;
	QColor getColorPer(double, double) const;
	QColor getColorGrand(double, double) const;

  private:
	QString name;
	hslTuple mxColor, miColor, nfColor, ceColor;
	QColor colorNf, colorCe;
	dddTuple grandComp, grandRate;
};

class Settings : public QObject {
	Q_OBJECT
  public:
	explicit Settings(QObject *parent = nullptr);

	int getDefaultFullScore() const;
	int getDefaultTimeLimit() const;
	int getDefaultMemoryLimit() const;
	int getCompileTimeLimit() const;
	int getSpecialJudgeTimeLimit() const;
	int getFileSizeLimit() const;
	int getRejudgeTimes() const;
	double getDefaultExtraTimeRatio() const;
	const QString &getDefaultInputFileExtension() const;
	const QString &getDefaultOutputFileExtension() const;
	const QStringList &getInputFileExtensions() const;
	const QStringList &getOutputFileExtensions() const;
	const QStringList &getRecentContest() const;
	const QList<Compiler *> &getCompilerList() const;
	const QList<ColorTheme *> &getColorThemeList() const;
	const QString &getUiLanguage() const;
	const QString &getDiffPath() const;
	int getSplashTime() const;

	void setDefaultFullScore(int);
	void setDefaultTimeLimit(int);
	void setDefaultExtraTimeRatio(double);
	void setDefaultMemoryLimit(int);
	void setCompileTimeLimit(int);
	void setSpecialJudgeTimeLimit(int);
	void setFileSizeLimit(int);
	void setRejudgeTimes(int);
	void setDefaultInputFileExtension(const QString &);
	void setDefaultOutputFileExtension(const QString &);
	void setInputFileExtensions(const QString &);
	void setOutputFileExtensions(const QString &);
	void setRecentContest(const QStringList &);
	void setUiLanguage(const QString &);
	void setSplashTime(int);

	void addCompiler(Compiler *);
	void deleteCompiler(int);
	Compiler *getCompiler(int);
	void swapCompiler(int, int);

	void addColorTheme(ColorTheme *);
	void deleteColorTheme(int);
	ColorTheme *getColorTheme(int);
	const ColorTheme *getCurrentColorTheme() const;
	int getCurrentColorThemeIndex() const;

	void setColorTheme(ColorTheme *, int);
	void setCurrendColorTheme(ColorTheme *);
	void setCurrentColorThemeIndex(int);

	void copyFrom(Settings *);
	void saveSettings();
	void loadSettings();

	static void setTextAndColor(ResultState, QString &, QString &, QString &);
	static int upperBoundForFullScore();
	static int upperBoundForTimeLimit();
	static int upperBoundForMemoryLimit();
	static int upperBoundForFileSizeLimit();
	static int upperBoundForRejudgeTimes();
	static double upperBoundForExtraTimeRatio();
	static QString dataPath();
	static QString sourcePath();
	static QString temporaryPath();
	static QString selfTestPath();

  private:
	QList<Compiler *> compilerList;
	int defaultFullScore{};
	int defaultTimeLimit{};
	int defaultMemoryLimit{};
	int compileTimeLimit{};
	int specialJudgeTimeLimit{};
	int fileSizeLimit{};
	int rejudgeTimes{};
	double defaultExtraTimeRatio{};
	QString defaultInputFileExtension;
	QString defaultOutputFileExtension;
	QStringList inputFileExtensions;
	QStringList outputFileExtensions;
	QStringList recentContest;
	QString uiLanguage;
	QString diffPath;
	QList<ColorTheme *> colorThemeList;
	int currentColorTheme{};

	int splashTime{};
};
