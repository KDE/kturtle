/*
     Copyright (C) 2003 by Walter Schreppers 
     Copyright (C) 2004 by Cies Breijs   
     
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

    
#ifndef _LEXER_H_
#define _LEXER_H_

#include <qmap.h>
#include <qstring.h>
#include <qtextstream.h>

#include "token.h"
#include "translate.h"
#include "value.h"

class Lexer
{
	public:
	//constructor and destructor
	Lexer(QTextIStream&);
	~Lexer() {}

	//public members
	token lex(); // returns the next token, skipping spaces
// 	QString name2key(QString);
	
	private:
	//private members
	QChar getChar();
	void ungetChar(QChar);
	void skipComment();
	void skipSpaces();
	void setTokenType(token&);
	int getNumber(Value&, QString&);
	int getWord(QString&);
	void getString(token&);

	//private locals
	QTextIStream    *inputStream;
	uint             row, col, prevCol;
	QChar            putBackChar;
	
	Translate       *translate;
};


#endif // _LEXER_H_


