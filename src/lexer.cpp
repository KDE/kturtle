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

#include <qdom.h>
#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>

#include "settings.h"

#include "lexer.h"


Lexer::Lexer(QTextIStream& iStream)
{
	inputStream = &iStream;
	row = 1;
	col = 1;
	prevCol = 1;
	translate = new Translate();
}


Token Lexer::lex()
{
	skipSpaces(); // skips the white space that it quite likely (indentation) infront of the Token

	Token currentToken;
	currentToken.type = tokNotSet; // not really needed
	currentToken.look = "";
	currentToken.value = 0;
	currentToken.start.row = row;
	currentToken.start.col = col;

	QChar currentChar = getChar();

	if ( inputStream->atEnd() )
	{
		kdDebug(0)<<"Lexer::lex(), got EOF."<<endl;
		currentToken.type = tokEOF;
		currentToken.look = "EOF";
		ungetChar(currentChar); // unget the currentChar and fix the row/col values
		return currentToken;
	}

	if (currentChar == '#')
	{
		while ( !inputStream->atEnd() && !(currentChar == '\x0a' || currentChar == '\n') )
			currentChar = getChar();
	}
	
	// if (currentChar.category() == QChar::Separator_Line) somehow doesnt work
	if (currentChar == '\x0a' || currentChar == '\n')
	{
		currentToken.type = tokEOL;
		currentToken.look = "EOL";
	}
	else if (currentChar.isLetter() || currentChar == '[' || currentChar == ']')
	{
		ungetChar(currentChar);
		// sets currentToken.look by reference, and set the currentToken.type to tokUnknown
		currentToken.type = getWord(currentToken.look);
		setTokenType(currentToken); // gets the actual tokenType
	}
	else if ( currentChar.isNumber() )
	{
		ungetChar(currentChar);
		// set currentToken.value/look by reference, and set the currentToken.type to tokNumber
		currentToken.type = getNumber(currentToken.value, currentToken.look);
	}
	else if (currentChar == '>')
	{
		currentChar = getChar();
		if (currentChar == '=')
		{
			currentToken.type = tokGe;
			currentToken.look = ">=";
		}
		else
		{
			ungetChar(currentChar);
			currentToken.type = tokGt;
			currentToken.look = ">";
		}
	}
	else if (currentChar == '<')
	{
		currentChar = getChar();
		if ( currentChar == '=' )
		{
			currentToken.type = tokLe;
			currentToken.look = "<=";
		}
		else
		{
			ungetChar(currentChar);
			currentToken.type = tokLt;
			currentToken.look = ">";
		}
	}
	else if (currentChar == '!')
	{
		currentChar = getChar();
		if (currentChar == '=')
		{
			currentToken.type = tokNe;
			currentToken.look = "!=";
		}
		else
		{
			ungetChar(currentChar);
			currentToken.type = tokNot;
			currentToken.look = "!";
		}
	}
	else if (currentChar == '=')
	{
		currentChar = getChar();
		if (currentChar == '=')
		{
			currentToken.type = tokEq;
			currentToken.look = "==";
		}
		else
		{
			ungetChar(currentChar);
			currentToken.type = tokAssign;
			currentToken.look = "=";
		}
	}
	else if (currentChar == '(')
	{
		currentToken.type = tokBraceOpen;
		currentToken.look = "(";
	}
	else if (currentChar == ')')
	{
		currentToken.type = tokBraceClose;
		currentToken.look = ")";
	}
	else if (currentChar == '+')
	{
		currentToken.type = tokPlus;
		currentToken.look = "+";
	}
	else if (currentChar == '-')
	{
		currentToken.type = tokMinus;
		currentToken.look = "-";
	}
	else if (currentChar == '*')
	{
		currentToken.type = tokMul;
		currentToken.look = "*";
	}
	else if (currentChar == '/')
	{
		currentToken.type = tokDev;
		currentToken.look = "/";
	}
	else if (currentChar == ',')
	{
		currentToken.type = tokComma;
		currentToken.look = ",";
	}
	else if (currentChar == '"')
	{
		getString(currentToken);
	}
	else
	{
		currentToken.type = tokUnknown;
		currentToken.look = currentChar;
	}
	
	currentToken.end.row = row;
	currentToken.end.col = col;
	return currentToken;
}



// PRIVATEs

