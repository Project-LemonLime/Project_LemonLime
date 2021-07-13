/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "settings.h"
//
#include "base/LemonLog.hpp"
#include "base/compiler.h"
//
#include <cmath>
//
#define LEMON_MODULE_NAME "Setting"

ColorTheme::ColorTheme(QObject *parent) : QObject(parent) {}

void ColorTheme::setColor(const hslTuple &_mx, const hslTuple &_mi, const hslTuple &_nf, const hslTuple &_ce,
                          const dddTuple &_gc, const dddTuple &_gr) {
	mxColor = _mx;
	miColor = _mi;
	nfColor = _nf;
	ceColor = _ce;
	colorNf = _nf.toHsl();
	colorCe = _ce.toHsl();
	grandComp = _gc;
	grandRate = _gr;
}

void ColorTheme::setName(QString newName) { name = newName; }

QString ColorTheme::getName() const { return name; }

hslTuple ColorTheme::getMxColor() const { return mxColor; }

hslTuple ColorTheme::getMiColor() const { return miColor; }

hslTuple ColorTheme::getNfColor() const { return nfColor; }

hslTuple ColorTheme::getCeColor() const { return ceColor; }

dddTuple ColorTheme::getGrandComp() const { return grandComp; }

dddTuple ColorTheme::getGrandRate() const { return grandRate; }

QColor ColorTheme::getColorNf() const { return colorNf; }

QColor ColorTheme::getColorCe() const { return colorCe; }

void ColorTheme::copyFrom(ColorTheme *others) {
	setName(others->getName());
	setColor(others->getMxColor(), others->getMiColor(), others->getNfColor(), others->getCeColor(),
	         others->getGrandComp(), others->getGrandRate());
}

double makePer(double p, double l, double r, double bound, double distanRate = 1.00, double resComp = 0.00) {
	double distan = distanRate * (r - l) / bound / 110.00;
	double res = (resComp + l) / bound + 100.00 * p * distan;
	if (p > 0)
		res += distan * 5;
	if (p >= 1 - 1e-12)
		res += distan * 5;
	while (res < -1e-12)
		res += 1;
	while (res > 1 + 1e-12)
		res -= 1;
	res = fmax(0, fmin(res, 1));
	return res;
}

QColor ColorTheme::getColorPer(double p) const {
	if (qIsNaN(p) || qIsInf(p))
		p = 0;

	return QColor::fromHslF(makePer(p, miColor.h, mxColor.h, 360.00),
	                        makePer(p, miColor.s, mxColor.s, 100.00),
	                        makePer(p, miColor.l, mxColor.l, 100.00));
}

QColor ColorTheme::getColorGrand(double p) const {
	if (qIsNaN(p) || qIsInf(p))
		p = 0;

	return QColor::fromHslF(makePer(p, miColor.h, mxColor.h, 360.00, grandRate.h, grandComp.h),
	                        makePer(p, miColor.s, mxColor.s, 100.00, grandRate.s, grandComp.s),
	                        makePer(p, miColor.l, mxColor.l, 100.00, grandRate.l, grandComp.l));
}

QColor ColorTheme::getColorPer(double a, double b) const { return getColorPer(a / b); }

QColor ColorTheme::getColorGrand(double a, double b) const { return getColorGrand(a / b); }

Settings::Settings(QObject *parent) : QObject(parent) {}

auto Settings::getDefaultFullScore() const -> int { return defaultFullScore; }

auto Settings::getDefaultTimeLimit() const -> int { return defaultTimeLimit; }

auto Settings::getDefaultMemoryLimit() const -> int { return defaultMemoryLimit; }

auto Settings::getCompileTimeLimit() const -> int { return compileTimeLimit; }

auto Settings::getSpecialJudgeTimeLimit() const -> int { return specialJudgeTimeLimit; }

auto Settings::getFileSizeLimit() const -> int { return fileSizeLimit; }

auto Settings::getRejudgeTimes() const -> int { return rejudgeTimes; }

