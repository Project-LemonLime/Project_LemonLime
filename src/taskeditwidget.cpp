/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019-2021 Project LemonLime
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

TaskEditWidget::TaskEditWidget(QWidget *parent) : QWidget(parent), ui(new Ui::TaskEditWidget) {
	ui->setupUi(this);
	editTask = nullptr;
	ui->specialJudge->setFilters(QDir::Files | QDir::Executable);
	ui->interactorPath->setFilters(QDir::Files);
	ui->graderPath->setFilters(QDir::Files);
	connect(this, &TaskEditWidget::dataPathChanged, ui->specialJudge, &FileLineEdit::refreshFileList);
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
	connect(ui->problemTitle, &QLineEdit::textChanged, this, &TaskEditWidget::problemTitleChanged);
	connect(ui->traditionalButton, &QRadioButton::toggled, this, &TaskEditWidget::setToTraditional);
	connect(ui->answersOnlyButton, &QRadioButton::toggled, this, &TaskEditWidget::setToAnswersOnly);
	connect(ui->interactionButton, &QRadioButton::toggled, this, &TaskEditWidget::setToInteraction);
	connect(ui->communicationButton, &QRadioButton::toggled, this, &TaskEditWidget::setToCommunication);
	connect(ui->communicationExecButton, &QRadioButton::toggled, this,
	        &TaskEditWidget::setToCommunicationExec);
	connect(ui->sourceFileName, &QLineEdit::textChanged, this, &TaskEditWidget::sourceFileNameChanged);
	connect(ui->subFolderCheck, &QCheckBox::stateChanged, this, &TaskEditWidget::subFolderCheckChanged);
	connect(ui->inputFileName, &QLineEdit::textChanged, this, &TaskEditWidget::inputFileNameChanged);
	connect(ui->outputFileName, &QLineEdit::textChanged, this, &TaskEditWidget::outputFileNameChanged);
	connect(ui->standardInputCheck, &QCheckBox::stateChanged, this,
	        &TaskEditWidget::standardInputCheckChanged);
	connect(ui->standardOutputCheck, &QCheckBox::stateChanged, this,
	        &TaskEditWidget::standardOutputCheckChanged);
	connect(ui->comparisonMode, qOverload<int>(&QComboBox::currentIndexChanged), this,
	        &TaskEditWidget::comparisonModeChanged);
	connect(ui->diffArguments, &QLineEdit::textChanged, this, &TaskEditWidget::diffArgumentsChanged);
	connect(ui->realPrecision, qOverload<int>(&QSpinBox::valueChanged), this,
	        &TaskEditWidget::realPrecisionChanged);
	connect(ui->specialJudge, &QLineEdit::textChanged, this, &TaskEditWidget::specialJudgeChanged);
	connect(ui->interactorPath, &QLineEdit::textChanged, this, &TaskEditWidget::interactorChanged);
	connect(ui->interactorName, &QLineEdit::textChanged, this, &TaskEditWidget::interactorNameChanged);
	connect(ui->graderPath, &QLineEdit::textChanged, this, &TaskEditWidget::graderChanged);
	connect(ui->compilersList, &QListWidget::currentRowChanged, this,
	        &TaskEditWidget::compilerSelectionChanged);
	connect(ui->configurationSelect, qOverload<int>(&QComboBox::currentIndexChanged), this,
	        &TaskEditWidget::configurationSelectionChanged);
	connect(ui->answerFileExtension, &QLineEdit::textChanged, this,
	        &TaskEditWidget::answerFileExtensionChanged);
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
		disconnect(editTask, SIGNAL(problemTitleChanged(QString)), this, SLOT(refreshProblemTitle(QString)));
		disconnect(editTask, SIGNAL(compilerConfigurationRefreshed()), this,
		           SLOT(refreshCompilerConfiguration()));
	}

	editTask = task;

	if (! task)
		return;

	connect(editTask, SIGNAL(problemTitleChanged(QString)), this, SLOT(refreshProblemTitle(QString)));
	connect(editTask, SIGNAL(compilerConfigurationRefreshed()), this, SLOT(refreshCompilerConfiguration()));
	ui->problemTitle->setText(editTask->getProblemTile());
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
	ui->specialJudge->setText(editTask->getSpecialJudge());
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

	if (editTask->getTaskType() == Task::Traditional) {
		ui->traditionalButton->setChecked(true);
	}

	if (editTask->getTaskType() == Task::AnswersOnly) {
		ui->answersOnlyButton->setChecked(true);
	}

	if (editTask->getTaskType() == Task::Interaction) {
		ui->interactionButton->setChecked(true);
	}

	if (editTask->getTaskType() == Task::Communication) {
		ui->communicationButton->setChecked(true);
	}

	if (editTask->getTaskType() == Task::CommunicationExec) {
		ui->communicationExecButton->setChecked(true);
	}

	refreshWidgetState();
}