QChar Lexer::getChar()
{
	QChar c;
	if ( !putBackChar.isNull() )
	{
		c = putBackChar; // use the char that is stored to be put back
		// kdDebug(0)<<"Lexer::getChar(), restored: '"<<c<<"' @ ("<<row<<", "<<col<<")"<<endl;
		putBackChar = QChar(); // and set putBackChar back to NULL
		if (c == '\x0a' || c == '\n')
		{
			row++;
			prevCol = col;
			col = 1;
		}
		else
		{
			col++;
		}
	}
	else
	{
		*inputStream >> c; // take a QChar of the inputStream
		// kdDebug(0)<<"Lexer::getChar(): '"<<c<<"' @ ("<<row<<", "<<col<<")"<<endl;
		if (c == '\x0a' || c == '\n')
		{
			row++;
			prevCol = col;
			col = 1;
		}
		else
		{
			col++;
		}
	}
	return c;
}

void Lexer::ungetChar(QChar c)
{
	if (c == '\x0a' || c == '\n')
	{
		row--;
		col = prevCol;
	}
	else
	{
		col--;
	}
	putBackChar = c;
	// kdDebug(0)<<"Lexer::ungetChar(), saved char: '"<<c<<"' and steped back to ("<<row<<", "<<col<<")"<<endl;
}

int Lexer::getWord(QString& word)
{
	// kdDebug(0)<<"Lexer::getWord()"<<endl;
	QChar currentChar = getChar();
	if ( currentChar.isLetter() || currentChar == '[' || currentChar == ']' ) {
		while ( ( currentChar.isLetterOrNumber() || currentChar == '_' || currentChar == '[' || currentChar == ']' ) && !inputStream->atEnd() )
		{
			word += currentChar;
			currentChar = getChar();
		}
		kdDebug(0)<<"Lexer::getWord(), got NAME: '"<<word<<"'"<<endl;
		ungetChar(currentChar); //read one too much
		return tokUnknown; // returns tokUnknown, actual token is to be determained later in Lexer::setTokenType
	}
	else return tokError;
}

void Lexer::setTokenType(Token& currentToken)
{
	if (currentToken.type == tokUnknown)
	{
		// make lowercase copy of the word as it was found in the inputStream
		QString k = currentToken.look.lower();
		// if the key is an alias translate that alias to a key
		if ( !translate->alias2key(k).isEmpty() ) k = translate->alias2key(k);
		
		if      (k == translate->name2key("begin")          ) currentToken.type = tokBegin;
		else if (k == translate->name2key("end")            ) currentToken.type = tokEnd;
		else if (k == translate->name2key("while")          ) currentToken.type = tokWhile;
		else if (k == translate->name2key("if")             ) currentToken.type = tokIf;
		else if (k == translate->name2key("else")           ) currentToken.type = tokElse;
		else if (k == translate->name2key("for")            ) currentToken.type = tokFor;
		else if (k == translate->name2key("to")             ) currentToken.type = tokTo;
		else if (k == translate->name2key("step")           ) currentToken.type = tokStep;
		else if (k == translate->name2key("and")            ) currentToken.type = tokAnd;
		else if (k == translate->name2key("or")             ) currentToken.type = tokOr;
		else if (k == translate->name2key("not")            ) currentToken.type = tokNot;
		else if (k == translate->name2key("return")         ) currentToken.type = tokReturn;
		else if (k == translate->name2key("break")          ) currentToken.type = tokBreak;
		else if (k == translate->name2key("run")            ) currentToken.type = tokRun;
		else if (k == translate->name2key("foreach")        ) currentToken.type = tokForEach;
		else if (k == translate->name2key("in")             ) currentToken.type = tokIn;

		else if (k == translate->name2key("learn")          ) currentToken.type = tokLearn;
		
		else if (k == translate->name2key("clear")          ) currentToken.type = tokClear;
		else if (k == translate->name2key("go")             ) currentToken.type = tokGo;
		else if (k == translate->name2key("gox")            ) currentToken.type = tokGoX;
		else if (k == translate->name2key("goy")            ) currentToken.type = tokGoY;
		else if (k == translate->name2key("forward")        ) currentToken.type = tokForward;
		else if (k == translate->name2key("backward")       ) currentToken.type = tokBackward;
		else if (k == translate->name2key("direction")      ) currentToken.type = tokDirection;
		else if (k == translate->name2key("turnleft")       ) currentToken.type = tokTurnLeft;
		else if (k == translate->name2key("turnright")      ) currentToken.type = tokTurnRight;
		else if (k == translate->name2key("center")         ) currentToken.type = tokCenter;
		else if (k == translate->name2key("setpenwidth")    ) currentToken.type = tokSetPenWidth;
		else if (k == translate->name2key("penup")          ) currentToken.type = tokPenUp;
		else if (k == translate->name2key("pendown")        ) currentToken.type = tokPenDown;
		else if (k == translate->name2key("setfgcolor")     ) currentToken.type = tokSetFgColor;
		else if (k == translate->name2key("setbgcolor")     ) currentToken.type = tokSetBgColor;
		else if (k == translate->name2key("resizecanvas")   ) currentToken.type = tokResizeCanvas;
		else if (k == translate->name2key("spriteshow")     ) currentToken.type = tokSpriteShow;
		else if (k == translate->name2key("spritehide")     ) currentToken.type = tokSpriteHide;
		else if (k == translate->name2key("spritepress")    ) currentToken.type = tokSpritePress;
		else if (k == translate->name2key("spritechange")   ) currentToken.type = tokSpriteChange;
		
		else if (k == translate->name2key("do")             ) currentToken.type = tokDo; // dummy commands

		else if (k == translate->name2key("message")        ) currentToken.type = tokMessage;
		else if (k == translate->name2key("inputwindow")    ) currentToken.type = tokInputWindow;
		else if (k == translate->name2key("print")          ) currentToken.type = tokPrint;
		else if (k == translate->name2key("fonttype")       ) currentToken.type = tokFontType;
		else if (k == translate->name2key("fontsize")       ) currentToken.type = tokFontSize;
		else if (k == translate->name2key("repeat")         ) currentToken.type = tokRepeat;
		else if (k == translate->name2key("random")         ) currentToken.type = tokRandom;
		else if (k == translate->name2key("wait")           ) currentToken.type = tokWait;
		else if (k == translate->name2key("wrapon")         ) currentToken.type = tokWrapOn;
		else if (k == translate->name2key("wrapoff")        ) currentToken.type = tokWrapOff;
		else if (k == translate->name2key("reset")          ) currentToken.type = tokReset;
		else
		{
			kdDebug(0)<<"Lexer::setTokenType, found UNKNOWN word @ ("<<currentToken.start.row<<", "<<currentToken.start.col<<"), can be anything"<<endl;
			// t.type = tokUnknown; is allready
		}
		
		kdDebug(0)<<"Lexer::setTokenType, found tok-number: '"<<currentToken.type<<"' with the key: '"<<k<<"' @ ("<<currentToken.start.row<<", "<<currentToken.start.col<<")"<<endl;
	}
}


