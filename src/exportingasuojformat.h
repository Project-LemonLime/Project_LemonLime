#ifndef EXPORTINGASUOJFORMAT_H
#define EXPORTINGASUOJFORMAT_H

#include <QAction>
#include <QDialog>

namespace Ui {
	class ExportingAsUojFormat;
}

class Task;

class ExportingAsUojFormat : public QDialog {
	Q_OBJECT

  public:
	explicit ExportingAsUojFormat(QWidget *parent = nullptr);
	~ExportingAsUojFormat();

	void init(Task *task);
	void exportData();

  public slots:
	void specialJudgeChanged(int);

  private:
	Ui::ExportingAsUojFormat *ui;
	Task *task;
	bool copyFile(QString, QString);
};

#endif // EXPORTINGASUOJFORMAT_H
