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

#include "highlighter.h"

#include "interpreter/tokenizer.h"

#include <kdebug.h>
#include <kurl.h>


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

Highlighter::~Highlighter()
{
	delete tokenizer;
}


QTextCharFormat* Highlighter::formatForStatement(const QString &text)
{
	// TODO store the Token (pointer) in the respective inspector lists so we don't have to re-tokenize here...
	tokenizer->initialize(text);
	return tokenToFormat(tokenizer->getToken());
}

Token* Highlighter::checkOrApplyHighlighting(const QString& text, int cursorIndex)
{
	tokenizer->initialize(text);

	QTextCharFormat* format;
	Token* token = tokenizer->getToken();
	while (token->type() != Token::EndOfInput) {
		format = tokenToFormat(token);
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

QTextCharFormat* Highlighter::tokenToFormat(Token* token)
{
	switch (token->category()) {
		case Token::VariableCategory:          return &variableFormat;
		case Token::TrueFalseCategory:         return &trueFalseFormat;
		case Token::NumberCategory:            return &numberFormat;
		case Token::CommentCategory:           return &commentFormat;
		case Token::StringCategory:            return &stringFormat;
		case Token::ScopeCategory:             return &scopeFormat;
		case Token::CommandCategory:           return &otherCommandFormat;
		case Token::ControllerCommandCategory: return &controllerCommandFormat;
		case Token::LearnCommandCategory:      return &learnCommandFormat;

		// do nothing with these... yet.
		case Token::MathOperatorCategory:      return 0;
		case Token::ParenthesisCategory:       return 0;
		case Token::FunctionCallCategory:      return 0;
		case Token::ExpressionCategory:        return 0;
		case Token::AssignmentCategory:        return 0;
		case Token::ArgumentSeparatorCategory: return 0;
		case Token::BooleanOperatorCategory:   return 0;
	}
	return 0;
}


#include "highlighter.moc"
