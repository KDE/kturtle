/*=============================================================================
author        :Walter Schreppers
filename      :executer.h
description   :Execute a parse tree.
bugreport(log):/
=============================================================================*/

#ifndef _EXECUTER_H_
#define _EXECUTER_H_

#include <fstream>
#include <iostream>
#include <map>
#include <stack>
#include <string>

#include <qobject.h>

#include "treenode.h"


typedef map<string,Number>    symtable;
typedef map<string,TreeNode*> functable;

typedef stack<Number> runstack;

//using namespace std;

class Executer : public QObject
{   Q_OBJECT

  public:
    Executer(TreeNode*);
    virtual ~Executer();

    void run();

  signals:
    void ErrorMsg(QString s, int row, int col, int code = 2000);
    void Finished();
  
    void Clear();
    void Go(int x, int y);
    void GoX(int x);
    void GoY(int y);
    void Forward(int x);
    void Backward(int x);
    void Direction(double x);
    void TurnLeft(double x);
    void TurnRight(double x);
    void Center();
    void SetPenWidth(int w);
    void PenUp();
    void PenDown();
    void SetFgColor(int r, int g, int b);
    void SetBgColor(int r, int g, int b);
    void ResizeCanvas(int x, int y);
    void SpriteShow();
    void SpriteHide();
    void SpritePress();
    void SpriteChange(int x);
    
    void Print(QString text);
    void FontType(QString family, QString extra);
    void FontSize(int px);
    void WrapOn();
    void WrapOff();
    void Reset();

  private:
    void execute( TreeNode* );  

    void execBlock      ( TreeNode* );
    void execFor        ( TreeNode* );
    void execForEach    ( TreeNode* );
    void execWhile      ( TreeNode* );
    void execIf         ( TreeNode* );
    void execAssign     ( TreeNode* );
    void execExpression ( TreeNode* );
    void execId         ( TreeNode* );
    void execConstant   ( TreeNode* );
    
    void execFunction   ( TreeNode* );
    void execRetFunction( TreeNode* );
    void execReturn     ( TreeNode* );
    void execBreak      ( TreeNode* );
 
    Number getVal ( TreeNode* );
    
    void execAdd  ( TreeNode* );
    void execMul  ( TreeNode* );
    void execDiv  ( TreeNode* );
    void execSub  ( TreeNode* );
    void execNot  ( TreeNode* );

    void execGE   ( TreeNode* );
    void execGT   ( TreeNode* );
    void execLE   ( TreeNode* );
    void execLT   ( TreeNode* );
    void execNE   ( TreeNode* );
    void execEQ   ( TreeNode* );
    
    void execAnd   ( TreeNode* );
    void execOr    ( TreeNode* );
    void execMinus ( TreeNode* );

    void execRun   ( TreeNode* );
    void execWrite ( TreeNode* );
    void execSubstr( TreeNode* );

    void execClear         ( TreeNode* );
    void execGo            ( TreeNode* );
    void execGoX           ( TreeNode* );
    void execGoY           ( TreeNode* );
    void execForward       ( TreeNode* );
    void execBackward      ( TreeNode* );
    void execDirection     ( TreeNode* );
    void execTurnLeft      ( TreeNode* );
    void execTurnRight     ( TreeNode* );
    void execCenter        ( TreeNode* );
    void execSetPenWidth   ( TreeNode* );
    void execPenUp         ( TreeNode* );
    void execPenDown       ( TreeNode* );
    void execSetFgColor    ( TreeNode* );
    void execSetBgColor    ( TreeNode* );
    void execResizeCanvas  ( TreeNode* );
    void execSpriteShow    ( TreeNode* );
    void execSpriteHide    ( TreeNode* );
    void execSpritePress   ( TreeNode* );
    void execSpriteChange  ( TreeNode* );

    void execInput         ( TreeNode* );
    void execInputWindow   ( TreeNode* );
    void execPrint         ( TreeNode* );
    void execFontType      ( TreeNode* );
    void execFontSize      ( TreeNode* );
    void execRepeat        ( TreeNode* );
    void  execRandom       ( TreeNode* );
    void execWait          ( TreeNode* );
    void execWrapOn        ( TreeNode* );
    void execWrapOff       ( TreeNode* );
    void execReset         ( TreeNode* );
        
    string runCommand( const string& );
    
    void StartWaiting(float sec);
        
    //private locals
    //==============
    TreeNode* tree;
    stack<symtable> symbolTables;
    functable       functionTable;  //keep track of functionNode's
    runstack        runStack;       //stores parameters and return value of functions
    
    bool bReturn;  //used for return statements
    bool bBreak;   //used for break statement
    bool stopWaiting;
    
  private slots:
    void slotStopWaiting();
};

#endif // _EXECUTER_H_
