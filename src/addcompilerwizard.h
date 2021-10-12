/*
 * SPDX-FileCopyrightText: 2018-2019 Project LemonPlus, Dust1404
 * SPDX-FileCopyrightText: 2019-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QList>
#include <QWizard>

namespace Ui {
	class AddCompilerWizard;
}

class Compiler;

class AddCompilerWizard : public QWizard {
	Q_OBJECT

  public:
	explicit AddCompilerWizard(QWidget *parent = nullptr);
	~AddCompilerWizard();
	void accept();
	const QList<Compiler *> &getCompilerList() const;

  private:
	Ui::AddCompilerWizard *ui;
	QList<Compiler *> compilerList;
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
