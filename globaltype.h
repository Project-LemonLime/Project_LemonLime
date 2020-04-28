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
                   SpecialJudgeRunTimeError, Skipped, InteractorError,
                   PresentationError, OutputLimitExceeded
                 };

inline int stateToStatus(ResultState in)
{
	if (in == CorrectAnswer) return 2;

	if (in == PartlyCorrect) return 1;

	return 0;
}


#endif // GLOBALTYPE_H
