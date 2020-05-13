#ifndef EXTTESTCASEUPDATER_H
#define EXTTESTCASEUPDATER_H

#include <QDialog>
#include "task.h"
#include "testcase.h"
#include "settings.h"

namespace Ui
{
	class extTestCaseUpdater;
}

class extTestCaseUpdater : public QDialog
{
	Q_OBJECT

public:
	explicit extTestCaseUpdater(QWidget *parent = nullptr, Task *nowTask = nullptr, const Settings *nowSettings = nullptr, int nowCaseNumber = 0, int editScore = 0, int editData = 0, int editTime = 0, int editMemory = 0, int editDepend = 0, QList<int> tempDepends = QList<int>());
	~extTestCaseUpdater();

	int getScore()
	{
		return score;
	}

	QString getInput()
	{
		return input;
	}

	QString getOutput()
	{
		return output;
	}

	int getTimeLimit()
	{
		return timeLimit;
	}

	int getMemoryLimit()
	{
		return memoryLimit;
	}

	QStringList getDepends()
	{
		return depends;
	}

	int checkDepends();

protected:
	virtual void accept() override;

private:
	Ui::extTestCaseUpdater *ui;
	Task *nowTask;
	const Settings *nowSettings;
	int nowCaseNumber;
	int editScore;
	int editData;
	int editTime;
	int editMemory;
	int editDepend;


	int score;
	QString input, output;
	int timeLimit, memoryLimit;
	QStringList depends;

	int defScore, defTimeLimit, defMemoryLimit;

private slots:

	void whenButtonFindInputClicked();
	void whenButtonFindOutputClicked();
	void fullScoreChanged(const QString &);
	void timeLimitChanged(const QString &);
	void memoryLimitChanged(const QString &);
	void inputFileChanged(const QString &);
	void outputFileChanged(const QString &);
	void dependsChanged(const QString &);

};

#endif // EXTTESTCASEUPDATER_H
