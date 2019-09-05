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
 * judgingdialog.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * judgingdialog.cpp @Project LemonPt
 * Update 2019 iotang
 **/

#include "judgingdialog.h"
#include "ui_judgingdialog.h"
#include "contest.h"
#include "task.h"
#include <QScrollBar>

JudgingDialog::JudgingDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::JudgingDialog)
{
	ui->setupUi(this);
	cursor = new QTextCursor(ui->logViewer->document());
	connect(ui->cancelButton, SIGNAL(clicked()),
			  this, SLOT(stopJudgingSlot()));
	connect(ui->skipButton, SIGNAL(clicked()),
			  this, SLOT(skipJudging()));
}

JudgingDialog::~JudgingDialog()
{
	delete ui;
	delete cursor;
}

void JudgingDialog::setContest(Contest *contest)
{
	curContest = contest;
	connect(curContest, SIGNAL(singleCaseFinished(int, int, int, int, int, int, int)),
			  this, SLOT(singleCaseFinished(int, int, int, int, int, int, int)));
	connect(curContest, SIGNAL(singleSubtaskDependenceFinished(int, int, double)),
			  this, SLOT(singleSubtaskDependenceFinished(int, int, double)));
	connect(curContest, SIGNAL(taskJudgingStarted(QString)),
			  this, SLOT(taskJudgingStarted(QString)));
	connect(curContest, SIGNAL(contestantJudgingStart(QString)),
			  this, SLOT(contestantJudgingStart(QString)));
	connect(curContest, SIGNAL(contestantJudgingFinished()),
			  this, SLOT(contestantJudgingFinished()));
	connect(curContest, SIGNAL(compileError(int, int)),
			  this, SLOT(compileError(int, int)));
	connect(this, SIGNAL(stopJudgingSignal()),
			  curContest, SLOT(stopJudgingSlot()));
}

void JudgingDialog::judge(const QStringList &nameList)
{
	stopJudging = false;
	ui->progressBar->setMaximum(curContest->getTotalTimeLimit() * nameList.size());

	for(int i = 0; i < nameList.size(); i ++)
	{
		curContest->judge(nameList[i]);

		if(stopJudging) break;
	}

#ifdef Q_OS_LINUX
	QString text = "notify-send --expire-time=2000 --urgency=normal " + tr("Finished") + " \"" + tr("Judge Finished - LemonPt") + "\"";
	QProcess::execute(text);
#endif
	accept();
}

void JudgingDialog::judge(const QString &name, int index)
{
	stopJudging = false;
	ui->progressBar->setMaximum(curContest->getTask(index)->getTotalTimeLimit());
	curContest->judge(name, index);

#ifdef Q_OS_LINUX
	QString text = "notify-send --expire-time=2000 --urgency=normal " + tr("Finished") + " \"" + tr("Judge Finished - LemonPt") + "\"";
	QProcess::execute(text);
#endif
	accept();
}

void JudgingDialog::judge(const QList<QPair<QString, QSet<int> > > &lists)
{
	stopJudging = false;

	long long allTime = 0;

	int listsSize = lists.size();

	for(int i = 0; i < listsSize; i++)
	{
		for(QSet<int>::const_iterator j = lists[i].second.begin(); j != lists[i].second.end(); j++)
		{
			allTime += curContest->getTask(*j)->getTotalTimeLimit();
		}
	}

	ui->progressBar->setMaximum(allTime);

	for(int i = 0; i < listsSize; i++)
	{
		curContest->judge(lists[i].first, lists[i].second);

		if(stopJudging) break;
	}

#ifdef Q_OS_LINUX
	QString text = "notify-send --expire-time=2000 --urgency=normal " + tr("Finished") + " \"" + tr("Judge Finished - LemonPt") + "\"";
	QProcess::execute(text);
#endif
}

void JudgingDialog::judgeAll()
{
	stopJudging = false;
	ui->progressBar->setMaximum(curContest->getTotalTimeLimit() * curContest->getContestantList().size());
	curContest->judgeAll();

#ifdef Q_OS_LINUX
	QString text = "notify-send --expire-time=2000 --urgency=normal " + tr("Finished") + " \"" + tr("Judge Finished - LemonPt") + "\"";
	QProcess::execute(text);
#endif
	accept();
}

