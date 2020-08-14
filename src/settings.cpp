/***************************************************************************
    This file is part of Project Lemon
    Copyright (C) 2011 Zhipeng Jia

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/
/**
 * settings.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * settings.cpp @Project LemonLime
 * Update 2019 iotang
 **/
/**
 * settings.cpp @Project LemonLime
 * Update 2020 Coelacanthus
 **/

#include "settings.h"
#include "compiler.h"
#include <cmath>

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#define swapItemsAt swap
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#define QT_SkipEmptyParts Qt::SkipEmptyParts
#else
#define QT_SkipEmptyParts QString::SkipEmptyParts
#endif

Settings::Settings(QObject *parent) : QObject(parent) {}

auto Settings::getDefaultFullScore() const -> int { return defaultFullScore; }

auto Settings::getDefaultTimeLimit() const -> int { return defaultTimeLimit; }

auto Settings::getDefaultMemoryLimit() const -> int { return defaultMemoryLimit; }

auto Settings::getCompileTimeLimit() const -> int { return compileTimeLimit; }

auto Settings::getSpecialJudgeTimeLimit() const -> int { return specialJudgeTimeLimit; }

auto Settings::getFileSizeLimit() const -> int { return fileSizeLimit; }

auto Settings::getRejudgeTimes() const -> int { return rejudgeTimes; }

auto Settings::getDefaultInputFileExtension() const -> const QString & { return defaultInputFileExtension; }

auto Settings::getDefaultOutputFileExtension() const -> const QString & { return defaultOutputFileExtension; }

auto Settings::getInputFileExtensions() const -> const QStringList & { return inputFileExtensions; }

auto Settings::getOutputFileExtensions() const -> const QStringList & { return outputFileExtensions; }

auto Settings::getRecentContest() const -> const QStringList & { return recentContest; }

auto Settings::getCompilerList() const -> const QList<Compiler *> & { return compilerList; }

auto Settings::getUiLanguage() const -> const QString & { return uiLanguage; }

auto Settings::getDiffPath() const -> const QString & { return diffPath; }

auto Settings::getColorMxH() -> int { return colorMxH; }

auto Settings::getColorMxS() -> double { return colorMxS; }

auto Settings::getColorMxL() -> double { return colorMxL; }

auto Settings::getColorMiH() -> int { return colorMiH; }

auto Settings::getColorMiS() -> double { return colorMiS; }

auto Settings::getColorMiL() -> double { return colorMiL; }

auto Settings::getColorNfH() -> int { return colorNfH; }

auto Settings::getColorNfS() -> double { return colorNfS; }

auto Settings::getColorNfL() -> double { return colorNfL; }

auto Settings::getColorCeH() -> int { return colorCeH; }

auto Settings::getColorCeS() -> double { return colorCeS; }

auto Settings::getColorCeL() -> double { return colorCeL; }

auto Settings::getGrandCompH() -> double { return grandCompH; }

auto Settings::getGrandCompS() -> double { return grandCompS; }

auto Settings::getGrandCompL() -> double { return grandCompL; }

auto Settings::getGrandRateH() -> double { return grandRateH; }

auto Settings::getGrandRateS() -> double { return grandRateS; }

auto Settings::getGrandRateL() -> double { return grandRateL; }

auto Settings::getSplashTime() -> int { return splashTime; }

auto Settings::getColorMx() -> QColor
{
	return QColor::fromHslF(colorMxH / 360.00, colorMxS / 100.00, colorMxL / 100.00);
}

auto Settings::getColorMi() -> QColor
{
	return QColor::fromHslF(colorMiH / 360.00, colorMiS / 100.00, colorMiL / 100.00);
}

auto Settings::getColorNf() -> QColor
{
	return QColor::fromHslF(colorNfH / 360.00, colorNfS / 100.00, colorNfL / 100.00);
}

auto Settings::getColorCe() -> QColor
{
	return QColor::fromHslF(colorCeH / 360.00, colorCeS / 100.00, colorCeL / 100.00);
}

auto Settings::getColorAntiMi() -> QColor
{
	return QColor::fromHslF(colorMiH / 360.00, colorMiS / 100.00, 1.00 - colorMiL / 100.00);
}

