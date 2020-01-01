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
 * addtestcasewizard.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "addtestcaseswizard.h"
#include "ui_addtestcaseswizard.h"
#include "settings.h"
#include <algorithm>
#include <QMessageBox>

AddTestCasesWizard::AddTestCasesWizard(QWidget *parent) :
	QWizard(parent),
	ui(new Ui::AddTestCasesWizard)
{
	ui->setupUi(this);

	ui->fullScore->setValidator(new QIntValidator(1, Settings::upperBoundForFullScore(), this));
	ui->timeLimit->setValidator(new QIntValidator(1, Settings::upperBoundForTimeLimit(), this));
	ui->memoryLimit->setValidator(new QIntValidator(1, Settings::upperBoundForMemoryLimit(), this));

	connect(ui->fullScore, SIGNAL(textChanged(QString)),
	        this, SLOT(fullScoreChanged(QString)));
	connect(ui->timeLimit, SIGNAL(textChanged(QString)),
	        this, SLOT(timeLimitChanged(QString)));
	connect(ui->memoryLimit, SIGNAL(textChanged(QString)),
	        this, SLOT(memoryLimitChanged(QString)));

	QHeaderView *header = ui->argumentList->horizontalHeader();

	for (int i = 0; i < 3; i ++)
	{
		header->resizeSection(i, header->sectionSizeHint(i));
	}

	connect(ui->inputFilesPattern, SIGNAL(textChanged(QString)),
	        this, SLOT(inputFilesPatternChanged(QString)));
	connect(ui->outputFilesPattern, SIGNAL(textChanged(QString)),
	        this, SLOT(outputFilesPatternChanged(QString)));
	connect(ui->addArgumentButton, SIGNAL(clicked()),
	        this, SLOT(addArgument()));
	connect(ui->deleteArgumentButton, SIGNAL(clicked()),
	        this, SLOT(deleteArgument()));
}

AddTestCasesWizard::~AddTestCasesWizard()
{
	delete ui;
}

void AddTestCasesWizard::setSettings(Settings *_settings, bool check)
{
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
	//ui->outputFilesPattern->setDisabled(interactorCheck);
	//ui->outputFilesPatternLabel->setDisabled(interactorCheck);
	refreshButtonState();
}

int AddTestCasesWizard::getFullScore() const
{
	return fullScore;
}

int AddTestCasesWizard::getTimeLimit() const
{
	return timeLimit;
}

int AddTestCasesWizard::getMemoryLimit() const
{
	return memoryLimit;
}

const QList<QStringList> &AddTestCasesWizard::getMatchedInputFiles() const
{
	return matchedInputFiles;
}

const QList<QStringList> &AddTestCasesWizard::getMatchedOutputFiles() const
{
	return matchedOutputFiles;
}

void AddTestCasesWizard::fullScoreChanged(const QString &text)
{
	fullScore = text.toInt();
}

void AddTestCasesWizard::timeLimitChanged(const QString &text)
{
	timeLimit = text.toInt();
}

void AddTestCasesWizard::memoryLimitChanged(const QString &text)
{
	memoryLimit = text.toInt();
}

void AddTestCasesWizard::inputFilesPatternChanged(const QString &text)
{
	inputFilesPattern = text;
}

void AddTestCasesWizard::outputFilesPatternChanged(const QString &text)
{
	outputFilesPattern = text;
}

void AddTestCasesWizard::addArgument()
{
	ui->argumentList->setRowCount(ui->argumentList->rowCount() + 1);
	int index = ui->argumentList->rowCount() - 1;
	ui->argumentList->setItem(index, 0, new QTableWidgetItem(QString("<%1>").arg(index + 1)));
	ui->argumentList->item(index, 0)->setTextAlignment(Qt::AlignCenter);
	ui->argumentList->item(index, 0)->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
	ui->argumentList->item(index, 0)->setCheckState(Qt::Checked);
	ui->argumentList->setItem(index, 1, new QTableWidgetItem());
	ui->argumentList->setFocus();
	ui->argumentList->editItem(ui->argumentList->item(index, 1));
	refreshButtonState();
}

