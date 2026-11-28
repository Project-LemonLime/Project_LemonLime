/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include <QObject>

class Contest;
class Contestant;
class QWidget;

// Opens a contestant's submitted sources with the user's own editor via the
// platform file association, instead of embedding a syntax-highlighting viewer.
class SourceViewer final : public QObject {
	Q_OBJECT

  public:
	/** Open the source file(s) of the contestant's taskIndex-th task in the system editor. */
	static void viewSource(QWidget *parent, Contest *contest, Contestant *contestant, int taskIndex);
};
