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
 * compilersettings.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "compilersettings.h"
#include "ui_compilersettings.h"
#include "advancedcompilersettingsdialog.h"
#include "addcompilerwizard.h"
#include "settings.h"
#include "compiler.h"
#include <QMessageBox>
#include <QAction>

CompilerSettings::CompilerSettings(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::CompilerSettings)
{
	ui->setupUi(this);

	ui->sourceExtensions->setValidator(new QRegExpValidator(QRegExp("(\\w+;)*\\w+"), this));
	deleteCompilerKeyAction = new QAction(ui->compilerList);
	deleteCompilerKeyAction->setShortcutContext(Qt::WidgetShortcut);
	deleteCompilerKeyAction->setShortcut(QKeySequence::Delete);
	deleteCompilerKeyAction->setEnabled(false);
	ui->compilerList->addAction(deleteCompilerKeyAction);

	connect(ui->moveUpButton, SIGNAL(clicked()),
			  this, SLOT(moveUpCompiler()));
	connect(ui->moveDownButton, SIGNAL(clicked()),
			  this, SLOT(moveDownCompiler()));
	connect(ui->addCompilerButton, SIGNAL(clicked()),
			  this, SLOT(addCompiler()));
	connect(ui->deleteCompilerButton, SIGNAL(clicked()),
			  this, SLOT(deleteCompiler()));
	connect(ui->compilerName, SIGNAL(textChanged(QString)),
			  this, SLOT(compilerNameChanged(QString)));
	connect(ui->sourceExtensions, SIGNAL(textChanged(QString)),
			  this, SLOT(sourceExtensionsChanged(QString)));
	connect(ui->compilerList, SIGNAL(currentRowChanged(int)),
			  this, SLOT(compilerListCurrentRowChanged()));
	connect(ui->advancedButton, SIGNAL(clicked()),
			  this, SLOT(advancedButtonClicked()));
	connect(deleteCompilerKeyAction, SIGNAL(triggered()),
			  this, SLOT(deleteCompiler()));
}

CompilerSettings::~CompilerSettings()
{
	delete ui;
}

void CompilerSettings::resetEditSettings(Settings *settings)
{
	editSettings = settings;

	const QList<Compiler *> &compilerList = editSettings->getCompilerList();
	ui->compilerList->clear();

	for(int i = 0; i < compilerList.size(); i ++)
	{
		ui->compilerList->addItem(compilerList[i]->getCompilerName());
	}

	if(compilerList.size() > 0)
	{
		ui->compilerList->setCurrentRow(0);
		setCurrentCompiler(compilerList[0]);
	}
	else
	{
		setCurrentCompiler(0);
	}

	refreshItemState();
}

bool CompilerSettings::checkValid()
{
	const QList<Compiler *> &compilerList = editSettings->getCompilerList();
	QStringList compilerNames;

	for(int i = 0; i < compilerList.size(); i ++)
	{
		compilerNames.append(compilerList[i]->getCompilerName());
	}

	for(int i = 0; i < compilerList.size(); i ++)
	{
		if(compilerNames.count(compilerNames[i]) > 1)
		{
			ui->compilerList->setFocus();
			ui->compilerList->setCurrentRow(i);
			QMessageBox::warning(this, tr("Error"),
										tr("Compiler %1 appears more than once!").arg(compilerNames[i]),
										QMessageBox::Close);
			return false;
		}
	}

	for(int i = 0; i < compilerList.size(); i ++)
	{
		if(compilerList[i]->getCompilerName().isEmpty())
		{
			ui->compilerList->setCurrentRow(i);
			ui->compilerName->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty compiler name!"), QMessageBox::Close);
			return false;
		}

		if(compilerList[i]->getSourceExtensions().isEmpty())
		{
			ui->compilerList->setCurrentRow(i);
			ui->sourceExtensions->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty source file extensions!"), QMessageBox::Close);
			return false;
		}
	}

	return true;
}

void CompilerSettings::moveUpCompiler()
{
	int index = ui->compilerList->currentRow();
	QString curCompilerName = ui->compilerList->currentItem()->text();
	QString upCompilerName = ui->compilerList->item(index - 1)->text();
	ui->compilerList->currentItem()->setText(upCompilerName);
	ui->compilerList->item(index - 1)->setText(curCompilerName);
	editSettings->swapCompiler(index - 1, index);
	ui->compilerList->setCurrentRow(index - 1);
}

void CompilerSettings::moveDownCompiler()
{
	int index = ui->compilerList->currentRow();
	QString curCompilerName = ui->compilerList->currentItem()->text();
	QString downCompilerName = ui->compilerList->item(index + 1)->text();
	ui->compilerList->currentItem()->setText(downCompilerName);
	ui->compilerList->item(index + 1)->setText(curCompilerName);
	editSettings->swapCompiler(index + 1, index);
	ui->compilerList->setCurrentRow(index + 1);
}