auto Settings::getColorPer(double p) -> QColor
{
	if (qIsNaN(p) || qIsInf(p))
		p = 0;

	double distan = NAN;
	double h = NAN;
	double s = NAN;
	double l = NAN;
	distan = (colorMxH - colorMiH) / 360.00 / 110.00;
	h = colorMiH / 360.00 + 100 * p * distan;

	if (p > 0)
		h += distan * 5;

	if (p >= 1 - 1e-12)
		h += distan * 5;

	distan = (colorMxS - colorMiS) / 100.00 / 110.00;
	s = colorMiS / 100.00 + 100 * p * distan;

	if (p > 0)
		s += distan * 5;

	if (p >= 1 - 1e-12)
		s += distan * 5;

	distan = (colorMxL - colorMiL) / 100.00 / 110.00;
	l = colorMiL / 100.00 + 100 * p * distan;

	if (p > 0)
		l += distan * 5;

	if (p >= 1 - 1e-12)
		l += distan * 5;

	while (h < -1e-12)
		h += 1;

	while (h > 1 + 1e-12)
		h -= 1;

	h = fmax(0, fmin(h, 1));
	s = fmax(0, fmin(s, 1));
	l = fmax(0, fmin(l, 1));
	return QColor::fromHslF(h, s, l);
}

auto Settings::getColorGrand(double p) -> QColor
{
	if (qIsNaN(p) || qIsInf(p))
		p = 0;

	double distan = NAN;
	double h = NAN;
	double s = NAN;
	double l = NAN;
	distan = grandRateH * (colorMxH - colorMiH) / 360.00 / 110.00;
	h = grandCompH / 360.00 + colorMiH / 360.00 + 100 * p * distan;

	if (p > 0)
		h += distan * 5;

	if (p >= 1 - 1e-12)
		h += distan * 5;

	distan = grandRateS * (colorMxS - colorMiS) / 100.00 / 110.00;
	s = grandCompS / 100.00 + colorMiS / 100.00 + 100 * p * distan;

	if (p > 0)
		s += distan * 5;

	if (p >= 1 - 1e-12)
		s += distan * 5;

	distan = grandRateL * (colorMxL - colorMiL) / 100.00 / 110.00;
	l = grandCompL / 100.0 + colorMiL / 100.00 + 100 * p * distan;

	if (p > 0)
		l += distan * 5;

	if (p >= 1 - 1e-12)
		l += distan * 5;

	while (h < -1e-12)
		h += 1;

	while (h > 1 + 1e-12)
		h -= 1;

	h = fmax(0, fmin(h, 1));
	s = fmax(0, fmin(s, 1));
	l = fmax(0, fmin(l, 1));
	return QColor::fromHslF(h, s, l);
}

auto Settings::getColorPer(double a, double b) -> QColor { return getColorPer(a / b); }

auto Settings::getColorGrand(double a, double b) -> QColor { return getColorGrand(a / b); }

void Settings::setDefaultFullScore(int score) { defaultFullScore = score; }

void Settings::setDefaultTimeLimit(int limit) { defaultTimeLimit = limit; }

void Settings::setDefaultMemoryLimit(int limit) { defaultMemoryLimit = limit; }

void Settings::setCompileTimeLimit(int limit) { compileTimeLimit = limit; }

void Settings::setSpecialJudgeTimeLimit(int limit) { specialJudgeTimeLimit = limit; }

void Settings::setFileSizeLimit(int limit) { fileSizeLimit = limit; }

void Settings::setRejudgeTimes(int number) { rejudgeTimes = number; }

void Settings::setDefaultInputFileExtension(const QString &extension)
{
	defaultInputFileExtension = extension;
}

void Settings::setDefaultOutputFileExtension(const QString &extension)
{
	defaultOutputFileExtension = extension;
}

void Settings::setInputFileExtensions(const QString &extensions)
{
	inputFileExtensions = extensions.split(";", QT_SkipEmptyParts);
}

void Settings::setOutputFileExtensions(const QString &extensions)
{
	outputFileExtensions = extensions.split(";", QT_SkipEmptyParts);
}

void Settings::setRecentContest(const QStringList &list) { recentContest = list; }

void Settings::setUiLanguage(const QString &language) { uiLanguage = language; }

