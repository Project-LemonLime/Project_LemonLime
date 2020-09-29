/*
 * SPDX-FileCopyrightText: 2019  Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef SUBTASKDEPENDENCELIB_H
#define SUBTASKDEPENDENCELIB_H

#include "base/LemonType.hpp"
#include <QObject>
#include <QString>

const int mxDependValue = 1000000;

inline int stateToStatus(ResultState in, int score, int mxScore)
{
	if (in == CorrectAnswer)
		return mxDependValue;

	if (in == PartlyCorrect && mxScore == 0)
		return mxDependValue;

	if (score <= 0)
		return -1;

	return 1ll * mxDependValue * score / mxScore;
}

inline int statusToScore(int ratio, int mxScore) { return 1ll * mxScore * ratio / mxDependValue; }

inline QString statusRankingText(int ratio)
{
	if (ratio >= mxDependValue)
		return QObject::tr("Pure");

	if (ratio < 0)
		return QObject::tr("Lost");

	return QString::number(100.00 * ratio / mxDependValue, 'f', 3) + "%";
}

#endif // SUBTASKDEPENDENCELIB_H
