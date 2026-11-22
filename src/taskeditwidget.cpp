/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "taskeditwidget.h"
#include "ui_taskeditwidget.h"
//
#include "base/compiler.h"
#include "base/settings.h"
#include "core/task.h"
#include <array>
#include <initializer_list>
#include <utility>

namespace {
	auto taskTypeButtons(const Ui::TaskEditWidget &ui) {
		return std::array{std::pair{ui.traditionalButton, Task::Traditional},
		                  std::pair{ui.answersOnlyButton, Task::AnswersOnly},
		                  std::pair{ui.interactionButton, Task::Interaction},
		                  std::pair{ui.communicationButton, Task::Communication},
		                  std::pair{ui.communicationExecButton, Task::CommunicationExec}};
	}
} // namespace

TaskEditWidget::TaskEditWidget(QWidget *parent) : QWidget(parent), ui(new Ui::TaskEditWidget) {
	ui->setupUi(this);
	editTask = nullptr;
	ui->lemonSpecialJudge->setFilters(QDir::Files | QDir::Executable);
	ui->testlibSpecialJudge->setFilters(QDir::Files | QDir::Executable);
	ui->interactorPath->setFilters(QDir::Files);
	ui->graderPath->setFilters(QDir::Files);
	connect(this, &TaskEditWidget::dataPathChanged, ui->lemonSpecialJudge, &FileLineEdit::refreshFileList);
	connect(this, &TaskEditWidget::dataPathChanged, ui->testlibSpecialJudge, &FileLineEdit::refreshFileList);
	connect(this, &TaskEditWidget::dataPathChanged, ui->interactorPath, &FileLineEdit::refreshFileList);
	connect(this, &TaskEditWidget::dataPathChanged, ui->graderPath, &FileLineEdit::refreshFileList);
	ui->sourceFileName->setValidator(new QRegularExpressionValidator(QRegularExpression("\\w+"), this));
	ui->inputFileName->setValidator(
	    new QRegularExpressionValidator(QRegularExpression(R"((\w+)(\.\w+)?)"), this));
	ui->outputFileName->setValidator(
	    new QRegularExpressionValidator(QRegularExpression(R"((\w+)(\.\w+)?)"), this));
	ui->interactorName->setValidator(
	    new QRegularExpressionValidator(QRegularExpression(R"((\w+)(\.\w+)?)"), this));
	ui->answerFileExtension->setValidator(new QRegularExpressionValidator(QRegularExpression("\\w+"), this));
	ui->sourceFilesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->graderFilesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	// ui->interactionButton->setVisible(false); //rebuilding interaction, remove it temporarily
	for (const auto &[button, type] : taskTypeButtons(*ui)) {
		connect(button, &QRadioButton::toggled, this, [this, type](bool checked) {
			if (checked && editTask) {
				// Changing the task type preserves the standard I/O settings.
				editTask->setTaskType(type);
				refreshWidgetState();
			}
		});
	}

	const auto bindText = [this](QLineEdit *field, auto setter) {
		connect(field, &QLineEdit::textChanged, this, [this, setter](const QString &text) {
			if (editTask)
				(editTask->*setter)(text);
		});
	};
	bindText(ui->problemTitle, &Task::setProblemTitle);
	bindText(ui->inputFileName, &Task::setInputFileName);
	bindText(ui->outputFileName, &Task::setOutputFileName);
	bindText(ui->diffArguments, &Task::setDiffArguments);
	bindText(ui->lemonSpecialJudge, &Task::setSpecialJudge);
	bindText(ui->testlibSpecialJudge, &Task::setSpecialJudge);
	bindText(ui->interactorPath, &Task::setInteractor);
	bindText(ui->interactorName, &Task::setInteractorName);
	bindText(ui->graderPath, &Task::setGrader);
	bindText(ui->answerFileExtension, &Task::setAnswerFileExtension);

	connect(ui->sourceFileName, &QLineEdit::textChanged, this, &TaskEditWidget::sourceFileNameChanged);
	connect(ui->subFolderCheck, &QCheckBox::checkStateChanged, this, &TaskEditWidget::subFolderCheckChanged);
	connect(ui->standardInputCheck, &QCheckBox::checkStateChanged, this,
	        &TaskEditWidget::standardInputCheckChanged);
	connect(ui->standardOutputCheck, &QCheckBox::checkStateChanged, this,
	        &TaskEditWidget::standardOutputCheckChanged);
	connect(ui->comparisonMode, qOverload<int>(&QComboBox::currentIndexChanged), this,
	        &TaskEditWidget::comparisonModeChanged);
	connect(ui->realPrecision, qOverload<int>(&QSpinBox::valueChanged), this,
	        &TaskEditWidget::realPrecisionChanged);
	connect(ui->compilersList, &QListWidget::currentRowChanged, this,
	        &TaskEditWidget::compilerSelectionChanged);
	connect(ui->configurationSelect, qOverload<int>(&QComboBox::currentIndexChanged), this,
	        &TaskEditWidget::configurationSelectionChanged);
	connect(ui->sourceFilesAppendButton, &QPushButton::clicked, this, &TaskEditWidget::addSourceFileClicked);
	connect(ui->graderFilesAppendButton, &QPushButton::clicked, this, &TaskEditWidget::addGraderFileClicked);
	connect(ui->sourceFilesRemoveButton, &QPushButton::clicked, this, &TaskEditWidget::rmSourceFileClicked);
	connect(ui->graderFilesRemoveButton, &QPushButton::clicked, this, &TaskEditWidget::rmGraderFileClicked);
}

