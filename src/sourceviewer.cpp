/*
 * SPDX-FileCopyrightText: 2026 Project LemonLime
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "sourceviewer.h"
//
#include "base/settings.h"
#include "core/contest.h"
#include "core/contestant.h"
#include "core/task.h"
//
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMessageBox>
#include <QPalette>
#include <QPlainTextEdit>
#include <QSet>
#include <QString>
#include <QStringConverter>
#include <QStringDecoder>
#include <QTabBar>
#include <QTabWidget>
#include <QTextCharFormat>
#include <QVBoxLayout>

namespace {

enum class Language { CLike, Pascal };

struct HighlightPalette {
	QColor keyword;
	QColor number;
	QColor string;
	QColor comment;
	QColor preprocessor;
};

HighlightPalette highlightPalette() {
	const QPalette &palette = QApplication::palette();

	if (palette.color(QPalette::WindowText).lightness() < palette.color(QPalette::Window).lightness()) {
		// Dark background
		return {QColor(0x6f, 0xa8, 0xff), QColor(0xe8, 0x7c, 0xb4), QColor(0x7c, 0xd6, 0x7c),
		        QColor(0x8a, 0x8a, 0x8a), QColor(0xe0, 0xa0, 0x60)};
	}

	// Light background
	return {QColor(0x00, 0x00, 0xc0), QColor(0xb0, 0x00, 0x60), QColor(0x00, 0x70, 0x00),
	        QColor(0x80, 0x80, 0x80), QColor(0x80, 0x40, 0x00)};
}

QTextCharFormat makeFormat(const QColor &color) {
	QTextCharFormat format;
	format.setForeground(color);
	return format;
}

} // namespace

class SourceHighlighter final : public QSyntaxHighlighter {
  public:
	explicit SourceHighlighter(QTextDocument *document) : QSyntaxHighlighter(document) {
		const HighlightPalette palette = highlightPalette();
		keywordFormat = makeFormat(palette.keyword);
		numberFormat = makeFormat(palette.number);
		stringFormat = makeFormat(palette.string);
		commentFormat = makeFormat(palette.comment);
		preprocessorFormat = makeFormat(palette.preprocessor);
	}

	void setLanguage(Language newLanguage) { language = newLanguage; }

  protected:
	void highlightBlock(const QString &text) override {

		const QSet<QString> &keywords = (language == Language::Pascal) ? pascalKeywords() : cKeywords();

		int index = 0;
		const int length = text.length();

		// Continue a multi-line comment started in a previous block: 1 = /* */ or { }, 2 = (* *)
		if (previousBlockState() == 1 || previousBlockState() == 2) {
			const QString endMark = previousBlockState() == 1
			                            ? (language == Language::Pascal ? QStringLiteral("}") : QStringLiteral("*/"))
			                            : QStringLiteral("*)");
			const int end = text.indexOf(endMark);

			if (end < 0) {
				setFormat(0, length, commentFormat);
				setCurrentBlockState(previousBlockState());
				return;
			}

			setFormat(0, end + endMark.length(), commentFormat);
			index = end + endMark.length();
		}

		const bool isPascal = language == Language::Pascal;

		while (index < length) {
			const QChar current = text.at(index);

			// Line comments
			if (current == QLatin1Char('/') && index + 1 < length && text.at(index + 1) == QLatin1Char('/')) {
				setFormat(index, length - index, commentFormat);
				break;
			}

			// Multi-line comment starts
			if (isPascal && current == QLatin1Char('{')) {
				const int end = text.indexOf(QLatin1Char('}'), index + 1);

				if (end < 0) {
					setFormat(index, length - index, commentFormat);
					setCurrentBlockState(1);
					break;
				}

				setFormat(index, end - index + 1, commentFormat);
				index = end + 1;
				continue;
			}

			if (isPascal && current == QLatin1Char('(') && index + 1 < length &&
			    text.at(index + 1) == QLatin1Char('*')) {
				const int end = text.indexOf(QLatin1String("*)"), index + 2);

				if (end < 0) {
					setFormat(index, length - index, commentFormat);
					setCurrentBlockState(2);
					break;
				}

				setFormat(index, end - index + 2, commentFormat);
				index = end + 2;
				continue;
			}

			if (! isPascal && current == QLatin1Char('/') && index + 1 < length &&
			    text.at(index + 1) == QLatin1Char('*')) {
				const int end = text.indexOf(QLatin1String("*/"), index + 2);

				if (end < 0) {
					setFormat(index, length - index, commentFormat);
					setCurrentBlockState(1);
					break;
				}

				setFormat(index, end - index + 2, commentFormat);
				index = end + 2;
				continue;
			}

			// Strings ('...' for Pascal, "..." and '...' for C-like)
			if (current == QLatin1Char('"') || current == QLatin1Char('\'')) {
				int end = index + 1;

				while (end < length) {
					if (! isPascal && text.at(end) == QLatin1Char('\\')) {
						end += 2;
						continue;
					}

					if (isPascal && text.at(end) == QLatin1Char('\'') && end + 1 < length &&
					    text.at(end + 1) == QLatin1Char('\'')) {
						end += 2;
						continue;
					}

					if (text.at(end) == current) {
						++end;
						break;
					}

					++end;
				}

				setFormat(index, qMin(end, length) - index, stringFormat);
				index = qMin(end, length);
				continue;
			}

			// Numbers
			if (current.isDigit()) {
				int end = index + 1;

				while (end < length && (text.at(end).isDigit() || text.at(end) == QLatin1Char('.') ||
				                        (text.at(end).toLower() >= QLatin1Char('a') &&
				                         text.at(end).toLower() <= QLatin1Char('f')) ||
				                        text.at(end).toLower() == QLatin1Char('x') ||
				                        text.at(end).toLower() == QLatin1Char('u') ||
				                        text.at(end).toLower() == QLatin1Char('l'))) {
					++end;
				}

				setFormat(index, end - index, numberFormat);
				index = end;
				continue;
			}

			// Identifiers and keywords
			if (current.isLetter() || current == QLatin1Char('_')) {
				int end = index + 1;

				while (end < length &&
				       (text.at(end).isLetterOrNumber() || text.at(end) == QLatin1Char('_'))) {
					++end;
				}

				if (keywords.contains(text.mid(index, end - index)))
					setFormat(index, end - index, keywordFormat);

				index = end;
				continue;
			}

			// Preprocessor directives (C-like, first non-whitespace of the line)
			if (! isPascal && current == QLatin1Char('#')) {
				int start = index;

				while (start > 0 && (text.at(start - 1) == QLatin1Char(' ') ||
				                     text.at(start - 1) == QLatin1Char('\t'))) {
					--start;
				}

				if (text.left(start).trimmed().isEmpty()) {
					setFormat(index, length - index, preprocessorFormat);
					break;
				}
			}

			++index;
		}
	}

  private:
	static const QSet<QString> &cKeywords() {
		static const QSet<QString> keywords = {
		    "alignas", "alignof", "and", "asm", "auto", "bool", "break", "case", "catch", "char",
		    "char8_t", "char16_t", "char32_t", "class", "co_await", "co_return", "co_yield", "concept",
		    "const", "consteval", "constexpr", "constinit", "const_cast", "continue", "decltype",
		    "default", "delete", "do", "double", "dynamic_cast", "else", "enum", "explicit", "export",
		    "extern", "false", "float", "for", "friend", "goto", "if", "inline", "int", "java", "long",
		    "mutable", "namespace", "new", "noexcept", "not", "nullptr", "operator", "override",
		    "private", "protected", "public", "register", "reinterpret_cast", "requires", "return",
		    "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct", "switch",
		    "template", "this", "throw", "true", "try", "typedef", "typeid", "typename", "union",
		    "unsigned", "using", "virtual", "void", "volatile", "wchar_t", "while", "final", "package",
		    "implements", "import", "interface", "boolean", "extends"};
		return keywords;
	}

	static const QSet<QString> &pascalKeywords() {
		static const QSet<QString> keywords = {
		    "and", "array", "begin", "case", "const", "div", "do", "downto", "else", "end", "file",
		    "for", "function", "goto", "if", "in", "label", "mod", "nil", "not", "of", "or",
		    "procedure", "program", "record", "repeat", "set", "shl", "shr", "string", "then", "to",
		    "type", "until", "uses", "var", "while", "with", "xor", "boolean", "byte", "char",
		    "integer", "longint", "real", "word", "read", "readln", "write", "writeln"};
		return keywords;
	}

	Language language = Language::CLike;
	QTextCharFormat keywordFormat;
	QTextCharFormat numberFormat;
	QTextCharFormat stringFormat;
	QTextCharFormat commentFormat;
	QTextCharFormat preprocessorFormat;
};

