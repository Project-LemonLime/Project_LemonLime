#include "exportingasuojformat.h"
#include "base/settings.h"
#include "core/task.h"
#include "core/testcase.h"
#include "ui_exportingasuojformat.h"
#include <qvalidator.h>

ExportingAsUojFormat::ExportingAsUojFormat(QWidget *parent)
    : QDialog(parent), ui(new Ui::ExportingAsUojFormat) {
	ui->setupUi(this);

	class QValidator *timeLimitValidator = new QDoubleValidator(0, 86400, 2, this);
	ui->timeLimit->setValidator(timeLimitValidator);
	class QValidator *memoryLimitValidator = new QIntValidator(0, 4096, this);
	ui->memoryLimit->setValidator(memoryLimitValidator);

	connect(ui->specialJudge, &QCheckBox::stateChanged, this, &ExportingAsUojFormat::specialJudgeChanged);

	ui->checker->refreshFileList();
}

ExportingAsUojFormat::~ExportingAsUojFormat() { delete ui; }

void ExportingAsUojFormat::init(Task *task) {
	this->task = task;

	ui->problemTitle->setText(task->getProblemTitle());

	QList<QString> inputFileName = task->getInputFileName().split(".");
	if (inputFileName.length() == 2) {
		ui->inputFilePrefix->setText(inputFileName[0]);
		ui->inputFileSuffix->setText(inputFileName[1]);
	}

	QList<QString> outputFileName = task->getOutputFileName().split(".");
	if (outputFileName.length() == 2) {
		ui->outputFilePrefix->setText(outputFileName[0]);
		ui->outputFileSuffix->setText(outputFileName[1]);
	}

	int timeLimit = 0, memoryLimit = 0;
	for (auto testcase : task->getTestCaseList()) {
		timeLimit = std::max(timeLimit, testcase->getTimeLimit());
		memoryLimit = std::max(memoryLimit, testcase->getMemoryLimit());
	}

	if (timeLimit % 1000 == 0) {
		ui->timeLimit->setText(QString::number(timeLimit / 1000));
	} else {
		ui->timeLimit->setText(QString::number(timeLimit / 1000., 'f', 2));
	}
	ui->memoryLimit->setText(QString::number(memoryLimit));

	switch (task->getComparisonMode()) {
		case Task::LineByLineMode:
			ui->checker->setText("bcmp");
			break;
		case Task::IgnoreSpacesMode:
			ui->checker->setText("wcmp");
			break;
		case Task::RealNumberMode:
			ui->checker->setText(QString("rcmp%1").arg(task->getRealPrecision()));
			break;
		case Task::ExternalToolMode:
		case Task::SpecialJudgeMode:
		default:
			ui->specialJudge->setChecked(true);
			break;
	}
}

void ExportingAsUojFormat::specialJudgeChanged(int new_state) {
	if (new_state) {
		ui->checker->setFilters(QDir::Files);
	} else {
		ui->checker->setFilters(QDir::Filters());
	}
	ui->checker->refreshFileList();
}

