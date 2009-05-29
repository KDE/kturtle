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


// #include <kdebug.h>

#include "tokenizer.h"


void Tokenizer::initialize(const QString& inString)
{
	translator  = Translator::instance();
	inputString = inString + '\n';  // the certainty of a hard break at the end makes parsing much easier
	at  = 0;
	row = 1;
	col = 1;
	prevCol = 1;
	atEnd = false;
}


Token* Tokenizer::getToken()
{
// 	kDebug() << "Tokenizer::getToken()";

	int startRow = row;
	int startCol = col;

	QChar c = getChar();  // get and store the next character from the string

	// catch the end of the string
	if (atEnd)
		return new Token(Token::EndOfInput, "END", row, col, row, col);

	int cType = translator->look2type(c);  // since we need to know it often we store it

	// catch spaces
	if (isSpace(c)) {
		QString look;
		do {
			look += c;
			c = getChar();
		} while (c.category() == QChar::Separator_Space && !atEnd);
		ungetChar();
		return new Token(Token::WhiteSpace, look, startRow, startCol, row, col);
	}

	// catch EndOfLine's
	if (isBreak(c)) {
		return new Token(Token::EndOfLine, "\\n", startRow, startCol, startRow+1, 1);
	}

	// catch comments
	if (cType == Token::Comment) {
		QString look;
		do {
			look += c;
			c = getChar();
		} while (!isBreak(c) && !atEnd);
		ungetChar();
		return new Token(Token::Comment, look, startRow, startCol, row, col);
	}

	// catch strings
	if (cType == Token::StringDelimiter) {
		QString look = QString(c);
		do {
			c = getChar();
			look += c;
		} while (!(translator->look2type(c) == Token::StringDelimiter && look.right(2) != "\\\"") &&
		         !isBreak(c) &&
		         !atEnd);
		return new Token(Token::String, look, startRow, startCol, row, col);
	}

	// catch variables
	if (cType == Token::VariablePrefix) {
		QString look;
		do {
			look += c;
			c = getChar();
		} while (c.isLetter() ||
		         c.category() == QChar::Number_DecimalDigit ||
		         c == '_');
		ungetChar();
		return new Token(Token::Variable, look, startRow, startCol, row, col);
	}

	// catch words (known or unknown function calls)
	if (c.isLetter()) {
		QString look;
		do {
			look += c;
			c = getChar();
		} while (c.isLetter() || c.isDigit() || c == '_');
		ungetChar();
		return new Token(translator->look2type(look), look, startRow, startCol, row, col);
	}

	// catch numbers
	if (c.isDigit() || cType == Token::DecimalSeparator) {
		bool hasDot = false;

		int localType = cType;
		QString look;
		do {
			if (localType == Token::DecimalSeparator) hasDot = true;
			look += c;
			c = getChar();
			localType = translator->look2type(c);
		} while (c.isDigit() || (localType == Token::DecimalSeparator && !hasDot));
		ungetChar();
		
		// if all we got is a dot then this is not a number, so return an Error token here
		if (translator->look2type(look) == Token::DecimalSeparator)
			return new Token(Token::Error, look, startRow, startCol, row, col);
		
		return new Token(Token::Number, look, startRow, startCol, row, col);
	}

	// catch previously uncatched 'double charactered tokens' (tokens that ar not in letters, like: == != >= <=)
	{
		QString look = QString(c).append(getChar());
		int localType = translator->look2type(look);
		if (localType != Token::Unknown)
			return new Token(localType, look, startRow, startCol, row, col);
		ungetChar();
	}

	// catch known tokens of a single character (as last...)
	if (cType != Token::Unknown)
		return new Token(cType, static_cast<QString>(c), startRow, startCol, row, col);

	// fall throu: return an Error token (TODO does this neglect used def'ed function names of one character?)
	return new Token(Token::Error, static_cast<QString>(c), startRow, startCol, row, col);
}


QChar Tokenizer::getChar()
{
	if (at >= inputString.size()) {
		atEnd = true;
// 		kDebug() << "Tokenizer::getChar() returns: a ZERO CHAR " << " @ " << at - 1;
		return QChar();
	}
	QChar c(inputString.at(at));
	at++;
	if (isBreak(c)) {
		row++;
		prevCol = col;
		col = 1;
	} else {
		col++;
	}
// 	kDebug() << "Tokenizer::getChar() returns: " << c << " @ " << at - 1;
	return c;
}


void Tokenizer::ungetChar()
{
	if (at <= 0) return;  // do nothing when trying to go before the first character

	at--;
	if (atEnd) atEnd = false;

	QChar c(inputString.at(at));
	if (isBreak(c)) {
		row--;
		col = prevCol;
	} else {
		col--;
	}
}


bool Tokenizer::isBreak(const QChar& c)
{
	return (c == '\x0a' || c == '\n');
// 	  c.category() == QChar::Other_Control  // one of these also contains the tab (\t)
// 	  c.category() == QChar::Separator_Line
// 	  c.category() == QChar::Separator_Paragraph
}


bool Tokenizer::isSpace(const QChar& c)
{
	return (c.category() == QChar::Separator_Space || c == '\x09' || c == ' ' || c == '\t');
}
