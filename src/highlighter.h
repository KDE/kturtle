/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

	This program is free software; you can redistribute it and/or
	modify it under the terms of version 2 of the GNU General Public
	License as published by the Free Software Foundation.

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

#include <QHash>
#include <QTextCharFormat>

#include "interpreter/tokenizer.h"


class QTextDocument;

class Highlighter : public QSyntaxHighlighter
{
	Q_OBJECT

	public:
		Highlighter(QTextDocument *parent = 0);
		~Highlighter() {}

		Token* formatType(const QString &text, int cursorIndex) { return checkOrApplyHighlighting(text, cursorIndex); }


	protected:
		void highlightBlock(const QString &text) { checkOrApplyHighlighting(text); }


	private:
		Token* checkOrApplyHighlighting(const QString &text, int cursorIndex = -1);

		Tokenizer *tokenizer;

		QTextCharFormat variableFormat;
		QTextCharFormat trueFalseFormat;
		QTextCharFormat commentFormat;
		QTextCharFormat stringFormat;
		QTextCharFormat numberFormat;
		QTextCharFormat scopeFormat;
		QTextCharFormat controllerCommandFormat;
		QTextCharFormat otherCommandFormat;
		QTextCharFormat learnCommandFormat;
};


#endif  // _HIGHLIGHTER_H_