void Settings::addCompiler(Compiler *compiler)
{
	compiler->setParent(this);
	compilerList.append(compiler);
}

void Settings::deleteCompiler(int index)
{
	if (0 <= index && index < compilerList.size())
	{
		delete compilerList[index];
		compilerList.removeAt(index);
	}
}

auto Settings::getCompiler(int index) -> Compiler *
{
	if (0 <= index && index < compilerList.size())
	{
		return compilerList[index];
	}

	return nullptr;
}

void Settings::swapCompiler(int a, int b)
{
	if (0 <= a && a < compilerList.size())
	{
		if (0 <= b && b < compilerList.size())
		{
			compilerList.swapItemsAt(a, b);
		}
	}
}

void Settings::setColorMxH(int x) { colorMxH = x; }

void Settings::setColorMxS(double x) { colorMxS = x; }

void Settings::setColorMxL(double x) { colorMxL = x; }

void Settings::setColorMiH(int x) { colorMiH = x; }

void Settings::setColorMiS(double x) { colorMiS = x; }

void Settings::setColorMiL(double x) { colorMiL = x; }

void Settings::setColorNfH(int x) { colorNfH = x; }

void Settings::setColorNfS(double x) { colorNfS = x; }

void Settings::setColorNfL(double x) { colorNfL = x; }

void Settings::setColorCeH(int x) { colorCeH = x; }

void Settings::setColorCeS(double x) { colorCeS = x; }

void Settings::setColorCeL(double x) { colorCeL = x; }

void Settings::setGrandCompH(double x) { grandCompH = x; }

void Settings::setGrandCompS(double x) { grandCompS = x; }

void Settings::setGrandCompL(double x) { grandCompL = x; }

void Settings::setGrandRateH(double x) { grandRateH = x; }

void Settings::setGrandRateS(double x) { grandRateS = x; }

void Settings::setGrandRateL(double x) { grandRateL = x; }

void Settings::setSplashTime(int x) { splashTime = x; }

void Settings::setTextAndColor(ResultState result, QString &text, QString &frColor, QString &bgColor)
{
	text = "";
	bgColor = "rgb(255, 255, 255)";
	frColor = "rgb(0, 0, 0)";

	switch (result)
	{
		case CorrectAnswer:
			text = tr("Correct Answer");
			bgColor = "rgb(192, 255, 192)";
			break;

		case WrongAnswer:
			text = tr("Wrong Answer");
			bgColor = "rgb(255, 192, 192)";
			break;

		case PartlyCorrect:
			text = tr("Partly Correct");
			bgColor = "rgb(192, 255, 255)";
			break;

		case PresentationError:
			text = tr("Presentation Error");
			bgColor = "rgb(255, 216, 192)";
			break;

		case TimeLimitExceeded:
			text = tr("Time Limit Exceeded");
			bgColor = "rgb(255, 255, 192)";
			break;

		case MemoryLimitExceeded:
			text = tr("Memory Limit Exceeded");
			bgColor = "rgb(192, 192, 255)";
			break;

		case OutputLimitExceeded:
			text = tr("Output Limit Exceeded");
			bgColor = "rgb(216, 192, 255)";
			break;

		case CannotStartProgram:
			text = tr("Cannot Start Program");
			frColor = "rgb(255, 64, 64)";
			bgColor = "rgb(192, 192, 192)";
			break;

		case FileError:
			text = tr("File Error");
			frColor = "rgb(255, 255, 64)";
			bgColor = "rgb(192, 192, 192)";
			break;

		case RunTimeError:
			text = tr("Run Time Error");
			bgColor = "rgb(255, 192, 255)";
			break;

		case InvalidSpecialJudge:
			text = tr("Invalid Special Judge");
			frColor = "rgb(255, 255, 255)";
			bgColor = "rgb(128, 0, 0)";
			break;

		case SpecialJudgeTimeLimitExceeded:
			text = tr("Special Judge Time Limit Exceeded");
			frColor = "rgb(255, 255, 255)";
			bgColor = "rgb(128, 128, 0)";
			break;

		case SpecialJudgeRunTimeError:
			text = tr("Special Judge Run Time Error");
			frColor = "rgb(255, 255, 255)";
			bgColor = "rgb(128, 0, 128)";
			break;

		case Skipped:
			text = tr("Skipped");
			frColor = "rgb(192, 192, 192)";
			bgColor = "rgb(255, 255, 255)";
			break;

		case InteractorError:
			text = tr("Interactor Error");
			frColor = "rgb(255, 255, 255)";
			bgColor = "rgb(0, 0, 128)";
			break;
	}
}

