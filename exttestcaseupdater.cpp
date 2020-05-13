#include "exttestcaseupdater.h"
#include "ui_exttestcaseupdater.h"
#include <QValidator>
#include <QMessageBox>
#include <QFileDialog>

extTestCaseUpdater::extTestCaseUpdater(QWidget *parent, Task *nowTask, const Settings *nowSettings, int nowCaseNumber, int editScore, int editData, int editTime, int editMemory, int editDepend, QList<int> tempDepends) :
	QDialog(parent),
	ui(new Ui::extTestCaseUpdater), nowTask(nowTask), nowSettings(nowSettings), nowCaseNumber(nowCaseNumber), editScore(editScore), editData(editData), editTime(editTime), editMemory(editMemory), editDepend(editDepend)
{
	ui->setupUi(this);

	if (! nowTask)return;

	if (nowCaseNumber > 0)setWindowTitle(QString(tr("Configure Test Case #%1")).arg(nowCaseNumber));
	else setWindowTitle(QString(tr("Configure")));

	ui->lineEditScore->setValidator(new QIntValidator(0, nowSettings->upperBoundForFullScore(), this));
	ui->lineEditTime->setValidator(new QIntValidator(1, nowSettings->upperBoundForTimeLimit(), this));
	ui->lineEditMemory->setValidator(new QIntValidator(1, nowSettings->upperBoundForMemoryLimit(), this));
	ui->lineEditInput->setFilters(QDir::Files);
	ui->lineEditOutput->setFilters(QDir::Files);
	QRegExp regx("[0-9,]+$");
	QValidator *subtaskDependenceValidator = new QRegExpValidator(regx, ui->lineEditDepends);
	ui->lineEditDepends->setValidator(subtaskDependenceValidator);

	connect(ui->lineEditScore, SIGNAL(textChanged(QString)),
	        this, SLOT(fullScoreChanged(QString)));
	connect(ui->lineEditTime, SIGNAL(textChanged(QString)),
	        this, SLOT(timeLimitChanged(QString)));
	connect(ui->lineEditMemory, SIGNAL(textChanged(QString)),
	        this, SLOT(memoryLimitChanged(QString)));
	connect(ui->buttonFindInput, SIGNAL(clicked()),
	        this, SLOT(whenButtonFindInputClicked()));
	connect(ui->buttonFindOutput, SIGNAL(clicked()),
	        this, SLOT(whenButtonFindOutputClicked()));
	connect(ui->lineEditInput, SIGNAL(textChanged(QString)),
	        this, SLOT(inputFileChanged(QString)));
	connect(ui->lineEditOutput, SIGNAL(textChanged(QString)),
	        this, SLOT(outputFileChanged(QString)));
	connect(ui->lineEditDepends, SIGNAL(textChanged(QString)),
	        this, SLOT(dependsChanged(QString)));


	if (editScore <= -2)ui->labelScore->hide(), ui->lineEditScore->hide();
	else if (editScore == -1)defScore = nowSettings->getDefaultFullScore();
	else defScore = editScore;

	if (editData <= 0)
	{
		ui->labelInput->hide(), ui->labelOutput->hide();
		ui->lineEditInput->hide(), ui->lineEditOutput->hide();
		ui->buttonFindInput->hide(), ui->buttonFindOutput->hide();
	}

	if (editTime <= -2 || nowTask->getTaskType() == Task::AnswersOnly)
	{
		ui->labelTimeLimit->hide(), ui->labelMemoryLimit->hide();
		ui->lineEditTime->hide(), ui->lineEditMemory->hide();
		ui->label->hide(), ui->label_2->hide();
	}
	else if (editTime == -1)
	{
		defTimeLimit = nowSettings->getDefaultTimeLimit();
		defMemoryLimit = nowSettings->getDefaultMemoryLimit();
	}
	else
	{
		defTimeLimit = editTime;
		defMemoryLimit = editMemory;
	}

	if (editDepend <= -2)ui->labelDepend->hide(), ui->lineEditDepends->hide();


	ui->lineEditScore->setText(QString::number(defScore));
	ui->lineEditTime->setText(QString::number(defTimeLimit));
	ui->lineEditMemory->setText(QString::number(defMemoryLimit));

	QString tempStr = "";

	for (auto i : tempDepends)tempStr = tempStr + QString::number(i) + ",";

	if (!tempStr.isEmpty())tempStr.resize(tempStr.size() - 1);

	ui->lineEditDepends->setText(tempStr);
}

