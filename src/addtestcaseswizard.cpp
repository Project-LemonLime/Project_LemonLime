/*
 * SPDX-FileCopyrightText: 2011-2018 Project Lemon, Zhipeng Jia
 *                         2018-2019 Project LemonPlus, Dust1404
 *                         2019      Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "addtestcaseswizard.h"
#include "ui_addtestcaseswizard.h"
//
#include "base/settings.h"
//
#include <QMessageBox>
#include <algorithm>

AddTestCasesWizard::AddTestCasesWizard(QWidget *parent) : QWizard(parent), ui(new Ui::AddTestCasesWizard) {
	ui->setupUi(this);
	ui->fullScore->setValidator(new QIntValidator(1, Settings::upperBoundForFullScore(), this));
	ui->timeLimit->setValidator(new QIntValidator(1, Settings::upperBoundForTimeLimit(), this));
	ui->memoryLimit->setValidator(new QIntValidator(1, Settings::upperBoundForMemoryLimit(), this));
	connect(ui->fullScore, &QLineEdit::textChanged, this, &AddTestCasesWizard::fullScoreChanged);
	connect(ui->timeLimit, &QLineEdit::textChanged, this, &AddTestCasesWizard::timeLimitChanged);
	connect(ui->memoryLimit, &QLineEdit::textChanged, this, &AddTestCasesWizard::memoryLimitChanged);
	QHeaderView *header = ui->argumentList->horizontalHeader();

	for (int i = 0; i < 3; i++) {
		header->resizeSection(i, header->sectionSizeHint(i));
	}

	connect(ui->inputFilesPattern, &QLineEdit::textChanged, this,
	        &AddTestCasesWizard::inputFilesPatternChanged);
	connect(ui->outputFilesPattern, &QLineEdit::textChanged, this,
	        &AddTestCasesWizard::outputFilesPatternChanged);
	connect(ui->addArgumentButton, &QToolButton::clicked, this, &AddTestCasesWizard::addArgument);
	connect(ui->deleteArgumentButton, &QToolButton::clicked, this, &AddTestCasesWizard::deleteArgument);
}

AddTestCasesWizard::~AddTestCasesWizard() { delete ui; }

void AddTestCasesWizard::setSettings(Settings *_settings, bool check) {
	settings = _settings;
	ui->fullScore->setText(QString("%1").arg(settings->getDefaultFullScore()));
	ui->timeLimit->setText(QString("%1").arg(settings->getDefaultTimeLimit()));
	ui->memoryLimit->setText(QString("%1").arg(settings->getDefaultMemoryLimit()));
	ui->timeLimit->setEnabled(check);
	ui->timeLimitLabel->setEnabled(check);
	ui->msLabel->setEnabled(check);
	ui->memoryLimit->setEnabled(check);
	ui->memoryLimitLabel->setEnabled(check);
	ui->mbLabel->setEnabled(check);
	// ui->outputFilesPattern->setDisabled(interactorCheck);
	// ui->outputFilesPatternLabel->setDisabled(interactorCheck);
	refreshButtonState();
}

auto AddTestCasesWizard::getFullScore() const -> int { return fullScore; }

auto AddTestCasesWizard::getTimeLimit() const -> int { return timeLimit; }

auto AddTestCasesWizard::getMemoryLimit() const -> int { return memoryLimit; }

auto AddTestCasesWizard::getMatchedInputFiles() const -> const QList<QStringList> & {
	return matchedInputFiles;
}

auto AddTestCasesWizard::getMatchedOutputFiles() const -> const QList<QStringList> & {
	return matchedOutputFiles;
}

void AddTestCasesWizard::fullScoreChanged(const QString &text) { fullScore = text.toInt(); }

void AddTestCasesWizard::timeLimitChanged(const QString &text) { timeLimit = text.toInt(); }

void AddTestCasesWizard::memoryLimitChanged(const QString &text) { memoryLimit = text.toInt(); }

void AddTestCasesWizard::inputFilesPatternChanged(const QString &text) { inputFilesPattern = text; }

void AddTestCasesWizard::outputFilesPatternChanged(const QString &text) { outputFilesPattern = text; }

void AddTestCasesWizard::addArgument() {
	ui->argumentList->setRowCount(ui->argumentList->rowCount() + 1);
	int index = ui->argumentList->rowCount() - 1;
	ui->argumentList->setItem(index, 0, new QTableWidgetItem(QString("<%1>").arg(index + 1)));
	ui->argumentList->item(index, 0)->setTextAlignment(Qt::AlignCenter);
	ui->argumentList->item(index, 0)->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled |
	                                           Qt::ItemIsSelectable);
	ui->argumentList->item(index, 0)->setCheckState(Qt::Checked);
	ui->argumentList->setItem(index, 1, new QTableWidgetItem());
	ui->argumentList->setFocus();
	ui->argumentList->editItem(ui->argumentList->item(index, 1));
	refreshButtonState();
}

void AddTestCasesWizard::deleteArgument() {
	int index = ui->argumentList->currentRow();

	for (int i = index; i + 1 < ui->argumentList->rowCount(); i++) {
		ui->argumentList->item(i, 0)->setCheckState(ui->argumentList->item(i + 1, 0)->checkState());
		delete ui->argumentList->item(i, 1);
		ui->argumentList->setItem(i, 1, new QTableWidgetItem(ui->argumentList->item(i + 1, 1)->text()));
	}

	ui->argumentList->setRowCount(ui->argumentList->rowCount() - 1);
	refreshButtonState();
}

void AddTestCasesWizard::refreshButtonState() {
	if (ui->argumentList->rowCount() < 9) {
		ui->addArgumentButton->setEnabled(true);
	} else {
		ui->addArgumentButton->setEnabled(false);
	}

	if (ui->argumentList->currentRow() == -1) {
		ui->deleteArgumentButton->setEnabled(false);
	} else {
		ui->deleteArgumentButton->setEnabled(true);
	}
}

void AddTestCasesWizard::getFiles(const QString &curDir, const QString &prefix, QStringList &files) {
	QStringList list = QDir(curDir).entryList(QDir::Files);

	for (auto &list_i : list) {
		list_i = prefix + list_i;
	}

	files.append(list);
	list = QDir(curDir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (const auto &i : list) {
		getFiles(curDir + i + QDir::separator(), prefix + i + QDir::separator(), files);
	}
}

auto AddTestCasesWizard::getFullRegExp(const QString &pattern) -> QString {
	QString result = pattern;
	result.replace("\\", "\\\\");
	result.replace("^", "\\^");
	result.replace("$", "\\$");
	result.replace("(", "\\(");
	result.replace(")", "\\)");
	result.replace("*", "\\*");
	result.replace("+", "\\+");
	result.replace("?", "\\?");
	result.replace(".", "\\.");
	result.replace("[", "\\[");
	result.replace("{", "\\{");
	result.replace("|", "\\|");

	for (int i = 0; i < ui->argumentList->rowCount(); i++) {
		QString text = ui->argumentList->item(i, 1)->text();
		result.replace(QString("<%1>").arg(i + 1), QString("(%1)").arg(text));
	}

	return result;
}

auto AddTestCasesWizard::getMatchedPart(const QString &str, const QString &pattern) -> QStringList {
	QStringList result;

	for (int i = 0; i < ui->argumentList->rowCount(); i++)
		result.append("");

	for (int pos = 0, i = 0; pos < pattern.length(); i++, pos++) {
		if (pos + 2 < pattern.length()) {
			if (pattern[pos] == '<' && pattern[pos + 1].isDigit() && pattern[pos + 1] != '0' &&
			    pattern[pos + 2] == '>') {
				int index = pattern[pos + 1].toLatin1() - 49;
				QString regExp = ui->argumentList->item(index, 1)->text();

				for (int j = i; j < str.length(); j++) {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
					regExp = QRegularExpression::anchoredPattern(regExp);
					if (QRegularExpression(regExp).match(str.mid(i, j - i + 1)).hasMatch()) {
						if (QRegularExpression(
						        QRegularExpression::anchoredPattern(getFullRegExp(pattern.mid(pos + 3))))
						        .match(str.mid(j + 1))
						        .hasMatch()) {
							result[index] = str.mid(i, j - i + 1);
							i = j;
							break;
						}
					}
#else
					if (QRegExp(regExp).exactMatch(str.mid(i, j - i + 1))) {
						if (QRegExp(getFullRegExp(pattern.mid(pos + 3))).exactMatch(str.mid(j + 1))) {
							result[index] = str.mid(i, j - i + 1);
							i = j;
							break;
						}
					}
#endif
				}
				pos += 2;
			}
		}
	}

	return result;
}

void AddTestCasesWizard::searchMatchedFiles() {
	QStringList inputFiles;
	QStringList outputFiles;
	getFiles(Settings::dataPath(), "", inputFiles);
	getFiles(Settings::dataPath(), "", outputFiles);
	QString regExp = getFullRegExp(inputFilesPattern);

	for (int i = 0; i < inputFiles.size(); i++) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
		if (! QRegularExpression(QRegularExpression::anchoredPattern(regExp)).match(inputFiles[i]).hasMatch())
#else
		if (! QRegExp(regExp).exactMatch(inputFiles[i]))
#endif
		{
			inputFiles.removeAt(i);
			i--;
		}
	}

	regExp = getFullRegExp(outputFilesPattern);

	for (int i = 0; i < outputFiles.size(); i++) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
		if (! QRegularExpression(QRegularExpression::anchoredPattern(regExp))
		          .match(outputFiles[i])
		          .hasMatch())
#else
		if (! QRegExp(regExp).exactMatch(outputFiles[i]))
#endif
		{
			outputFiles.removeAt(i);
			i--;
		}
	}

	std::sort(inputFiles.begin(), inputFiles.end(), compareFileName);
	std::sort(outputFiles.begin(), outputFiles.end(), compareFileName);
	QList<QStringList> inputFilesMatchedPart;
	QList<QStringList> outputFilesMatchedPart;

	for (auto &inputFile : inputFiles) {
		inputFilesMatchedPart.append(getMatchedPart(inputFile, inputFilesPattern));
	}

	for (auto &outputFile : outputFiles) {
		outputFilesMatchedPart.append(getMatchedPart(outputFile, outputFilesPattern));
	}

	QMultiMap<QString, int> loc;
	QList<QPair<QString, QString>> singleCases;
	QList<QStringList> matchedPart;

	// if(ui->outputFilesPattern->isEnabled()) {
	for (int i = 0; i < inputFiles.size(); i++) {
		loc.replace(inputFilesMatchedPart[i].join("*"), i);
	}

	for (int i = 0; i < outputFiles.size(); i++) {
		if (loc.count(outputFilesMatchedPart[i].join("*")) > 0) {
			int partner = loc.value(outputFilesMatchedPart[i].join("*"));
			singleCases.append(qMakePair(inputFiles[partner], outputFiles[i]));
			matchedPart.append(outputFilesMatchedPart[i]);
		}
	}

	/*} else {
	    for (int i = 0; i < inputFiles.size(); i ++) {
	            singleCases.append(qMakePair(inputFiles[i], QString("")));
	            matchedPart.append(inputFilesMatchedPart[i]);
	    }
	}*/
	loc.clear();

	for (int i = 0; i < singleCases.size(); i++) {
		QStringList key;

		for (int j = 0; j < ui->argumentList->rowCount(); j++) {
			if (ui->argumentList->item(j, 0)->checkState() == Qt::Checked) {
				key.append(matchedPart[i][j]);
			}
		}

		loc.insert(key.join("*"), i);
	}

	matchedInputFiles.clear();
	matchedOutputFiles.clear();
	ui->testCasesViewer->clear();
	QList<QString> keys = loc.uniqueKeys();
	std::sort(keys.begin(), keys.end(), compareFileName);

	for (int i = 0; i < keys.size(); i++) {
		QList<int> values = loc.values(keys[i]);
		std::sort(values.begin(), values.end());
		QStringList inputFiles;
		QStringList outputFiles;
		auto *item = new QTreeWidgetItem(ui->testCasesViewer);
		item->setText(0, tr("Test Case #%1").arg(i + 1));

		for (int value : qAsConst(values)) {
			inputFiles.append(singleCases[value].first);
			outputFiles.append(singleCases[value].second);
			auto *child = new QTreeWidgetItem(item);
			child->setText(0, singleCases[value].first);
			child->setText(1, singleCases[value].second);
		}

		matchedInputFiles.append(inputFiles);
		matchedOutputFiles.append(outputFiles);
	}

	ui->testCasesViewer->resizeColumnToContents(0);
	ui->testCasesViewer->resizeColumnToContents(1);
}

