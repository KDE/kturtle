/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the Free
	Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.
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
