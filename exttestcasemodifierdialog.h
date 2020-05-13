#ifndef EXTTESTCASEMODIFIERDIALOG_H
#define EXTTESTCASEMODIFIERDIALOG_H

#include <QDialog>
#include "task.h"
#include "settings.h"

namespace Ui
{
	class extTestCaseModifierDialog;
}

class extTestCaseModifierDialog : public QDialog
{
	Q_OBJECT

public:
	explicit extTestCaseModifierDialog(QWidget *parent = nullptr);
	~extTestCaseModifierDialog();

	void init(Task *, const Settings *);
	Task *getEditTask();

private:
	Ui::extTestCaseModifierDialog *ui;

	Task *editTask;
};

#endif // EXTTESTCASEMODIFIERDIALOG_H