void TaskEditWidget::setSettings(Settings *_settings) { settings = _settings; }

void TaskEditWidget::refreshWidgetState() {
	if (! editTask)
		return;

	int types = editTask->getTaskType();
	ui->interactorPathLabel->setVisible(types == Task::Interaction);
	ui->interactorPath->setVisible(types == Task::Interaction);
	ui->graderPathLabel->setVisible(types == Task::Interaction);
	ui->graderPath->setVisible(types == Task::Interaction);
	ui->interactorNameLabel->setVisible(types == Task::Interaction);
	ui->interactorName->setVisible(types == Task::Interaction);
	// ui->comparisonSetting->setVisible(types != Task::Interaction);
	ui->sourceFileName->setEnabled(types == Task::Traditional || types == Task::Interaction ||
	                               types == Task::AnswersOnly || types == Task::Communication ||
	                               types == Task::CommunicationExec);
	ui->sourceFileNameLabel->setEnabled(types == Task::Traditional || types == Task::Interaction ||
	                                    types == Task::AnswersOnly || types == Task::Communication ||
	                                    types == Task::CommunicationExec);
	ui->sourceFileName->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                               types == Task::AnswersOnly || types == Task::Communication ||
	                               types == Task::CommunicationExec);
	ui->sourceFileNameLabel->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                                    types == Task::AnswersOnly || types == Task::Communication ||
	                                    types == Task::CommunicationExec);
	ui->subFolderCheck->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                               types == Task::AnswersOnly || types == Task::Communication ||
	                               types == Task::CommunicationExec);
	ui->inputFileName->setEnabled((types == Task::Traditional || types == Task::Interaction ||
	                               types == Task::Communication || types == Task::CommunicationExec) &&
	                              ! editTask->getStandardInputCheck());
	ui->inputFileName->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                              types == Task::Communication || types == Task::CommunicationExec);
	ui->inputFileNameLabel->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                                   types == Task::Communication || types == Task::CommunicationExec);
	ui->standardInputCheck->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                                   types == Task::Communication || types == Task::CommunicationExec);
	ui->outputFileName->setEnabled((types == Task::Traditional || types == Task::Interaction ||
	                                types == Task::Communication || types == Task::CommunicationExec) &&
	                               ! editTask->getStandardOutputCheck());
	ui->outputFileName->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                               types == Task::Communication || types == Task::CommunicationExec);
	ui->outputFileNameLabel->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                                    types == Task::Communication || types == Task::CommunicationExec);
	ui->standardOutputCheck->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                                    types == Task::Communication || types == Task::CommunicationExec);
	ui->compilerSettingsLabel->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                                      types == Task::Communication || types == Task::CommunicationExec);
	ui->compilersList->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                              types == Task::Communication || types == Task::CommunicationExec);
	ui->configurationLabel->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                                   types == Task::Communication || types == Task::CommunicationExec);
	ui->configurationSelect->setVisible(types == Task::Traditional || types == Task::Interaction ||
	                                    types == Task::Communication || types == Task::CommunicationExec);
	// ui->comparisonMode->setEnabled(types == Task::Traditional || types == Task::AnswersOnly);
	ui->answerFileExtension->setVisible(types == Task::AnswersOnly);
	ui->answerFileExtensionLabel->setVisible(types == Task::AnswersOnly);
	ui->comparisonSetting->setCurrentIndex(ui->comparisonMode->currentIndex());
	ui->sourceFilesLabel->setVisible(types == Task::Communication || types == Task::CommunicationExec);
	ui->sourceFilesTable->setVisible(types == Task::Communication || types == Task::CommunicationExec);
	ui->graderFilesLabel->setVisible(types == Task::Communication || types == Task::CommunicationExec);
	ui->graderFilesTable->setVisible(types == Task::Communication || types == Task::CommunicationExec);
	ui->sourceFilesAppendButton->setVisible(types == Task::Communication || types == Task::CommunicationExec);
	ui->graderFilesAppendButton->setVisible(types == Task::Communication || types == Task::CommunicationExec);
	ui->sourceFilesRemoveButton->setVisible(types == Task::Communication || types == Task::CommunicationExec);
	ui->graderFilesRemoveButton->setVisible(types == Task::Communication || types == Task::CommunicationExec);
	ui->multiFilesPathLineEdit->setVisible(types == Task::Communication || types == Task::CommunicationExec);
	ui->multiFilesNameLineEdit->setVisible(types == Task::Communication || types == Task::CommunicationExec);
	ui->multiFilesPathNameLabel->setVisible(types == Task::Communication || types == Task::CommunicationExec);
	multiFilesRefresh();
}

