#ifndef EXTTESTCASEMODIFIER_H
#define EXTTESTCASEMODIFIER_H

#include <QWidget>
#include "contest.h"
#include "contestant.h"
#include "settings.h"
#include "task.h"

namespace Ui
{
	class extTestCaseModifier;
}

class extTestCaseModifier : public QWidget
{
	Q_OBJECT

public:
	explicit extTestCaseModifier(QWidget *parent = nullptr);
	~extTestCaseModifier();

	void refresh();
	void init(Task *, const Settings *);

private:
	Ui::extTestCaseModifier *ui;

	Task *editTask;
	const Settings *editSettings;

private slots:
	void whenTestCaseSelectionChanged();

	void modifySelected();
	void moveUpSelected();
	void moveDownSelected();
	void removeSelected();
	void mergeSelected();
	void splitSelected();

	void appendNewSub();
	void appendNewCase();
};

#endif // EXTTESTCASEMODIFIER_H