QString SourceViewerDialog::contestantSourceFilePath(Contest *contest, Contestant *contestant, int taskIndex,
                                                     int fileIndex) {
	const Task *task = contest->getTask(taskIndex);

	if (task == nullptr || task->getTaskType() == Task::AnswersOnly)
		return {};

	// Communication tasks store a space-separated list of submitted files
	const QStringList files =
	    contestant->getSourceFile(taskIndex).split(' ', Qt::SkipEmptyParts);

	if (fileIndex < 0 || fileIndex >= files.size())
		return {};

	const QString &fileName = files[fileIndex];

	if (fileName.isEmpty())
		return {};

	QString path = Settings::sourcePath() + contestant->getContestantName();

	if (task->getSubFolderCheck())
		path += QDir::separator() + task->getSourceFileName();

	path += QDir::separator() + fileName;
	return path;
}

SourceViewerDialog::SourceViewerDialog(QWidget *parent, const QStringList &filePaths, const QString &heading)
    : QDialog(parent) {
	setModal(true);
	setWindowTitle(heading);
	resize(760, 560);

	auto *layout = new QVBoxLayout(this);
	layout->setContentsMargins(8, 8, 8, 8);

	tabWidget = new QTabWidget(this);
	layout->addWidget(tabWidget);

	if (filePaths.size() == 1)
		tabWidget->tabBar()->hide();

	for (const QString &filePath : filePaths)
		addEditorTab(filePath);
}

