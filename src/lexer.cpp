/*
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

// This file is originally written by Walter Scheppers, but allmost
// every aspect of it is slightly changed by Cies Breijs.

  
#include <qdom.h>
#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>

#include "settings.h"
#include "lexer.h"


Lexer::Lexer(QTextIStream& QTextIncomingStream) {
	loadKeywords();
	inputStream = &QTextIncomingStream;
	row = 1;
	col = 1;
	prevCol = 1;
}


token Lexer::lex() {
	token t;
	t.val = 0;
	t.str = "";
	t.type = tokEof;
	
	skipWhite();
	skipComment(); // this skips all comments (also multilined with whitelines, see code)
	// skipWhite();
	
	QChar look = getChar();
	if ( inputStream->atEnd() ) {
		kdDebug(0)<<"Lexer::lex(), got EOF."<<endl;
		t.type = tokEof;
		return t;
	}
	
	if ( look.isLetter() || look == '[' || look == ']' ) {
		ungetChar(look);
		t.type = getName(t.str); // read the whole name and store it in t.str, and return tokId when finished
		getToken(t);
	} else if ( look.isNumber() ) {
		ungetChar(look);
		t.type = getNumber(t.val);
	} else if (look == '>') {
		look = getChar();
		if (look == '=') {
			t.type = tokGe;
		} else {
			ungetChar(look);
		t.type = tokGt;
		}
	} else if (look == '<') {
		look = getChar();
		if ( look == '=' ) {
			t.type = tokLe;
		} else {
			ungetChar(look);
		t.type = tokLt;
		}
	} else if (look == '!') {
		look = getChar();
		if (look == '=') {
			t.type = tokNe;
		} else {
			ungetChar(look);
			t.type='!';
		}
	} else if (look == '=') {
		look = getChar();
		if( look == '=' ) {
			t.type = tokEq;
		} else {
			ungetChar(look);
			t.type = tokAssign;
		}
	} else if (look == '"') {
		getStringConstant(t);
	} else if (look == '\n' || look == '\x0a') {
		// do something here to break at linebreak
	} else {
		t.type = look;
	}
	
	return t;
}

QChar Lexer::getChar() {
	QChar c;
	if ( !putBackChar.isNull() ) {
		c = putBackChar;       // use the char that is stored to be put back
		kdDebug(0)<<"Lexer::getChar(), restored: '"<<c<<"' @ ("<<row<<", "<<col<<")"<<endl;
		putBackChar = QChar(); // and set putBackChar back to NULL
		if (c == '\n' || c == '\x0a') {
			row++;
			prevCol = col;
			col = 1;
		} else {
			col++;
		}
	} else {
		*inputStream >> c; // take one QChar of the QStream
		kdDebug(0)<<"Lexer::getChar(): '"<<c<<"' @ ("<<row<<", "<<col<<")"<<endl;
		if (c == '\n' || c == '\x0a') {
			row++;
			prevCol = col;
			col = 1;
		} else {
			col++;
		}
	}
	return c;
}

void Lexer::ungetChar(QChar ch) {
	if (ch == '\n' || ch == '\x0a') {
		row--;
		col = prevCol;
	} else {
		col--;
	}
	kdDebug(0)<<"Lexer::ungetChar(), saved char: '"<<ch<<"' and steped back to ("<<row<<", "<<col<<")"<<endl;
	putBackChar = ch;
}

QString Lexer::translateCommand(QString name) {
	QString key = KeyMap[name];
	if( !ReverseAliasMap[name].isEmpty() ) { // translate the alias
		return QString("%1 (%2)").arg(key).arg(ReverseAliasMap[name]);
	}
	return key;
}


// PRIVATEs

int Lexer::getNumber(Number& n) {
	QString s;
	bool havePoint = false;
	QChar look = getChar();
	if ( look.isNumber() ) {
		while ( ( look.isNumber() || (look == '.' && !havePoint) ) && !inputStream->atEnd() ) {
			if (look == '.') {
				havePoint = true;
			}
			s += look;
			look = getChar();
		}
		ungetChar(look); //read one too much
		n.bString = false;
		n.val = s.toDouble();
		kdDebug(0)<<"Lexer::getNumber(), got NUMBER: '"<<n.val<<"'"<<endl;
		return tokNumber;
	} else {
		return tokError;  
	}
}


int Lexer::getName(QString& s) {
	QChar look = getChar();
	if ( look.isLetter() || look=='[' || look==']' ) {
		while ( ( look.isLetterOrNumber() || look == '_' || look=='[' || look==']' ) && !inputStream->atEnd() ) {
			s += look;
			look = getChar();
		}
		kdDebug(0)<<"Lexer::getName(), got NAME: '"<<s<<"'"<<endl;
		ungetChar(look); //read one too much
		return tokId;
	} else {
		return tokError;  
	}
}


void Lexer::loadKeywords() {
	QDomDocument KeywordsXML;
	// Read the specified translation file
	QFile xmlfile( locate("data", "kturtle/data/logokeywords." + Settings::logoLanguage() + ".xml") );
	
	if ( !xmlfile.open(IO_ReadOnly) ) {
			return;
	}
	if ( !KeywordsXML.setContent(&xmlfile) ) {
			xmlfile.close();
			return;
	}
	xmlfile.close();

	// get into the first child of the root element (in our case a <command>"
	QDomElement rootElement = KeywordsXML.documentElement();
	QDomNode n = rootElement.firstChild();
	
	while( !n.isNull() ) {
		QString name, key, alias;
		name = n.toElement().attribute("name"); // get the name attribute of <command>
		QDomNode m = n.firstChild(); // get into the first child of a <command>
		while (true) {
			if( !m.toElement().text().isEmpty() ) { // no need for checking empty's
				if (m.toElement().tagName() == "keyword") {
					key = m.toElement().text();
					KeyMap.insert(name, key); // put it in the map
				}
				if (m.toElement().tagName() == "alias") {
					alias = m.toElement().text();
					AliasMap.insert(alias, key); // to find an key by an alias
					ReverseAliasMap.insert(key, alias); // to find an alias by a key
				}
			}
			// break when just read the last child of the current <command>
			if ( m == n.lastChild() ) { break; }
			m = m.nextSibling(); // goto the next element in the current <command>
		}
		n = n.nextSibling(); // goto the next <command>
	}
}

void Lexer::getToken(token& t) {
	if(t.type == tokId) {
		QString key = t.str.lower(); // make lowercase copy

		t.row = row; // store the tokens row/col info
		t.col = col;
		
		if( !AliasMap[key].isEmpty() ) { // if the key is an alias translate that alias to a key
			key = AliasMap[key];
		}
		
		if(      key == KeyMap["begin"]          ) t.type = tokBegin;
		else if( key == KeyMap["end"]            ) t.type = tokEnd;
		else if( key == KeyMap["while"]          ) t.type = tokWhile;
		else if( key == KeyMap["if"]             ) t.type = tokIf;
		else if( key == KeyMap["else"]           ) t.type = tokElse;
		else if( key == KeyMap["for"]            ) t.type = tokFor;
		else if( key == KeyMap["to"]             ) t.type = tokTo;
		else if( key == KeyMap["step"]           ) t.type = tokStep;
		else if( key == KeyMap["and"]            ) t.type = tokAnd;
		else if( key == KeyMap["or"]             ) t.type = tokOr;
		else if( key == KeyMap["not"]            ) t.type = tokNot;
		else if( key == KeyMap["return"]         ) t.type = tokReturn;
		else if( key == KeyMap["break"]          ) t.type = tokBreak;
		else if( key == KeyMap["run"]            ) t.type = tokRun;
		else if( key == KeyMap["foreach"]        ) t.type = tokForEach;
		else if( key == KeyMap["in"]             ) t.type = tokIn;

		else if( key == KeyMap["learn"]          ) t.type = tokLearn;
		
		else if( key == KeyMap["clear"]          ) t.type = tokClear;
		else if( key == KeyMap["go"]             ) t.type = tokGo;
		else if( key == KeyMap["gox"]            ) t.type = tokGoX;
		else if( key == KeyMap["goy"]            ) t.type = tokGoY;
		else if( key == KeyMap["forward"]        ) t.type = tokForward;
		else if( key == KeyMap["backward"]       ) t.type = tokBackward;
		else if( key == KeyMap["direction"]      ) t.type = tokDirection;
		else if( key == KeyMap["turnleft"]       ) t.type = tokTurnLeft;
		else if( key == KeyMap["turnright"]      ) t.type = tokTurnRight;
		else if( key == KeyMap["center"]         ) t.type = tokCenter;
		else if( key == KeyMap["setpenwidth"]    ) t.type = tokSetPenWidth;
		else if( key == KeyMap["penup"]          ) t.type = tokPenUp;
		else if( key == KeyMap["pendown"]        ) t.type = tokPenDown;
		else if( key == KeyMap["setfgcolor"]     ) t.type = tokSetFgColor;
		else if( key == KeyMap["setbgcolor"]     ) t.type = tokSetBgColor;
		else if( key == KeyMap["resizecanvas"]   ) t.type = tokResizeCanvas;
		else if( key == KeyMap["spriteshow"]     ) t.type = tokSpriteShow;
		else if( key == KeyMap["spritehide"]     ) t.type = tokSpriteHide;
		else if( key == KeyMap["spritepress"]    ) t.type = tokSpritePress;
		else if( key == KeyMap["spritechange"]   ) t.type = tokSpriteChange;
		
		else if( key == KeyMap["do"]             ) t.type = tokDo; // dummy commands

		else if( key == KeyMap["message"]        ) t.type = tokMessage;
		else if( key == KeyMap["inputwindow"]    ) t.type = tokInputWindow;
		else if( key == KeyMap["print"]          ) t.type = tokPrint;
		else if( key == KeyMap["fonttype"]       ) t.type = tokFontType;
		else if( key == KeyMap["fontsize"]       ) t.type = tokFontSize;
		else if( key == KeyMap["repeat"]         ) t.type = tokRepeat;
		else if( key == KeyMap["random"]         ) t.type = tokRandom;
		else if( key == KeyMap["wait"]           ) t.type = tokWait;
		else if( key == KeyMap["wrapon"]         ) t.type = tokWrapOn;
		else if( key == KeyMap["wrapoff"]        ) t.type = tokWrapOff;
		else if( key == KeyMap["reset"]          ) t.type = tokReset;
		else                                       t.type = tokId;
		
		kdDebug(0)<<"Lexer::getToken, got: '"<<key<<"' @ ("<<t.row<<", "<<t.col<<")"<<endl;
	}
	
}

void Lexer::skipComment() {
	kdDebug(0)<<"Lexer::getChar(), skipping COMMENT."<<endl;
	QChar look = getChar();
	while ( !inputStream->atEnd() && look == '#' ) {
		while( !inputStream->atEnd() && ( look != '\n' || look != '\x0a' ) ) {
			look = getChar();
		}
		skipWhite(); // see?
		look = getChar();
	}
	ungetChar(look);
}

void Lexer::skipWhite() {
	kdDebug(0)<<"Lexer::skipWhite(), skipping WHITESPACE."<<endl;
	QChar look = getChar();
	while( !inputStream->atEnd() && look.isSpace() ) {
		look = getChar();
	}
	ungetChar(look);
}


void Lexer::getStringConstant(token& t) {
	QString constStr;
	QChar ch = getChar();
	while( ch != '"' && !inputStream->atEnd() ) {
		
		if(ch == '\\') { //escape sequence 
			ch = getChar();
			switch(ch){
				case 'n': constStr += '\n'; break;
				case 't': constStr += '\t'; break;
				case 'f': constStr += '\f'; break;
				case '"': constStr += '"';  break;
			}
		}
		else if(ch != '"'){ //anything but closing char
			constStr += ch;
		}
		
		ch = getChar();
	}
	
	t.str = constStr;
	t.type = tokString;
	t.val = 0;
	
	kdDebug(0)<<"Lexer::getStringConstant, got STRINGCONSTANT: "<<t.str<<"'"<<endl;
	
	if( inputStream->atEnd() ) {
		t.type = tokEof;
	}
}
