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

#include <qdom.h>
#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>

#include "settings.h"

#include "lexer.h"


Lexer::Lexer(QTextIStream& istream)
{
	inputStream = &istream;
	row = 1;
	col = 1;
	prevCol = 1;
	loadTranslations();
}


token Lexer::lex()
{
	skipSpaces(); // skips the white space that it quite likely infront of the token

	token currentToken;
	currentToken.type = tokNotSet; // not really needed
	currentToken.look = "";
	currentToken.value = 0;
	currentToken.string = "";
	currentToken.start.row = row;
	currentToken.start.col = col;

	QChar gottenChar = getChar();

	if ( inputStream->atEnd() )
	{
		kdDebug(0)<<"Lexer::lex(), got EOF."<<endl;
		currentToken.type = tokEOF;
		ungetChar(gottenChar); // unget the gottenChar and fix the row/col values
		return currentToken;
	}

	if (gottenChar == '#')
	{
		ungetChar(gottenChar);
		skipComment();
		gottenChar = getChar();
	}
	
	// if (gottenChar.category() == QChar::Separator_Line) somehow doesnt work
	if (gottenChar == '\x0a' || gottenChar == '\n')
	{
		currentToken.type = tokEOL;
	}
	else if (gottenChar.isLetter() || gottenChar == '[' || gottenChar == ']')
	{
		ungetChar(gottenChar);
		// sets currentToken.look by reference, and set the currentToken.type to tokUnknown
		currentToken.type = getWord(currentToken.look);
		getTokenType(currentToken); // gets the actual tokenType
	}
	else if ( gottenChar.isNumber() )
	{
		ungetChar(gottenChar);
		// set currentToken.value by reference, and set the currentToken.type to tokNumber
		currentToken.type = getNumber(currentToken.value);
	}
	else if (gottenChar == '>')
	{
		gottenChar = getChar();
		if (gottenChar == '=')
		{
			currentToken.type = tokGe;
		}
		else
		{
			ungetChar(gottenChar);
			currentToken.type = tokGt;
		}
	}
	else if (gottenChar == '<')
	{
		gottenChar = getChar();
		if ( gottenChar == '=' )
		{
			currentToken.type = tokLe;
		}
		else
		{
			ungetChar(gottenChar);
			currentToken.type = tokLt;
		}
	}
	else if (gottenChar == '!')
	{
		gottenChar = getChar();
		if (gottenChar == '=')
		{
			currentToken.type = tokNe;
		}
		else
		{
			ungetChar(gottenChar);
			currentToken.type = tokNot;
		}
	}
	else if (gottenChar == '=')
	{
		gottenChar = getChar();
		if (gottenChar == '=')
		{
			currentToken.type = tokEq;
		}
		else
		{
			ungetChar(gottenChar);
			currentToken.type = tokAssign;
		}
	}
	else if (gottenChar == '(')
	{
		currentToken.type = tokBraceOpen;
	}
	else if (gottenChar == ')')
	{
		currentToken.type = tokBraceClose;
	}
	else if (gottenChar == '+')
	{
		currentToken.type = tokPlus;
	}
	else if (gottenChar == '-')
	{
		currentToken.type = tokMinus;
	}
	else if (gottenChar == '*')
	{
		currentToken.type = tokMul;
	}
	else if (gottenChar == '/')
	{
		currentToken.type = tokDev;
	}
	else if (gottenChar == ',')
	{
		currentToken.type = tokComma;
	}
	else if (gottenChar == '"')
	{
		ungetChar(gottenChar);
		getString(currentToken);
	}
	else
	{
		currentToken.type = tokUnknown;
		currentToken.look = gottenChar;
	}
	
	currentToken.end.row = row;
	currentToken.end.col = col;
	return currentToken;
}


QChar Lexer::getChar()
{
	QChar c;
	if ( !putBackChar.isNull() )
	{
		c = putBackChar; // use the char that is stored to be put back
		kdDebug(0)<<"Lexer::getChar(), restored: '"<<c<<"' @ ("<<row<<", "<<col<<")"<<endl;
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
		kdDebug(0)<<"Lexer::getChar(): '"<<c<<"' @ ("<<row<<", "<<col<<")"<<endl;
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
	kdDebug(0)<<"Lexer::ungetChar(), saved char: '"<<c<<"' and steped back to ("<<row<<", "<<col<<")"<<endl;
}

// PRIVATEs

int Lexer::getNumber(Number& n)
{
	kdDebug(0)<<"Lexer::getNumber()"<<endl;
	QString s;
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
			s += currentChar;
			currentChar = getChar();
		}
		ungetChar(currentChar); //read one too much
		n.bString = false;
		n.val = s.toDouble();
		kdDebug(0)<<"Lexer::getNumber(), got NUMBER: '"<<n.val<<"'"<<endl;
		return tokNumber;
	}
	else
	{
		return tokError;  
	}
}


