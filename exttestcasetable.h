#ifndef EXTTESTCASETABLE_H
#define EXTTESTCASETABLE_H

#include <QTableWidget>
#include "task.h"
#include "testcase.h"

class extTestCaseTable : public QTableWidget
{
	Q_OBJECT
public:
	extTestCaseTable(QWidget *parent);

	void refreshTask(Task *);

	int canModify()
	{
		return isCanModify;
	}
	int canAddSub()
	{
		return isCanAddSub;
	}
	int canAddCase()
	{
		return isCanAddCase;
	}
	int canRemove()
	{
		return isCanRemove;
	}
	int canUp()
	{
		return isCanUp;
	}
	int canDown()
	{
		return isCanDown;
	}
	int canMerge()
	{
		return isCanMerge;
	}
	int canSplit()
	{
		return isCanSplit;
	}

	QList<int> getSelectedHaveSub()
	{
		return haveSub;
	}

	QList<QPair<int, QPair<int, int>>> getSelectedResSub()
	{
		return resSub;
	}

	QPair<int, int> getSelectRange()
	{
		return qMakePair(selectMi, selectMx);
	}

	void modifySelected(int, int);

private:

	Task *editTask;

	void addItem(int row, int column, const QString &text)
	{
		QTableWidgetItem *item = new QTableWidgetItem(text);
		item->setTextAlignment(Qt::AlignCenter);
		item->setToolTip(text);
		this->setItem(row, column, item);
	}

	void addItem(int row, int column, const QString &text, const QString &tipText)
	{
		QTableWidgetItem *item = new QTableWidgetItem(text);
		item->setTextAlignment(Qt::AlignCenter);
		item->setToolTip(tipText);
		this->setItem(row, column, item);
	}

	QList<int> haveSub;
	QList<QPair<int, QPair<int, int>>> resSub;

	int isCanModify;
	int isCanAddSub;
	int isCanAddCase;
	int isCanRemove;
	int isCanUp;
	int isCanDown;
	int isCanMerge;
	int isCanSplit;
	int selectMi;
	int selectMx;
	int noDfs;

private slots:
	void whenItemSelectionChanged();

signals:
	void testCaseSelectionChanged();
};

#endif // EXTTESTCASETABLE_H
