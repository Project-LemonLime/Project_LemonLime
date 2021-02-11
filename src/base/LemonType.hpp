/*
 * SPDX-FileCopyrightText: 2020-2021 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#pragma once
//

enum CompileState {
	CompileSuccessfully,
	NoValidSourceFile,
	CompileError,
	CompileTimeLimitExceeded,
	InvalidCompiler
};

enum ResultState {
	CorrectAnswer,
	WrongAnswer,
	PartlyCorrect,
	TimeLimitExceeded,
	MemoryLimitExceeded,
	CannotStartProgram,
	FileError,
	RunTimeError,
	InvalidSpecialJudge,
	SpecialJudgeTimeLimitExceeded,
	SpecialJudgeRunTimeError,
	Skipped,
	InteractorError,
	PresentationError,
	OutputLimitExceeded
};