auto Settings::getDefaultExtraTimeRatio() const -> double { return defaultExtraTimeRatio; }

auto Settings::getDefaultInputFileExtension() const -> const QString & { return defaultInputFileExtension; }

auto Settings::getDefaultOutputFileExtension() const -> const QString & { return defaultOutputFileExtension; }

auto Settings::getInputFileExtensions() const -> const QStringList & { return inputFileExtensions; }

auto Settings::getOutputFileExtensions() const -> const QStringList & { return outputFileExtensions; }

auto Settings::getRecentContest() const -> const QStringList & { return recentContest; }

auto Settings::getCompilerList() const -> const QList<Compiler *> & { return compilerList; }

const QList<ColorTheme *> &Settings::getColorThemeList() const { return colorThemeList; }

const ColorTheme *Settings::getCurrentColorTheme() const { return colorThemeList[currentColorTheme]; }

auto Settings::getUiLanguage() const -> const QString & { return uiLanguage; }

auto Settings::getDiffPath() const -> const QString & { return diffPath; }

auto Settings::getSplashTime() const -> int { return splashTime; }

void Settings::setDefaultFullScore(int score) {
	defaultFullScore = score;
    DEBUG("Set Default Full Score to " + QString::number(score));
}

void Settings::setDefaultTimeLimit(int limit) {
	defaultTimeLimit = limit;
    DEBUG("Set Default Time Limit to " + QString::number(limit));
}

void Settings::setDefaultExtraTimeRatio(double ratio) {
	defaultExtraTimeRatio = ratio;
    DEBUG("Set Default Extra Time Ratio to " + QString::number(ratio));
}
void Settings::setDefaultMemoryLimit(int limit) {
	defaultMemoryLimit = limit;
    DEBUG("Set Default Memory Limit to " + QString::number(limit));
}

void Settings::setCompileTimeLimit(int limit) {
	compileTimeLimit = limit;
    DEBUG("Set Compile Time Limit to " + QString::number(limit));
}

void Settings::setSpecialJudgeTimeLimit(int limit) {
	specialJudgeTimeLimit = limit;
    DEBUG("Set SpecialJudge Time Limit to " + QString::number(limit));
}

void Settings::setFileSizeLimit(int limit) {
	fileSizeLimit = limit;
    DEBUG("Set File Size Limit to " + QString::number(limit));
}

void Settings::setRejudgeTimes(int number) {
	rejudgeTimes = number;
    DEBUG("Set Rejudge Times to " + QString::number(number));
}

void Settings::setDefaultInputFileExtension(const QString &extension) {
	defaultInputFileExtension = extension;
    DEBUG("Set Default InputFile Extension to " + extension);
}

void Settings::setDefaultOutputFileExtension(const QString &extension) {
	defaultOutputFileExtension = extension;
    DEBUG("Set Default OutputFile Extension to " + extension);
}

void Settings::setInputFileExtensions(const QString &extensions) {
	inputFileExtensions = extensions.split(";", Qt::SkipEmptyParts);
    DEBUG("Set InputFile Extensions to " + extensions);
}

void Settings::setOutputFileExtensions(const QString &extensions) {
	outputFileExtensions = extensions.split(";", Qt::SkipEmptyParts);
    DEBUG("Set OutputFile Extensions to " + extensions);
}

void Settings::setRecentContest(const QStringList &list) { recentContest = list; }

void Settings::setUiLanguage(const QString &language) {
	uiLanguage = language;
    DEBUG("Set Language to " + language);
}

void Settings::addCompiler(Compiler *compiler) {
	compiler->setParent(this);
	compilerList.append(compiler);
}

void Settings::deleteCompiler(int index) {
	if (0 <= index && index < compilerList.size()) {
		delete compilerList[index];
		compilerList.removeAt(index);
	}
}

auto Settings::getCompiler(int index) -> Compiler * {
	if (0 <= index && index < compilerList.size()) {
		return compilerList[index];
	}

	return nullptr;
}

