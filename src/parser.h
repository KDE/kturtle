/*=============================================================================
author        :Walter Schreppers
filename      :parser.h
description   :Parse source code by calling lexer repeadetly and build a tree
               so that it can be executed by Executer
bugreport(log):/
=============================================================================*/
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

/*======================= THE GRAMMAR =============================

  BNF for arithmetic expressions (improved unary minus)

    <expression>    ::= <term> [<addop> <term>]*
    <term>          ::= <signed factor> [<mulop> <signed factor>]*
    <signed factor> ::= [<addop>] <factor>
    <factor>        ::= <integer> | <variable> | (<expression>)



  DONE DIFFERENTLY!: 
  BNF for boolean algebra

    <b-expression>::= <b-term> [<orop> <b-term>]*
    <b-term>      ::= <not-factor> [AND <not-factor>]*
    <not-factor>  ::= [NOT] <b-factor>
    <b-factor>    ::= <b-literal> | <b-variable> | (<b-expression>)


  ... if, while, write, writeln have also been done...

=================================================================*/

#ifndef _PARSER_H_
#define _PARSER_H_

#include <qobject.h>

#include "lexer.h"
#include "treenode.h"


class Parser : public QObject
{   Q_OBJECT

  public:
    Parser(istream& in);
    virtual ~Parser();

    bool parse();
    TreeNode* getTree();

  signals:
    void ErrorMsg(QString s, uint row, uint col, uint code);
  
  private:
    bool isAddOp(token);
    bool isMulOp(token);
    
    void getToken();
    void Match(int);
    void Error(const QString& s, uint code = 1000);

    TreeNode* Program();
    TreeNode* Function();
    TreeNode* IdList();
    TreeNode* ParamList();
    TreeNode* Block();
    TreeNode* Statement();
    
    TreeNode* runFunction();

    TreeNode* getId();
    TreeNode* signedFactor();
    TreeNode* Factor();
    TreeNode* Term();
    TreeNode* Expression();
    
    TreeNode* Assignment  ( const QString& );
    TreeNode* FunctionCall( const QString& );
    TreeNode* Other();
    
    TreeNode* While();
    TreeNode* For();
    TreeNode* ForEach();
    TreeNode* If();
    TreeNode* getString();
    TreeNode* Return();
    TreeNode* Break();
    
    TreeNode* Clear();
    TreeNode* Go();
    TreeNode* GoX();
    TreeNode* GoY();
    TreeNode* Forward();
    TreeNode* Backward();
    TreeNode* Direction();
    TreeNode* TurnLeft();
    TreeNode* TurnRight();
    TreeNode* Center();
    TreeNode* SetPenWidth();
    TreeNode* PenUp();
    TreeNode* PenDown();
    TreeNode* SetFgColor();
    TreeNode* SetBgColor();
    TreeNode* ResizeCanvas();
    TreeNode* SpriteShow();
    TreeNode* SpriteHide();
    TreeNode* SpritePress();
    TreeNode* SpriteChange();

    TreeNode* Message();
    TreeNode* InputWindow();
    TreeNode* Print();
    TreeNode* FontType();
    TreeNode* FontSize();
    TreeNode* Repeat();
    TreeNode* Random();
    TreeNode* Wait();
    TreeNode* WrapOn();
    TreeNode* WrapOff();
    TreeNode* Reset();
    
    TreeNode* Learn();
    
    //string toString(number);

    //private locals
    //==============    
    Lexer* lexer;
    token look;
    
    TreeNode* tree; 
    bool bNoErrors;
    uint row;
    uint col;
};

#endif // _PARSER_H_