void JudgingDialog::singleCaseFinished(int progress, int x, int y, int result, int scoreGot, int timeUsed, int memoryUsed)
{
	QTextBlockFormat blockFormat;
	blockFormat.setLeftMargin(30);
	cursor->insertBlock(blockFormat);
	QTextCharFormat charFormat, addcharFormat;
	charFormat.setFontPointSize(9);
	cursor->insertText(tr("Test case %1.%2: ").arg(x + 1).arg(y + 1), charFormat);
	addcharFormat.setFontPointSize(7);
	addcharFormat.setForeground(QBrush(Qt::darkGray));

	QString text, addtext = "";

	switch(ResultState(result))
	{
		case CorrectAnswer:
			text = tr("Correct answer");
			addtext = tr("+%1 Pt, %2 ms, %3 MB").arg(scoreGot).arg(timeUsed).arg((double)memoryUsed / 1024.00 / 1024.00);
			charFormat.setForeground(QBrush(Qt::darkGreen));
			break;

		case PartlyCorrect:
			text = tr("Partly correct");
			addtext = tr("+%1 Pt, %2 ms, %3 MB").arg(scoreGot).arg(timeUsed).arg((double)memoryUsed / 1024.00 / 1024.00);
			charFormat.setForeground(QBrush(Qt::darkCyan));
			break;

		case WrongAnswer:
			text = tr("Wrong answer");
			charFormat.setForeground(QBrush(Qt::red));
			break;

		case TimeLimitExceeded:
			text = tr("Time limit exceeded");
			charFormat.setForeground(QBrush(Qt::darkYellow));
			break;

		case MemoryLimitExceeded:
			text = tr("Memory limit exceeded");
			charFormat.setForeground(QBrush(Qt::darkBlue));
			break;

		case RunTimeError:
			text = tr("Run time error");
			charFormat.setForeground(QBrush(Qt::darkMagenta));
			break;

		case Skipped:
			text = tr("Skipped");
			charFormat.setForeground(QBrush(Qt::lightGray));
			break;

		case CannotStartProgram:
			text = tr("Cannot start program");
			charFormat.setForeground(QBrush(Qt::darkRed));
			charFormat.setBackground(QBrush(Qt::lightGray));
			break;

		case FileError:
			text = tr("File error");
			charFormat.setForeground(QBrush(Qt::darkYellow));
			charFormat.setBackground(QBrush(Qt::lightGray));
			break;

		case InteractorError:
			text = tr("Interactor error");
			charFormat.setForeground(QBrush(Qt::white));
			charFormat.setBackground(QBrush(Qt::darkBlue));
			break;

		case InvalidSpecialJudge:
			text = tr("Invalid special judge");
			charFormat.setForeground(QBrush(Qt::white));
			charFormat.setBackground(QBrush(Qt::darkRed));
			break;

		case SpecialJudgeTimeLimitExceeded:
			text = tr("Special judge time limit exceeded");
			charFormat.setForeground(QBrush(Qt::white));
			charFormat.setBackground(QBrush(Qt::darkYellow));
			break;

		case SpecialJudgeRunTimeError:
			text = tr("Special judge run time error");
			charFormat.setForeground(QBrush(Qt::white));
			charFormat.setBackground(QBrush(Qt::darkMagenta));
			break;
	}

	cursor->insertText(text, charFormat);

	if(addtext.length() > 0)cursor->insertText(addtext, addcharFormat);

	ui->progressBar->setValue(ui->progressBar->value() + progress);

	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::singleSubtaskDependenceFinished(int x, int y, double ratio)
{
	QTextBlockFormat blockFormat;
	blockFormat.setLeftMargin(30);
	cursor->insertBlock(blockFormat);
	QTextCharFormat charFormat;
	charFormat.setFontPointSize(9);
	cursor->insertText(tr("Subtask Dependence %1.%2: ").arg(x + 1).arg(y + 1), charFormat);

	QString text;
	int percentage = ratio * 10000;
	text = QString("%1.%2%3").arg(percentage / 100).arg(percentage % 100).arg('%');

	if(percentage == 10000)
		charFormat.setForeground(QBrush(Qt::darkGreen));
	else if(percentage == 0)
	{
		charFormat.setForeground(QBrush(Qt::red));
		charFormat.setFontWeight(QFont::Bold);
	}
	else charFormat.setForeground(QBrush(Qt::darkCyan));

	cursor->insertText(text, charFormat);

	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::taskJudgingStarted(const QString &taskName)
{
	QTextBlockFormat blockFormat;
	blockFormat.setLeftMargin(15);
	cursor->insertBlock(blockFormat);
	QTextCharFormat charFormat;
	charFormat.setFontPointSize(9);
	cursor->insertText(tr("Start judging task %1").arg(taskName), charFormat);
	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::contestantJudgingStart(const QString &contestantName)
{
	QTextCharFormat charFormat;
	charFormat.setFontPointSize(10);
	charFormat.setFontWeight(QFont::Bold);
	cursor->insertText(tr("Start judging contestant %1").arg(contestantName), charFormat);
	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::contestantJudgingFinished()
{
	QTextBlockFormat blockFormat;
	cursor->insertBlock(blockFormat);
	cursor->insertBlock(blockFormat);
	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::compileError(int progress, int compileState)
{
	QTextBlockFormat blockFormat;
	blockFormat.setLeftMargin(30);
	cursor->insertBlock(blockFormat);
	QTextCharFormat charFormat;
	charFormat.setFontPointSize(9);

	QString text;

	switch(CompileState(compileState))
	{
		case NoValidSourceFile:
			text = tr("Cannot find valid source file");
			charFormat.setForeground(QBrush(Qt::white));
			charFormat.setBackground(QBrush(Qt::black));
			break;

		case CompileError:
			text = tr("Compile error");
			charFormat.setForeground(QBrush(Qt::red));
			charFormat.setBackground(QBrush(Qt::black));
			break;

		case CompileTimeLimitExceeded:
			text = tr("Compile time limit exceeded");
			charFormat.setForeground(QBrush(Qt::yellow));
			charFormat.setBackground(QBrush(Qt::black));
			break;

		case InvalidCompiler:
			text = tr("Invalid compiler");
			charFormat.setForeground(QBrush(Qt::magenta));
			charFormat.setBackground(QBrush(Qt::black));
			break;

		case CompileSuccessfully:
			text = tr("Compile Successfully");
			charFormat.setForeground(QBrush(Qt::lightGray));
			break;
	}

	cursor->insertText(text, charFormat);
	ui->progressBar->setValue(ui->progressBar->value() + progress);

	QScrollBar *bar = ui->logViewer->verticalScrollBar();
	bar->setValue(bar->maximum());
}

void JudgingDialog::stopJudgingSlot()
{
	stopJudging = true;
	emit stopJudgingSignal();
}

extern int skipEnabled;
void JudgingDialog::skipJudging()
{
	skipEnabled = 1;
}

void JudgingDialog::reject()
{
	stopJudgingSlot();
}