void Settings::swapCompiler(int a, int b) {
	if (0 <= a && a < compilerList.size()) {
		if (0 <= b && b < compilerList.size()) {
			compilerList.swapItemsAt(a, b);
		}
	}
}

void Settings::addColorTheme(ColorTheme *colortheme) {
	colortheme->setParent(this);
	colorThemeList.append(colortheme);
}

void Settings::deleteColorTheme(int index) {
	if (0 <= index && index < colorThemeList.size()) {
		delete colorThemeList[index];
		colorThemeList.removeAt(index);
	}
}

ColorTheme *Settings::getColorTheme(int index) {
	if (0 <= index && index < colorThemeList.size()) {
		return colorThemeList[index];
	}

	return nullptr;
}

int Settings::getCurrentColorThemeIndex() const { return currentColorTheme; }

void Settings::setColorTheme(ColorTheme *x, int index) {
	if (0 <= index && index < colorThemeList.size()) {
		colorThemeList[index] = x;
	}
}

void Settings::setCurrendColorTheme(ColorTheme *x) { colorThemeList[currentColorTheme] = x; }

void Settings::setCurrentColorThemeIndex(int x) {
	if (0 <= x && x < colorThemeList.size())
		currentColorTheme = x;
}

void Settings::setSplashTime(int x) { splashTime = x; }