void TaskEditWidget::problemTitleChanged(const QString &text) {
	if (! editTask)
		return;

	editTask->setProblemTitle(text);
}

void TaskEditWidget::setToTraditional(bool check) {
	if (! check || ! editTask)
		return;

	editTask->setTaskType(Task::Traditional);
	// editTask->setStandardOutputCheck(false); //fix stdout not save
	// ui->standardOutputCheck->setCheckState(Qt::Unchecked);
	refreshWidgetState();
}

void TaskEditWidget::setToAnswersOnly(bool check) {
	if (! check || ! editTask)
		return;

	editTask->setTaskType(Task::AnswersOnly);
	// editTask->setStandardOutputCheck(false);
	// ui->standardOutputCheck->setCheckState(Qt::Unchecked);
	refreshWidgetState();
}

void TaskEditWidget::setToInteraction(bool check) {
	if (! check || ! editTask)
		return;

	editTask->setTaskType(Task::Interaction);
	// editTask->setStandardOutputCheck(true);
	// ui->standardOutputCheck->setCheckState(Qt::Checked);
	refreshWidgetState();
}

void TaskEditWidget::setToCommunication(bool check) {
	if (! check || ! editTask)
		return;

	editTask->setTaskType(Task::Communication);
	refreshWidgetState();
}

void TaskEditWidget::setToCommunicationExec(bool check) {
	if (! check || ! editTask)
		return;

	editTask->setTaskType(Task::CommunicationExec);
	refreshWidgetState();
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

void TaskEditWidget::inputFileNameChanged(const QString &text) {
	if (! editTask)
		return;

	editTask->setInputFileName(text);
}

void TaskEditWidget::outputFileNameChanged(const QString &text) {
	if (! editTask)
		return;

	editTask->setOutputFileName(text);
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

void TaskEditWidget::diffArgumentsChanged(const QString &argumentsList) {
	if (! editTask)
		return;

	editTask->setDiffArguments(argumentsList);
}

void TaskEditWidget::realPrecisionChanged(int precision) {
	if (! editTask)
		return;

	editTask->setRealPrecision(precision);
}

void TaskEditWidget::specialJudgeChanged(const QString &text) {
	if (! editTask)
		return;

	editTask->setSpecialJudge(text);
}

void TaskEditWidget::interactorChanged(const QString &text) {
	if (! editTask)
		return;

	editTask->setInteractor(text);
}

void TaskEditWidget::interactorNameChanged(const QString &text) {
	if (! editTask)
		return;

	editTask->setInteractorName(text);
}

void TaskEditWidget::graderChanged(const QString &text) {
	if (! editTask)
		return;

	editTask->setGrader(text);
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

void TaskEditWidget::answerFileExtensionChanged(const QString &extension) {
	if (! editTask)
		return;

	editTask->setAnswerFileExtension(extension);
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
