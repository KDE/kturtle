/*=============================================================================
author        :Walter Schreppers
filename      :parser.h
description   :Parse source code by calling lexer repeadetly and build a tree
               so that it can be executed by Executer
bugreport(log):/
=============================================================================*/


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
    void ErrorMsg(QString s, int row, int col, int code);
  
  private:
    bool isAddOp(token);
    bool isMulOp(token);
    
    void getToken();
    void Match(int);
    void Error(const QString& s, int code = 1000);

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
    
    TreeNode* Assignment  ( const string& );
    TreeNode* FunctionCall( const string& );
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

    TreeNode* Input();
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
    int row;
    int col;
};

#endif // _PARSER_H_