void AddTestCasesWizard::deleteArgument()
{
	int index = ui->argumentList->currentRow();

	for (int i = index; i + 1 < ui->argumentList->rowCount(); i ++)
	{
		ui->argumentList->item(i, 0)->setCheckState(ui->argumentList->item(i + 1, 0)->checkState());
		delete ui->argumentList->item(i, 1);
		ui->argumentList->setItem(i, 1, new QTableWidgetItem(ui->argumentList->item(i + 1, 1)->text()));
	}

	ui->argumentList->setRowCount(ui->argumentList->rowCount() - 1);
	refreshButtonState();
}

void AddTestCasesWizard::refreshButtonState()
{
	if (ui->argumentList->rowCount() < 9)
	{
		ui->addArgumentButton->setEnabled(true);
	}
	else
	{
		ui->addArgumentButton->setEnabled(false);
	}

	if (ui->argumentList->currentRow() == -1)
	{
		ui->deleteArgumentButton->setEnabled(false);
	}
	else
	{
		ui->deleteArgumentButton->setEnabled(true);
	}
}

void AddTestCasesWizard::getFiles(const QString &curDir, const QString &prefix, QStringList &files)
{
	QStringList list = QDir(curDir).entryList(QDir::Files);

	for (int i = 0; i < list.size(); i ++)
	{
		list[i] = prefix + list[i];
	}

	files.append(list);
	list = QDir(curDir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (int i = 0; i < list.size(); i ++)
	{
		getFiles(curDir + list[i] + QDir::separator(),
		         prefix + list[i] + QDir::separator(), files);
	}
}

QString AddTestCasesWizard::getFullRegExp(const QString &pattern)
{
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

	for (int i = 0; i < ui->argumentList->rowCount(); i ++)
	{
		QString text = ui->argumentList->item(i, 1)->text();
		result.replace(QString("<%1>").arg(i + 1), QString("(%1)").arg(text));
	}

	return result;
}

QStringList AddTestCasesWizard::getMatchedPart(const QString &str, const QString &pattern)
{
	QStringList result;

	for (int i = 0; i < ui->argumentList->rowCount(); i ++)
		result.append("");

	for (int pos = 0, i = 0; pos < pattern.length(); i ++, pos ++)
	{
		if (pos + 2 < pattern.length())
		{
			if (pattern[pos] == '<' && pattern[pos + 1].isDigit() && pattern[pos + 1] != '0' && pattern[pos + 2] == '>')
			{
				int index = pattern[pos + 1].toLatin1() - 49;
				QString regExp = ui->argumentList->item(index, 1)->text();

				for (int j = i; j < str.length(); j ++)
				{
					if (QRegExp(regExp).exactMatch(str.mid(i, j - i + 1)))
					{
						if (QRegExp(getFullRegExp(pattern.mid(pos + 3))).exactMatch(str.mid(j + 1)))
						{
							result[index] = str.mid(i, j - i + 1);
							i = j;
							break;
						}
					}
				}

				pos += 2;
			}
		}
	}

	return result;
}

void AddTestCasesWizard::searchMatchedFiles()
{
	QStringList inputFiles, outputFiles;
	getFiles(Settings::dataPath(), "", inputFiles);
	getFiles(Settings::dataPath(), "", outputFiles);

	QString regExp = getFullRegExp(inputFilesPattern);

	for (int i = 0; i < inputFiles.size(); i ++)
	{
		if (! QRegExp(regExp).exactMatch(inputFiles[i]))
		{
			inputFiles.removeAt(i);
			i --;
		}
	}

	regExp = getFullRegExp(outputFilesPattern);

	for (int i = 0; i < outputFiles.size(); i ++)
	{
		if (! QRegExp(regExp).exactMatch(outputFiles[i]))
		{
			outputFiles.removeAt(i);
			i --;
		}
	}

	std::sort(inputFiles.begin(), inputFiles.end(), compareFileName);
	std::sort(outputFiles.begin(), outputFiles.end(), compareFileName);

	QList<QStringList> inputFilesMatchedPart;
	QList<QStringList> outputFilesMatchedPart;

	for (int i = 0; i < inputFiles.size(); i ++)
	{
		inputFilesMatchedPart.append(getMatchedPart(inputFiles[i], inputFilesPattern));
	}

	for (int i = 0; i < outputFiles.size(); i ++)
	{
		outputFilesMatchedPart.append(getMatchedPart(outputFiles[i], outputFilesPattern));
	}

	QMap<QString, int> loc;
	QList< QPair<QString, QString> > singleCases;
	QList< QStringList > matchedPart;

	//if(ui->outputFilesPattern->isEnabled()) {
	for (int i = 0; i < inputFiles.size(); i ++)
	{
		loc[inputFilesMatchedPart[i].join("*")] = i;
	}

	for (int i = 0; i < outputFiles.size(); i ++)
	{
		if (loc.count(outputFilesMatchedPart[i].join("*")) > 0)
		{
			int partner = loc[outputFilesMatchedPart[i].join("*")];
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

	for (int i = 0; i < singleCases.size(); i ++)
	{
		QStringList key;

		for (int j = 0; j < ui->argumentList->rowCount(); j ++)
		{
			if (ui->argumentList->item(j, 0)->checkState() == Qt::Checked)
			{
				key.append(matchedPart[i][j]);
			}
		}

		loc.insertMulti(key.join("*"), i);
	}

	matchedInputFiles.clear();
	matchedOutputFiles.clear();
	ui->testCasesViewer->clear();

	QList<QString> keys = loc.uniqueKeys();
	std::sort(keys.begin(), keys.end(), compareFileName);

	for (int i = 0; i < keys.size(); i ++)
	{
		QList<int> values = loc.values(keys[i]);
		std::sort(values.begin(), values.end());
		QStringList inputFiles, outputFiles;
		QTreeWidgetItem *item = new QTreeWidgetItem(ui->testCasesViewer);
		item->setText(0, tr("Test Case #%1").arg(i + 1));

		for (int j = 0; j < values.size(); j ++)
		{
			inputFiles.append(singleCases[values[j]].first);
			outputFiles.append(singleCases[values[j]].second);
			QTreeWidgetItem *child = new QTreeWidgetItem(item);
			child->setText(0, singleCases[values[j]].first);
			child->setText(1, singleCases[values[j]].second);
		}

		matchedInputFiles.append(inputFiles);
		matchedOutputFiles.append(outputFiles);
	}

	ui->testCasesViewer->resizeColumnToContents(0);
	ui->testCasesViewer->resizeColumnToContents(1);
}

bool AddTestCasesWizard::validateCurrentPage()
{
	if (currentId() == 0)
	{
		if (ui->fullScore->text().isEmpty())
		{
			ui->fullScore->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty full score!"), QMessageBox::Close);
			return false;
		}

		if (ui->timeLimit->text().isEmpty())
		{
			ui->timeLimit->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty time limit!"), QMessageBox::Close);
			return false;
		}

		if (ui->memoryLimit->text().isEmpty())
		{
			ui->memoryLimit->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty memory limit!"), QMessageBox::Close);
			return false;
		}

		return true;
	}

	if (currentId() == 1)
	{
		if (ui->inputFilesPattern->text().isEmpty())
		{
			ui->inputFilesPattern->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty input files pattern!"), QMessageBox::Close);
			return false;
		}

		if (ui->outputFilesPattern->text().isEmpty())
		{
			ui->outputFilesPattern->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty output files pattern!"), QMessageBox::Close);
			return false;
		}

		for (int i = 0; i < ui->argumentList->rowCount(); i ++)
		{
			if (inputFilesPattern.count(QString("<%1>").arg(i + 1)) > 1)
			{
				ui->inputFilesPattern->setFocus();
				QMessageBox::warning(this, tr("Error"),
				                     tr("Argument <%1> appears more than once in input files pattern!").arg(i + 1),
				                     QMessageBox::Close);
				return false;
			}

			if (outputFilesPattern.count(QString("<%1>").arg(i + 1)) > 1)
			{
				ui->outputFilesPattern->setFocus();
				QMessageBox::warning(this, tr("Error"),
				                     tr("Argument <%1> appears more than once in output files pattern!").arg(i + 1),
				                     QMessageBox::Close);
				return false;
			}

			QString regExp = ui->argumentList->item(i, 1)->text();

			if (! QRegExp(regExp).isValid())
			{
				ui->argumentList->setCurrentCell(i, 1);
				QMessageBox::warning(this, tr("Error"), tr("Invalid regular expression!"), QMessageBox::Close);
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

bool AddTestCasesWizard::compareFileName(const QString &a, const QString &b)
{
	return a.length() < b.length() || (a.length() == b.length() && QString::localeAwareCompare(a, b) < 0);
}