void Settings::copyFrom(Settings *other)
{
	setDefaultFullScore(other->getDefaultFullScore());
	setDefaultTimeLimit(other->getDefaultTimeLimit());
	setDefaultMemoryLimit(other->getDefaultMemoryLimit());
	setCompileTimeLimit(other->getCompileTimeLimit());
	setSpecialJudgeTimeLimit(other->getSpecialJudgeTimeLimit());
	setFileSizeLimit(other->getFileSizeLimit());
	setRejudgeTimes(other->getRejudgeTimes());
	setDefaultInputFileExtension(other->getDefaultInputFileExtension());
	setDefaultOutputFileExtension(other->getDefaultOutputFileExtension());
	setInputFileExtensions(other->getInputFileExtensions().join(";"));
	setOutputFileExtensions(other->getOutputFileExtensions().join(";"));
	setColorMxH(other->getColorMxH());
	setColorMxS(other->getColorMxS());
	setColorMxL(other->getColorMxL());
	setColorMiH(other->getColorMiH());
	setColorMiS(other->getColorMiS());
	setColorMiL(other->getColorMiL());
	setColorNfH(other->getColorNfH());
	setColorNfS(other->getColorNfS());
	setColorNfL(other->getColorNfL());
	setColorCeH(other->getColorCeH());
	setColorCeS(other->getColorCeS());
	setColorCeL(other->getColorCeL());
	setGrandCompH(other->getGrandCompH());
	setGrandCompS(other->getGrandCompS());
	setGrandCompL(other->getGrandCompL());
	setGrandRateH(other->getGrandRateH());
	setGrandRateS(other->getGrandRateS());
	setGrandRateL(other->getGrandRateL());
	setSplashTime(other->getSplashTime());

	for (auto &i : compilerList)
	{
		delete i;
	}

	compilerList.clear();
	const QList<Compiler *> &list = other->getCompilerList();

	for (auto i : list)
	{
		auto *compiler = new Compiler;
		compiler->copyFrom(i);
		addCompiler(compiler);
	}
}

