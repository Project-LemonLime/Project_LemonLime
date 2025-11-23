/*
 * SPDX-FileCopyrightText: 2020-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "exttestcaseupdaterdialog.h"
#include "ui_exttestcaseupdaterdialog.h"
//
#include "base/settings.h"
#include "core/task.h"
//
#include <QFileDialog>
#include <QMessageBox>
#include <QSet>
#include <QValidator>

ExtTestCaseUpdaterDialog::ExtTestCaseUpdaterDialog(QWidget *parent, Task *nowTask,
                                                   const Settings *nowSettings, int nowCaseNumber,
                                                   int editScore, int editData, int editTime, int editMemory,
                                                   int editDepend, QList<int> tempDepends)
    : QDialog(parent), ui(new Ui::ExtTestCaseUpdaterDialog), nowTask(nowTask), nowSettings(nowSettings),
      nowCaseNumber(nowCaseNumber), editScore(editScore), editData(editData), editTime(editTime),
      editMemory(editMemory), editDepend(editDepend) {
	ui->setupUi(this);

	if (! nowTask)
		return;

	if (nowCaseNumber > 0)
		setWindowTitle(QString(tr("Configure Test Case #%1")).arg(nowCaseNumber));
	else
		setWindowTitle(QString(tr("Configure")));

	ui->lineEditScore->setValidator(new QIntValidator(0, nowSettings->upperBoundForFullScore(), this));
	ui->lineEditTime->setValidator(new QIntValidator(1, nowSettings->upperBoundForTimeLimit(), this));
	ui->lineEditMemory->setValidator(new QIntValidator(1, nowSettings->upperBoundForMemoryLimit(), this));
	ui->lineEditInput->setFilters(QDir::Files);
	ui->lineEditOutput->setFilters(QDir::Files);
	QValidator *subtaskDependenceValidator =
	    new QRegularExpressionValidator(QRegularExpression("[0-9,]+$"), ui->lineEditDepends);
	ui->lineEditDepends->setValidator(subtaskDependenceValidator);

	connect(ui->lineEditScore, &QLineEdit::textChanged, this, &ExtTestCaseUpdaterDialog::fullScoreChanged);
	connect(ui->lineEditTime, &QLineEdit::textChanged, this, &ExtTestCaseUpdaterDialog::timeLimitChanged);
	connect(ui->lineEditMemory, &QLineEdit::textChanged, this, &ExtTestCaseUpdaterDialog::memoryLimitChanged);
	connect(ui->buttonFindInput, &QPushButton::clicked, this,
	        &ExtTestCaseUpdaterDialog::whenButtonFindInputClicked);
	connect(ui->buttonFindOutput, &QPushButton::clicked, this,
	        &ExtTestCaseUpdaterDialog::whenButtonFindOutputClicked);
	connect(ui->lineEditInput, &QLineEdit::textChanged, this, &ExtTestCaseUpdaterDialog::inputFileChanged);
	connect(ui->lineEditOutput, &QLineEdit::textChanged, this, &ExtTestCaseUpdaterDialog::outputFileChanged);
	connect(ui->lineEditDepends, &QLineEdit::textChanged, this, &ExtTestCaseUpdaterDialog::dependsChanged);

	if (editScore == NO_EDIT)
		ui->labelScore->hide(), ui->lineEditScore->hide(), defScore = NO_EDIT;
	else if (editScore == MAY_EDIT)
		score = defScore = MAY_EDIT;
	else if (editScore == EDIT_WITH_DEFAULT)
		score = defScore = nowSettings->getDefaultFullScore(),
		ui->lineEditScore->setText(QString::number(defScore));
	else
		score = defScore = editScore, ui->lineEditScore->setText(QString::number(defScore));

	if (editData == NO_EDIT) {
		ui->labelInput->hide(), ui->labelOutput->hide();
		ui->lineEditInput->hide(), ui->lineEditOutput->hide();
		ui->buttonFindInput->hide(), ui->buttonFindOutput->hide();
	}

	if (nowTask->getTaskType() == Task::AnswersOnly)
		editTime = editMemory = NO_EDIT;

	if (editTime == NO_EDIT) {
		ui->labelTimeLimit->hide(), ui->labelMemoryLimit->hide();
		ui->lineEditTime->hide(), ui->lineEditMemory->hide();
		ui->label->hide(), ui->label_2->hide();
	} else if (editTime == MAY_EDIT) {
		timeLimit = defTimeLimit = MAY_EDIT;
		memoryLimit = defMemoryLimit = MAY_EDIT;
	} else if (editTime == EDIT_WITH_DEFAULT) {
		timeLimit = defTimeLimit = nowSettings->getDefaultTimeLimit();
		memoryLimit = defMemoryLimit = nowSettings->getDefaultMemoryLimit();
		ui->lineEditTime->setText(QString::number(defTimeLimit));
		ui->lineEditMemory->setText(QString::number(defMemoryLimit));
	} else {
		timeLimit = defTimeLimit = editTime;
		memoryLimit = defMemoryLimit = editMemory;
		ui->lineEditTime->setText(QString::number(defTimeLimit));
		ui->lineEditMemory->setText(QString::number(defMemoryLimit));
	}

	if (editDepend == NO_EDIT)
		ui->labelDepend->hide(), ui->lineEditDepends->hide();

	QString tempStr = "";

	for (auto i : tempDepends)
		tempStr = tempStr + QString::number(i) + ",";

	if (! tempStr.isEmpty())
		tempStr.resize(tempStr.size() - 1);

	ui->lineEditDepends->setText(tempStr);
}

ExtTestCaseUpdaterDialog::~ExtTestCaseUpdaterDialog() { delete ui; }

auto ExtTestCaseUpdaterDialog::getScore() const -> int { return score; }

auto ExtTestCaseUpdaterDialog::getInput() const -> QString { return input; }

auto ExtTestCaseUpdaterDialog::getOutput() const -> QString { return output; }

auto ExtTestCaseUpdaterDialog::getTimeLimit() const -> int { return timeLimit; }

auto ExtTestCaseUpdaterDialog::getMemoryLimit() const -> int { return memoryLimit; }

auto ExtTestCaseUpdaterDialog::getDepends() const -> QStringList { return depends; }

void ExtTestCaseUpdaterDialog::whenButtonFindInputClicked() {
	QString filter = tr("Input Data") + " (";

	auto temp = nowSettings->getInputFileExtensions();

	for (const auto &i : temp)
		filter = filter + "*." + i + " ";

	filter.resize(filter.size() - 1);

	filter = filter + ");;" + tr("All Files (*)");

	QString curPath = QDir::currentPath() + nowSettings->dataPath();
	QString filePath = QFileDialog::getOpenFileName(
	    this, tr("Choose Input File"), nowSettings->dataPath() + nowTask->getSourceFileName(), filter);

	if (! filePath.isEmpty()) {
		QString realPath = filePath.mid(curPath.length() + 1);
		ui->lineEditInput->setText(realPath);
	}
}

void ExtTestCaseUpdaterDialog::whenButtonFindOutputClicked() {
	QString filter = tr("Output Data") + " (";

	auto temp = nowSettings->getOutputFileExtensions();

	for (const auto &i : temp)
		filter = filter + "*." + i + " ";

	filter.resize(filter.size() - 1);

	filter = filter + ");;" + tr("All Files (*)");

	QString curPath = QDir::currentPath() + nowSettings->dataPath();
	QString filePath = QFileDialog::getOpenFileName(
	    this, tr("Choose Output File"), nowSettings->dataPath() + nowTask->getSourceFileName(), filter);

	if (! filePath.isEmpty()) {
		QString realPath = filePath.mid(curPath.length() + 1);
		ui->lineEditOutput->setText(realPath);
	}
}

void ExtTestCaseUpdaterDialog::accept() {
	if (editData != NO_EDIT && input.isEmpty()) {
		QMessageBox::warning(this, tr("Error"), tr("Input File is Empty!"), QMessageBox::Close);
		return;
	}

	if (editData != NO_EDIT && output.isEmpty()) {
		QMessageBox::warning(this, tr("Error"), tr("Output File is Empty!"), QMessageBox::Close);
		return;
	}

	if (editDepend != NO_EDIT && checkDepends() != 0) {
		QMessageBox::warning(this, tr("Error"), tr("Dependence subtask index error!"), QMessageBox::Close);
		return;
	}

	return QDialog::accept();
}

void ExtTestCaseUpdaterDialog::fullScoreChanged(const QString &text) {
	if (text.isEmpty()) {
		if (editScore == MAY_EDIT)
			score = MAY_EDIT;
		else
			ui->lineEditScore->setText(QString::number(defScore));
	} else
		score = text.toInt();
}

void ExtTestCaseUpdaterDialog::timeLimitChanged(const QString &text) {
	if (text.isEmpty()) {
		if (editTime == MAY_EDIT)
			timeLimit = MAY_EDIT;
		else
			ui->lineEditTime->setText(QString::number(defTimeLimit));
	} else
		timeLimit = text.toInt();
}

void ExtTestCaseUpdaterDialog::memoryLimitChanged(const QString &text) {
	if (text.isEmpty()) {
		if (editMemory == MAY_EDIT)
			memoryLimit = MAY_EDIT;
		else
			ui->lineEditMemory->setText(QString::number(defMemoryLimit));
	} else
		memoryLimit = text.toInt();
}

void ExtTestCaseUpdaterDialog::inputFileChanged(const QString &text) { input = text; }

void ExtTestCaseUpdaterDialog::outputFileChanged(const QString &text) { output = text; }

void ExtTestCaseUpdaterDialog::dependsChanged(const QString &text) {
	depends = text.isEmpty() ? QStringList() : text.split(',');
}

auto ExtTestCaseUpdaterDialog::checkDepends() -> int {
	QSet<int> hav;

	for (const auto &i_ : std::as_const(depends)) {
		int i = i_.toInt();

		if (i <= 0 || i >= nowCaseNumber || hav.contains(i))
			return -1;

		hav.insert(i);
	}

	return 0;
}