int Lexer::getWord(QString& word) {
	kdDebug(0)<<"Lexer::getWord()"<<endl;
	QChar currentChar = getChar();
	if ( currentChar.isLetter() || currentChar == '[' || currentChar == ']' ) {
		while ( ( currentChar.isLetterOrNumber() || currentChar == '_' || currentChar == '[' || currentChar == ']' ) && !inputStream->atEnd() )
		{
			word += currentChar;
			currentChar = getChar();
		}
		kdDebug(0)<<"Lexer::getWord(), got NAME: '"<<word<<"'"<<endl;
		ungetChar(currentChar); //read one too much
		return tokUnknown; // returns tokUnknown, actual token is to be determained later in Lexer::getTokenType
	}
	else
	{
		return tokError;  
	}
}

void Lexer::getTokenType(token& currentToken)
{
	if (currentToken.type == tokUnknown)
	{
		QString key = currentToken.look.lower(); // make lowercase copy of the word as it was found in the inputStream
// 		t.row = row; // store the tokens row/col info
// 		t.col = col;                 <-- done in the Lexer::lex 
		
		if( !aliasMap[key].isEmpty() ) // if the key is an alias translate that alias to a key
		{
			key = aliasMap[key];
		}
		
		if(      key == keyMap["begin"]          ) currentToken.type = tokBegin;
		else if( key == keyMap["end"]            ) currentToken.type = tokEnd;
		else if( key == keyMap["while"]          ) currentToken.type = tokWhile;
		else if( key == keyMap["if"]             ) currentToken.type = tokIf;
		else if( key == keyMap["else"]           ) currentToken.type = tokElse;
		else if( key == keyMap["for"]            ) currentToken.type = tokFor;
		else if( key == keyMap["to"]             ) currentToken.type = tokTo;
		else if( key == keyMap["step"]           ) currentToken.type = tokStep;
		else if( key == keyMap["and"]            ) currentToken.type = tokAnd;
		else if( key == keyMap["or"]             ) currentToken.type = tokOr;
		else if( key == keyMap["not"]            ) currentToken.type = tokNot;
		else if( key == keyMap["return"]         ) currentToken.type = tokReturn;
		else if( key == keyMap["break"]          ) currentToken.type = tokBreak;
		else if( key == keyMap["run"]            ) currentToken.type = tokRun;
		else if( key == keyMap["foreach"]        ) currentToken.type = tokForEach;
		else if( key == keyMap["in"]             ) currentToken.type = tokIn;

		else if( key == keyMap["learn"]          ) currentToken.type = tokLearn;
		
		else if( key == keyMap["clear"]          ) currentToken.type = tokClear;
		else if( key == keyMap["go"]             ) currentToken.type = tokGo;
		else if( key == keyMap["gox"]            ) currentToken.type = tokGoX;
		else if( key == keyMap["goy"]            ) currentToken.type = tokGoY;
		else if( key == keyMap["forward"]        ) currentToken.type = tokForward;
		else if( key == keyMap["backward"]       ) currentToken.type = tokBackward;
		else if( key == keyMap["direction"]      ) currentToken.type = tokDirection;
		else if( key == keyMap["turnleft"]       ) currentToken.type = tokTurnLeft;
		else if( key == keyMap["turnright"]      ) currentToken.type = tokTurnRight;
		else if( key == keyMap["center"]         ) currentToken.type = tokCenter;
		else if( key == keyMap["setpenwidth"]    ) currentToken.type = tokSetPenWidth;
		else if( key == keyMap["penup"]          ) currentToken.type = tokPenUp;
		else if( key == keyMap["pendown"]        ) currentToken.type = tokPenDown;
		else if( key == keyMap["setfgcolor"]     ) currentToken.type = tokSetFgColor;
		else if( key == keyMap["setbgcolor"]     ) currentToken.type = tokSetBgColor;
		else if( key == keyMap["resizecanvas"]   ) currentToken.type = tokResizeCanvas;
		else if( key == keyMap["spriteshow"]     ) currentToken.type = tokSpriteShow;
		else if( key == keyMap["spritehide"]     ) currentToken.type = tokSpriteHide;
		else if( key == keyMap["spritepress"]    ) currentToken.type = tokSpritePress;
		else if( key == keyMap["spritechange"]   ) currentToken.type = tokSpriteChange;
		
		else if( key == keyMap["do"]             ) currentToken.type = tokDo; // dummy commands

		else if( key == keyMap["message"]        ) currentToken.type = tokMessage;
		else if( key == keyMap["inputwindow"]    ) currentToken.type = tokInputWindow;
		else if( key == keyMap["print"]          ) currentToken.type = tokPrint;
		else if( key == keyMap["fonttype"]       ) currentToken.type = tokFontType;
		else if( key == keyMap["fontsize"]       ) currentToken.type = tokFontSize;
		else if( key == keyMap["repeat"]         ) currentToken.type = tokRepeat;
		else if( key == keyMap["random"]         ) currentToken.type = tokRandom;
		else if( key == keyMap["wait"]           ) currentToken.type = tokWait;
		else if( key == keyMap["wrapon"]         ) currentToken.type = tokWrapOn;
		else if( key == keyMap["wrapoff"]        ) currentToken.type = tokWrapOff;
		else if( key == keyMap["reset"]          ) currentToken.type = tokReset;
		else
		{
			kdDebug(0)<<"Lexer::getTokenType, found UNKNOWN word @ ("<<currentToken.start.row<<", "<<currentToken.start.col<<"), can be anything"<<endl;
			// t.type = tokUnknown; is allready
		}
		
		kdDebug(0)<<"Lexer::getTokenType, found tok-number: '"<<currentToken.type<<"' with the key: '"<<key<<"' @ ("<<currentToken.start.row<<", "<<currentToken.start.col<<")"<<endl;
	}
}