void Settings::saveSettings()
{
	QSettings settings("LemonLime", "lemon");
	settings.setValue("UiLanguage", uiLanguage);
	settings.beginGroup("GeneralSettings");
	settings.setValue("DefaultFullScore", defaultFullScore);
	settings.setValue("DefaultTimeLimit", defaultTimeLimit);
	settings.setValue("DefaultMemoryLimit", defaultMemoryLimit);
	settings.setValue("CompileTimeLimit", compileTimeLimit);
	settings.setValue("SpecialJudgeTimeLimit", specialJudgeTimeLimit);
	settings.setValue("FileSizeLimit", fileSizeLimit);
	settings.setValue("MaximumRejudgeTimes", rejudgeTimes);
	settings.setValue("DefaultInputFileExtension", defaultInputFileExtension);
	settings.setValue("DefaultOutputFileExtension", defaultOutputFileExtension);
	settings.setValue("InputFileExtensions", inputFileExtensions);
	settings.setValue("OutputFileExtensions", outputFileExtensions);
	settings.endGroup();
	settings.beginGroup("VisualSettings");
	settings.setValue("ColorMxH", colorMxH);
	settings.setValue("ColorMxS", colorMxS);
	settings.setValue("ColorMxL", colorMxL);
	settings.setValue("ColorMiH", colorMiH);
	settings.setValue("ColorMiS", colorMiS);
	settings.setValue("ColorMiL", colorMiL);
	settings.setValue("ColorNfH", colorNfH);
	settings.setValue("ColorNfS", colorNfS);
	settings.setValue("ColorNfL", colorNfL);
	settings.setValue("ColorCeH", colorCeH);
	settings.setValue("ColorCeS", colorCeS);
	settings.setValue("ColorCeL", colorCeL);
	settings.setValue("GrandCompH", grandCompH);
	settings.setValue("GrandCompS", grandCompS);
	settings.setValue("GrandCompL", grandCompL);
	settings.setValue("GrandRateH", grandRateH);
	settings.setValue("GrandRateS", grandRateS);
	settings.setValue("GrandRateL", grandRateL);
	settings.setValue("SplashTime", splashTime);
	settings.endGroup();
	settings.beginWriteArray("v1.2/CompilerSettings");

	for (int i = 0; i < compilerList.size(); i++)
	{
		settings.setArrayIndex(i);
		settings.setValue("CompilerType", static_cast<int>(compilerList[i]->getCompilerType()));
		settings.setValue("CompilerName", compilerList[i]->getCompilerName());
		settings.setValue("SourceExtensions", compilerList[i]->getSourceExtensions());
		settings.setValue("CompilerLocation", compilerList[i]->getCompilerLocation());
		settings.setValue("InterpreterLocation", compilerList[i]->getInterpreterLocation());
		settings.setValue("BytecodeExtensions", compilerList[i]->getBytecodeExtensions());
		settings.setValue("TimeLimitRatio", compilerList[i]->getTimeLimitRatio());
		settings.setValue("MemoryLimitRatio", compilerList[i]->getMemoryLimitRatio());
		settings.setValue("DisableMemoryLimitCheck", compilerList[i]->getDisableMemoryLimitCheck());
		QStringList configurationNames = compilerList[i]->getConfigurationNames();
		QStringList compilerArguments = compilerList[i]->getCompilerArguments();
		QStringList interpreterArguments = compilerList[i]->getInterpreterArguments();
		settings.beginWriteArray("Configuration");

		for (int j = 0; j < configurationNames.size(); j++)
		{
			settings.setArrayIndex(j);
			settings.setValue("Name", configurationNames[j]);
			settings.setValue("CompilerArguments", compilerArguments[j]);
			settings.setValue("InterpreterArguments", interpreterArguments[j]);
		}

		settings.setValue("EnvironmentVariables", compilerList[i]->getEnvironment().toStringList());
		settings.endArray();
	}

	settings.endArray();
	settings.beginWriteArray("v1.2/RecentContest");

	for (int i = 0; i < recentContest.size(); i++)
	{
		settings.setArrayIndex(i);
		settings.setValue("Location", recentContest[i]);
	}

	settings.endArray();
}

