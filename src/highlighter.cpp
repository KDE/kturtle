/*
	Copyright (C) 2003-2006 Cies Breijs <cies # kde ! nl>

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

#include <QtGui>
#include <QTextOStream>

#include <kdebug.h>
#include <kurl.h>

#include "interpreter/tokenizer.h"

#include "highlighter.h"


Highlighter::Highlighter(QTextDocument *parent)
	: QSyntaxHighlighter(parent)
{
	variableFormat.setFontWeight(QFont::Bold);
	variableFormat.setForeground(Qt::darkMagenta);

	trueFalseFormat.setForeground(Qt::darkRed);

	numberFormat.setForeground(Qt::darkRed);

	commentFormat.setForeground(Qt::gray);

	scopeFormat.setFontWeight(QFont::Bold);
	scopeFormat.setForeground(Qt::darkGreen);

	stringFormat.setForeground(Qt::red);

	controllerCommandFormat.setFontWeight(QFont::Bold);
	controllerCommandFormat.setForeground(Qt::black);

	otherCommandFormat.setForeground(Qt::darkBlue);

	learnCommandFormat.setFontWeight(QFont::Bold);
	learnCommandFormat.setForeground(Qt::green);

	tokenizer = new Tokenizer();
}

Token* Highlighter::checkOrApplyHighlighting(const QString& text, int cursorIndex)
{
	tokenizer->initialize(text);

	QTextCharFormat* format;
	Token* token = tokenizer->getToken();
	while (token->type() != Token::EndOfInput) {
		format = 0;
		switch (token->category()) {
				case Token::VariableCategory:          format = &variableFormat;          break;
				case Token::TrueFalseCategory:         format = &trueFalseFormat;         break;
				case Token::NumberCategory:            format = &numberFormat;            break;
				case Token::CommentCategory:           format = &commentFormat;           break;
				case Token::StringCategory:            format = &stringFormat;            break;
				case Token::ScopeCategory:             format = &scopeFormat;             break;
				case Token::CommandCategory:           format = &otherCommandFormat;      break;
				case Token::ControllerCommandCategory: format = &controllerCommandFormat; break;
				case Token::LearnCommandCategory:      format = &learnCommandFormat;      break;
	
				case Token::MetaCategory:
				case Token::MathOperatorCategory:
				case Token::WhiteSpaceCategory:
				case Token::ParenthesisCategory:
				case Token::DecimalSeparatorCategory:
				case Token::FunctionCallCategory:
				case Token::ExpressionCategory:
				case Token::AssignmentCategory:
				case Token::ArgumentSeparatorCategory:
				case Token::BooleanOperatorCategory:
					// do nothing with these... yet.
					break;
		}
		if (format != 0) {
			if (cursorIndex == -1)
				setFormat(token->startCol() - 1, token->endCol() - token->startCol(), *format);
			else if (cursorIndex >= token->startCol() && cursorIndex <= token->endCol())
				return token;
		}
		delete token;
		token = tokenizer->getToken();
	}
	delete token;
	return 0;
}


#include "highlighter.moc"