void Settings::setTextAndColor(ResultState result, QString &text, QString &frColor, QString &bgColor) {
	text = "";
	bgColor = "rgb(255, 255, 255)";
	frColor = "rgb(0, 0, 0)";

	switch (result) {
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

void Settings::copyFrom(Settings *other) {
	setDefaultFullScore(other->getDefaultFullScore());
	setDefaultTimeLimit(other->getDefaultTimeLimit());
	setDefaultExtraTimeRatio(other->getDefaultExtraTimeRatio());
	setDefaultMemoryLimit(other->getDefaultMemoryLimit());
	setCompileTimeLimit(other->getCompileTimeLimit());
	setSpecialJudgeTimeLimit(other->getSpecialJudgeTimeLimit());
	setFileSizeLimit(other->getFileSizeLimit());
	setRejudgeTimes(other->getRejudgeTimes());
	setDefaultInputFileExtension(other->getDefaultInputFileExtension());
	setDefaultOutputFileExtension(other->getDefaultOutputFileExtension());
	setInputFileExtensions(other->getInputFileExtensions().join(";"));
	setOutputFileExtensions(other->getOutputFileExtensions().join(";"));
	setSplashTime(other->getSplashTime());
	setUiLanguage(other->getUiLanguage()); // 为什么这个没复制啊！！！// 草

	for (auto &i : compilerList) {
		delete i;
	}

	compilerList.clear();
	const QList<Compiler *> &newComplierList = other->getCompilerList();

	for (auto *i : newComplierList) {
		auto *compiler = new Compiler;
		compiler->copyFrom(i);
		addCompiler(compiler);
	}

	for (auto &i : colorThemeList) {
		delete i;
	}

	colorThemeList.clear();
	const QList<ColorTheme *> &newColorThemeList = other->getColorThemeList();

	for (auto *i : newColorThemeList) {
		auto *colorTheme = new ColorTheme;
		colorTheme->copyFrom(i);
		addColorTheme(colorTheme);
	}

	setCurrentColorThemeIndex(other->getCurrentColorThemeIndex());
}

void Settings::saveSettings() {
	QSettings settings("LemonLime", "lemon");
	settings.setValue("UiLanguage", uiLanguage);
	settings.beginGroup("GeneralSettings");
	settings.setValue("DefaultFullScore", defaultFullScore);
	settings.setValue("DefaultTimeLimit", defaultTimeLimit);
	settings.setValue("DefaultExtraTimeRatio", defaultExtraTimeRatio);
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
	settings.beginWriteArray("ColorThemes");
	for (int i = 0; i < colorThemeList.size(); i++) {
		settings.setArrayIndex(i);
		settings.setValue("ThemeName", colorThemeList[i]->getName());
		settings.setValue("MxColorH", colorThemeList[i]->getMxColor().h);
		settings.setValue("MxColorS", colorThemeList[i]->getMxColor().s);
		settings.setValue("MxColorL", colorThemeList[i]->getMxColor().l);
		settings.setValue("MiColorH", colorThemeList[i]->getMiColor().h);
		settings.setValue("MiColorS", colorThemeList[i]->getMiColor().s);
		settings.setValue("MiColorL", colorThemeList[i]->getMiColor().l);
		settings.setValue("NfColorH", colorThemeList[i]->getNfColor().h);
		settings.setValue("NfColorS", colorThemeList[i]->getNfColor().s);
		settings.setValue("NfColorL", colorThemeList[i]->getNfColor().l);
		settings.setValue("CeColorH", colorThemeList[i]->getCeColor().h);
		settings.setValue("CeColorS", colorThemeList[i]->getCeColor().s);
		settings.setValue("CeColorL", colorThemeList[i]->getCeColor().l);
		settings.setValue("GrandCompH", colorThemeList[i]->getGrandComp().h);
		settings.setValue("GrandCompS", colorThemeList[i]->getGrandComp().s);
		settings.setValue("GrandCompL", colorThemeList[i]->getGrandComp().l);
		settings.setValue("GrandRateH", colorThemeList[i]->getGrandRate().h);
		settings.setValue("GrandRateS", colorThemeList[i]->getGrandRate().s);
		settings.setValue("GrandRateL", colorThemeList[i]->getGrandRate().l);
		/*
		settings.setValue("MxColor", QVariant::fromValue(colorThemeList[i]->getMxColor()));
		settings.setValue("MiColor", QVariant::fromValue(colorThemeList[i]->getMiColor()));
		settings.setValue("NfColor", QVariant::fromValue(colorThemeList[i]->getNfColor()));
		settings.setValue("CeColor", QVariant::fromValue(colorThemeList[i]->getCeColor()));
		settings.setValue("GrandComp", QVariant::fromValue(colorThemeList[i]->getGrandComp()));
		settings.setValue("GrandRate", QVariant::fromValue(colorThemeList[i]->getGrandRate()));
		*/
	}
	settings.endArray();
	settings.setValue("CurrentColorTheme", currentColorTheme);
	settings.setValue("SplashTime", splashTime);
	settings.endGroup();

	settings.beginWriteArray("v1.2/CompilerSettings");

	for (int i = 0; i < compilerList.size(); i++) {
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

		for (int j = 0; j < configurationNames.size(); j++) {
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

	for (int i = 0; i < recentContest.size(); i++) {
		settings.setArrayIndex(i);
		settings.setValue("Location", recentContest[i]);
	}

	settings.endArray();
}

void Settings::loadSettings() {
	for (auto &i : compilerList)
		delete i;
	for (auto &i : colorThemeList)
		delete i;

	compilerList.clear();
	colorThemeList.clear();
	recentContest.clear();
	QSettings settings("LemonLime", "lemon");
	uiLanguage = settings.value("UiLanguage", QLocale::system().name()).toString();
	settings.beginGroup("GeneralSettings");
	defaultFullScore = settings.value("DefaultFullScore", 10).toInt();
	defaultTimeLimit = settings.value("DefaultTimeLimit", 1000).toInt();
	defaultExtraTimeRatio = settings.value("DefaultExtraTimeRatio", 0.1).toDouble();
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

	int themesCount = settings.beginReadArray("ColorThemes");
	for (int i = 0; i < themesCount; i++) {
		settings.setArrayIndex(i);
		auto *colorTheme = new ColorTheme;
		colorTheme->setName(settings.value("ThemeName").toString());
		/*
		colorTheme->setColor(
		    settings.value("MxColor").value<hslTuple>(), settings.value("MiColor").value<hslTuple>(),
		    settings.value("NfColor").value<hslTuple>(), settings.value("CeColor").value<hslTuple>(),
		    settings.value("GrandComp").value<dddTuple>(), settings.value("GrandRate").value<dddTuple>());
		*/
		colorTheme->setColor(
		    hslTuple(settings.value("MxColorH").toInt(), settings.value("MxColorS").toDouble(),
		             settings.value("MxColorL").toDouble()),
		    hslTuple(settings.value("MiColorH").toInt(), settings.value("MiColorS").toDouble(),
		             settings.value("MiColorL").toDouble()),
		    hslTuple(settings.value("NfColorH").toInt(), settings.value("NfColorS").toDouble(),
		             settings.value("NfColorL").toDouble()),
		    hslTuple(settings.value("CeColorH").toInt(), settings.value("CeColorS").toDouble(),
		             settings.value("CeColorL").toDouble()),
		    dddTuple(settings.value("GrandCompH").toDouble(), settings.value("GrandCompS").toDouble(),
		             settings.value("GrandCompL").toDouble()),
		    dddTuple(settings.value("GrandRateH").toDouble(), settings.value("GrandRateS").toDouble(),
		             settings.value("GrandRateL").toDouble()));
		addColorTheme(colorTheme);
	}
	settings.endArray();
    currentColorTheme = settings.value("CurrentColorTheme", 0).toInt();
	splashTime = settings.value("SplashTime", 500).toInt();
	settings.endGroup();

	if (colorThemeList.isEmpty()) {
		{
			auto *colorTheme = new ColorTheme;
			colorTheme->setName(tr("ranking.ioi2021.sg"));
			colorTheme->setColor(hslTuple(120, 28.9, 64.7), hslTuple(120, 28.9, 100.0), hslTuple(0, 0, 75.0),
			                     hslTuple(320, 60.0, 75.0), dddTuple(0.00, 1.10, 0.00),
			                     dddTuple(1, 1, 64.7 / 55.0));

			addColorTheme(colorTheme);
		} // dmy ak ioi !!!

		currentColorTheme = 0;

		{
			auto *colorTheme = new ColorTheme;
			colorTheme->setName(tr("mock joi2020"));
			colorTheme->setColor(hslTuple(120, 70.0, 60.0), hslTuple(0, 70.0, 60.0), hslTuple(0, 0.0, 91.67),
			                     hslTuple(240, 70.0, 60.0), dddTuple(0.00, 0.00, 0.00), dddTuple(1, 1, 1.33));
			addColorTheme(colorTheme);
		}
	}

	int compilerCount = settings.beginReadArray("v1.2/CompilerSettings");

	for (int i = 0; i < compilerCount; i++) {
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

		for (int j = 0; j < configurationCount; j++) {
			settings.setArrayIndex(j);
			compiler->addConfiguration(settings.value("Name").toString(),
			                           settings.value("CompilerArguments").toString(),
			                           settings.value("InterpreterArguments").toString());
		}

		QStringList values = settings.value("EnvironmentVariables").toStringList();
		QProcessEnvironment environment;

		for (auto &i : values) {
			int tmp = i.indexOf('=');
			QString variable = i.mid(0, tmp);
			QString value = i.mid(tmp + 1);
			environment.insert(variable, value);
		}

		compiler->setEnvironment(environment);
		settings.endArray();
		addCompiler(compiler);
	}

	settings.endArray();
	int listCount = settings.beginReadArray("v1.2/RecentContest");

	for (int i = 0; i < listCount; i++) {
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

auto Settings::upperBoundForExtraTimeRatio() -> double { return 100; }

auto Settings::upperBoundForMemoryLimit() -> int { return 16777216; }

auto Settings::upperBoundForFileSizeLimit() -> int { return 256 * 1024; }

auto Settings::upperBoundForRejudgeTimes() -> int { return 12; }

auto Settings::dataPath() -> QString { return QString("data") + QDir::separator(); }

auto Settings::sourcePath() -> QString { return QString("source") + QDir::separator(); }

auto Settings::selfTestPath() -> QString { return QString("selftest") + QDir::separator(); }