TaskEditWidget::~TaskEditWidget() { delete ui; }

void TaskEditWidget::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Task *bak = editTask;
		setEditTask(nullptr);
		ui->retranslateUi(this);
		setEditTask(bak);
	}
}

void TaskEditWidget::setEditTask(Task *task) {
	if (editTask) {
		disconnect(editTask, &Task::problemTitleChanged, this, &TaskEditWidget::refreshProblemTitle);
		disconnect(editTask, &Task::compilerConfigurationRefreshed, this,
		           &TaskEditWidget::refreshCompilerConfiguration);
	}

	editTask = task;

	if (! task)
		return;

	connect(editTask, &Task::problemTitleChanged, this, &TaskEditWidget::refreshProblemTitle);
	connect(editTask, &Task::compilerConfigurationRefreshed, this,
	        &TaskEditWidget::refreshCompilerConfiguration);
	ui->problemTitle->setText(editTask->getProblemTitle());
	ui->sourceFileName->setEnabled(false);
	ui->sourceFileName->setText(editTask->getSourceFileName());

	if (ui->sourceFileName->text().length() <= 0)
		ui->sourceFileName->setText(ui->problemTitle->text());

	ui->sourceFileName->setEnabled(true);
	ui->subFolderCheck->setChecked(editTask->getSubFolderCheck());
	ui->inputFileName->setText(editTask->getInputFileName());
	ui->outputFileName->setText(editTask->getOutputFileName());
	ui->comparisonMode->setCurrentIndex(int(editTask->getComparisonMode()));
	ui->diffArguments->setText(editTask->getDiffArguments());
	ui->realPrecision->setValue(editTask->getRealPrecision());
	ui->lemonSpecialJudge->setText(editTask->getSpecialJudge());
	ui->testlibSpecialJudge->setText(editTask->getSpecialJudge());
	ui->interactorPath->setText(editTask->getInteractor());
	ui->interactorName->setText(editTask->getInteractorName());
	ui->graderPath->setText(editTask->getGrader());
	ui->standardInputCheck->setChecked(editTask->getStandardInputCheck());
	ui->standardOutputCheck->setChecked(editTask->getStandardOutputCheck());
	// ui->interactorPathLabel->setVisible(editTask->getTaskType() == Task::Interaction);
	// ui->interactorPath->setVisible(editTask->getTaskType() == Task::Interaction);
	// ui->graderPathLabel->setVisible(editTask->getTaskType() == Task::Interaction);
	// ui->graderPath->setVisible(editTask->getTaskType() == Task::Interaction);
	ui->answerFileExtension->setText(editTask->getAnswerFileExtension());
	refreshCompilerConfiguration();

	for (const auto &[button, type] : taskTypeButtons(*ui)) {
		if (editTask->getTaskType() == type)
			button->setChecked(true);
	}

	refreshWidgetState();
}

void TaskEditWidget::setSettings(Settings *_settings) { settings = _settings; }

