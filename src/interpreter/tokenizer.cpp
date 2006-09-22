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


#include <QtDebug>

#include <kdebug.h>

#include "tokenizer.h"


// TODO write in the Dictionary about the limitations of specific looks


void Tokenizer::initialize(QTextStream& inStream) {
	translator  = Translator::instance();
	inputStream = &inStream;
	row = 1;
	col = 1;
	prevCol = 1;
	prevChar = QChar();
}


Token* Tokenizer::getToken() {
	int startRow = row;
	int startCol = col;
	
	if (inputStream->atEnd() && prevChar.isNull())
		return new Token(Token::EndOfFile, "EOF", row, col, row, col);

	QChar c = getChar();

	int cType = translator->look2type(c);  // since we need to know it often we store it
	
// 	qDebug() << "got character: " << c;

	// catch spaces
	if (c.category() == QChar::Separator_Space) {
		QString look;
		while (c.category() == QChar::Separator_Space) {
			look += c;
			c = getChar();
		}
		ungetChar(c);
		return new Token(Token::WhiteSpace, look, startRow, startCol, row, col);
	}

	// catch EndOfLine's
	if (c.category() == QChar::Other_Control ||
	    c.category() == QChar::Separator_Line ||
	    c.category() == QChar::Separator_Paragraph) {
		return new Token(Token::EndOfLine, "\\n", startRow, startCol, startRow+1, 1);
	}
	
	// catch comments
	if (cType == Token::Comment) {
		QString look;
// 		while (c.category() != QChar::Other_Control &&
// 			c.category() != QChar::Separator_Line &&
// 			c.category() != QChar::Separator_Paragraph && !inputStream->atEnd())
		while (c != '\x0a' && c != '\n' &&
// 		       c != '\x0d' && c != '\x1e' &&
// 		       c.category() != QChar::Other_Control &&
// 		       c.category() != QChar::Separator_Line &&
// 		       c.category() != QChar::Separator_Paragraph &&
		       !inputStream->atEnd()) {
			look += c;
			c = getChar();
		}
		if (!inputStream->atEnd()) ungetChar(c);
		return new Token(Token::Comment, look, startRow, startCol, row, col);
	}
	
	// catch strings
	if (cType == Token::StringDelimiter) {
		QString look = QString(c);
		do {
			c = getChar();
			look += c;
		} while (!(translator->look2type(c) == Token::StringDelimiter && look.right(2) != "\\\"") &&
		         c != '\x0a' && c != '\n' &&
		         c.category() != QChar::Other_Control &&
		         c.category() != QChar::Separator_Line &&
		         c.category() != QChar::Separator_Paragraph && !inputStream->atEnd());
		return new Token(Token::String, look, startRow, startCol, row, col);
	}
	
	// catch variables
	if (cType == Token::VariablePrefix) {
		QString look;
		do {
			look += c;
			c = getChar();
		} while (c.isLetter() || c.category() == QChar::Number_DecimalDigit || c == '_');
		ungetChar(c);
		return new Token(Token::Variable, look, startRow, startCol, row, col);
	}
	
	// catch words (known or unknown function calls)
	if (c.isLetter()) {
		QString look;
		do {
			look += c;
			c = getChar();
		} while (c.isLetter() || c.category() == QChar::Number_DecimalDigit || c == '_');
		ungetChar(c);
		return new Token(translator->look2type(look), look, startRow, startCol, row, col);
	}
	
	// catch numbers
	if (c.isDigit() || cType == Token::DecimalSeparator) {
		bool hasDot = false;
		
		int localType = cType; // needed to keep cType unchanged
		QString look;
		do {
			if (localType == Token::DecimalSeparator) hasDot = true;
			look += c;
			c = getChar();
			localType = translator->look2type(c);
		} while (c.isDigit() || (localType == Token::DecimalSeparator && !hasDot));
		ungetChar(c);
		
		// if all we got is a dot then this is not a number, so return an Error token here
		if (translator->look2type(look) == Token::DecimalSeparator)
			return new Token(Token::Error, look, startRow, startCol, row, col);
		
		return new Token(Token::Number, look, startRow, startCol, row, col);
	}
	
	// catch previously uncatched 'double charactered tokens' (like: == != >= <=)
	{
		QString look = QString(c);
		QChar cc = getChar();
		look += cc;
		int lookType = translator->look2type(look);
		if (lookType != Token::Unknown)
			return new Token(lookType, look, startRow, startCol, row, col);
		ungetChar(cc);
	}
	
	// catch known tokens of a single character
	if (cType != Token::Unknown)
		return new Token(cType, static_cast<QString>(c), startRow, startCol, row, col);
	
	// fall throu: return an Error token
	return new Token(Token::Error, static_cast<QString>(c), startRow, startCol, row, col);
}


QChar Tokenizer::getChar() {
	QChar c;
	if (prevChar.isNull()) {
// // // // // 		if (inputStream->atEnd()) {             // these 5x commeted blocks are nice but currently not working
// // // // // 			qDebug() << "[end] did nothing";
// // // // // 			return QChar();
// // // // // 		}
		*inputStream >> c; // take a QChar of the inputStream
// 		qDebug() << "[get]:"<<c<<" ("<<c.category()<<") @ ("<<row<<", "<<col<<")";
		updatePosition(c);
	} else {
		c = prevChar; // use the char that is stored to be put back
// 		qDebug() << "[rst]:"<<c<<" ("<<c.category()<<") @ ("<<row<<", "<<col<<")";
		prevChar = QChar(); // and set prevChar back to NULL
		updatePosition(c);
	}
	return c;
}


void Tokenizer::ungetChar(QChar c) {
// // // // // 	if (inputStream->atEnd()) {
// // // // // 		prevChar = QChar();
// // // // // 		return;
// // // // // 	}
	if (c == '\x0a' || c == '\n') {
		row--;
		col = prevCol;
	} else {
		col--;
	}
	// it is not possible to write chars back to the stream, since we only
	// need write back one char at most we just remember thatone.
	prevChar = c;
// 	qDebug() << "[std]:"<<c<<" ("<<c.category()<<") @ ("<<row<<", "<<col<<")";
}


void Tokenizer::updatePosition(QChar c) {
	if (c == '\x0a' || c == '\n') {  // TODO something to not use these
		row++;
		prevCol = col;
		col = 1;
	} else {
		col++;
	}
}


