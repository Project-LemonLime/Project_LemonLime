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
 * advancedcompilersettingsdialog.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * addcompilerwizard.cpp @Project LemonLime
 * Update 2019 iotang
 **/

#include "advancedcompilersettingsdialog.h"
#include "ui_advancedcompilersettingsdialog.h"
#include "environmentvariablesdialog.h"
#include "compiler.h"
#include <QMessageBox>
#include <QFileDialog>

AdvancedCompilerSettingsDialog::AdvancedCompilerSettingsDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AdvancedCompilerSettingsDialog)
{
	ui->setupUi(this);

	editCompiler = new Compiler(this);
	ui->bytecodeExtension->setValidator(new QRegExpValidator(QRegExp("(\\w+;)*\\w+"), this));
	ui->configurationSelect->setLineEdit(new QLineEdit(this));

	connect(ui->buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
	        this, SLOT(okayButtonClicked()));
	connect(ui->typeSelect, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(compilerTypeChanged()));
	connect(ui->compilerLocation, SIGNAL(textChanged(QString)),
	        this, SLOT(compilerLocationChanged()));
	connect(ui->interpreterLocation, SIGNAL(textChanged(QString)),
	        this, SLOT(interpreterLocationChanged()));
	connect(ui->compilerSelectButton, SIGNAL(clicked()),
	        this, SLOT(selectCompilerLocation()));
	connect(ui->interpreterSelectButton, SIGNAL(clicked()),
	        this, SLOT(selectInterpreterLocation()));
	connect(ui->bytecodeExtension, SIGNAL(textChanged(QString)),
	        this, SLOT(bytecodeExtensionsChanged()));
	connect(ui->timeLimitRatio, SIGNAL(valueChanged(double)),
	        this, SLOT(timeLimitRatioChanged()));
	connect(ui->memoryLimitRatio, SIGNAL(valueChanged(double)),
	        this, SLOT(memoryLimitRatioChanged()));
	connect(ui->disableMemoryLimit, SIGNAL(stateChanged(int)),
	        this, SLOT(disableMemoryLimitCheckChanged()));
	connect(ui->configurationSelect, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(configurationIndexChanged()));
	connect(ui->configurationSelect, SIGNAL(editTextChanged(QString)),
	        this, SLOT(configurationTextChanged()));
	connect(ui->deleteConfigurationButton, SIGNAL(clicked()),
	        this, SLOT(deleteConfiguration()));
	connect(ui->compilerArguments, SIGNAL(textChanged(QString)),
	        this, SLOT(compilerArgumentsChanged()));
	connect(ui->interpreterArguments, SIGNAL(textChanged(QString)),
	        this, SLOT(interpreterArgumentsChanged()));
	connect(ui->environmentVariablesButton, SIGNAL(clicked()),
	        this, SLOT(environmentVariablesButtonClicked()));
}

AdvancedCompilerSettingsDialog::~AdvancedCompilerSettingsDialog()
{
	delete ui;
}

void AdvancedCompilerSettingsDialog::resetEditCompiler(Compiler *compiler)
{
	configCount = 0;
	editCompiler->copyFrom(compiler);
	ui->typeSelect->setCurrentIndex(int (editCompiler->getCompilerType()));
	compilerTypeChanged();
	ui->compilerLocation->setText(editCompiler->getCompilerLocation());
	ui->interpreterLocation->setText(editCompiler->getInterpreterLocation());
	ui->bytecodeExtension->setText(editCompiler->getBytecodeExtensions().join(";"));
	ui->timeLimitRatio->setValue(editCompiler->getTimeLimitRatio());
	ui->memoryLimitRatio->setValue(editCompiler->getMemoryLimitRatio());
	ui->disableMemoryLimit->setChecked(editCompiler->getDisableMemoryLimitCheck());
	ui->memoryLimitRatio->setEnabled(! editCompiler->getDisableMemoryLimitCheck());
	QStringList configurationNames = editCompiler->getConfigurationNames();
	ui->configurationSelect->setEnabled(false);

	for (int i = 0; i < configurationNames.size(); i ++)
	{
		ui->configurationSelect->addItem(configurationNames[i]);
	}

	ui->configurationSelect->addItem(tr("Add new ..."));
	ui->configurationSelect->setEnabled(true);
	ui->configurationSelect->setCurrentIndex(0);
	configurationIndexChanged();
}

auto AdvancedCompilerSettingsDialog::getEditCompiler() const -> Compiler *
{
	return editCompiler;
}

