#include <qdom.h>
#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>

#include "settings.h"
#include "lexer.h"


Lexer::~Lexer() {
}

Lexer::Lexer(istream& ifstr) {
  getKeywords();
  in = &ifstr;
  row = 1;
  col = 1;
  prevCol=1;
}


unsigned int Lexer::getRow() {
  return row;
}

unsigned int Lexer::getCol() {
  return col;
}

int Lexer::getChar() {
  int i = in->get();
  if( i == '\n' ) {
    row++;
    prevCol = col;
    col = 1;
  } else {
    col++;
  }
  return i;
}

void Lexer::ungetChar(int ch){
  if( ch == '\n' ){
    row--;
    col=prevCol;
  }
  else{
    col--;
  }
  
  in->unget();
}


int Lexer::getNumber(Number& n) {
  (*in)>>n;
  if( in->good() ) {
    return tokNumber;
  }
  else if( in->eof() ) {
    return tokEof;
  }
  else{
    return tokError;
  }
}


int Lexer::getName(QString& s) {
  char look=getChar();
  if( isalpha(look) || look=='[' || look==']' ) {
    while( ( isalpha(look) || isdigit(look) || look == '_' || look=='[' || look==']' ) && !in->eof() ) {
      s+=look;
      look=getChar();
    }
    ungetChar(look); //read one too much
    return tokId;
  }
  else{
    return tokError;  
  }
}


void Lexer::getKeywords() {
    QDomDocument KeywordsXML;
    
    kdDebug(0) << "TRname:"<< "kturtle/data/logokeywords." + Settings::logoLanguage() + ".xml" <<endl;
    kdDebug(0) << "TRfile:"<< locate("data", "kturtle/data/logokeywords." + Settings::logoLanguage() + ".xml") <<endl;

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
                    AliasMap.insert(alias, key); // put it in the map
                }
            }
            // break when just read the last child of the current <command>
            if ( m == n.lastChild() ) { break; }
            m = m.nextSibling(); // goto the next element in the current <command>
        }
        n = n.nextSibling(); // goto the next <command>
    }
}

QString Lexer::translateCommand(string s) {
    QString qs = s.c_str();
    QString name = "'" + KeyMap[qs] + "'";
    QString alias = "";
    if( !AliasMap[qs].isEmpty() ) { // translate the alias
      alias = " (" + AliasMap[qs] + ")";
    }
    return QString(name + alias);
}

void Lexer::checkKeywords(token& t) {
  if(t.type == tokId) {
    QString tt = t.str; // small *hack* to make it work ;-)
    tt = tt.lower();
    
    if( !AliasMap[tt].isEmpty() ) { // translate the alias
      tt = AliasMap[tt];
    }
    
    if(      tt == KeyMap["begin"]          ) t.type=tokBegin;
    else if( tt == KeyMap["end"]            ) t.type=tokEnd;
    else if( tt == KeyMap["while"]          ) t.type=tokWhile;
    else if( tt == KeyMap["if"]             ) t.type=tokIf;
    else if( tt == KeyMap["else"]           ) t.type=tokElse;
    else if( tt == KeyMap["for"]            ) t.type=tokFor;
    else if( tt == KeyMap["to"]             ) t.type=tokTo;
    else if( tt == KeyMap["step"]           ) t.type=tokStep;
    else if( tt == KeyMap["and"]            ) t.type=tokAnd;
    else if( tt == KeyMap["or"]             ) t.type=tokOr;
    else if( tt == KeyMap["not"]            ) t.type=tokNot;
    else if( tt == KeyMap["return"]         ) t.type=tokReturn;
    else if( tt == KeyMap["break"]          ) t.type=tokBreak;
    else if( tt == KeyMap["run"]            ) t.type=tokRun;
    else if( tt == KeyMap["foreach"]        ) t.type=tokForEach;
    else if( tt == KeyMap["in"]             ) t.type=tokIn;

    else if( tt == KeyMap["learn"]          ) t.type=tokLearn;
    
    else if( tt == KeyMap["clear"]          ) t.type=tokClear;
    else if( tt == KeyMap["go"]             ) t.type=tokGo;
    else if( tt == KeyMap["gox"]            ) t.type=tokGoX;
    else if( tt == KeyMap["goy"]            ) t.type=tokGoY;
    else if( tt == KeyMap["forward"]        ) t.type=tokForward;
    else if( tt == KeyMap["backward"]       ) t.type=tokBackward;
    else if( tt == KeyMap["direction"]      ) t.type=tokDirection;
    else if( tt == KeyMap["turnleft"]       ) t.type=tokTurnLeft;
    else if( tt == KeyMap["turnright"]      ) t.type=tokTurnRight;
    else if( tt == KeyMap["center"]         ) t.type=tokCenter;
    else if( tt == KeyMap["setpenwidth"]    ) t.type=tokSetPenWidth;
    else if( tt == KeyMap["penup"]          ) t.type=tokPenUp;
    else if( tt == KeyMap["pendown"]        ) t.type=tokPenDown;
    else if( tt == KeyMap["setfgcolor"]     ) t.type=tokSetFgColor;
    else if( tt == KeyMap["setbgcolor"]     ) t.type=tokSetBgColor;
    else if( tt == KeyMap["resizecanvas"]   ) t.type=tokResizeCanvas;
    else if( tt == KeyMap["spriteshow"]     ) t.type=tokSpriteShow;
    else if( tt == KeyMap["spritehide"]     ) t.type=tokSpriteHide;
    else if( tt == KeyMap["spritepress"]    ) t.type=tokSpritePress;
    else if( tt == KeyMap["spritechange"]   ) t.type=tokSpriteChange;
    
    else if( tt == KeyMap["do"]             ) t.type=tokDo; // dummy commands

    else if( tt == KeyMap["message"]        ) t.type=tokMessage;
    else if( tt == KeyMap["inputwindow"]    ) t.type=tokInputWindow;
    else if( tt == KeyMap["print"]          ) t.type=tokPrint;
    else if( tt == KeyMap["fonttype"]       ) t.type=tokFontType;
    else if( tt == KeyMap["fontsize"]       ) t.type=tokFontSize;
    else if( tt == KeyMap["repeat"]         ) t.type=tokRepeat;
    else if( tt == KeyMap["random"]         ) t.type=tokRandom;
    else if( tt == KeyMap["wait"]           ) t.type=tokWait;
    else if( tt == KeyMap["wrapon"]         ) t.type=tokWrapOn;
    else if( tt == KeyMap["wrapoff"]        ) t.type=tokWrapOff;
    else if( tt == KeyMap["reset"]          ) t.type=tokReset;
       
    else t.type = tokId;
  }
}

