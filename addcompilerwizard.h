/**
 * addcompilerwizard.h @Project Lemon+
 * Update 2018 Dust1404
 **/

#ifndef ADDCOMPILERWIZARD_H
#define ADDCOMPILERWIZARD_H

#include <QtCore>
#include <QtGui>
#include <QWizard>

namespace Ui {
    class AddCompilerWizard;
}

class Compiler;

class AddCompilerWizard : public QWizard
{
    Q_OBJECT

public:
    explicit AddCompilerWizard(QWidget *parent = 0);
    ~AddCompilerWizard();
    void accept();
    const QList<Compiler*>& getCompilerList() const;

private:
    Ui::AddCompilerWizard *ui;
    QList<Compiler*> compilerList;
    int nextId() const;
    bool validateCurrentPage();

private slots:
    void compilerTypeChanged();
    void selectCompilerLocation();
    void selectInterpreterLocation();
    void selectGccPath();
    void selectGppPath();
    void selectFpcPath();
    void selectFbcPath();
    void selectJavacPath();
    void selectJavaPath();
    void selectPythonPath();
};

#endif // ADDCOMPILERWIZARD_H
