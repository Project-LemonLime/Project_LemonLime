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
 * environmentvariablesdialog.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "environmentvariablesdialog.h"
#include "editvariabledialog.h"
#include "ui_environmentvariablesdialog.h"
#include <QMessageBox>

EnvironmentVariablesDialog::EnvironmentVariablesDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::EnvironmentVariablesDialog)
{
	ui->setupUi(this);
	connect(ui->addButton, SIGNAL(clicked()), this, SLOT(addButtonClicked()));
	connect(ui->editButton, SIGNAL(clicked()), this, SLOT(editButtonClicked()));
	connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(deleteButtonClicked()));
	connect(ui->valueViewer, SIGNAL(itemSelectionChanged()), this, SLOT(viewerSelectionChanged()));
}

EnvironmentVariablesDialog::~EnvironmentVariablesDialog() { delete ui; }

void EnvironmentVariablesDialog::setProcessEnvironment(const QProcessEnvironment &environment)
{
	QStringList values = environment.toStringList();
	ui->valueViewer->setRowCount(values.size());

	for (int i = 0; i < values.size(); i++)
	{
		int tmp = values[i].indexOf('=');
		QString variable = values[i].mid(0, tmp);
		QString value = values[i].mid(tmp + 1);
		ui->valueViewer->setItem(i, 0, new QTableWidgetItem(variable));
		ui->valueViewer->setItem(i, 1, new QTableWidgetItem(value));
	}
}

auto EnvironmentVariablesDialog::getProcessEnvironment() const -> QProcessEnvironment
{
	QProcessEnvironment environment;

	for (int i = 0; i < ui->valueViewer->rowCount(); i++)
	{
		QString variable = ui->valueViewer->item(i, 0)->text();
		QString value = ui->valueViewer->item(i, 1)->text();
		environment.insert(variable, value);
	}

	return environment;
}

void EnvironmentVariablesDialog::addButtonClicked()
{
	auto *dialog = new EditVariableDialog(this);
	dialog->setWindowTitle(tr("Add New Variable"));

	if (dialog->exec() == QDialog::Accepted)
	{
		ui->valueViewer->setRowCount(ui->valueViewer->rowCount() + 1);
		ui->valueViewer->setItem(ui->valueViewer->rowCount() - 1, 0,
		                         new QTableWidgetItem(dialog->getVariableName()));
		ui->valueViewer->setItem(ui->valueViewer->rowCount() - 1, 1,
		                         new QTableWidgetItem(dialog->getVariableValue()));
	}

	delete dialog;
}

void EnvironmentVariablesDialog::editButtonClicked()
{
	int index = ui->valueViewer->currentRow();
	auto *dialog = new EditVariableDialog(this);
	dialog->setWindowTitle(tr("Edit Variable"));
	dialog->setVariableName(ui->valueViewer->item(index, 0)->text());
	dialog->setVariableValue(ui->valueViewer->item(index, 1)->text());

	if (dialog->exec() == QDialog::Accepted)
	{
		ui->valueViewer->setItem(index, 0, new QTableWidgetItem(dialog->getVariableName()));
		ui->valueViewer->setItem(index, 1, new QTableWidgetItem(dialog->getVariableValue()));
	}

	delete dialog;
}

void EnvironmentVariablesDialog::deleteButtonClicked()
{
	int index = ui->valueViewer->currentRow();
	QString variable = ui->valueViewer->item(index, 0)->text();

	if (QMessageBox::question(this, tr("LemonLime"), tr("Are you sure to delete variable %1?").arg(variable),
	                          QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok)
	{
		return;
	}

	for (int i = index + 1; i < ui->valueViewer->rowCount(); i++)
	{
		ui->valueViewer->setItem(i - 1, 0, new QTableWidgetItem(ui->valueViewer->item(i, 0)->text()));
		ui->valueViewer->setItem(i - 1, 1, new QTableWidgetItem(ui->valueViewer->item(i, 1)->text()));
	}

	ui->valueViewer->setRowCount(ui->valueViewer->rowCount() - 1);
}

void EnvironmentVariablesDialog::viewerSelectionChanged()
{
	QList<QTableWidgetSelectionRange> range = ui->valueViewer->selectedRanges();

	if (! range.empty())
	{
		ui->editButton->setEnabled(true);
		ui->deleteButton->setEnabled(true);
	}
	else
	{
		ui->editButton->setEnabled(false);
		ui->deleteButton->setEnabled(false);
	}
}
