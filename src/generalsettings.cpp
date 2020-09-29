/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "generalsettings.h"
#include "core/settings.h"
#include "ui_generalsettings.h"
#include <QMessageBox>

GeneralSettings::GeneralSettings(QWidget *parent) : QWidget(parent), ui(new Ui::GeneralSettings)
{
	ui->setupUi(this);
	ui->defaultFullScore->setValidator(new QIntValidator(1, Settings::upperBoundForFullScore(), this));
	ui->defaultTimeLimit->setValidator(new QIntValidator(1, Settings::upperBoundForTimeLimit(), this));
	ui->defaultMemoryLimit->setValidator(new QIntValidator(1, Settings::upperBoundForMemoryLimit(), this));
	ui->compileTimeLimit->setValidator(new QIntValidator(1, Settings::upperBoundForTimeLimit(), this));
	ui->specialJudgeTimeLimit->setValidator(new QIntValidator(1, Settings::upperBoundForTimeLimit(), this));
	ui->fileSizeLimit->setValidator(new QIntValidator(1, Settings::upperBoundForFileSizeLimit(), this));
	ui->rejudgeTimes->setValidator(new QIntValidator(0, Settings::upperBoundForRejudgeTimes(), this));
	ui->inputFileExtensions->setValidator(new QRegExpValidator(QRegExp("(\\w+;)*\\w+"), this));
	ui->outputFileExtensions->setValidator(new QRegExpValidator(QRegExp("(\\w+;)*\\w+"), this));
	connect(ui->defaultFullScore, SIGNAL(textChanged(QString)), this, SLOT(defaultFullScoreChanged(QString)));
	connect(ui->defaultTimeLimit, SIGNAL(textChanged(QString)), this, SLOT(defaultTimeLimitChanged(QString)));
	connect(ui->defaultMemoryLimit, SIGNAL(textChanged(QString)), this,
	        SLOT(defaultMemoryLimitChanged(QString)));
	connect(ui->compileTimeLimit, SIGNAL(textChanged(QString)), this, SLOT(compileTimeLimitChanged(QString)));
	connect(ui->specialJudgeTimeLimit, SIGNAL(textChanged(QString)), this,
	        SLOT(specialJudgeTimeLimitChanged(QString)));
	connect(ui->fileSizeLimit, SIGNAL(textChanged(QString)), this, SLOT(fileSizeLimitChanged(QString)));
	connect(ui->rejudgeTimes, SIGNAL(textChanged(QString)), this, SLOT(rejudgeTimesChanged(QString)));
	connect(ui->inputFileExtensions, SIGNAL(textChanged(QString)), this,
	        SLOT(inputFileExtensionsChanged(QString)));
	connect(ui->outputFileExtensions, SIGNAL(textChanged(QString)), this,
	        SLOT(outputFileExtensionsChanged(QString)));
}

GeneralSettings::~GeneralSettings() { delete ui; }

void GeneralSettings::resetEditSettings(Settings *settings)
{
	editSettings = settings;
	ui->defaultFullScore->setText(QString("%1").arg(editSettings->getDefaultFullScore()));
	ui->defaultTimeLimit->setText(QString("%1").arg(editSettings->getDefaultTimeLimit()));
	ui->defaultMemoryLimit->setText(QString("%1").arg(editSettings->getDefaultMemoryLimit()));
	ui->compileTimeLimit->setText(QString("%1").arg(editSettings->getCompileTimeLimit()));
	ui->specialJudgeTimeLimit->setText(QString("%1").arg(editSettings->getSpecialJudgeTimeLimit()));
	ui->fileSizeLimit->setText(QString("%1").arg(editSettings->getFileSizeLimit()));
	ui->rejudgeTimes->setText(QString("%1").arg(editSettings->getRejudgeTimes()));
	ui->inputFileExtensions->setText(editSettings->getInputFileExtensions().join(";"));
	ui->outputFileExtensions->setText(editSettings->getOutputFileExtensions().join(";"));
}

auto GeneralSettings::checkValid() -> bool
{
	if (ui->defaultFullScore->text().isEmpty())
	{
		ui->defaultFullScore->setFocus();
		QMessageBox::warning(this, tr("Error"), tr("Empty default full score!"), QMessageBox::Close);
		return false;
	}

	if (ui->defaultTimeLimit->text().isEmpty())
	{
		ui->defaultTimeLimit->setFocus();
		QMessageBox::warning(this, tr("Error"), tr("Empty default time limit!"), QMessageBox::Close);
		return false;
	}

	if (ui->defaultMemoryLimit->text().isEmpty())
	{
		ui->defaultMemoryLimit->setFocus();
		QMessageBox::warning(this, tr("Error"), tr("Empty default memory limit!"), QMessageBox::Close);
		return false;
	}

	if (ui->compileTimeLimit->text().isEmpty())
	{
		ui->compileTimeLimit->setFocus();
		QMessageBox::warning(this, tr("Error"), tr("Empty compile time limit!"), QMessageBox::Close);
		return false;
	}

	if (ui->specialJudgeTimeLimit->text().isEmpty())
	{
		ui->specialJudgeTimeLimit->setFocus();
		QMessageBox::warning(this, tr("Error"), tr("Empty special judge time limit!"), QMessageBox::Close);
		return false;
	}

	if (ui->fileSizeLimit->text().isEmpty())
	{
		ui->fileSizeLimit->setFocus();
		QMessageBox::warning(this, tr("Error"), tr("Empty source file size limit!"), QMessageBox::Close);
		return false;
	}

	if (ui->rejudgeTimes->text().isEmpty())
	{
		ui->rejudgeTimes->setFocus();
		QMessageBox::warning(this, tr("Error"), tr("Empty maximum rejudge times!"), QMessageBox::Close);
	}

	return true;
}

void GeneralSettings::defaultFullScoreChanged(const QString &text)
{
	editSettings->setDefaultFullScore(text.toInt());
}

void GeneralSettings::defaultTimeLimitChanged(const QString &text)
{
	editSettings->setDefaultTimeLimit(text.toInt());
}

void GeneralSettings::defaultMemoryLimitChanged(const QString &text)
{
	editSettings->setDefaultMemoryLimit(text.toInt());
}

void GeneralSettings::compileTimeLimitChanged(const QString &text)
{
	editSettings->setCompileTimeLimit(text.toInt());
}

void GeneralSettings::specialJudgeTimeLimitChanged(const QString &text)
{
	editSettings->setSpecialJudgeTimeLimit(text.toInt());
}

void GeneralSettings::fileSizeLimitChanged(const QString &text)
{
	editSettings->setFileSizeLimit(text.toInt());
}

void GeneralSettings::rejudgeTimesChanged(const QString &text)
{
	editSettings->setRejudgeTimes(text.toInt());
}

void GeneralSettings::inputFileExtensionsChanged(const QString &text)
{
	editSettings->setInputFileExtensions(text);
}

void GeneralSettings::outputFileExtensionsChanged(const QString &text)
{
	editSettings->setOutputFileExtensions(text);
}
