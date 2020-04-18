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
 * addcompilerwizard.cpp @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * addcompilerwizard.cpp @Project LemonLime
 * Update 2019 iotang
 **/

#include "addcompilerwizard.h"
#include "ui_addcompilerwizard.h"
#include "compiler.h"
#include <QFileDialog>
#include <QMessageBox>

AddCompilerWizard::AddCompilerWizard(QWidget *parent) :
	QWizard(parent),
	ui(new Ui::AddCompilerWizard)
{
	ui->setupUi(this);
	ui->sourceFileExtensions->setValidator(new QRegExpValidator(QRegExp("(\\w+;)*\\w+"), this));
	ui->bytecodeFileExtensions->setValidator(new QRegExpValidator(QRegExp("(\\w+;)*\\w+"), this));
	ui->javaMemoryLimit->setValidator(new QIntValidator(64, 2048, this));
#ifdef Q_OS_LINUX

	if (QFileInfo::exists("/usr/bin/gcc"))
		ui->gccPath->setText("/usr/bin/gcc");

	if (QFileInfo::exists("/usr/bin/g++"))
		ui->gppPath->setText("/usr/bin/g++");

	if (QFileInfo::exists("/usr/bin/fpc"))
		ui->fpcPath->setText("/usr/bin/fpc");

	if (QFileInfo::exists("/usr/bin/javac"))
		ui->javacPath->setText("/usr/bin/javac");

	if (QFileInfo::exists("/usr/bin/java"))
		ui->javaPath->setText("/usr/bin/java");

	if (QFileInfo::exists("/usr/bin/python"))
		ui->pythonPath->setText("/usr/bin/python");

#endif
	connect(ui->typeSelect, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(compilerTypeChanged()));
	connect(ui->compilerSelectButton, SIGNAL(clicked()),
	        this, SLOT(selectCompilerLocation()));
	connect(ui->interpreterSelectButton, SIGNAL(clicked()),
	        this, SLOT(selectInterpreterLocation()));
	connect(ui->gccSelectButton, SIGNAL(clicked()),
	        this, SLOT(selectGccPath()));
	connect(ui->gppSelectButton, SIGNAL(clicked()),
	        this, SLOT(selectGppPath()));
	connect(ui->fpcSelectButton, SIGNAL(clicked()),
	        this, SLOT(selectFpcPath()));
	connect(ui->fbcSelectButton, SIGNAL(clicked()),
	        this, SLOT(selectFbcPath()));
	connect(ui->javacSelectButton, SIGNAL(clicked()),
	        this, SLOT(selectJavacPath()));
	connect(ui->javaSelectButton, SIGNAL(clicked()),
	        this, SLOT(selectJavaPath()));
	connect(ui->pythonSelectButton, SIGNAL(clicked()),
	        this, SLOT(selectPythonPath()));
}

AddCompilerWizard::~AddCompilerWizard()
{
	delete ui;
}

auto AddCompilerWizard::getCompilerList() const -> const QList<Compiler *> &
{
	return compilerList;
}

auto AddCompilerWizard::nextId() const -> int
{
	if (currentId() == 0)
	{
		if (ui->customRadioButton->isChecked())
		{
			return 1;
		}

		return 2;
	}

	if (currentId() == 3) return -1;

	return 3;
}