void Lexer::skipSpaces()
{
	// kdDebug(0)<<"Lexer::skipSpaces(), skipping SPACES."<<endl;
	QChar currentChar = getChar();
	// when the Separator_* groups can be identified in the QChar thing would be easier
	while ( !inputStream->atEnd() && ( currentChar.isSpace() && !(currentChar == '\x0a' || currentChar == '\n') ) )
	{
		currentChar = getChar();
	}
	ungetChar(currentChar); // unget the tokEOL we likely just found
}


int Lexer::getNumber(Value& num, QString& look)
{
	// by reference the value (Value) and look part are set
	// kdDebug(0)<<"Lexer::getNumber()"<<endl;
	bool hasPoint = false;
	QChar currentChar = getChar();
	if ( currentChar.isNumber() )
	{
		while ( ( currentChar.isNumber() || (currentChar == '.' && !hasPoint) ) && !inputStream->atEnd() )
		{
			if (currentChar == '.')
			{
				hasPoint = true;
			}
			look += currentChar;
			currentChar = getChar();
		}
		ungetChar(currentChar); //read one too much
		num.setNumber( look.toDouble() );
		kdDebug(0)<<"Lexer::getNumber(), got NUMBER: '"<<num.Number()<<"'"<<endl;
		return tokNumber;
	}
	else return tokError;
}

void Lexer::getString(Token& currentToken)
{
	QString str = "\""; // start with a " cauz it just got lost
	QChar currentChar = QChar(); // start empty
	while ( currentChar != '"' && !(currentChar == '\x0a' || currentChar == '\n') && !inputStream->atEnd() )
	{
		currentChar = getChar();
		if (currentChar == '\\') // escape sequence 
		{
			currentChar = getChar();
			switch (currentChar)
			{
				case 'n': str += '\n'; break;
				case 't': str += '\t'; break;
				case 'f': str += '\f'; break;
				case '"': str += '"';  break;
			}
		}
		else if (currentChar == '\x0a' || currentChar == '\n') // if the user forgot to delimit the string
		{
			ungetChar(currentChar);
			break;
		}
		else str += currentChar;
	}
	currentToken.type = tokString;
	currentToken.look = str;
	
	kdDebug(0)<<"Lexer::getStringConstant, got STRINGCONSTANT: "<<currentToken.look<<"'"<<endl;
}

