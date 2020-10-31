/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "compilersettings.h"
#include "ui_compilersettings.h"
//
#include "addcompilerwizard.h"
#include "advancedcompilersettingsdialog.h"
#include "core/compiler.h"
#include "core/settings.h"
//
#include <QAction>
#include <QMessageBox>

CompilerSettings::CompilerSettings(QWidget *parent) : QWidget(parent), ui(new Ui::CompilerSettings)
{
	ui->setupUi(this);
	ui->sourceExtensions->setValidator(
	    new QRegularExpressionValidator(QRegularExpression("(\\w+;)*\\w+"), this));
	deleteCompilerKeyAction = new QAction(ui->compilerList);
	deleteCompilerKeyAction->setShortcutContext(Qt::WidgetShortcut);
	deleteCompilerKeyAction->setShortcut(QKeySequence::Delete);
	deleteCompilerKeyAction->setEnabled(false);
	ui->compilerList->addAction(deleteCompilerKeyAction);
	connect(ui->moveUpButton, SIGNAL(clicked()), this, SLOT(moveUpCompiler()));
	connect(ui->moveDownButton, SIGNAL(clicked()), this, SLOT(moveDownCompiler()));
	connect(ui->addCompilerButton, SIGNAL(clicked()), this, SLOT(addCompiler()));
	connect(ui->deleteCompilerButton, SIGNAL(clicked()), this, SLOT(deleteCompiler()));
	connect(ui->compilerName, SIGNAL(textChanged(QString)), this, SLOT(compilerNameChanged(QString)));
	connect(ui->sourceExtensions, SIGNAL(textChanged(QString)), this, SLOT(sourceExtensionsChanged(QString)));
	connect(ui->compilerList, SIGNAL(currentRowChanged(int)), this, SLOT(compilerListCurrentRowChanged()));
	connect(ui->advancedButton, SIGNAL(clicked()), this, SLOT(advancedButtonClicked()));
	connect(deleteCompilerKeyAction, SIGNAL(triggered()), this, SLOT(deleteCompiler()));
}

CompilerSettings::~CompilerSettings() { delete ui; }

void CompilerSettings::resetEditSettings(Settings *settings)
{
	editSettings = settings;
	const QList<Compiler *> &compilerList = editSettings->getCompilerList();
	ui->compilerList->clear();

	for (auto i : compilerList)
	{
		ui->compilerList->addItem(i->getCompilerName());
	}

	if (! compilerList.empty())
	{
		ui->compilerList->setCurrentRow(0);
		setCurrentCompiler(compilerList[0]);
	}
	else
	{
		setCurrentCompiler(nullptr);
	}

	refreshItemState();
}

auto CompilerSettings::checkValid() -> bool
{
	const QList<Compiler *> &compilerList = editSettings->getCompilerList();
	QStringList compilerNames;

	for (auto i : compilerList)
	{
		compilerNames.append(i->getCompilerName());
	}

	for (int i = 0; i < compilerList.size(); i++)
	{
		if (compilerNames.count(compilerNames[i]) > 1)
		{
			ui->compilerList->setFocus();
			ui->compilerList->setCurrentRow(i);
			QMessageBox::warning(this, tr("Error"),
			                     tr("Compiler %1 appears more than once!").arg(compilerNames[i]),
			                     QMessageBox::Close);
			return false;
		}
	}

	for (int i = 0; i < compilerList.size(); i++)
	{
		if (compilerList[i]->getCompilerName().isEmpty())
		{
			ui->compilerList->setCurrentRow(i);
			ui->compilerName->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty compiler name!"), QMessageBox::Close);
			return false;
		}

		if (compilerList[i]->getSourceExtensions().isEmpty())
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
	auto *wizard = new AddCompilerWizard(this);

	if (wizard->exec() == QDialog::Accepted)
	{
		QList<Compiler *> compilerList = editSettings->getCompilerList();
		QStringList compilerNames;

		for (auto &i : compilerList)
		{
			compilerNames.append(i->getCompilerName());
		}

		compilerList = wizard->getCompilerList();

		for (auto &i : compilerList)
		{
			if (compilerNames.contains(i->getCompilerName()))
			{
				int cnt = 2;
				QString name = i->getCompilerName();

				while (compilerNames.contains(QString("%1 (%2)").arg(name).arg(cnt)))
				{
					cnt++;
				}

				i->setCompilerName(QString("%1 (%2)").arg(name).arg(cnt));
			}

			editSettings->addCompiler(i);
			ui->compilerList->addItem(new QListWidgetItem(i->getCompilerName()));
			ui->compilerList->setCurrentRow(ui->compilerList->count() - 1);
			refreshItemState();
		}
	}

	delete wizard;
}

void CompilerSettings::deleteCompiler()
{
	if (QMessageBox::question(this, tr("LemonLime"),
	                          tr("Are you sure to delete compiler %1?").arg(curCompiler->getCompilerName()),
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

	if (! compiler)
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
	if (ui->compilerList->count() == 0)
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
		if (ui->compilerList->currentRow() > 0)
		{
			ui->moveUpButton->setEnabled(true);
		}
		else
		{
			ui->moveUpButton->setEnabled(false);
		}

		if (ui->compilerList->currentRow() + 1 < ui->compilerList->count())
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
	if (curCompiler)
	{
		curCompiler->setCompilerName(text);
		ui->compilerList->currentItem()->setText(text);
	}
}

void CompilerSettings::sourceExtensionsChanged(const QString &text)
{
	if (curCompiler)
		curCompiler->setSourceExtensions(text);
}

void CompilerSettings::compilerListCurrentRowChanged()
{
	if (ui->compilerList->currentItem())
	{
		int index = ui->compilerList->currentRow();
		setCurrentCompiler(editSettings->getCompiler(index));
	}
	else
	{
		setCurrentCompiler(nullptr);
	}

	refreshItemState();
}

void CompilerSettings::advancedButtonClicked()
{
	auto *dialog = new AdvancedCompilerSettingsDialog(this);
	dialog->resetEditCompiler(curCompiler);

	if (dialog->exec() == QDialog::Accepted)
	{
		curCompiler->copyFrom(dialog->getEditCompiler());
	}

	delete dialog;
}