auto AddCompilerWizard::validateCurrentPage() -> bool
{
	if (currentId() == 1)
	{
		if (ui->compilerName->text().isEmpty())
		{
			ui->compilerName->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty compiler name!"), QMessageBox::Close);
			return false;
		}

		if (ui->compilerLocation->isEnabled() && ui->compilerLocation->text().isEmpty())
		{
			ui->compilerLocation->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty compiler location!"), QMessageBox::Close);
			return false;
		}

		if (ui->interpreterLocation->isEnabled() && ui->interpreterLocation->text().isEmpty())
		{
			ui->interpreterLocation->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty interpreter location!"), QMessageBox::Close);
			return false;
		}

		if (ui->sourceFileExtensions->text().isEmpty())
		{
			ui->sourceFileExtensions->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty source file extensions!"), QMessageBox::Close);
			return false;
		}

		if (ui->bytecodeFileExtensions->isEnabled() && ui->bytecodeFileExtensions->text().isEmpty())
		{
			ui->bytecodeFileExtensions->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty byte-code file extensions!"), QMessageBox::Close);
			return false;
		}

		QString text;
		text += tr("[Custom Compiler]") + "\n";
		text += tr("Compiler Name: ") + ui->compilerName->text() + "\n";
		text += tr("Compiler Type: ") + ui->typeSelect->currentText() + "\n";

		if (ui->compilerLocation->isEnabled())
		{
			text += tr("Compiler\'s Location: ") + ui->compilerLocation->text() + "\n";
		}

		if (ui->interpreterLocation->isEnabled())
		{
			text += tr("Interpreter\'s Location: ") + ui->interpreterLocation->text() + "\n";
		}

		text += tr("Source File Extensions: ") + ui->sourceFileExtensions->text() + "\n";

		if (ui->bytecodeFileExtensions->isEnabled())
		{
			text += tr("Byte-code File Extensions: ") + ui->bytecodeFileExtensions->text() + "\n";
		}

		if (ui->defaultCompilerArguments->isEnabled())
		{
			text += tr("Default Compiler\'s Arguments: ") + ui->defaultCompilerArguments->text() + "\n";
		}

		if (ui->defaultInterpreterArguments->isEnabled())
		{
			text += tr("Default Interpreter\'s Arguments: ") + ui->defaultInterpreterArguments->text() + "\n";
		}

		ui->logViewer->setPlainText(text);
	}

	if (currentId() == 2)
	{
		if (ui->gccGroupBox->isEnabled() && ui->gccPath->text().isEmpty())
		{
			ui->gccPath->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty gcc path!"), QMessageBox::Close);
			return false;
		}

		if (ui->gppGroupBox->isEnabled() && ui->gppPath->text().isEmpty())
		{
			ui->gppPath->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty g++ path!"), QMessageBox::Close);
			return false;
		}

		if (ui->fpcGroupBox->isEnabled() && ui->fpcPath->text().isEmpty())
		{
			ui->fpcPath->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty fpc path!"), QMessageBox::Close);
			return false;
		}

		if (ui->fbcGroupBox->isEnabled() && ui->fbcPath->text().isEmpty())
		{
			ui->fbcPath->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty fbc path!"), QMessageBox::Close);
			return false;
		}

		if (ui->javaGroupBox->isEnabled() && ui->javacPath->text().isEmpty())
		{
			ui->javacPath->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty javac path!"), QMessageBox::Close);
			return false;
		}

		if (ui->javaGroupBox->isEnabled() && ui->javaPath->text().isEmpty())
		{
			ui->javaPath->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty java path!"), QMessageBox::Close);
			return false;
		}

		if (ui->pythonGroupBox->isEnabled() && ui->pythonPath->text().isEmpty())
		{
			ui->pythonPath->setFocus();
			QMessageBox::warning(this, tr("Error"), tr("Empty python path!"), QMessageBox::Close);
			return false;
		}

		QString text;

		if (ui->gccGroupBox->isEnabled())
		{
			text += tr("[gcc Compiler]") + "\n";
			text += tr("gcc Path: ") + ui->gccPath->text() + "\n";

			if (ui->gccRecommendedCheck->isChecked())
			{
				text += tr("Add recommended configurations") + "\n";
			}

			text += "\n";
		}

		if (ui->gppGroupBox->isEnabled())
		{
			text += tr("[g++ Compiler]") + "\n";
			text += tr("g++ Path: ") + ui->gppPath->text() + "\n";

			if (ui->gppRecommendedCheck->isChecked())
			{
				text += tr("Add recommended configurations") + "\n";
			}

			text += "\n";
		}

		if (ui->fpcGroupBox->isEnabled())
		{
			text += tr("[fpc Compiler]") + "\n";
			text += tr("fpc Path: ") + ui->fpcPath->text() + "\n";

			if (ui->fpcRecommendedCheck->isChecked())
			{
				text += tr("Add recommended configurations") + "\n";
			}

			text += "\n";
		}

		if (ui->fbcGroupBox->isEnabled())
		{
			text += tr("[fbc Compiler]") + "\n";
			text += tr("fbc Path: ") + ui->fbcPath->text() + "\n\n";
		}

		if (ui->javaGroupBox->isEnabled())
		{
			text += tr("[Java Compiler]") + "\n";
			text += tr("javac Path: ") + ui->javacPath->text() + "\n";
			text += tr("java Path: ") + ui->javaPath->text() + "\n";
			text += tr("Memory Limit: %1 MB").arg(ui->javaMemoryLimit->text()) + "\n";
			text += "\n";
		}

		if (ui->pythonGroupBox->isEnabled())
		{
			text += tr("[Python Compiler]") + "\n";
			text += tr("python Path: ") + ui->pythonPath->text() + "\n";
			text += "\n";
		}

		ui->logViewer->setPlainText(text);
	}

	return true;
}