void TaskEditWidget::refreshWidgetState() {
	if (! editTask)
		return;

	const auto type = editTask->getTaskType();
	const bool communication = type == Task::Communication || type == Task::CommunicationExec;
	const bool hasProgram = type == Task::Traditional || type == Task::Interaction || communication;
	const bool hasSourceName = hasProgram || type == Task::AnswersOnly;
	const auto setVisible = [](bool visible, std::initializer_list<QWidget *> widgets) {
		for (auto *widget : widgets)
			widget->setVisible(visible);
	};

	setVisible(type == Task::Interaction, {ui->interactorPathLabel, ui->interactorPath, ui->graderPathLabel,
	                                       ui->graderPath, ui->interactorNameLabel, ui->interactorName});
	ui->sourceFileName->setEnabled(hasSourceName);
	ui->sourceFileNameLabel->setEnabled(hasSourceName);
	setVisible(hasSourceName, {ui->sourceFileName, ui->sourceFileNameLabel, ui->subFolderCheck});
	ui->inputFileName->setEnabled(hasProgram && ! editTask->getStandardInputCheck());
	setVisible(hasProgram, {ui->inputFileName, ui->inputFileNameLabel, ui->standardInputCheck});
	ui->outputFileName->setEnabled(hasProgram && ! editTask->getStandardOutputCheck());
	setVisible(hasProgram, {ui->outputFileName, ui->outputFileNameLabel, ui->standardOutputCheck,
	                        ui->compilerSettingsLabel, ui->compilersList, ui->configurationLabel,
	                        ui->configurationSelect});
	// The comparison mode remains available for all task types.
	setVisible(type == Task::AnswersOnly, {ui->answerFileExtension, ui->answerFileExtensionLabel});
	ui->comparisonSetting->setCurrentIndex(ui->comparisonMode->currentIndex());
	setVisible(communication,
	           {ui->sourceFilesLabel, ui->sourceFilesTable, ui->graderFilesLabel, ui->graderFilesTable,
	            ui->sourceFilesAppendButton, ui->graderFilesAppendButton, ui->sourceFilesRemoveButton,
	            ui->graderFilesRemoveButton, ui->multiFilesPathLineEdit, ui->multiFilesNameLineEdit,
	            ui->multiFilesPathNameLabel});
	multiFilesRefresh();
}

void TaskEditWidget::sourceFileNameChanged(const QString &text) {
	if (! editTask)
		return;

	if (! ui->sourceFileName->isEnabled())
		return;

	QString trueText = text;

	if (trueText.length() <= 0)
		trueText = ui->problemTitle->text();

	editTask->setSourceFileName(trueText);

	if (ui->inputFileName->isEnabled()) {
		ui->inputFileName->setText(trueText + "." + settings->getDefaultInputFileExtension());
	}

	if (ui->outputFileName->isEnabled()) {
		ui->outputFileName->setText(trueText + "." + settings->getDefaultOutputFileExtension());
	}
}

void TaskEditWidget::subFolderCheckChanged() {
	if (! editTask)
		return;

	bool check = ui->subFolderCheck->isChecked();
	editTask->setSubFolderCheck(check);
}

void TaskEditWidget::standardInputCheckChanged() {
	if (! editTask)
		return;

	bool check = ui->standardInputCheck->isChecked();
	editTask->setStandardInputCheck(check);
	ui->inputFileName->setEnabled(! check);
}

void TaskEditWidget::standardOutputCheckChanged() {
	if (! editTask)
		return;

	bool check = ui->standardOutputCheck->isChecked();
	editTask->setStandardOutputCheck(check);
	ui->outputFileName->setEnabled(! check);
}

void TaskEditWidget::comparisonModeChanged() {
	if (! editTask)
		return;

	editTask->setComparisonMode(Task::ComparisonMode(ui->comparisonMode->currentIndex()));
}

void TaskEditWidget::realPrecisionChanged(int precision) {
	if (! editTask)
		return;

	editTask->setRealPrecision(precision);
}

void TaskEditWidget::refreshProblemTitle(const QString &title) {
	if (! editTask)
		return;

	ui->problemTitle->setText(title);
}

void TaskEditWidget::refreshCompilerConfiguration() {
	if (! editTask)
		return;

	ui->compilersList->setEnabled(false);
	ui->configurationSelect->setEnabled(false);
	ui->configurationLabel->setEnabled(false);
	ui->compilersList->clear();
	ui->configurationSelect->clear();
	const QList<Compiler *> &compilerList = settings->getCompilerList();

	if (compilerList.isEmpty())
		return;

	for (auto *i : compilerList) {
		ui->compilersList->addItem(i->getCompilerName());
	}

	ui->compilersList->setEnabled(true);
	ui->configurationSelect->setEnabled(true);
	ui->configurationLabel->setEnabled(true);
	ui->compilersList->setCurrentRow(0);
	compilerSelectionChanged();
}