void ExportingAsUojFormat::exportData() {
	QString exportFolder = "data_uoj_format/" + ui->problemTitle->text();

	QDir exportLoca;
	if (exportLoca.exists(exportFolder)) {
		exportLoca = QDir(exportFolder);
		if (! exportLoca.removeRecursively()) {
			QMessageBox::information(this, tr("Export as UOJ Format"),
			                         tr("Aborted: Cannot remove path `%1'.").arg(exportFolder));
			return;
		}
	}

	exportLoca = QDir();
	if (! exportLoca.mkpath(exportFolder)) {
		QMessageBox::information(this, tr("Export as UOJ Format"),
		                         tr("Aborted: Cannot make path `%1'.").arg(exportFolder));
		return;
	}

	exportLoca = QDir(exportFolder);
	QFile problemConfigFile(exportLoca.filePath("problem.conf"), this);
	if (! problemConfigFile.open(QFile::WriteOnly | QFile::Truncate)) {
		QMessageBox::information(this, tr("Export as UOJ Format"), tr("Aborted: Cannot open problem.conf."));
		return;
	}
	QTextStream problemConfig(&problemConfigFile);

	QString inputFilePrefix = ui->inputFilePrefix->text();
	QString outputFilePrefix = ui->outputFilePrefix->text();
	QString inputFileSuffix = ui->inputFileSuffix->text();
	QString outputFileSuffix = ui->outputFileSuffix->text();
	int numberOfTests = 0;
	QDir dataPath(Settings::dataPath());
	for (auto testcase : task->getTestCaseList()) {
		for (int i = 0; i < testcase->getInputFiles().length(); ++i) {
			++numberOfTests;
			QString inputFileName = testcase->getInputFiles()[i],
			        outputFileName = testcase->getOutputFiles()[i];
			QString newInputFileName =
			            inputFilePrefix + QString::number(numberOfTests) + "." + inputFileSuffix,
			        newOutputFileName =
			            outputFilePrefix + QString::number(numberOfTests) + "." + outputFileSuffix;
			if (! copyFile(dataPath.filePath(inputFileName), exportLoca.filePath(newInputFileName)))
				return;
			if (! copyFile(dataPath.filePath(outputFileName), exportLoca.filePath(newOutputFileName)))
				return;
		}
	}

	problemConfig << "use_builtin_judger on" << Qt::endl;
	if (! ui->specialJudge->isChecked()) {
		problemConfig << "use_builtin_checker " << ui->checker->text() << Qt::endl;
	} else if (ui->checker->text().length()) {
		if (! copyFile(dataPath.filePath(ui->checker->text()), exportLoca.filePath("chk.cpp")))
			return;
	}

	problemConfig << "n_tests " << numberOfTests << Qt::endl;
	problemConfig << "n_ex_tests 0" << Qt::endl;
	problemConfig << "n_sample_tests 0" << Qt::endl;

	problemConfig << "input_pre " << ui->inputFilePrefix->text() << Qt::endl;
	problemConfig << "input_suf " << ui->inputFileSuffix->text() << Qt::endl;
	problemConfig << "output_pre " << ui->outputFilePrefix->text() << Qt::endl;
	problemConfig << "output_suf " << ui->outputFileSuffix->text() << Qt::endl;

	problemConfig << "time_limit " << ui->timeLimit->text() << Qt::endl;
	problemConfig << "memory_limit " << ui->memoryLimit->text() << Qt::endl;
	problemConfig << "output_limit " << 64 << Qt::endl;

	bool subtask = false;
	for (auto testcase : task->getTestCaseList()) {
		if (testcase->getInputFiles().length() != 1 || ! testcase->getDependenceSubtask().empty())
			subtask = true;
	}

	if (subtask) {
		int subtaskEnd = 0, subtaskId = 0;
		problemConfig << "n_subtasks " << task->getTestCaseList().length() << Qt::endl;
		for (auto testcase : task->getTestCaseList()) {
			++subtaskId;
			subtaskEnd += testcase->getInputFiles().length();
			problemConfig << "subtask_end_" << subtaskId << " " << subtaskEnd << Qt::endl;
			problemConfig << "subtask_score_" << subtaskId << " " << testcase->getFullScore() << Qt::endl;
			if (auto dependences = testcase->getDependenceSubtask(); ! dependences.empty()) {
				problemConfig << "subtask_dependence_" << subtaskId << " many" << Qt::endl;
				int dependenceId = 0;
				for (auto i : dependences) {
					++dependenceId;
					problemConfig << "subtask_dependence_" << subtaskId << "_" << dependenceId << " " << i
					              << Qt::endl;
				}
			}
		}
	} else {
		int testcaseId = 0;
		for (auto testcase : task->getTestCaseList()) {
			++testcaseId;
			problemConfig << "point_score_" << testcaseId << " " << testcase->getFullScore() << Qt::endl;
		}
	}
}

bool ExportingAsUojFormat::copyFile(QString from, QString to) {
	if (! QFile::copy(from, to)) {
		QMessageBox::information(this, tr("Export as UOJ Format"),
		                         tr("Aborted: Cannot copy file `%1' to `%2'.").arg(from, to));
		return false;
	}
	return true;
}