void AddCompilerWizard::compilerTypeChanged()
{
	if (ui->typeSelect->currentIndex() == 0)
	{
		ui->interpreterLocationLabel->setEnabled(false);
		ui->interpreterLocation->setEnabled(false);
		ui->interpreterSelectButton->setEnabled(false);
		ui->defaultInterpreterArgumentsLabel->setEnabled(false);
		ui->defaultInterpreterArguments->setEnabled(false);
	}
	else
	{
		ui->interpreterLocationLabel->setEnabled(true);
		ui->interpreterLocation->setEnabled(true);
		ui->interpreterSelectButton->setEnabled(true);
		ui->defaultInterpreterArgumentsLabel->setEnabled(true);
		ui->defaultInterpreterArguments->setEnabled(true);
	}

	if (ui->typeSelect->currentIndex() == 1)
	{
		ui->bytecodeFileExtensionsLabel->setEnabled(true);
		ui->bytecodeFileExtensions->setEnabled(true);
	}
	else
	{
		ui->bytecodeFileExtensionsLabel->setEnabled(false);
		ui->bytecodeFileExtensions->setEnabled(false);
	}

	if (ui->typeSelect->currentIndex() == 2)
	{
		ui->compilerLocationLabel->setEnabled(false);
		ui->compilerLocation->setEnabled(false);
		ui->compilerSelectButton->setEnabled(false);
		ui->defaultCompilerArgumentsLabel->setEnabled(false);
		ui->defaultCompilerArguments->setEnabled(false);
	}
	else
	{
		ui->compilerLocationLabel->setEnabled(true);
		ui->compilerLocation->setEnabled(true);
		ui->compilerSelectButton->setEnabled(true);
		ui->defaultCompilerArgumentsLabel->setEnabled(true);
		ui->defaultCompilerArguments->setEnabled(true);
	}
}

void AddCompilerWizard::selectCompilerLocation()
{
#ifdef Q_OS_WIN32
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), tr("Executable files (*.exe)"));
#else
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), tr("Executable files (*.*)"));
#endif

	if (! location.isEmpty())
	{
		location = location.replace('/', QDir::separator());
		ui->compilerLocation->setText(location);
	}
}

void AddCompilerWizard::selectInterpreterLocation()
{
#ifdef Q_OS_WIN32
	QString location = QFileDialog::getOpenFileName(this, tr("Select Interpreter\'s Location"),
	                   QDir::rootPath(), tr("Executable files (*.exe)"));
#else
	QString location = QFileDialog::getOpenFileName(this, tr("Select Interpreter\'s Location"),
	                   QDir::rootPath(), tr("Executable files (*.*)"));
#endif

	if (! location.isEmpty())
	{
		location = location.replace('/', QDir::separator());
		ui->interpreterLocation->setText(location);
	}
}

