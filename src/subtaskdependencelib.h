/***************************************************************************
    This file is part of Project LemonLime
    Copyright (C) 2020 iotang

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#ifndef SUBTASKDEPENDENCELIB_H
#define SUBTASKDEPENDENCELIB_H

#include "globaltype.h"
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
