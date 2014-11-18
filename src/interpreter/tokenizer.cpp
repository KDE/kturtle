/*
	Copyright (C) 2003-2009 Cies Breijs <cies AT kde DOT nl>

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


#include <QDebug>

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
	int startRow = row;
	int startCol = col;

	QChar c = getChar();  // get and store the next character from the string

	// catch the end of the input string
	if (atEnd)
		return new Token(Token::EndOfInput, "END", row, col, row, col);

	int cType = translator->look2type(c);  // since we need to know it often we store it

	// catch spaces
	if (isSpace(c)) {
		QString look;
		do {
			look += (isTab(c) ? "  " : " ");
			c = getChar();
		} while (isSpace(c) && !atEnd);
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
		         !isBreak(c) && !atEnd);
		return new Token(Token::String, look, startRow, startCol, row, col);
	}

	// catch variables
	if (cType == Token::VariablePrefix) {
		QString look;
		do {
			look += c;
			c = getChar();
		} while (isWordChar(c) || c.category() == QChar::Number_DecimalDigit || c == '_');
		ungetChar();
		return new Token(Token::Variable, look, startRow, startCol, row, col);
	}

	// catch words (known commands or function calls)
	if (isWordChar(c)) {  // first char has to be a letter
		QString look;
		do {
			look += c;
			c = getChar();
		} while (isWordChar(c) || c.isDigit() || c == '_');  // next chars
		ungetChar();
		int type = translator->look2type(look);
		if (type == Token::Unknown)
			type = Token::FunctionCall;
		return new Token(type, look, startRow, startCol, row, col);
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
		int type = translator->look2type(look);
		if (type != Token::Unknown)
			return new Token(type, look, startRow, startCol, row, col);
		ungetChar();
	}

	// catch known tokens of a single character (as last...)
	if (cType != Token::Unknown)
		return new Token(cType, static_cast<QString>(c), startRow, startCol, row, col);

	// this does not neglect calls to functions with a name of length one (checked it)
	return new Token(Token::Error, static_cast<QString>(c), startRow, startCol, row, col);
}


QChar Tokenizer::getChar()
{
	if (at >= inputString.size()) {
		atEnd = true;
// 		//qDebug() << "Tokenizer::getChar() returns: a ZERO CHAR " << " @ " << at - 1;
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
// 	//qDebug() << "Tokenizer::getChar() returns: " << c << " (" << c.category() << ") " << " @ " << at - 1;
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


bool Tokenizer::isWordChar(const QChar& c)
{
	// this method exists because some languages have non-letter category charaters
	// mixed with their letter character to make words (like hindi)
	// NOTE: this has to be extended then languages give problems,
	//       just add a category in the following test
	return (c.isLetter() || c.isMark());
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
	return (c.category() == QChar::Separator_Space || c == ' ' || isTab(c));
}

bool Tokenizer::isTab(const QChar& c)
{
	return (c == '\x09' || c == '\t');
}
