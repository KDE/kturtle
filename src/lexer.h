/*=============================================================================
author        :Walter Schreppers
filename      :lexer.h
description   :Split an input stream up into tokens, eat up white space and
               comments and keep track of row and column
bugreport(log):column will not be incremented enough when numbers are read
=============================================================================*/

#ifndef _LEXER_H_
#define _LEXER_H_

#include <fstream>
#include <stdlib.h>

#include "number.h"


using namespace std;

enum types {
  
  tokIf=-40,
  tokElse,
  tokWhile,
  tokFor,
  tokTo,
  tokStep,
  tokNumber,
  tokString,
  tokId,
  tokProcId,
  tokBegin,
  tokEnd,
    
  tokOr,
  tokAnd,
  tokNot,
  
  tokGe,
  tokGt,
  tokLe,
  tokLt,
  tokNe,
  tokEq,
  tokAssign,

  tokReturn,
  tokBreak,

  tokForEach,
  tokIn,
    
  tokRun,
  tokEof,
  tokError,
  
  tokLearn,
  
  tokClear,
  tokGo,
  tokGoX,
  tokGoY,
  tokForward,
  tokBackward,
  tokDirection,
  tokTurnLeft,
  tokTurnRight,
  tokCenter,
  tokSetPenWidth,
  tokPenUp,
  tokPenDown,
  tokSetFgColor,
  tokSetBgColor,
  tokResizeCanvas,
  tokSpriteShow,
  tokSpriteHide,
  tokSpritePress,
  tokSpriteChange,
  
  tokDo, // this is a dummy command

  tokMessage,
  tokInputWindow,
  tokPrint,
  tokFontType,
  tokFontSize,
  tokRepeat,
  tokRandom,
  tokWait,
  tokWrapOn,
  tokWrapOff,
  tokReset
  
};


struct token {
  Number  val;
  QString str;
  int     type;
};


class Lexer {
  
  public:
    
    //constructor and destructor
    //==========================
    Lexer( istream& );
    ~Lexer();
    QString translateCommand(string s);
  
    //public members
    //==============
    token lex(); //return token
    unsigned int getRow();
    unsigned int getCol();
    
    
  private:
    
    //private members
    //===============
    int getChar();
    void ungetChar(int);
    void skipComment();
    void skipWhite();
    void getKeywords();
    void checkKeywords(token&);
    int getNumber(Number&);
    int getName(QString&);
    void getStringConstant(token& t);

    typedef QMap<QString, QString> StringMap;
    StringMap KeyMap;
    StringMap AliasMap;
      
    //private locals
    //==============
    istream* in;
    unsigned int row,col,prevCol;
};


#endif // _LEXER_H_


