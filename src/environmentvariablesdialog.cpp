/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "environmentvariablesdialog.h"
#include "ui_environmentvariablesdialog.h"
//
#include "editvariabledialog.h"
//
#include <QMessageBox>

EnvironmentVariablesDialog::EnvironmentVariablesDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::EnvironmentVariablesDialog) {
	ui->setupUi(this);
	connect(ui->addButton, &QPushButton::clicked, this, &EnvironmentVariablesDialog::addButtonClicked);
	connect(ui->editButton, &QPushButton::clicked, this, &EnvironmentVariablesDialog::editButtonClicked);
	connect(ui->deleteButton, &QPushButton::clicked, this, &EnvironmentVariablesDialog::deleteButtonClicked);
	connect(ui->valueViewer, &QTableWidget::itemSelectionChanged, this,
	        &EnvironmentVariablesDialog::viewerSelectionChanged);
}

EnvironmentVariablesDialog::~EnvironmentVariablesDialog() { delete ui; }

void EnvironmentVariablesDialog::setProcessEnvironment(const QProcessEnvironment &environment) {
	QStringList values = environment.toStringList();
	ui->valueViewer->setRowCount(values.size());

	for (int i = 0; i < values.size(); i++) {
		int tmp = values[i].indexOf('=');
		QString variable = values[i].mid(0, tmp);
		QString value = values[i].mid(tmp + 1);
		ui->valueViewer->setItem(i, 0, new QTableWidgetItem(variable));
		ui->valueViewer->setItem(i, 1, new QTableWidgetItem(value));
	}
}

auto EnvironmentVariablesDialog::getProcessEnvironment() const -> QProcessEnvironment {
	QProcessEnvironment environment;

	for (int i = 0; i < ui->valueViewer->rowCount(); i++) {
		QString variable = ui->valueViewer->item(i, 0)->text();
		QString value = ui->valueViewer->item(i, 1)->text();
		environment.insert(variable, value);
	}

	return environment;
}

void EnvironmentVariablesDialog::addButtonClicked() {
	auto *dialog = new EditVariableDialog(this);
	dialog->setWindowTitle(tr("Add New Variable"));

	if (dialog->exec() == QDialog::Accepted) {
		ui->valueViewer->setRowCount(ui->valueViewer->rowCount() + 1);
		ui->valueViewer->setItem(ui->valueViewer->rowCount() - 1, 0,
		                         new QTableWidgetItem(dialog->getVariableName()));
		ui->valueViewer->setItem(ui->valueViewer->rowCount() - 1, 1,
		                         new QTableWidgetItem(dialog->getVariableValue()));
	}

	delete dialog;
}

void EnvironmentVariablesDialog::editButtonClicked() {
	int index = ui->valueViewer->currentRow();
	auto *dialog = new EditVariableDialog(this);
	dialog->setWindowTitle(tr("Edit Variable"));
	dialog->setVariableName(ui->valueViewer->item(index, 0)->text());
	dialog->setVariableValue(ui->valueViewer->item(index, 1)->text());

	if (dialog->exec() == QDialog::Accepted) {
		ui->valueViewer->setItem(index, 0, new QTableWidgetItem(dialog->getVariableName()));
		ui->valueViewer->setItem(index, 1, new QTableWidgetItem(dialog->getVariableValue()));
	}

	delete dialog;
}

void EnvironmentVariablesDialog::deleteButtonClicked() {
	int index = ui->valueViewer->currentRow();
	QString variable = ui->valueViewer->item(index, 0)->text();

	if (QMessageBox::question(this, tr("LemonLime"), tr("Are you sure to delete variable %1?").arg(variable),
	                          QMessageBox::Ok | QMessageBox::Cancel) != QMessageBox::Ok) {
		return;
	}

	for (int i = index + 1; i < ui->valueViewer->rowCount(); i++) {
		ui->valueViewer->setItem(i - 1, 0, new QTableWidgetItem(ui->valueViewer->item(i, 0)->text()));
		ui->valueViewer->setItem(i - 1, 1, new QTableWidgetItem(ui->valueViewer->item(i, 1)->text()));
	}

	ui->valueViewer->setRowCount(ui->valueViewer->rowCount() - 1);
}

void EnvironmentVariablesDialog::viewerSelectionChanged() {
	QList<QTableWidgetSelectionRange> range = ui->valueViewer->selectedRanges();

	if (! range.empty()) {
		ui->editButton->setEnabled(true);
		ui->deleteButton->setEnabled(true);
	} else {
		ui->editButton->setEnabled(false);
		ui->deleteButton->setEnabled(false);
	}
}