void Lexer::skipComment()
{
	kdDebug(0)<<"Lexer::skipComment(), skipping COMMENT."<<endl;
	QChar currentChar = getChar();
	while ( !inputStream->atEnd() && currentChar == '#' )
	{
		while ( !inputStream->atEnd() && !(currentChar == '\x0a' || currentChar == '\n') )
		{
			currentChar = getChar();
		}
		kdDebug(0)<<"Lexer::skipComment(), skiped one commented line."<<endl;
		currentChar = getChar();
	}
	ungetChar(currentChar);
}

void Lexer::skipSpaces()
{
	kdDebug(0)<<"Lexer::skipSpaces(), skipping SPACES."<<endl;
	QChar currentChar = getChar();
	while (!inputStream->atEnd() && currentChar == ' ')
	{
		currentChar = getChar();
	}
	ungetChar(currentChar);
}


void Lexer::getString(token& currentToken)
{
	QString str;
	QChar currentChar = getChar();
	while ( currentChar != '"' && !inputStream->atEnd() ) {
		
		if (currentChar == '\\') //escape sequence 
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
		else if(currentChar != '"'){ //anything but closing char
			str += currentChar;
		}
		
		currentChar = getChar();
	}
	
	currentToken.type = tokString;
	currentToken.look = str;
	// currentToken.string = str;
	currentToken.value = 0;
	
	kdDebug(0)<<"Lexer::getStringConstant, got STRINGCONSTANT: "<<currentToken.look<<"'"<<endl;
	
// 	if( inputStream->atEnd() )       // DUNNO WHAT THIS IS FOR
// 	{
// 		currentToken.type = tokEOF;
// 	}
}


void Lexer::loadTranslations() {
	QDomDocument KeywordsXML;

	kdDebug(0) << "TRname:"<< "kturtle/data/logokeywords." + Settings::logoLanguage() + ".xml" <<endl;
	kdDebug(0) << "TRfile:"<< locate("data", "kturtle/data/logokeywords." + Settings::logoLanguage() + ".xml") <<endl;
  	// Read the specified translation file
	QFile xmlfile( locate("data", "kturtle/data/logokeywords." + Settings::logoLanguage() + ".xml") );

	if ( !xmlfile.open(IO_ReadOnly) )
	{
		return;
	}
	if ( !KeywordsXML.setContent(&xmlfile) )
	{
		xmlfile.close();
		return;
	}
	xmlfile.close();

	// get into the first child of the root element (in our case a <command> tag)
	QDomElement rootElement = KeywordsXML.documentElement();
	QDomNode n = rootElement.firstChild();

	while( !n.isNull() )
	{
		QString name, key, alias;
		name = n.toElement().attribute("name"); // get the name attribute of <command>
		QDomNode m = n.firstChild(); // get into the first child of a <command>
		while (true)
		{
			if( !m.toElement().text().isEmpty() ) {
				if (m.toElement().tagName() == "keyword")
				{
					key = m.toElement().text();
					keyMap.insert(name, key);
				}
				if (m.toElement().tagName() == "alias")
				{
					alias = m.toElement().text();
					aliasMap.insert(alias, key);
					reverseAliasMap.insert(key, alias);
				}
			}
			// break when read the last child of the current <command>
			if ( m == n.lastChild() ) break;
			m = m.nextSibling(); // goto the next element in the current <command>
		}
		n = n.nextSibling(); // goto the next <command>
	}
}

QString Lexer::name2key(QString name)
{
	if( !aliasMap[name].isEmpty() ) { // translate the alias
		return QString( i18n("'%1' (%2)").arg(keyMap[name]).arg(reverseAliasMap[name]) );
	}
	return QString( "'" + keyMap[name] + "'");
}
