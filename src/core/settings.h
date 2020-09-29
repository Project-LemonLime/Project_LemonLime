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

#include "base/LemonType.hpp"
#include <QColor>
#include <QObject>
#include <QtCore>

class Compiler;

class Settings : public QObject
{
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
	const QString &getDefaultInputFileExtension() const;
	const QString &getDefaultOutputFileExtension() const;
	const QStringList &getInputFileExtensions() const;
	const QStringList &getOutputFileExtensions() const;
	const QStringList &getRecentContest() const;
	const QList<Compiler *> &getCompilerList() const;
	const QString &getUiLanguage() const;
	const QString &getDiffPath() const;
	int getColorMxH();
	double getColorMxS();
	double getColorMxL();
	int getColorMiH();
	double getColorMiS();
	double getColorMiL();
	int getColorNfH();
	double getColorNfS();
	double getColorNfL();
	int getColorCeH();
	double getColorCeS();
	double getColorCeL();
	double getGrandCompH();
	double getGrandCompS();
	double getGrandCompL();
	double getGrandRateH();
	double getGrandRateS();
	double getGrandRateL();
	int getSplashTime();

	QColor getColorMx();
	QColor getColorMi();
	QColor getColorNf();
	QColor getColorCe();
	QColor getColorAntiMi();
	QColor getColorPer(double);
	QColor getColorGrand(double);
	QColor getColorPer(double, double);
	QColor getColorGrand(double, double);

	void setDefaultFullScore(int);
	void setDefaultTimeLimit(int);
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
	void setColorMxH(int);
	void setColorMxS(double);
	void setColorMxL(double);
	void setColorMiH(int);
	void setColorMiS(double);
	void setColorMiL(double);
	void setColorNfH(int);
	void setColorNfS(double);
	void setColorNfL(double);
	void setColorCeH(int);
	void setColorCeS(double);
	void setColorCeL(double);
	void setGrandCompH(double);
	void setGrandCompS(double);
	void setGrandCompL(double);
	void setGrandRateH(double);
	void setGrandRateS(double);
	void setGrandRateL(double);
	void setSplashTime(int);

	void addCompiler(Compiler *);
	void deleteCompiler(int);
	Compiler *getCompiler(int);
	void swapCompiler(int, int);
	void copyFrom(Settings *);
	void saveSettings();
	void loadSettings();

	static void setTextAndColor(ResultState, QString &, QString &, QString &);
	static int upperBoundForFullScore();
	static int upperBoundForTimeLimit();
	static int upperBoundForMemoryLimit();
	static int upperBoundForFileSizeLimit();
	static int upperBoundForRejudgeTimes();
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
	QString defaultInputFileExtension;
	QString defaultOutputFileExtension;
	QStringList inputFileExtensions;
	QStringList outputFileExtensions;
	QStringList recentContest;
	QString uiLanguage;
	QString diffPath;

	int colorMxH{};
	double colorMxS{};
	double colorMxL{};
	int colorMiH{};
	double colorMiS{};
	double colorMiL{};
	int colorNfH{};
	double colorNfS{};
	double colorNfL{};
	int colorCeH{};
	double colorCeS{};
	double colorCeL{};

	double grandCompH{};
	double grandCompS{};
	double grandCompL{};
	double grandRateH{};
	double grandRateS{};
	double grandRateL{};

	int splashTime{};
};