void SourceViewerDialog::addEditorTab(const QString &filePath) {
	auto *editor = new QPlainTextEdit(tabWidget);
	editor->setReadOnly(true);
	editor->setLineWrapMode(QPlainTextEdit::NoWrap);
	editor->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

	QFile file(filePath);

	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		const QByteArray data = file.readAll();
		QString content = QString::fromUtf8(data);

		// Fall back to the system's local encoding when the file is not valid UTF-8
		if (content.contains(QChar(0xfffd))) {
			QStringDecoder decoder(QStringConverter::System);
			content = decoder(data);
		}

		editor->setPlainText(content);
	}

	auto *highlighter = new SourceHighlighter(editor->document());
	const QString suffix = QFileInfo(filePath).suffix().toLower();
	highlighter->setLanguage(suffix == "pas" || suffix == "pp" || suffix == "dpr" ? Language::Pascal
	                                                                              : Language::CLike);
	highlighters.append(highlighter);

	tabWidget->addTab(editor, QFileInfo(filePath).fileName());
}

void SourceViewerDialog::viewSource(QWidget *parent, Contest *contest, Contestant *contestant, int taskIndex) {
	const Task *task = contest->getTask(taskIndex);
	QStringList filePaths;

	for (int i = 0;; i++) {
		const QString path = contestantSourceFilePath(contest, contestant, taskIndex, i);

		if (path.isEmpty())
			break;

		filePaths.append(path);
	}

	if (filePaths.isEmpty() || task == nullptr) {
		QMessageBox::information(parent, tr("View Source"), tr("Cannot find the source file of this task."));
		return;
	}

	for (const QString &path : filePaths) {
		if (! QFile::exists(path)) {
			QMessageBox::information(parent, tr("View Source"),
			                         tr("Source file %1 does not exist.").arg(QDir::toNativeSeparators(path)));
			return;
		}
	}

	const QString heading =
	    tr("Source Code: %1 - %2").arg(contestant->getContestantName(), task->getProblemTitle());
	SourceViewerDialog dialog(parent, filePaths, heading);
	dialog.exec();
}
