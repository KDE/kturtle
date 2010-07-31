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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

    
#ifndef _LEXER_H_
#define _LEXER_H_

#include <tqmap.h>
#include <tqstring.h>
#include <tqtextstream.h>

#include "token.h"
#include "translate.h"
#include "value.h"


class Lexer
{
	public:
		Lexer(TQTextIStream&);
		~Lexer() {}
	
		Token lex(); // returns the next Token, skipping spaces


	private:
		TQChar getChar();
		void ungetChar(TQChar);
		int getWord(TQString&);
		void skipSpaces();
		void setTokenType(Token&);
		int getNumber(Value&, TQString&);
		void getString(Token&);
	
		TQTextIStream    *inputStream;
		Translate       *translate;
		uint             row, col, prevCol;
		TQChar            putBackChar;
};


#endif // _LEXER_H_


