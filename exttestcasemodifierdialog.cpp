#include "exttestcasemodifierdialog.h"
#include "ui_exttestcasemodifierdialog.h"
#include "task.h"
#include <QtWidgets>

extTestCaseModifierDialog::extTestCaseModifierDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::extTestCaseModifierDialog)
{
	ui->setupUi(this);
	setWindowTitle(QString(tr("Advanced Test Case Modifier")));
	connect(ui->buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
	        this, SLOT(accept()));
	connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
	        this, SLOT(reject()));
}

extTestCaseModifierDialog::~extTestCaseModifierDialog()
{
	delete ui;
}

void extTestCaseModifierDialog::init(Task *theTask, const Settings *theSettings)
{
	editTask = new Task;

	theTask->copyTo(editTask);

	ui->widget->init(editTask, theSettings);
}

Task *extTestCaseModifierDialog::getEditTask()
{
	return editTask;
}