void TaskEditWidget::compilerSelectionChanged() {
	if (! editTask)
		return;

	if (! ui->compilersList->isEnabled())
		return;

	ui->configurationSelect->setEnabled(false);
	ui->configurationSelect->clear();
	ui->configurationSelect->addItem("disable");
	const QList<Compiler *> &compilerList = settings->getCompilerList();

	for (auto *i : compilerList) {
		if (i->getCompilerName() == ui->compilersList->currentItem()->text()) {
			ui->configurationSelect->addItems(i->getConfigurationNames());
		}
	}

	QString config = editTask->getCompilerConfiguration(ui->compilersList->currentItem()->text());
	ui->configurationSelect->setCurrentIndex(ui->configurationSelect->findText(config));
	ui->configurationSelect->setEnabled(true);
}

void TaskEditWidget::configurationSelectionChanged() {
	if (! editTask)
		return;

	if (! ui->configurationSelect->isEnabled())
		return;

	editTask->setCompilerConfiguration(ui->compilersList->currentItem()->text(),
	                                   ui->configurationSelect->currentText());
}

void TaskEditWidget::multiFilesRefresh() {
	if (! editTask)
		return;

	if (editTask->getTaskType() != Task::Communication && editTask->getTaskType() != Task::CommunicationExec)
		return;

	QStringList sourcePaths = editTask->getSourceFilesPath();
	QStringList sourceNames = editTask->getSourceFilesName();
	ui->sourceFilesTable->setRowCount(sourcePaths.length());

	for (int i = 0; i < sourcePaths.length(); i++) {
		ui->sourceFilesTable->setItem(i, 0, new QTableWidgetItem(sourcePaths[i]));
		ui->sourceFilesTable->setItem(i, 1, new QTableWidgetItem(sourceNames[i]));
	}

	QStringList graderPaths = editTask->getGraderFilesPath();
	QStringList graderNames = editTask->getGraderFilesName();
	ui->graderFilesTable->setRowCount(graderPaths.length());

	for (int i = 0; i < graderPaths.length(); i++) {
		ui->graderFilesTable->setItem(i, 0, new QTableWidgetItem(graderPaths[i]));
		ui->graderFilesTable->setItem(i, 1, new QTableWidgetItem(graderNames[i]));
	}
}

void TaskEditWidget::addSourceFiles(const QString &path, const QString &name) {
	if (! editTask)
		return;

	editTask->appendSourceFiles(path, name);
}

void TaskEditWidget::rmSourceFilesAt(int loca) {
	if (! editTask)
		return;

	editTask->removeSourceFilesAt(loca);
}

void TaskEditWidget::rmGraderFilesAt(int loca) {
	if (! editTask)
		return;

	editTask->removeGraderFilesAt(loca);
}

void TaskEditWidget::addGraderFiles(const QString &path, const QString &name) {
	if (! editTask)
		return;

	editTask->appendGraderFiles(path, name);
}

void TaskEditWidget::addSourceFileClicked() {
	if (! editTask)
		return;

	QString path = ui->multiFilesPathLineEdit->text();
	QString name = ui->multiFilesNameLineEdit->text();

	if (path.length() <= 0 || name.length() <= 0)
		return;

	addSourceFiles(path, name);
	ui->multiFilesPathLineEdit->clear();
	ui->multiFilesNameLineEdit->clear();
	multiFilesRefresh();
}

void TaskEditWidget::addGraderFileClicked() {
	if (! editTask)
		return;

	QString path = ui->multiFilesPathLineEdit->text();
	QString name = ui->multiFilesNameLineEdit->text();

	if (path.length() <= 0 || name.length() <= 0)
		return;

	addGraderFiles(path, name);
	ui->multiFilesPathLineEdit->clear();
	ui->multiFilesNameLineEdit->clear();
	multiFilesRefresh();
}

void TaskEditWidget::rmSourceFileClicked() {
	if (! editTask)
		return;

	QList<QTableWidgetSelectionRange> ranges = ui->sourceFilesTable->selectedRanges();

	if (ranges.length() <= 0)
		return;

	rmSourceFilesAt(ranges.at(0).topRow());
	multiFilesRefresh();
}

void TaskEditWidget::rmGraderFileClicked() {
	if (! editTask)
		return;

	QList<QTableWidgetSelectionRange> ranges = ui->graderFilesTable->selectedRanges();

	if (ranges.length() <= 0)
		return;

	rmGraderFilesAt(ranges.at(0).topRow());
	multiFilesRefresh();
}
