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
	InvalidCompiler,
	NoValidGraderFile
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

#include <QList>
#include <QString>
#include <QStringList>

/* struct TaskResult {
    CompileState compileState;
    QString compileMessage;
    QString sourceFile;
    QList<QList<ResultState>> resultState;
    QList<QStringList> resultMessage;
    QList<QList<int>> scores;
    QList<QList<int>> timeUsed;
    QList<QList<int>> memoryUsed;
    QList<QStringList> inputFiles;
}; */