void AddCompilerWizard::selectGccPath()
{
#ifdef Q_OS_WIN32
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), "gcc (gcc.exe)");
#else
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), "gcc (gcc)");
#endif

	if (! location.isEmpty())
	{
		location = location.replace('/', QDir::separator());
		ui->gccPath->setText(location);
	}
}

void AddCompilerWizard::selectGppPath()
{
#ifdef Q_OS_WIN32
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), "g++ (g++.exe)");
#else
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), "g++ (g++)");
#endif

	if (! location.isEmpty())
	{
		location = location.replace('/', QDir::separator());
		ui->gppPath->setText(location);
	}
}

void AddCompilerWizard::selectFpcPath()
{
#ifdef Q_OS_WIN32
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), "fpc (fpc.exe)");
#else
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), "fpc (fpc)");
#endif

	if (! location.isEmpty())
	{
		location = location.replace('/', QDir::separator());
		ui->fpcPath->setText(location);
	}
}

void AddCompilerWizard::selectFbcPath()
{
#ifdef Q_OS_WIN32
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), "fbc (fbc.exe)");
#else
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), "fbc (fbc)");
#endif

	if (! location.isEmpty())
	{
		location = location.replace('/', QDir::separator());
		ui->fbcPath->setText(location);
	}
}

void AddCompilerWizard::selectJavacPath()
{
#ifdef Q_OS_WIN32
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), "javac (javac.exe)");
#else
	QString location = QFileDialog::getOpenFileName(this, tr("Select Compiler\'s Location"),
	                   QDir::rootPath(), "javac (javac)");
#endif

	if (! location.isEmpty())
	{
		location = location.replace('/', QDir::separator());
		ui->javacPath->setText(location);
	}
}

void AddCompilerWizard::selectJavaPath()
{
#ifdef Q_OS_WIN32
	QString location = QFileDialog::getOpenFileName(this, tr("Select Interpreter\'s Location"),
	                   QDir::rootPath(), "java (java.exe)");
#else
	QString location = QFileDialog::getOpenFileName(this, tr("Select Interpreter\'s Location"),
	                   QDir::rootPath(), "java (java)");
#endif

	if (! location.isEmpty())
	{
		location = location.replace('/', QDir::separator());
		ui->javaPath->setText(location);
	}
}

void AddCompilerWizard::selectPythonPath()
{
#ifdef Q_OS_WIN32
	QString location = QFileDialog::getOpenFileName(this, tr("Select Interpreter\'s Location"),
	                   QDir::rootPath(), "python (python.exe)");
#else
	QString location = QFileDialog::getOpenFileName(this, tr("Select Interpreter\'s Location"),
	                   QDir::rootPath(), "python (python)");
#endif

	if (! location.isEmpty())
	{
		location = location.replace('/', QDir::separator());
		ui->pythonPath->setText(location);
	}
}