extTestCaseUpdater::~extTestCaseUpdater()
{
	delete ui;
}

void extTestCaseUpdater::whenButtonFindInputClicked()
{
	QString filter = tr("Input Data") + " (";

	auto temp = nowSettings->getInputFileExtensions();

	for (const auto &i : temp)filter = filter + "*." + i + " ";

	*filter.rbegin() = ')';

	filter = filter + ";;" + tr("All Files (*)");

	QString curPath = QDir::currentPath() + nowSettings->dataPath();
	QString filePath = QFileDialog::getOpenFileName(this,
	                   tr("Choose Input File"),
	                   nowSettings->dataPath() + nowTask->getSourceFileName(),
	                   filter);

	if (!filePath.isEmpty())
	{
		QString realPath = filePath.mid(curPath.length() + 1);
		ui->lineEditInput->setText(realPath);
	}
}

void extTestCaseUpdater::whenButtonFindOutputClicked()
{
	QString filter = tr("Output Data") + " (";

	auto temp = nowSettings->getOutputFileExtensions();

	for (const auto &i : temp)filter = filter + "*." + i + " ";

	*filter.rbegin() = ')';

	filter = filter + ";;" + tr("All Files (*)");

	QString curPath = QDir::currentPath() + nowSettings->dataPath();
	QString filePath = QFileDialog::getOpenFileName(this,
	                   tr("Choose Output File"),
	                   nowSettings->dataPath() + nowTask->getSourceFileName(),
	                   filter);

	if (!filePath.isEmpty())
	{
		QString realPath = filePath.mid(curPath.length() + 1);
		ui->lineEditOutput->setText(realPath);
	}
}

void extTestCaseUpdater::accept()
{
	if (editData >= -1 && input.isEmpty())
	{
		QMessageBox::warning(this, tr("Error"), tr("Input File is Empty!"), QMessageBox::Close);
		return;
	}

	if (editData >= -1 && output.isEmpty())
	{
		QMessageBox::warning(this, tr("Error"), tr("Output File is Empty!"), QMessageBox::Close);
		return;
	}

	if (editDepend >= -1 && checkDepends() != 0)
	{
		QMessageBox::warning(this, tr("Error"), tr("Dependence subtask index error!"), QMessageBox::Close);
		return;
	}

	return QDialog::accept();
}

void extTestCaseUpdater::fullScoreChanged(const QString &text)
{
	if (text.isEmpty())ui->lineEditScore->setText(QString::number(defScore));
	else score = text.toInt();
}

void extTestCaseUpdater::timeLimitChanged(const QString &text)
{
	if (text.isEmpty())ui->lineEditTime->setText(QString::number(defTimeLimit));
	else timeLimit = text.toInt();
}

void extTestCaseUpdater::memoryLimitChanged(const QString &text)
{
	if (text.isEmpty())ui->lineEditMemory->setText(QString::number(defMemoryLimit));
	else memoryLimit = text.toInt();
}

void extTestCaseUpdater::inputFileChanged(const QString &text)
{
	input = text;
}

void extTestCaseUpdater::outputFileChanged(const QString &text)
{
	output = text;
}

void extTestCaseUpdater::dependsChanged(const QString &text)
{
	depends = text.isEmpty() ? QStringList() : text.split(',');
}


int extTestCaseUpdater::checkDepends()
{
	QSet<int> hav;

	for (const auto &i_ : qAsConst(depends))
	{
		int i = i_.toInt();

		if (i <= 0 || i >= nowCaseNumber || hav.contains(i))return -1;

		hav.insert(i);
	}

	return 0;
}