void AdvancedCompilerSettingsDialog::okayButtonClicked()
{
	if (ui->compilerLocation->isEnabled() && ui->compilerLocation->text().isEmpty())
	{
		ui->compilerLocation->setFocus();
		QMessageBox::warning(this, tr("Error"), tr("Empty compiler\'s Location!"), QMessageBox::Close);
		return;
	}

	if (ui->interpreterLocation->isEnabled() && ui->interpreterLocation->text().isEmpty())
	{
		ui->interpreterLocation->setFocus();
		QMessageBox::warning(this, tr("Error"), tr("Empty interpreter\'s Location!"), QMessageBox::Close);
		return;
	}

	if (ui->bytecodeExtension->isEnabled() && ui->bytecodeExtension->text().isEmpty())
	{
		ui->bytecodeExtension->setFocus();
		QMessageBox::warning(this, tr("Error"), tr("Empty Byte-code Extensions!"), QMessageBox::Close);
		return;
	}

	const QStringList &configurationNames = editCompiler->getConfigurationNames();

	for (int j = 0; j < configurationNames.size(); j ++)
	{
		if (configurationNames[j].isEmpty())
		{
			ui->configurationSelect->setCurrentIndex(j);
			ui->configurationSelect->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty configuration name!"), QMessageBox::Close);
			return;
		}

		if (configurationNames.count(configurationNames[j]) > 1)
		{
			ui->configurationSelect->setCurrentIndex(j);
			ui->configurationSelect->setFocus();
			QMessageBox::warning(this, tr("Error"),
			                     tr("Configuration %1 appears more than once!").arg(configurationNames[j]),
			                     QMessageBox::Close);
			return;
		}

		if (configurationNames[j] == "disable")
		{
			ui->configurationSelect->setCurrentIndex(j);
			ui->configurationSelect->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Invalid configuration name \"disable\"!"), QMessageBox::Close);
			return;
		}
	}

	accept();
}

void AdvancedCompilerSettingsDialog::compilerTypeChanged()
{
	editCompiler->setCompilerType(static_cast<Compiler::CompilerType>(ui->typeSelect->currentIndex()));

	if (editCompiler->getCompilerType() == Compiler::Typical)
	{
		ui->interpreterLabel->setEnabled(false);
		ui->interpreterLocation->setEnabled(false);
		ui->interpreterSelectButton->setEnabled(false);
		ui->interpreterArgumentsLabel->setEnabled(false);
		ui->interpreterArguments->setEnabled(false);
	}
	else
	{
		ui->interpreterLabel->setEnabled(true);
		ui->interpreterLocation->setEnabled(true);
		ui->interpreterSelectButton->setEnabled(true);
		ui->interpreterArgumentsLabel->setEnabled(true);
		ui->interpreterArguments->setEnabled(true);
	}

	if (editCompiler->getCompilerType() == Compiler::InterpretiveWithByteCode)
	{
		ui->bytecodeExtensionLabel->setEnabled(true);
		ui->bytecodeExtension->setEnabled(true);
	}
	else
	{
		ui->bytecodeExtensionLabel->setEnabled(false);
		ui->bytecodeExtension->setEnabled(false);
	}

	if (editCompiler->getCompilerType() == Compiler::InterpretiveWithoutByteCode)
	{
		ui->compilerLabel->setEnabled(false);
		ui->compilerLocation->setEnabled(false);
		ui->compilerSelectButton->setEnabled(false);
		ui->compilerArgumentsLabel->setEnabled(false);
		ui->compilerArguments->setEnabled(false);
	}
	else
	{
		ui->compilerLabel->setEnabled(true);
		ui->compilerLocation->setEnabled(true);
		ui->compilerSelectButton->setEnabled(true);
		ui->compilerArgumentsLabel->setEnabled(true);
		ui->compilerArguments->setEnabled(true);
	}
}

void AdvancedCompilerSettingsDialog::compilerLocationChanged()
{
	editCompiler->setCompilerLocation(ui->compilerLocation->text());
}

void AdvancedCompilerSettingsDialog::interpreterLocationChanged()
{
	editCompiler->setInterpreterLocation(ui->interpreterLocation->text());
}

void AdvancedCompilerSettingsDialog::selectCompilerLocation()
{
#ifdef Q_OS_WIN32
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), tr("Executable files (*.exe)"));
#else
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), tr("Executable files (*.*)"));
#endif

	if (! location.isEmpty())
	{
		location = location.replace('/', QDir::separator());
		ui->compilerLocation->setText(location);
	}
}