auto AddTestCasesWizard::validateCurrentPage() -> bool {
	if (currentId() == 0) {
		if (ui->fullScore->text().isEmpty()) {
			ui->fullScore->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty full score!"), QMessageBox::Close);
			return false;
		}

		if (ui->timeLimit->text().isEmpty()) {
			ui->timeLimit->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty time limit!"), QMessageBox::Close);
			return false;
		}

		if (ui->memoryLimit->text().isEmpty()) {
			ui->memoryLimit->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty memory limit!"), QMessageBox::Close);
			return false;
		}

		return true;
	}

	if (currentId() == 1) {
		if (ui->inputFilesPattern->text().isEmpty()) {
			ui->inputFilesPattern->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty input files pattern!"), QMessageBox::Close);
			return false;
		}

		if (ui->outputFilesPattern->text().isEmpty()) {
			ui->outputFilesPattern->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty output files pattern!"), QMessageBox::Close);
			return false;
		}

		for (int i = 0; i < ui->argumentList->rowCount(); i++) {
			if (inputFilesPattern.count(QString("<%1>").arg(i + 1)) > 1) {
				ui->inputFilesPattern->setFocus();
				QMessageBox::warning(
				    this, tr("Error"),
				    tr("Argument <%1> appears more than once in input files pattern!").arg(i + 1),
				    QMessageBox::Close);
				return false;
			}

			if (outputFilesPattern.count(QString("<%1>").arg(i + 1)) > 1) {
				ui->outputFilesPattern->setFocus();
				QMessageBox::warning(
				    this, tr("Error"),
				    tr("Argument <%1> appears more than once in output files pattern!").arg(i + 1),
				    QMessageBox::Close);
				return false;
			}

			QString regExp = ui->argumentList->item(i, 1)->text();
			if (! QRegularExpression(regExp).isValid()) {
				ui->argumentList->setCurrentCell(i, 1);
				QMessageBox::warning(this, tr("Error"), tr("Invalid regular expression!"),
				                     QMessageBox::Close);
				return false;
			}
		}

		QApplication::setOverrideCursor(Qt::WaitCursor);
		searchMatchedFiles();
		QApplication::restoreOverrideCursor();
		return true;
	}

	return true;
}

auto AddTestCasesWizard::compareFileName(const QString &a, const QString &b) -> bool {
	return a.length() < b.length() || (a.length() == b.length() && QString::localeAwareCompare(a, b) < 0);
}