void Settings::loadSettings()
{
	for (auto &i : compilerList)
		delete i;

	compilerList.clear();
	recentContest.clear();
	QSettings settings("LemonLime", "lemon");
	uiLanguage = settings.value("UiLanguage", QLocale::system().name()).toString();
	settings.beginGroup("GeneralSettings");
	defaultFullScore = settings.value("DefaultFullScore", 10).toInt();
	defaultTimeLimit = settings.value("DefaultTimeLimit", 1000).toInt();
	defaultMemoryLimit = settings.value("DefaultMemoryLimit", 512).toInt();
	compileTimeLimit = settings.value("CompileTimeLimit", 10000).toInt();
	specialJudgeTimeLimit = settings.value("SpecialJudgeTimeLimit", 10000).toInt();
	fileSizeLimit = settings.value("FileSizeLimit", 50).toInt();
	rejudgeTimes = settings.value("MaximumRejudgeTimes", 1).toInt();
	defaultInputFileExtension = settings.value("DefaultInputFileExtension", "in").toString();
	defaultOutputFileExtension = settings.value("DefaultOuputFileExtension", "out").toString();
	inputFileExtensions = settings.value("InputFileExtensions", QStringList() << "in").toStringList();
	outputFileExtensions = settings
	                           .value("OutputFileExtensions", QStringList() << "out"
	                                                                        << "ans")
	                           .toStringList();
	settings.endGroup();
	settings.beginGroup("VisualSettings");
	colorMxH = settings.value("ColorMxH", 120).toInt();
	colorMxS = settings.value("ColorMxS", 50).toDouble();
	colorMxL = settings.value("ColorMxL", 57.5).toDouble();
	colorMiH = settings.value("ColorMiH", 120).toInt();
	colorMiS = settings.value("ColorMiS", 50).toDouble();
	colorMiL = settings.value("ColorMiL", 100).toDouble();
	colorNfH = settings.value("ColorNfH", 0).toInt();
	colorNfS = settings.value("ColorNfS", 0).toDouble();
	colorNfL = settings.value("ColorNfL", 91.67).toDouble();
	colorCeH = settings.value("ColorCeH", 300).toInt();
	colorCeS = settings.value("ColorCeS", 100).toDouble();
	colorCeL = settings.value("ColorCeL", 83.33).toDouble();
	grandCompH = settings.value("GrandCompH", 0).toDouble();
	grandCompS = settings.value("GrandCompS", 0).toDouble();
	grandCompL = settings.value("GrandCompL", 0).toDouble();
	grandRateH = settings.value("GrandRateH", 1).toDouble();
	grandRateS = settings.value("GrandRateS", 1).toDouble();
	grandRateL = settings.value("GrandRateL", 1.33).toDouble();
	splashTime = settings.value("SplashTime", 500).toInt();
	settings.endGroup();
	int compilerCount = settings.beginReadArray("v1.2/CompilerSettings");

	for (int i = 0; i < compilerCount; i++)
	{
		settings.setArrayIndex(i);
		auto *compiler = new Compiler;
		compiler->setCompilerType(
		    static_cast<Compiler::CompilerType>(settings.value("CompilerType").toInt()));
		compiler->setCompilerName(settings.value("CompilerName").toString());
		compiler->setSourceExtensions(settings.value("SourceExtensions").toStringList().join(";"));
		compiler->setCompilerLocation(settings.value("CompilerLocation").toString());
		compiler->setInterpreterLocation(settings.value("InterpreterLocation").toString());
		compiler->setBytecodeExtensions(settings.value("BytecodeExtensions").toStringList().join(";"));
		compiler->setTimeLimitRatio(settings.value("TimeLimitRatio").toDouble());
		compiler->setMemoryLimitRatio(settings.value("MemoryLimitRatio").toDouble());
		compiler->setDisableMemoryLimitCheck(settings.value("DisableMemoryLimitCheck").toBool());
		int configurationCount = settings.beginReadArray("Configuration");

		for (int j = 0; j < configurationCount; j++)
		{
			settings.setArrayIndex(j);
			compiler->addConfiguration(settings.value("Name").toString(),
			                           settings.value("CompilerArguments").toString(),
			                           settings.value("InterpreterArguments").toString());
		}

		QStringList values = settings.value("EnvironmentVariables").toStringList();
		QProcessEnvironment environment;

		for (int i = 0; i < values.size(); i++)
		{
			int tmp = values[i].indexOf('=');
			QString variable = values[i].mid(0, tmp);
			QString value = values[i].mid(tmp + 1);
			environment.insert(variable, value);
		}

		compiler->setEnvironment(environment);
		settings.endArray();
		addCompiler(compiler);
	}

	settings.endArray();
	int listCount = settings.beginReadArray("v1.2/RecentContest");

	for (int i = 0; i < listCount; i++)
	{
		settings.setArrayIndex(i);
		recentContest.append(settings.value("Location").toString());
	}

	settings.endArray();
#ifdef Q_OS_WIN32
	diffPath = QDir::toNativeSeparators(QDir::currentPath()) + QDir::separator() + "diff.exe";
#else
	diffPath = "/usr/bin/diff";
#endif
}

auto Settings::upperBoundForFullScore() -> int { return 10000000; }

auto Settings::upperBoundForTimeLimit() -> int { return 1000 * 60 * 60 * 24; }

auto Settings::upperBoundForMemoryLimit() -> int { return 16777216; }

auto Settings::upperBoundForFileSizeLimit() -> int { return 256 * 1024; }

auto Settings::upperBoundForRejudgeTimes() -> int { return 12; }

auto Settings::dataPath() -> QString { return QString("data") + QDir::separator(); }

auto Settings::sourcePath() -> QString { return QString("source") + QDir::separator(); }

auto Settings::temporaryPath() -> QString { return QString("temp") + QDir::separator(); }

auto Settings::selfTestPath() -> QString { return QString("selftest") + QDir::separator(); }