void CompilerSettings::addCompiler()
{
	AddCompilerWizard *wizard = new AddCompilerWizard(this);

	if(wizard->exec() == QDialog::Accepted)
	{
		QList<Compiler *> compilerList = editSettings->getCompilerList();
		QStringList compilerNames;

		for(int i = 0; i < compilerList.size(); i ++)
		{
			compilerNames.append(compilerList[i]->getCompilerName());
		}

		compilerList = wizard->getCompilerList();

		for(int i = 0; i < compilerList.size(); i ++)
		{
			if(compilerNames.contains(compilerList[i]->getCompilerName()))
			{
				int cnt = 2;
				QString name = compilerList[i]->getCompilerName();

				while(compilerNames.contains(QString("%1 (%2)").arg(name).arg(cnt)))
				{
					cnt ++;
				}

				compilerList[i]->setCompilerName(QString("%1 (%2)").arg(name).arg(cnt));
			}

			editSettings->addCompiler(compilerList[i]);
			ui->compilerList->addItem(new QListWidgetItem(compilerList[i]->getCompilerName()));
			ui->compilerList->setCurrentRow(ui->compilerList->count() - 1);
			refreshItemState();
		}
	}

	delete wizard;
}

void CompilerSettings::deleteCompiler()
{
	if(QMessageBox::question(this, tr("LemonPt"), tr("Are you sure to delete compiler %1?")
									 .arg(curCompiler->getCompilerName()),
									 QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel)
	{
		return;
	}

	int index = ui->compilerList->currentRow();
	delete ui->compilerList->item(index);
	editSettings->deleteCompiler(index);
	refreshItemState();
}

void CompilerSettings::setCurrentCompiler(Compiler *compiler)
{
	curCompiler = compiler;

	if(! compiler)
	{
		ui->compilerName->clear();
		ui->sourceExtensions->clear();
		return;
	}

	ui->compilerName->setText(curCompiler->getCompilerName());
	ui->sourceExtensions->setText(curCompiler->getSourceExtensions().join(";"));
}

void CompilerSettings::refreshItemState()
{
	if(ui->compilerList->count() == 0)
	{
		ui->moveUpButton->setEnabled(false);
		ui->moveDownButton->setEnabled(false);
		ui->addCompilerButton->setEnabled(true);
		ui->deleteCompilerButton->setEnabled(false);
		ui->compilerName->setEnabled(false);
		ui->sourceExtensions->setEnabled(false);
		ui->compilerNameLabel->setEnabled(false);
		ui->sourceExtensionsLabel->setEnabled(false);
		ui->advancedButton->setEnabled(false);
		deleteCompilerKeyAction->setEnabled(false);
	}
	else
	{
		if(ui->compilerList->currentRow() > 0)
		{
			ui->moveUpButton->setEnabled(true);
		}
		else
		{
			ui->moveUpButton->setEnabled(false);
		}

		if(ui->compilerList->currentRow() + 1 < ui->compilerList->count())
		{
			ui->moveDownButton->setEnabled(true);
		}
		else
		{
			ui->moveDownButton->setEnabled(false);
		}

		ui->addCompilerButton->setEnabled(true);
		ui->deleteCompilerButton->setEnabled(true);
		ui->compilerName->setEnabled(true);
		ui->sourceExtensions->setEnabled(true);
		ui->compilerNameLabel->setEnabled(true);
		ui->sourceExtensionsLabel->setEnabled(true);
		ui->advancedButton->setEnabled(true);
		deleteCompilerKeyAction->setEnabled(true);
	}
}

void CompilerSettings::compilerNameChanged(const QString &text)
{
	if(curCompiler)
	{
		curCompiler->setCompilerName(text);
		ui->compilerList->currentItem()->setText(text);
	}
}

void CompilerSettings::sourceExtensionsChanged(const QString &text)
{
	if(curCompiler) curCompiler->setSourceExtensions(text);
}

void CompilerSettings::compilerListCurrentRowChanged()
{
	if(ui->compilerList->currentItem())
	{
		int index = ui->compilerList->currentRow();
		setCurrentCompiler(editSettings->getCompiler(index));
	}
	else
	{
		setCurrentCompiler(0);
	}

	refreshItemState();
}

void CompilerSettings::advancedButtonClicked()
{
	AdvancedCompilerSettingsDialog *dialog = new AdvancedCompilerSettingsDialog(this);
	dialog->resetEditCompiler(curCompiler);

	if(dialog->exec() == QDialog::Accepted)
	{
		curCompiler->copyFrom(dialog->getEditCompiler());
	}

	delete dialog;
}
