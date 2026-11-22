/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QDialog>
#include <QSyntaxHighlighter>

class Contest;
class Contestant;
class SourceHighlighter;
class QTabWidget;

class SourceViewerDialog : public QDialog {
	Q_OBJECT

  public:
	/** Resolve the path of one submitted source file, mirroring TaskJudger's layout. */
	static QString contestantSourceFilePath(Contest *contest, Contestant *contestant, int taskIndex,
	                                        int fileIndex);
	/** Resolve, load and display the source files of contestant's taskIndex-th task. */
	static void viewSource(QWidget *parent, Contest *contest, Contestant *contestant, int taskIndex);

  private:
	explicit SourceViewerDialog(QWidget *parent, const QStringList &filePaths, const QString &heading);
	void addEditorTab(const QString &filePath);

	QTabWidget *tabWidget{};
	QList<SourceHighlighter *> highlighters;
};
