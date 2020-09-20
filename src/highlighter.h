/*
	SPDX-FileCopyrightText: 2003-2006 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _HIGHLIGHTER_H_
#define _HIGHLIGHTER_H_

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

#include "interpreter/tokenizer.h"


class Highlighter : public QSyntaxHighlighter
{
	Q_OBJECT

	public:
        explicit Highlighter(QTextDocument *parent = nullptr);
		~Highlighter();

		/// used by the Editor for highlighting
		Token* formatType(const QString &text, int cursorIndex) { return checkOrApplyHighlighting(text, cursorIndex); }

		/// used by the Inspector to give the text format for a single statement (first in the text)
		QTextCharFormat* formatForStatement(const QString &text);

		/// used by internally and by the Inspector
		QTextCharFormat* tokenToFormat(Token* token);

	protected:
		void highlightBlock(const QString &text) Q_DECL_OVERRIDE { checkOrApplyHighlighting(text); }

	private:
		Token* checkOrApplyHighlighting(const QString &text, int cursorIndex = -1);

		Tokenizer* tokenizer;

		QTextCharFormat variableFormat;
		QTextCharFormat trueFalseFormat;
		QTextCharFormat commentFormat;
		QTextCharFormat stringFormat;
		QTextCharFormat numberFormat;
		QTextCharFormat scopeFormat;
		QTextCharFormat controllerCommandFormat;
		QTextCharFormat otherCommandFormat;
		QTextCharFormat learnCommandFormat;
		QTextCharFormat booleanOperatorFormat;
		QTextCharFormat expressionFormat;
		QTextCharFormat assignmentFormat;
		QTextCharFormat mathOperatorFormat;
};


#endif  // _HIGHLIGHTER_H_