void AdvancedCompilerSettingsDialog::selectInterpreterLocation()
{
#ifdef Q_OS_WIN32
	QString location = QFileDialog::getOpenFileName(this, tr("Select Interpreter\'s Location"),
	                   QDir::rootPath(), tr("Executable files (*.exe)"));
#else
	QString location = QFileDialog::getOpenFileName(this, tr("Select Interpreter\'s Location"),
	                   QDir::rootPath(), tr("Executable files (*.*)"));
#endif

	if (! location.isEmpty())
	{
		location = location.replace('/', QDir::separator());
		ui->interpreterLocation->setText(location);
	}
}

void AdvancedCompilerSettingsDialog::bytecodeExtensionsChanged()
{
	editCompiler->setBytecodeExtensions(ui->bytecodeExtension->text());
}

void AdvancedCompilerSettingsDialog::timeLimitRatioChanged()
{
	editCompiler->setTimeLimitRatio(ui->timeLimitRatio->value());
}

void AdvancedCompilerSettingsDialog::memoryLimitRatioChanged()
{
	editCompiler->setMemoryLimitRatio(ui->memoryLimitRatio->value());
}

void AdvancedCompilerSettingsDialog::disableMemoryLimitCheckChanged()
{
	bool check = ui->disableMemoryLimit->isChecked();
	editCompiler->setDisableMemoryLimitCheck(check);
	ui->memoryLimitRatioLabel->setEnabled(! check);
	ui->memoryLimitRatio->setEnabled(! check);
}

void AdvancedCompilerSettingsDialog::configurationIndexChanged()
{
	if (! ui->configurationSelect->isEnabled()) return;

	int index = ui->configurationSelect->currentIndex();

	if (index == -1) return;

	if (index == ui->configurationSelect->count() - 1)
	{
		ui->configurationSelect->setItemText(index, tr("New configuration %1").arg(++ configCount));
		editCompiler->addConfiguration(ui->configurationSelect->currentText(), "", "");
		ui->compilerArguments->clear();
		ui->interpreterArguments->clear();
		ui->configurationSelect->addItem(tr("Add new ..."));
		ui->configurationSelect->lineEdit()->setSelection(0, ui->configurationSelect->currentText().length());
	}
	else
	{
		ui->configurationSelect->lineEdit()->setText(ui->configurationSelect->itemText(index));
		ui->compilerArguments->setText(editCompiler->getCompilerArguments().at(index));
		ui->interpreterArguments->setText(editCompiler->getInterpreterArguments().at(index));
	}

	ui->deleteConfigurationButton->setEnabled(index > 0);
}

void AdvancedCompilerSettingsDialog::configurationTextChanged()
{
	if (! ui->configurationSelect->isEnabled()) return;

	if (ui->configurationSelect->currentIndex() == 0)
	{
		if (ui->configurationSelect->lineEdit()->text() != "default")
		{
			ui->configurationSelect->lineEdit()->setText("default");
		}
	}
	else
	{
		ui->configurationSelect->setItemText(ui->configurationSelect->currentIndex(),
		                                     ui->configurationSelect->lineEdit()->text());
		editCompiler->setConfigName(ui->configurationSelect->currentIndex(),
		                            ui->configurationSelect->lineEdit()->text());
	}
}

void AdvancedCompilerSettingsDialog::deleteConfiguration()
{
	int index = ui->configurationSelect->currentIndex();

	if (index + 1 < ui->configurationSelect->count() - 1)
	{
		ui->configurationSelect->setCurrentIndex(index + 1);
	}
	else
	{
		ui->configurationSelect->setCurrentIndex(index - 1);
	}

	editCompiler->deleteConfiguration(index);
	ui->configurationSelect->removeItem(index);
}

void AdvancedCompilerSettingsDialog::compilerArgumentsChanged()
{
	if (! ui->configurationSelect->isEnabled()) return;

	int index = ui->configurationSelect->currentIndex();
	editCompiler->setCompilerArguments(index, ui->compilerArguments->text());
}

void AdvancedCompilerSettingsDialog::interpreterArgumentsChanged()
{
	if (! ui->configurationSelect->isEnabled()) return;

	int index = ui->configurationSelect->currentIndex();
	editCompiler->setInterpreterArguments(index, ui->interpreterArguments->text());
}

void AdvancedCompilerSettingsDialog::environmentVariablesButtonClicked()
{
	auto *dialog = new EnvironmentVariablesDialog(this);
	dialog->setProcessEnvironment(editCompiler->getEnvironment());

	if (dialog->exec() == QDialog::Accepted)
	{
		editCompiler->setEnvironment(dialog->getProcessEnvironment());
	}

	delete dialog;
}