void AddCompilerWizard::accept()
{
	if (ui->customRadioButton->isChecked())
	{
		auto *compiler = new Compiler;
		compiler->setCompilerType(static_cast<Compiler::CompilerType>(ui->typeSelect->currentIndex()));
		compiler->setCompilerName(ui->compilerName->text());
		compiler->setCompilerLocation(ui->compilerLocation->text());
		compiler->setInterpreterLocation(ui->interpreterLocation->text());
		compiler->setSourceExtensions(ui->sourceFileExtensions->text());
		compiler->setBytecodeExtensions(ui->bytecodeFileExtensions->text());
		compiler->addConfiguration("default",
		                           ui->defaultCompilerArguments->text(),
		                           ui->defaultInterpreterArguments->text());
		compilerList.append(compiler);
	}

	if (ui->builtinRadioButton->isChecked())
	{
		if (ui->gccGroupBox->isEnabled())
		{
			auto *compiler = new Compiler;
			compiler->setCompilerName("gcc");
			compiler->setCompilerLocation(ui->gccPath->text());
			compiler->setSourceExtensions("c");
			QString stackArg = "";
#ifdef Q_OS_WIN32
			stackArg = " -Wl,--stack=2147483647";
#endif
			compiler->addConfiguration("default", "-o %s %s.* -lm" + stackArg, "");

			if (ui->gccRecommendedCheck->isChecked())
			{
				compiler->addConfiguration("C89", "-o %s %s.* -lm -std=c89" + stackArg, "");
				compiler->addConfiguration("C89 O2", "-o %s %s.* -lm -std=c89 -O2" + stackArg, "");
				compiler->addConfiguration("C99", "-o %s %s.* -lm -std=c99" + stackArg, "");
				compiler->addConfiguration("C99 O2", "-o %s %s.* -lm -std=c99 -O2" + stackArg, "");
				compiler->addConfiguration("C11", "-o %s %s.* -lm -std=c11" + stackArg, "");
				compiler->addConfiguration("C11 O2", "-o %s %s.* -lm -std=c11 -O2" + stackArg, "");
				compiler->addConfiguration("C17", "-o %s %s.* -lm -std=c17" + stackArg, "");
				compiler->addConfiguration("C17 O2", "-o %s %s.* -lm -std=c17 -O2" + stackArg, "");
				compiler->addConfiguration("C17 O3", "-o %s %s.* -lm -std=c17 -O3" + stackArg, "");
#ifdef Q_OS_LINUX
				compiler->addConfiguration("C89 UB Catching", "-o %s %s.* -lm -std=c89 -fsanitize=undefined" + stackArg, "");
				compiler->addConfiguration("C99 UB Catching", "-o %s %s.* -lm -std=c99 -fsanitize=undefined" + stackArg, "");
				compiler->addConfiguration("C11 UB Catching", "-o %s %s.* -lm -std=c11 -fsanitize=undefined" + stackArg, "");
				compiler->addConfiguration("C17 UB Catching", "-o %s %s.* -lm -std=c17 -fsanitize=undefined" + stackArg, "");
#endif
			}

#ifdef Q_OS_WIN32
			QProcessEnvironment environment;
			QString path = QFileInfo(ui->gccPath->text()).absolutePath();
			path.replace('/', QDir::separator());
			environment.insert("PATH", path);
			compiler->setEnvironment(environment);
#endif
			compilerList.append(compiler);
		}

		if (ui->gppGroupBox->isEnabled())
		{
			auto *compiler = new Compiler;
			compiler->setCompilerName("g++");
			compiler->setCompilerLocation(ui->gppPath->text());
			compiler->setSourceExtensions("cpp;cc;cxx");
			QString stackArg = "";
#ifdef Q_OS_WIN32
			stackArg = " -Wl,--stack=2147483647";
#endif
			compiler->addConfiguration("default", "-o %s %s.* -lm" + stackArg, "");

			if (ui->gppRecommendedCheck->isChecked())
			{
				compiler->addConfiguration("C++98", "-o %s %s.* -lm -std=c++98" + stackArg, "");
				compiler->addConfiguration("C++98 O2", "-o %s %s.* -lm -std=c++98 -O2" + stackArg, "");
				compiler->addConfiguration("C++03", "-o %s %s.* -lm -std=c++03" + stackArg, "");
				compiler->addConfiguration("C++03 O2", "-o %s %s.* -lm -std=c++03 -O2" + stackArg, "");
				compiler->addConfiguration("C++11", "-o %s %s.* -lm -std=c++11" + stackArg, "");
				compiler->addConfiguration("C++11 O2", "-o %s %s.* -lm -std=c++11 -O2" + stackArg, "");
				compiler->addConfiguration("C++14", "-o %s %s.* -lm -std=c++14" + stackArg, "");
				compiler->addConfiguration("C++14 O2", "-o %s %s.* -lm -std=c++14 -O2" + stackArg, "");
				compiler->addConfiguration("C++17", "-o %s %s.* -lm -std=c++17" + stackArg, "");
				compiler->addConfiguration("C++17 O2", "-o %s %s.* -lm -std=c++17 -O2" + stackArg, "");
				compiler->addConfiguration("C++17 O3", "-o %s %s.* -lm -std=c++17 -O3" + stackArg, "");
#ifdef Q_OS_LINUX
				compiler->addConfiguration("C++98 UB Catching", "-o %s %s.* -lm -std=c++98 -fsanitize=undefined" + stackArg, "");
				compiler->addConfiguration("C++03 UB Catching", "-o %s %s.* -lm -std=c++03 -fsanitize=undefined" + stackArg, "");
				compiler->addConfiguration("C++11 UB Catching", "-o %s %s.* -lm -std=c++11 -fsanitize=undefined" + stackArg, "");
				compiler->addConfiguration("C++14 UB Catching", "-o %s %s.* -lm -std=c++14 -fsanitize=undefined" + stackArg, "");
				compiler->addConfiguration("C++17 UB Catching", "-o %s %s.* -lm -std=c++17 -fsanitize=undefined" + stackArg, "");
#endif
			}

#ifdef Q_OS_WIN32
			QProcessEnvironment environment;
			QString path = QFileInfo(ui->gppPath->text()).absolutePath();
			path.replace('/', QDir::separator());
			environment.insert("PATH", path);
			compiler->setEnvironment(environment);
#endif
			compilerList.append(compiler);
		}

		if (ui->fpcGroupBox->isEnabled())
		{
			auto *compiler = new Compiler;
			compiler->setCompilerName("fpc");
			compiler->setCompilerLocation(ui->fpcPath->text());
			compiler->setSourceExtensions("pas;pp;inc");
			compiler->addConfiguration("default", "%s.*", "");

			if (ui->fpcRecommendedCheck->isChecked())
			{
				compiler->addConfiguration("O2", "%s.* -O2", "");
			}

			compilerList.append(compiler);
		}

		if (ui->fbcGroupBox->isEnabled())
		{
			auto *compiler = new Compiler;
			compiler->setCompilerName("fbc");
			compiler->setCompilerLocation(ui->fbcPath->text());
			compiler->setSourceExtensions("bas");
			compiler->addConfiguration("default", "%s.*", "");
			compilerList.append(compiler);
		}

		if (ui->javaGroupBox->isEnabled())
		{
			auto *compiler = new Compiler;
			compiler->setCompilerName("jdk");
			compiler->setCompilerType(Compiler::InterpretiveWithByteCode);
			compiler->setCompilerLocation(ui->javacPath->text());
			compiler->setInterpreterLocation(ui->javaPath->text());
			compiler->setSourceExtensions("java");
			compiler->setBytecodeExtensions("class");
			compiler->setTimeLimitRatio(5);
			compiler->setDisableMemoryLimitCheck(true);
			compiler->addConfiguration("default", "%s.*", QString("-Xmx%1m %s").arg(ui->javaMemoryLimit->text()));
			compilerList.append(compiler);
		}

		if (ui->pythonGroupBox->isEnabled())
		{
			auto *compiler = new Compiler;
			compiler->setCompilerName("python");
			compiler->setSourceExtensions("py");
			compiler->setTimeLimitRatio(10);
			compiler->setMemoryLimitRatio(5);
			compiler->setCompilerType(Compiler::InterpretiveWithoutByteCode);
			compiler->setInterpreterLocation(ui->pythonPath->text());
			compiler->addConfiguration("default", "", "%s.*");
			compilerList.append(compiler);
		}
	}

	QWizard::accept();
}
