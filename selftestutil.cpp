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
 * selftestutil.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/

#include "selftestutil.h"
#include "contest.h"
#include "task.h"
#include "testcase.h"
#include "settings.h"
#include <QMessageBox>
#include <QApplication>

SelfTestUtil::SelfTestUtil(QObject *parent) :
	QObject(parent)
{
}

void SelfTestUtil::clearPath(const QString &curDir)
{
	QDir dir(curDir);
	QStringList fileList = dir.entryList(QDir::Files);

	for(int i = 0; i < fileList.size(); i ++)
		if(! dir.remove(fileList[i]))
		{
#ifdef Q_OS_WIN32
			QProcess::execute(QString("attrib -R \"") + curDir + fileList[i] + "\"");
#endif
#ifdef Q_OS_LINUX
			QProcess::execute(QString("chmod +w \"") + curDir + fileList[i] + "\"");
#endif
			dir.remove(fileList[i]);
		}

	QStringList dirList = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

	for(int i = 0; i < dirList.size(); i ++)
	{
		clearPath(curDir + dirList[i] + QDir::separator());
		dir.rmdir(dirList[i]);
	}
}

void SelfTestUtil::makeSelfTest(QWidget *widget, Contest *contest)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	if(QDir(Settings::selfTestPath()).exists())
		clearPath(Settings::selfTestPath());

	if(! QDir(Settings::selfTestPath()).exists() && ! QDir().mkdir(Settings::selfTestPath()))
	{
		QApplication::restoreOverrideCursor();
		QMessageBox::warning(widget, tr("LemonPt"), tr("Cannot make directory"), QMessageBox::Ok);
		return;
	}

	QList<Task*> taskList = contest->getTaskList();

	for(int i = 0; i < taskList.size(); i ++)
	{
		QDir(Settings::selfTestPath()).mkdir(taskList[i]->getProblemTile());
		QList<TestCase*> testCaseList = taskList[i]->getTestCaseList();
#ifdef Q_OS_WIN32
		QFile file(Settings::selfTestPath() + taskList[i]->getProblemTile() + QDir::separator() + "check.bat");

		if(! file.open(QFile::WriteOnly | QFile::Text))
		{
			QApplication::restoreOverrideCursor();
			QMessageBox::warning(widget, tr("LemonPt"), tr("Cannot write check.bat"), QMessageBox::Ok);
			return;
		}

		QFile dummy(Settings::selfTestPath() + taskList[i]->getProblemTile() + QDir::separator() + "enter");

		if(! dummy.open(QFile::WriteOnly | QFile::Text))
		{
			QApplication::restoreOverrideCursor();
			QMessageBox::warning(widget, tr("LemonPt"), tr("Cannot write enter"), QMessageBox::Ok);
			return;
		}

		QTextStream dummyStream(&dummy);
		dummyStream << endl;
		dummy.close();
#endif
#ifdef Q_OS_LINUX
		QFile file(Settings::selfTestPath() + taskList[i]->getProblemTile() + QDir::separator() + "check.sh");

		if(! file.open(QFile::WriteOnly | QFile::Text))
		{
			QApplication::restoreOverrideCursor();
			QMessageBox::warning(widget, tr("LemonPt"), tr("Cannot write check.sh"), QMessageBox::Ok);
			return;
		}

#endif
		QTextStream out(&file);
#ifdef Q_OS_WIN32
		out << "@echo off" << endl;
#endif
#ifdef Q_OS_LINUX
		out << "#!/bin/bash" << endl;
#endif

		if(taskList[i]->getComparisonMode() == Task::RealNumberMode)
		{
#ifdef Q_OS_WIN32
			QFile::copy(":/judge/realjudge_win32.exe",
			            Settings::selfTestPath() + taskList[i]->getProblemTile()
			            + QDir::separator() + "realjudge.exe");
			QProcess::execute(QString("attrib -R \"") + Settings::selfTestPath() + taskList[i]->getProblemTile()
			                  + QDir::separator() + "realjudge.exe" + "\"");
#endif
#ifdef Q_OS_LINUX
			QFile::copy(":/judge/realjudge_linux",
			            Settings::selfTestPath() + taskList[i]->getProblemTile()
			            + QDir::separator() + "realjudge");
			QProcess::execute(QString("chmod +wx \"") + Settings::selfTestPath() + taskList[i]->getProblemTile()
			                  + QDir::separator() + "realjudge" + "\"");
#endif
		}

		if(taskList[i]->getComparisonMode() == Task::SpecialJudgeMode)
		{
			if(! QFile::copy(Settings::dataPath() + taskList[i]->getSpecialJudge(),
			                 Settings::selfTestPath() + taskList[i]->getProblemTile() + QDir::separator()
			                 + QFileInfo(taskList[i]->getSpecialJudge()).fileName()))
			{
				QApplication::restoreOverrideCursor();
				QMessageBox::warning(widget, tr("LemonPt"),
				                     tr("Cannot copy %1").arg(QFileInfo(taskList[i]->getSpecialJudge()).fileName()),
				                     QMessageBox::Ok);
				return;
			}
		}

		int index = 0;

		for(int j = 0; j < testCaseList.size(); j ++)
		{
			QStringList inputFiles = testCaseList[j]->getInputFiles();
			QStringList outputFiles = testCaseList[j]->getOutputFiles();

			for(int k = 0; k < inputFiles.size(); k ++)
			{
				index ++;
				QString inputFile, outputFile;

				if(taskList[i]->getTaskType() == Task::Traditional)
				{
					inputFile = taskList[i]->getSourceFileName();
					inputFile += QString("%1.in").arg(index);
					outputFile = taskList[i]->getSourceFileName();
					outputFile += QString("%1.out").arg(index);
				}
				else
				{
					inputFile = QFileInfo(inputFiles[k]).fileName();
					outputFile = QFileInfo(outputFiles[k]).fileName();
				}

				if(! QFile::copy(Settings::dataPath() + inputFiles[k],
				                 Settings::selfTestPath() + taskList[i]->getProblemTile() + QDir::separator()
				                 + inputFile))
				{
					QApplication::restoreOverrideCursor();
					QMessageBox::warning(widget, tr("LemonPt"),
					                     tr("Cannot copy %1").arg(QFileInfo(inputFiles[k]).fileName()),
					                     QMessageBox::Ok);
					return;
				}

				if(! QFile::copy(Settings::dataPath() + outputFiles[k],
				                 Settings::selfTestPath() + taskList[i]->getProblemTile() + QDir::separator()
				                 + outputFile))
				{
					QApplication::restoreOverrideCursor();
					QMessageBox::warning(widget, tr("LemonPt"),
					                     tr("Cannot copy %1").arg(QFileInfo(outputFiles[k]).fileName()),
					                     QMessageBox::Ok);
					return;
				}

#ifdef Q_OS_WIN32

				if(! taskList[i]->getStandardInputCheck() && taskList[i]->getTaskType() == Task::Traditional)
				{
					out << QString("copy \"%1\" \"%2\" >nul").arg(inputFile,
					        taskList[i]->getInputFileName()) << endl;
				}

				out << QString("echo Test Case: %1").arg(index) << endl;

				if(taskList[i]->getTaskType() == Task::Traditional)
				{
					out << "time<enter" << endl;
					QString cmd = QString("\"") + taskList[i]->getSourceFileName() + ".exe" + "\"";

					if(taskList[i]->getStandardInputCheck())
					{
						cmd += QString(" <\"%1\"").arg(inputFile);
					}

					if(taskList[i]->getStandardOutputCheck())
					{
						cmd += QString(" >\"%1\"").arg("_tmpout");
					}

					out << cmd << endl;
					out << "time<enter" << endl;
				}

				QString outputFileName;

				if(taskList[i]->getTaskType() == Task::Traditional)
				{
					if(taskList[i]->getStandardOutputCheck())
					{
						outputFileName = "_tmpout";
					}
					else
					{
						outputFileName = taskList[i]->getOutputFileName();
					}
				}
				else
				{
					outputFileName = QFileInfo(inputFiles[k]).completeBaseName() + "."
					                 + taskList[i]->getAnswerFileExtension();
				}

				if(taskList[i]->getComparisonMode() == Task::LineByLineMode)
				{
					out << QString("fc \"%1\" \"%2\"")
					    .arg(outputFileName, outputFile) << endl;
				}

				if(taskList[i]->getComparisonMode() == Task::IgnoreSpacesMode)
				{
					out << QString("fc \"%1\" \"%2\" /W")
					    .arg(outputFileName, outputFile) << endl;
				}

				if(taskList[i]->getComparisonMode() == Task::ExternalToolMode)
				{
					out << QString("fc \"%1\" \"%2\"")
					    .arg(outputFileName, outputFile) << endl;
				}

				if(taskList[i]->getComparisonMode() == Task::RealNumberMode)
				{
					out << QString("realjudge.exe \"%1\" \"%2\" \"%3\"")
					    .arg(outputFileName).arg(outputFile).arg(taskList[i]->getRealPrecision()) << endl;
				}

				if(taskList[i]->getComparisonMode() == Task::SpecialJudgeMode)
				{
					out << QString("\"%1\" \"%2\" \"%3\" \"%4\" \"%5\" \"%6\" \"%7\"")
					    .arg(QFileInfo(taskList[i]->getSpecialJudge()).fileName(),
					         inputFile, outputFileName, outputFile,
					         QString("%1").arg(testCaseList[j]->getFullScore()),
					         "_score", "_message") << endl;
					out << "echo Your score:" << endl << "type _score" << endl;
					out << "if exist _message (" << endl;
					out << "echo Message:" << endl << "type _message" << endl << ")" << endl;
				}

				out << "pause" << endl;

				if(! taskList[i]->getStandardInputCheck() && taskList[i]->getTaskType() == Task::Traditional)
				{
					out << QString("del \"%1\"").arg(taskList[i]->getInputFileName()) << endl;
				}

				if(taskList[i]->getTaskType() == Task::Traditional)
				{
					if(! taskList[i]->getStandardOutputCheck())
					{
						out << QString("del \"%1\"").arg(taskList[i]->getOutputFileName()) << endl;
					}
					else
					{
						out << "del _tmpout" << endl;
					}
				}

				if(taskList[i]->getComparisonMode() == Task::SpecialJudgeMode)
				{
					out << "del _score" << endl << "del _message" << endl;
				}

				out << "echo." << endl << endl;
#endif
#ifdef Q_OS_LINUX

				if(! taskList[i]->getStandardInputCheck() && taskList[i]->getTaskType() == Task::Traditional)
				{
					out << QString("cp %1 %2").arg(inputFile,
					                               taskList[i]->getInputFileName()) << endl;
				}

				out << QString("echo \"Test Case: %1\"").arg(index) << endl;

				if(taskList[i]->getTaskType() == Task::Traditional)
				{
					QString cmd = QString("\"") + taskList[i]->getSourceFileName() + "\"";

					if(taskList[i]->getStandardInputCheck())
					{
						cmd += QString(" <\"%1\"").arg(inputFile);
					}

					if(taskList[i]->getStandardOutputCheck())
					{
						cmd += QString(" >\"%1\"").arg("_tmpout");
					}

					out << QString("time ./") << cmd << endl;
				}

				QString outputFileName;

				if(taskList[i]->getTaskType() == Task::Traditional)
				{
					if(taskList[i]->getStandardOutputCheck())
					{
						outputFileName = "_tmpout";
					}
					else
					{
						outputFileName = taskList[i]->getOutputFileName();
					}
				}
				else
				{
					outputFileName = QFileInfo(inputFiles[k]).completeBaseName() + "."
					                 + taskList[i]->getAnswerFileExtension();
				}

				if(taskList[i]->getComparisonMode() == Task::LineByLineMode)
				{
					QString arg = QString("\"%1\" \"%2\"").arg(outputFileName, outputFile);
					out << "if ! diff " << arg << " --strip-trailing-cr -q;then" << endl;
					out << "diff " << arg << " --strip-trailing-cr -y" << endl;
					out << QString("echo \"Wrong answer\"") << endl;
					out << "else" << endl;
					out << QString("echo \"Correct answer\"") << endl;
					out << "fi" << endl;
				}

				if(taskList[i]->getComparisonMode() == Task::IgnoreSpacesMode)
				{
					QString arg = QString(" \"%1\" \"%2\"").arg(outputFileName, outputFile);
					out << "if ! diff" << " --strip-trailing-cr -q --ignore-space-change" << arg << ";then" << endl;
					out << "diff" << " --strip-trailing-cr -y --ignore-space-change" << arg << endl;
					out << QString("echo \"Wrong answer\"") << endl;
					out << "else" << endl;
					out << QString("echo \"Correct answer\"") << endl;
					out << "fi" << endl;
				}

				if(taskList[i]->getComparisonMode() == Task::ExternalToolMode)
				{
					QString arg = QString(" \"%1\" \"%2\"").arg(outputFileName, outputFile);
					out << "if ! diff " << taskList[i]->getDiffArguments() << arg << ";then" << endl;
					out << "diff " << taskList[i]->getDiffArguments() << arg << endl;
					out << QString("echo \"Wrong answer\"") << endl;
					out << "else" << endl;
					out << QString("echo \"Correct answer\"") << endl;
					out << "fi" << endl;
				}

				if(taskList[i]->getComparisonMode() == Task::RealNumberMode)
				{
					out << QString("./realjudge \"%1\" \"%2\" \"%3\"")
					    .arg(outputFileName).arg(outputFile).arg(taskList[i]->getRealPrecision()) << endl;
				}

				if(taskList[i]->getComparisonMode() == Task::SpecialJudgeMode)
				{
					out << QString("./%1 \"%2\" \"%3\" \"%4\" \"%5\" \"%6\" \"%7\"")
					    .arg(QFileInfo(taskList[i]->getSpecialJudge()).fileName(),
					         inputFile, outputFileName, outputFile,
					         QString("%1").arg(testCaseList[j]->getFullScore()),
					         "_score", "_message") << endl;
					out << "echo \"Your score:\"" << endl << "cat _score" << endl;
					out << "if [ -e _message ];then" << endl;
					out << "echo \"Message:\"" << endl << "cat _message" << endl << "fi" << endl;
				}

				out << "read -n1 -p \"Press enter to continue...\"" << endl;

				if(! taskList[i]->getStandardInputCheck() && taskList[i]->getTaskType() == Task::Traditional)
				{
					out << QString("rm \"%1\"").arg(taskList[i]->getInputFileName()) << endl;
				}

				if(taskList[i]->getTaskType() == Task::Traditional)
				{
					if(! taskList[i]->getStandardOutputCheck())
					{
						out << QString("rm \"%1\"").arg(taskList[i]->getOutputFileName()) << endl;
					}
					else
					{
						out << "rm _tmpout" << endl;
					}
				}

				if(taskList[i]->getComparisonMode() == Task::SpecialJudgeMode)
				{
					out << "rm _score" << endl << "rm _message" << endl;
				}

				out << "echo" << endl << endl;
#endif
			}
		}

		file.close();
#ifdef Q_OS_LINUX
		QProcess::execute(QString("chmod +x \"") + Settings::selfTestPath() + taskList[i]->getProblemTile()
		                  + QDir::separator() + "check.sh" + "\"");
#endif
	}

	QApplication::restoreOverrideCursor();
	QMessageBox::information(widget, tr("LemonPt"), tr("Self-test folder has been made"), QMessageBox::Ok);
}
