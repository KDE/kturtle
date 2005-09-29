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
 
#ifndef _EXECUTER_H_
#define _EXECUTER_H_

#include <stack>
#include <map>

#include "token.h"
#include "treenode.h"


typedef map<QString,Value>     symtable;
typedef map<QString,TreeNode*> functable;

typedef stack<Value>           runstack;


class Executer : public QObject
{
	Q_OBJECT

	public:
		Executer(TreeNode*);
		virtual ~Executer();
	
		bool run();
		void pause();
		void abort();


	signals:
		void Finished();
		void ErrorMsg(Token&, const QString&, uint code);
		void setSelection(uint, uint, uint, uint);
		
		void InputDialog(QString& value);
		void MessageDialog(QString text);
	
		void Clear();
		void Go(double x, double y);
		void GoX(double x);
		void GoY(double y);
		void Forward(double x);
		void Backward(double x);
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
		void execute           (TreeNode*);  
	
		void execBlock         (TreeNode*);
		void execFor           (TreeNode*);
		void execForEach       (TreeNode*);
		void execWhile         (TreeNode*);
		void execIf            (TreeNode*);
		void execAssign        (TreeNode*);
		void execExpression    (/*TreeNode*/);
		void execId            (TreeNode*);
		void execConstant      (/*TreeNode*/);
		
		void createFunction    (TreeNode*);
		void execFunction      (TreeNode*);
		void execRetFunction   (TreeNode*);
		void execReturn        (TreeNode*);
		void execBreak         (/*TreeNode*/);
		
		void execAdd           (TreeNode*);
		void execMul           (TreeNode*);
		void execDiv           (TreeNode*);
		void execSub           (TreeNode*);
		void execNot           (TreeNode*);
	
		void execGE            (TreeNode*);
		void execGT            (TreeNode*);
		void execLE            (TreeNode*);
		void execLT            (TreeNode*);
		void execNE            (TreeNode*);
		void execEQ            (TreeNode*);
		
		void execAnd           (TreeNode*);
		void execOr            (TreeNode*);
		void execMinus         (TreeNode*);
	
		void execRun           (TreeNode*);
	
		void execClear         (TreeNode*);
		void execGo            (TreeNode*);
		void execGoX           (TreeNode*);
		void execGoY           (TreeNode*);
		void execForward       (TreeNode*);
		void execBackward      (TreeNode*);
		void execDirection     (TreeNode*);
		void execTurnLeft      (TreeNode*);
		void execTurnRight     (TreeNode*);
		void execCenter        (TreeNode*);
		void execSetPenWidth   (TreeNode*);
		void execPenUp         (TreeNode*);
		void execPenDown       (TreeNode*);
		void execSetFgColor    (TreeNode*);
		void execSetBgColor    (TreeNode*);
		void execResizeCanvas  (TreeNode*);
		void execSpriteShow    (TreeNode*);
		void execSpriteHide    (TreeNode*);
		void execSpritePress   (TreeNode*);
		void execSpriteChange  (TreeNode*);
	
		void execMessage       (TreeNode*);
		void execInputWindow   (TreeNode*);
		void execPrint         (TreeNode*);
		void execFontType      (TreeNode*);
		void execFontSize      (TreeNode*);
		void execRepeat        (TreeNode*);
		void execRandom        (TreeNode*);
		void execWait          (TreeNode*);
		void execWrapOn        (TreeNode*);
		void execWrapOff       (TreeNode*);
		void execReset         (TreeNode*);
	
		Value exec2getValue    (TreeNode*);
		
		QString runCommand(const QString&);
		
		bool checkParameterQuantity(TreeNode*, uint quantity, int errorCode);
		bool checkParameterType(TreeNode*, int valueType, int errorCode);
	
		void slowDown(TreeNode*);
		void startWaiting(int msec);
		void startPausing();
		
		// private locals
		TreeNode*        tree;
		stack<symtable>  symbolTables;
		functable        functionTable;  // keep track of functionNode's
		runstack         runStack;       // stores parameters and return value of functions
		
		int              runSpeed;
		bool             bReturn;       // used for return statements
		bool             bBreak;        // used for break statement
		bool             bPause;        // used to pause execution
		bool             bAbort;        // used to abort execution
		bool             bStopWaiting;  // used for wait-command


	private slots:
		void slotStopWaiting();
		void slotChangeSpeed(int speed);
		void slotStopPausing();
};

#endif // _EXECUTER_H_
