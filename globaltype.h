/***************************************************************************
    This file is part of Project Lemon
    Copyright (C) 2011 Zhipeng Jia

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
/**
 * globaltype.h @Project Lemon+
 * Update 2018 Dust1404
 **/
/**
 * globaltype.h @Project LemonLime
 * Update 2019 iotang
 **/

#ifndef GLOBALTYPE_H
#define GLOBALTYPE_H

enum CompileState { CompileSuccessfully, NoValidSourceFile, CompileError,
                    CompileTimeLimitExceeded, InvalidCompiler
                  };

enum ResultState { CorrectAnswer, WrongAnswer, PartlyCorrect,
                   TimeLimitExceeded, MemoryLimitExceeded,
                   CannotStartProgram, FileError, RunTimeError,
                   InvalidSpecialJudge, SpecialJudgeTimeLimitExceeded,
                   SpecialJudgeRunTimeError, Skipped, InteractorError
                 };

const int oriBaseColorHI = 120;
const double oriBaseColorHF = 1.00 / 3.00;
const double oriBaseColorSF = 0.50; // 0.30

const int nofBaseColorHI = 0;
const double nofBaseColorHF = 0.00;
const double nofBaseColorSF = 0.00;
const double nofBaseColorLF = 5.50 / 6.00;

const int cmeBaseColorHI = 300;
const double cmeBaseColorHF = 5.00 / 6.00;
const double cmeBaseColorSF = 1.00;
const double cmeBaseColorLF = 5.00 / 6.00;

inline double oriBaseColorLF(int score, int mxscore, double rem = 0.30)
{
	if (mxscore == 0)return 1.00;

	return 1.00 - 1.25 * rem * score / mxscore
	       - 0.03 * (score > 0)
	       - 0.02 * (score >= mxscore);
}




#endif // GLOBALTYPE_H