void Lexer::skipComment(){
  char look=getChar();
  while ( !in->eof() && look == '#' ){
    while( !in->eof() && look!='\n' ){
      look=getChar();
    }
    skipWhite();
    look=getChar();
  }
  ungetChar(look);
}

void Lexer::skipWhite(){
  char look=getChar();
  while( !in->eof() && isspace(look) ) look=getChar();
  ungetChar(look);
}


void Lexer::getStringConstant(token& t){
  string constStr="";
  int ch=getChar();
  while( ch != '"' && !in->eof() ){
    
    if(ch == '\\'){ //escape sequence 
      ch=getChar();
      switch(ch){
        case 'n': constStr+='\n'; break;
        case 't': constStr+='\t'; break;
        case 'f': constStr+='\f'; break;
        case '"': constStr+='"';  break;
        default : cerr<<"Unrecognized escape char \\"
                      <<(char)ch<<" in stringconstant, skipping!"
                      <<endl; break; // cies calls it debug information... no error dialog
      }
    }
    else if(ch != '"'){ //anything but closing char
      constStr+=(char) ch;
    }
    
    ch=getChar();
  }
  
  t.str=constStr;
  t.type=tokString;
  t.val=0;
  
  if(in->eof()) t.type=tokEof;
}


token Lexer::lex(){
  token t;
  t.val=0;
  t.str="";
  t.type=tokEof;
  
  skipWhite();
  skipComment();
  skipWhite();
  
  char look=getChar();
  
  if( in->eof() ) {
    t.type=tokEof;
    return t;
  }
  
  if( isalpha(look) || look=='[' || look==']' ) { //haha
    ungetChar(look);
    t.type=getName(t.str);
    checkKeywords(t);
  } else if( isdigit(look) ) {
    ungetChar(look);
    t.type=getNumber(t.val);
  } else {
    switch(look) {
      case '>': if( getChar() == '=' ) {
                  t.type=tokGe;
                } else {
                  ungetChar(look);
                  t.type=tokGt;
                }
                break;
      
      case '<': if( getChar() == '=' ) {
                  t.type=tokLe;
                } else {
                  ungetChar(look);
                  t.type=tokLt;
                }
                break;
      
      case '!': if( getChar() == '=' ) {
                  t.type=tokNe;
                } else {
                  ungetChar(look);
                  t.type='!';
                }
                break;
      
      case '=': if( getChar() == '=' ){
                  t.type=tokEq;
                } else {
                  ungetChar(look);
                  t.type=tokAssign;
                }
                break;
      
      case '"': getStringConstant( t ); break;
      
      default : t.type=look;   break;
    }
  }
  
  return t;
}


