/*
 * SPDX-FileCopyrightText: 2019-2022 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

#include "base/LemonType.hpp"
#include <QObject>
#include <QString>

const int maxDependValue = 1000000;

inline int stateToStatus(ResultState in, int score, int maxScore) {
	if (in == CorrectAnswer)
		return maxDependValue;

	if (in == PartlyCorrect && maxScore == 0)
		return maxDependValue;

	if (score <= 0)
		return -1;

	return 1ll * maxDependValue * score / maxScore;
}

inline int statusToScore(int ratio, int maxScore) { return 1ll * maxScore * ratio / maxDependValue; }

inline QString statusRankingText(int ratio) {
	if (ratio >= maxDependValue)
		return QObject::tr("Pure");

	if (ratio < 0)
		return QObject::tr("Lost");

	return QString::number(100.00 * ratio / maxDependValue, 'f', 3) + "%";
}
