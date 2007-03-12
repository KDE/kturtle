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

// This file is originally written by Walter Scheppers, but almost
// every aspect of it is slightly changed by Cies Breijs.
  
 
#include <unistd.h> // for usleep();
#include <stdlib.h>

#include <qtimer.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>

#include "executer.h"

// this function is used in executer and canvas:
#define ROUND2INT(x) ( (x) >= 0 ? (int)( (x) + .5 ) : (int)( (x) - .5 ) )


Executer::Executer(TreeNode* tree)
{
	this->tree = tree;
	functionTable.clear();
	bBreak = false;
	bReturn = false;
}

Executer::~Executer()
{
	emit Finished();
}

bool Executer::run()
{
	bBreak = false;
	bReturn = false;
	bPause = false;
	bAbort = false;
	symtable main;
	symbolTables.push(main); // new symbol table for main block
	
	TreeNode::const_iterator i;
	for (i = tree->begin(); i != tree->end(); ++i)
	{
		if (bAbort) return false;
		kapp->processEvents();
		execute(*i);
	
		symbolTables.pop(); //free up stack
	}
	return true;
}

void Executer::slowDown(TreeNode* node)
{
	switch (runSpeed)
	{
		case 1: // slow
			startWaiting(0);
			break;
		
		case 2: // slower
			startWaiting(250);
			break;
		
		case 3: // slowest
			startWaiting(900);
			break;
		
		default:
			kdDebug(0)<<"Executer: Wrong execution speed given"<<endl;
			break;
	}
	Token tok = node->getToken();
	emit setSelection(tok.start.row, tok.start.col, tok.end.row, tok.end.col);
}

void Executer::slotChangeSpeed(int speed)
{
	runSpeed = speed;
}

void Executer::pause()
{
	// The next line is within all loops of the Executer
	//     if (bAbort) return;
	// mostly before
	//     kapp->processEvents();
	// this to keep the GUI of KTurtle accessible while executing the logo code
	// so the Abort button can be clicked, and will function
	bPause = true;
}

void Executer::abort()
{
	// The next line is within all loops of the Executer
	//     if(bAbort) return;
	// mostly before
	//     kapp->processEvents();
	// this to keep the GUI of KTurtle accessible while executing the logo code
	// so the Abort button can be clicked, and will function
	bAbort = true;
}


void Executer::execute(TreeNode* node)
{
	switch ( node->getType() )
	{
		case blockNode          : execBlock(node);          break;
		case forNode            : execFor(node);            break;
		case forEachNode        : execForEach(node);        break;
		case whileNode          : execWhile(node);          break;
		case ifNode             : execIf(node);             break;
		case assignNode         : execAssign(node);         break;
		case expressionNode     : execExpression(/*node*/); break;
		case idNode             : execId(node);             break;
		case constantNode       : execConstant(/*node*/);   break; // does nothing value allready set
		
		case addNode            : execAdd(node);            break;
		case mulNode            : execMul(node);            break;
		case divNode            : execDiv(node);            break;
		case subNode            : execSub(node);            break;
		case minusNode          : execMinus(node);          break;
	
		case nodeGE             : execGE(node);             break; 
		case nodeGT             : execGT(node);             break;
		case nodeLE             : execLE(node);             break;
		case nodeLT             : execLT(node);             break;
		case nodeNE             : execNE(node);             break;
		case nodeEQ             : execEQ(node);             break;
		
		case andNode            : execAnd(node);            break;
		case orNode             : execOr(node);             break;
		case notNode            : execNot(node);            break;
		
		case functionNode       : createFunction(node);     break;
		case functionCallNode   : execFunction(node);       break;
		case funcReturnNode     : execRetFunction(node);    break;
		case returnNode         : execReturn(node);         break;
		case breakNode          : execBreak(/*node*/);      break;
		
		case runNode            : execRun(node);            break;
		
		case ClearNode          : execClear(node);          break;
		case GoNode             : execGo(node);             break;
		case GoXNode            : execGoX(node);            break;
		case GoYNode            : execGoY(node);            break;
		case ForwardNode        : execForward(node);        break;
		case BackwardNode       : execBackward(node);       break;
		case DirectionNode      : execDirection(node);      break;
		case TurnLeftNode       : execTurnLeft(node);       break;
		case TurnRightNode      : execTurnRight(node);      break;
		case CenterNode         : execCenter(node);         break;
		case SetPenWidthNode    : execSetPenWidth(node);    break;
		case PenUpNode          : execPenUp(node);          break;
		case PenDownNode        : execPenDown(node);        break;
		case SetFgColorNode     : execSetFgColor(node);     break;
		case SetBgColorNode     : execSetBgColor(node);     break;
		case ResizeCanvasNode   : execResizeCanvas(node);   break;
		case SpriteShowNode     : execSpriteShow(node);     break;
		case SpriteHideNode     : execSpriteHide(node);     break;
		case SpritePressNode    : execSpritePress(node);    break;
		case SpriteChangeNode   : execSpriteChange(node);   break;

		case MessageNode        : execMessage(node);        break;
		case InputWindowNode    : execInputWindow(node);    break;
		case printNode          : execPrint(node);          break;
		case FontTypeNode       : execFontType(node);       break;
		case FontSizeNode       : execFontSize(node);       break;
		case RepeatNode         : execRepeat(node);         break;
		case RandomNode         : execRandom(node);         break;
		case WaitNode           : execWait(node);           break;
		case WrapOnNode         : execWrapOn(node);         break;
		case WrapOffNode        : execWrapOff(node);        break;
		case ResetNode          : execReset(node);          break;
		
		case EndOfFileNode      : break; // just do nothing is enough
		
		case Unknown            : // dont break but fallthrough to default
		
		default:
			kdDebug(0)<<"Found unsupported node named '"<<node->getLook()<<"' in the tree @ ("<<node->getRow()<<", "<<node->getCol()<<")"<<endl;
			break;
	}  
}


void Executer::createFunction(TreeNode* node)
{
	QString funcname = node->getLook();
	functionTable[funcname] = node; //store for later use
}


//execute a function
void Executer::execFunction(TreeNode* node)
{
	QString funcname = node->getLook();

	// locate function node  
	functable::iterator p = functionTable.find(funcname);
	if ( p == functionTable.end() )
	{
		emit ErrorMsg(node->getToken(), i18n("Call to undefined function: %1.").arg(funcname), 5010);
		return;
	}
	
	TreeNode* funcnode   = p->second;
	TreeNode* funcIds    = funcnode->firstChild();
	TreeNode* callparams = node->firstChild();
		
	// check if number of parameters match
	if ( callparams->size() != funcIds->size() )
	{
		emit ErrorMsg(node->getToken(), i18n("Call to function '%1' with wrong number of parameters.").arg(funcname), 5020);
		return;
	}

	// pass parameters to function
	// by adding them to it's symboltable and setting the values
	TreeNode::iterator pfrom, pto = funcIds->begin();
	symtable funcSymTable;
	
	for (pfrom = callparams->begin(); pfrom != callparams->end(); ++pfrom )
	{
		if (bAbort) return;
		if (bPause) startPausing();
		kapp->processEvents();
		
		// execute the parameters which can be expressions
		execute(*pfrom); 
		
		QString idname = (*pto)->getLook();
		funcSymTable[idname] = (*pfrom)->getValue();
		++pto;
	}
	
	symbolTables.push(funcSymTable); // use new symboltable for current function
	
	// execute function statement block
	bReturn = false; // set to true when return is called
	execute( funcnode->secondChild() );
	bReturn = false; // function execution done
	
	symbolTables.pop(); // release function symboltable    
}


// execute a function and expect and get return 
// value from stack
// first child   = function name
// second child  = parameters
void Executer::execRetFunction(TreeNode* node)
{
	execFunction(node);
	if (runStack.size() == 0)
	{
		emit ErrorMsg(node->getToken(), i18n("Function %1 did not return a value.").arg( node->getLook() ), 5030);
		return;
	}
	node->setValue( runStack.top() ); // set return val
	runStack.pop(); // remove from stack
}


void Executer::execReturn(TreeNode* node)
{
	execute( node->firstChild() ); // execute return expression
	runStack.push( node->firstChild()->getValue() );
	bReturn = true; // notify blocks of return
}


void Executer::execBreak(/*TreeNode* node*/)
{
	bBreak = true; // stops loop block execution
}


void Executer::execBlock(TreeNode* node)
{
	// execute all statements in block
	TreeNode::iterator i;
	for (i = node->begin(); i != node->end(); ++i)
	{
		if (runSpeed != 0) slowDown(*i);
		if (bAbort) return;
		if (bPause) startPausing();
		kapp->processEvents();
		
		execute(*i);
	
		if (bReturn || bBreak) break; //jump out of block		
	}
}


void Executer::execForEach(TreeNode* node)
{
	// sorry, not fully implemented/tested yet
	TreeNode* expr1      = node->firstChild();
	TreeNode* expr2      = node->secondChild();
	TreeNode* statements = node->thirdChild();
	
	execute(expr1);
	execute(expr2);
	
	QString expStr1 = expr1->getValue().String();
	QString expStr2 = expr2->getValue().String();
	
	bBreak = false;
	
	int i = expStr2.contains(expStr1, false);
	for ( ; i > 0; i-- )
	{
		if (bAbort) return;
		if (bPause) startPausing();
		kapp->processEvents();
		
		execute(statements);
		if (bBreak || bReturn) break; //jump out loop
	}
	bBreak = false;
}



void Executer::execFor(TreeNode* node)
{
	TreeNode* id = node->firstChild();
	TreeNode* startNode = node->secondChild();
	TreeNode* stopNode = node->thirdChild();
	TreeNode* statements = node->fourthChild();
	
	QString name = id->getLook();
	
	execute(startNode);
	//assign startval to id
	Value startVal = startNode->getValue();
	( symbolTables.top() )[ name ] = startVal;
	
	
	execute(stopNode);
	Value stopVal = stopNode->getValue();
	
	if(node->size() == 4 ) //for loop without step part
	{
		bBreak = false;
		for (double d = startVal.Number(); d <= stopVal.Number(); d = d + 1)
		{
			if (bAbort) return;
			if (bPause) startPausing();
			kapp->processEvents();
			( symbolTables.top() )[name] = d;
			execute( statements );
			if (bBreak || bReturn) break; //jump out loop
		}
		bBreak = false;
	}
	else //for loop with step part
	{
		TreeNode* step = node->fourthChild();
		statements = node->fifthChild();
		
		execute(step);
		Value stepVal = step->getValue();
		bBreak = false;
		if ( (stepVal.Number() >= 0.0) && (startVal.Number() <= stopVal.Number() ) )
		{
			for ( double d = startVal.Number(); d <= stopVal.Number(); d = d + stepVal.Number() )
			{
				if (bAbort) return;
				if (bPause) startPausing();
				kapp->processEvents();
				
				(symbolTables.top() )[name] = d;
				execute( statements );
				if (bBreak || bReturn) break; //jump out loop
			}
		}
		else if ( (stepVal.Number() < 0.0) && (startVal.Number() >= stopVal.Number() ) )
		{
			for (double d = startVal.Number(); d >= stopVal.Number(); d = d + stepVal.Number() )
			{
				if (bAbort) return;
				if (bPause) startPausing();
				kapp->processEvents();
				
				( symbolTables.top() )[name] = d;
				execute(statements);
				if (bBreak || bReturn) break; //jump out loop
			}
		}
		bBreak = false;
	}
}



void Executer::execRepeat(TreeNode* node)
{
	TreeNode* value = node->firstChild();
	TreeNode* statements = node->secondChild();
	
	bBreak = false;
	execute(value);
	for ( int i = ROUND2INT( value->getValue().Number() ); i > 0; i-- )
	{
		if (bAbort) return;
		if (bPause) startPausing();
		kapp->processEvents();
		
		execute(statements);
		if (bBreak || bReturn) break; //jump out loop
	}
	bBreak = false;
}


void Executer::execWhile(TreeNode* node)
{
	TreeNode* condition = node->firstChild();
	TreeNode* statements = node->secondChild();
	
	bBreak = false;
	execute(condition);
	while (condition->getValue().Number() != 0)
	{
		if (bAbort) return;
		if (bPause) startPausing();
		kapp->processEvents();
		
		execute(statements);
		if (bBreak || bReturn) break; //jump out loop
		execute(condition);
	}
	bBreak = false;
}

     
void Executer::execIf(TreeNode* node)
{
	TreeNode* condition = node->firstChild();
	TreeNode* ifblok = node->secondChild();
	
	//determine if there is an else part
	if (node->size() == 2) // no else
	{
		execute( condition );
		if( condition->getValue().Number() != 0 ) execute(ifblok);   
	}
	else // else part given
	{
		TreeNode* elseblok = node->thirdChild();
		execute( condition );
		if( condition->getValue().Number() != 0 ) execute(ifblok);
		else execute( elseblok );
	}
}



void Executer::execAssign(TreeNode* node)
{
	TreeNode* expr = node->firstChild();

	execute(expr);
	( symbolTables.top() )[ node->getLook() ] = expr->getValue();
}
    
void Executer::execExpression(/*TreeNode* node*/)
{
	// execExpression is not implemented, because it should not be needed!
}

void Executer::execId(TreeNode* node)
{
	node->setValue( ( symbolTables.top() )[ node->getLook() ] );
}

void Executer::execConstant(/*TreeNode* node*/)
{
	// do nothing, value is already set
}

Value Executer::exec2getValue(TreeNode* node)
{
	execute(node);
	return node->getValue();
}

        
void Executer::execAdd(TreeNode* node)
{
	Value left( exec2getValue( node->firstChild() ) );
	Value right( exec2getValue( node->secondChild() ) );
	
	if (left.Type() == numberValue && right.Type() == numberValue) node->setValue( left + right );
	else node->setValue( left.String().append( right.String() ) );
}

       
void Executer::execMul(TreeNode* node)
{
	Value left( exec2getValue( node->firstChild() ) );
	Value right( exec2getValue( node->secondChild() ) );
	
	if (left.Type() == numberValue && right.Type() == numberValue) node->setValue( left * right );
	else emit ErrorMsg(node->getToken(), i18n("Can only multiply numbers."), 9000);
}

       
void Executer::execDiv(TreeNode* node)
{
	Value left( exec2getValue( node->firstChild() ) );
	Value right( exec2getValue( node->secondChild() ) );
	
	if (left.Type() == numberValue && right.Type() == numberValue)
	{
		if (right.Number() == 0) emit ErrorMsg(node->getToken(), i18n("Cannot divide by zero."), 9000);
		else node->setValue( left / right );
	}
	else emit ErrorMsg(node->getToken(), i18n("Can only divide numbers."), 9000);
}

       
void Executer::execSub(TreeNode* node)
{
	Value left( exec2getValue( node->firstChild() ) );
	Value right( exec2getValue( node->secondChild() ) );
	
	if (left.Type() == numberValue && right.Type() == numberValue)
		node->setValue( left - right );
	
	else emit ErrorMsg(node->getToken(), i18n("Can only subtract numbers."), 9000);
}


       
void Executer::execLT(TreeNode* node)
{
	Value left( exec2getValue( node->firstChild() ) );
	Value right( exec2getValue( node->secondChild() ) );
	
	node->setValue( left < right );
}

void Executer::execLE(TreeNode* node)
{
	Value left( exec2getValue( node->firstChild() ) );
	Value right( exec2getValue( node->secondChild() ) );
	
	node->setValue( left <= right );
}

void Executer::execGT(TreeNode* node)
{
	Value left( exec2getValue( node->firstChild() ) );
	Value right( exec2getValue( node->secondChild() ) );
	
	node->setValue( left > right );
}

void Executer::execGE(TreeNode* node)
{
	Value left( exec2getValue( node->firstChild() ) );
	Value right( exec2getValue( node->secondChild() ) );
	
	node->setValue( left >= right );
}


void Executer::execEQ(TreeNode* node)
{
	Value left( exec2getValue( node->firstChild() ) );
	Value right( exec2getValue( node->secondChild() ) );
	
	node->setValue( left == right );
}


void Executer::execNE(TreeNode* node)
{
	Value left( exec2getValue( node->firstChild() ) );
	Value right( exec2getValue( node->secondChild() ) );
	
	node->setValue( left != right );
}


  
void Executer::execAnd(TreeNode* node)
{
	bool nl = exec2getValue( node->firstChild() ).Number() != 0;
	bool nr = exec2getValue( node->secondChild() ).Number() != 0;
	node->setValue( (double) (nl && nr) );
}

       
void Executer::execOr(TreeNode* node)
{
	bool nl = exec2getValue( node->firstChild() ).Number() != 0;
	bool nr = exec2getValue( node->secondChild() ).Number() != 0;
	node->setValue(nl || nr);
}


void Executer::execNot(TreeNode* node)
{
	node->setValue( exec2getValue( node->firstChild() ).Number() == 0 ); 
}


void Executer::execMinus(TreeNode* node)
{
	node->setValue( -exec2getValue( node->firstChild() ).Number() ); 
}


QString Executer::runCommand(const QString& command)
{
	FILE *pstream;
	
	if ( ( pstream = popen( command.ascii(), "r" ) ) == NULL ) return ("");
	
	QString Line;
	char buf[100];
	
	while( fgets(buf, sizeof(buf), pstream) !=NULL) {
		if (bAbort) return ("");
		kapp->processEvents();
		
		Line += buf;
	}
	pclose(pstream);
	return Line;
}


void Executer::execRun(TreeNode* node)
{
	QString cmd = exec2getValue( node->firstChild() ).String();
	node->setValue( runCommand(cmd) );
}




void Executer::execClear(TreeNode* node)
{
	if ( checkParameterQuantity(node, 0, 5060) ) emit Clear();
}

void Executer::execCenter(TreeNode* node)
{
	if ( checkParameterQuantity(node, 0, 5060) ) emit Center();
}

void Executer::execPenUp(TreeNode* node)
{
	if ( checkParameterQuantity(node, 0, 5060) ) emit PenUp();
}

void Executer::execPenDown(TreeNode* node)
{
	if ( checkParameterQuantity(node, 0, 5060) ) emit PenDown();
}

void Executer::execSpriteShow(TreeNode* node)
{
	if ( checkParameterQuantity(node, 0, 5060) ) emit SpriteShow();
}

void Executer::execSpriteHide(TreeNode* node)
{
	if ( checkParameterQuantity(node, 0, 5060) ) emit SpriteHide();
}

void Executer::execSpritePress(TreeNode* node)
{
	if ( checkParameterQuantity(node, 0, 5060) ) emit SpritePress();
}

void Executer::execWrapOn(TreeNode* node)
{
	if ( checkParameterQuantity(node, 0, 5060) ) emit WrapOn();
}

void Executer::execWrapOff(TreeNode* node)
{
	if ( checkParameterQuantity(node, 0, 5060) ) emit WrapOff();
}

void Executer::execReset(TreeNode* node)
{
	if ( checkParameterQuantity(node, 0, 5060) ) emit Reset();
}

void Executer::execMessage(TreeNode* node)
{
	if ( checkParameterQuantity(node, 1, 5060) && checkParameterType(node, stringValue, 5060) )
		emit MessageDialog( node->firstChild()->getValue().String() );
}






void Executer::execGoX(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 1, 5060) ) return;
	TreeNode* param1 = node->firstChild();
	execute(param1);
	if ( checkParameterType(node, numberValue, 5060) )
	{
		emit GoX( param1->getValue().Number() );
	}
}

void Executer::execGoY(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 1, 5060) ) return;
	TreeNode* param1 = node->firstChild();
	execute(param1);
	if ( checkParameterType(node, numberValue, 5060) )
	{
		emit GoY( param1->getValue().Number() );
	}
}

void Executer::execForward(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 1, 5060) ) return;
	TreeNode* param1 = node->firstChild();
	execute(param1);
	if ( checkParameterType(node, numberValue, 5060) )
	{
		emit Forward( param1->getValue().Number() );
	}
}

void Executer::execBackward(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 1, 5060) ) return;
	TreeNode* param1 = node->firstChild();
	execute(param1);
	if ( checkParameterType(node, numberValue, 5060) )
	{
		emit Backward( param1->getValue().Number() );
	}
}

void Executer::execDirection(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 1, 5060) ) return;
	TreeNode* param1 = node->firstChild();
	execute(param1);
	if ( checkParameterType(node, numberValue, 5060) )
	{
		emit Direction( param1->getValue().Number() );
	}
}

void Executer::execTurnLeft(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 1, 5060) ) return;
	TreeNode* param1 = node->firstChild();
	execute(param1);
	if ( checkParameterType(node, numberValue, 5060) )
	{
		emit TurnLeft( param1->getValue().Number() );
	}
}

void Executer::execTurnRight(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 1, 5060) ) return;
	TreeNode* param1 = node->firstChild();
	execute(param1);
	if ( checkParameterType(node, numberValue, 5060) )
	{
		emit TurnRight( param1->getValue().Number() );
	}
}

void Executer::execSetPenWidth(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 1, 5060) ) return;
	TreeNode* param1 = node->firstChild();
	execute(param1);
	if ( checkParameterType(node, numberValue, 5060) )
	{
		int x = ROUND2INT( param1->getValue().Number() ); // pull the number value & round it to int
		if (x < 1 || x > 10000)
			emit ErrorMsg(node->getToken(), i18n("The penwidth cannot be set to something smaller than 1, or bigger than 10000."), 6050);
		else
			emit SetPenWidth(x);
	}
}

void Executer::execSpriteChange(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 1, 5060) ) return;
	TreeNode* param1 = node->firstChild();
	execute(param1);
	if ( checkParameterType(node, numberValue, 5060) )
	{
		int x = ROUND2INT( param1->getValue().Number() ); // pull the number value & round it to int
		emit SpriteChange(x);
	}
}

void Executer::execFontSize(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 1, 5060) ) return;
	TreeNode* param1 = node->firstChild();
	execute(param1);
	if ( checkParameterType(node, numberValue, 5060) )
	{
		int x = ROUND2INT( param1->getValue().Number() ); // pull the number value & round it to int
		if ( x < 0 || x > 350 )
			emit ErrorMsg(node->getToken(), i18n("The parameters of function %1 must be within range: 0 to 350.").arg( node->getLook() ), 5065);
		else
			emit FontSize(x);
	}
}






void Executer::execGo(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 2, 5060) ) return;	
	TreeNode* nodeX = node->firstChild(); // getting
	TreeNode* nodeY = node->secondChild();
	execute(nodeX); // executing
	execute(nodeY);
	
	if ( checkParameterType(node, numberValue, 5060) )
	{
		emit Go( nodeX->getValue().Number(), nodeY->getValue().Number() );
	}
}

void Executer::execResizeCanvas(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 2, 5060) ) return;
	TreeNode* nodeX = node->firstChild(); // getting
	TreeNode* nodeY = node->secondChild();
	execute(nodeX); // executing
	execute(nodeY);
	
	if ( checkParameterType(node, numberValue, 5060) )
	{
		int x = ROUND2INT( nodeX->getValue().Number() ); // converting & rounding to int
		int y = ROUND2INT( nodeY->getValue().Number() );
		if ( ( x < 1 || y < 1 ) || ( x > 10000 || y > 10000 ) )
			emit ErrorMsg(node->getToken(), i18n("The parameters of the %1 command must be numbers in the range: 1 to 10000.").arg( node->getLook() ), 7030);
		else
			emit ResizeCanvas(x, y);
	}
}

void Executer::execRandom(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 2, 5060) ) return;
	TreeNode* nodeX = node->firstChild(); // getting
	TreeNode* nodeY = node->secondChild();
	execute(nodeX); // executing
	execute(nodeY);
	
	if ( !checkParameterType(node, numberValue, 5060) ) return;
	double x = nodeX->getValue().Number();
	double y = nodeY->getValue().Number();
	double r = (double)( KApplication::random() ) / RAND_MAX;
	node->setValue( r * ( y - x ) + x );
}





void Executer::execSetFgColor(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 3, 5060) ) return;
	TreeNode* nodeR = node->firstChild(); // getting
	TreeNode* nodeG = node->secondChild();
	TreeNode* nodeB = node->thirdChild();
	execute(nodeR); // executing
	execute(nodeG);
	execute(nodeB);
	if ( checkParameterType(node, numberValue, 5060) )
	{
		int r = ROUND2INT( nodeR->getValue().Number() ); // converting & rounding to int
		int g = ROUND2INT( nodeG->getValue().Number() );
		int b = ROUND2INT( nodeB->getValue().Number() );
		if ( ( r < 0 || g < 0 || b < 0 ) || ( r > 255 || g > 255 || b > 255 ) )
			emit ErrorMsg(node->getToken(), i18n("The parameters of the %1 command must be numbers in the range: 0 to 255.").arg( node->getLook() ), 6090);
		else
			emit SetFgColor(r, g, b);
	}
}

void Executer::execSetBgColor(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 3, 5060) ) return;
	TreeNode* nodeR = node->firstChild(); // getting
	TreeNode* nodeG = node->secondChild();
	TreeNode* nodeB = node->thirdChild();
	execute(nodeR); // executing
	execute(nodeG);
	execute(nodeB);
	if ( checkParameterType(node, numberValue, 5060) )
	{
		int r = ROUND2INT( nodeR->getValue().Number() ); // converting & rounding to int
		int g = ROUND2INT( nodeG->getValue().Number() );
		int b = ROUND2INT( nodeB->getValue().Number() );
		if ( ( r < 0 || g < 0 || b < 0 ) || ( r > 255 || g > 255 || b > 255 ) )
			emit ErrorMsg(node->getToken(), i18n("The parameters of the %1 command must be numbers in the range: 0 to 255.").arg( node->getLook() ), 6090);
		else
			emit SetBgColor(r, g, b);
	}
}











void Executer::execInputWindow(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 1, 5060) ) return;

	QString value = node->firstChild()->getValue().String();
	emit InputDialog(value);
	
	node->setType(constantNode);
	if ( value.isEmpty() ) node->getValue().resetValue(); // set value back to empty
	else
	{
		bool ok = true;
		double num = value.toDouble(&ok); // to see if the value from the InpDialog is a float
		if (ok) node->setValue(num);
		else    node->setValue(value);
	}
}

void Executer::execPrint(TreeNode* node)
{
	if (node->size() == 0) 
	{
		emit ErrorMsg(node->getToken(), i18n("The print command needs input"), 5050);
		return;
	}
	TreeNode::iterator i;
	QString str = "";
	for (i = node->begin(); i != node->end(); ++i)
	{
		execute(*i); // execute expression
		str = str + (*i)->getValue().String();
	}
	emit Print(str);
}

void Executer::execFontType(TreeNode* node)
{
	// if not 2 params go staight to the checkParam, diplay the error, and return to prevent a crash
	if ( !checkParameterQuantity(node, 2, 5060) && !checkParameterType(node, stringValue, 5060) ) return;
	
	QString extra;
	if (node->size() == 2) QString extra = node->secondChild()->getValue().String();
	QString family = node->firstChild()->getValue().String();
	emit FontType(family, extra);
}


void Executer::execWait(TreeNode* node)
{
	if ( !checkParameterQuantity(node, 1, 5060) ) return;
	TreeNode* param1 = node->firstChild();
	execute(param1);
	if ( !checkParameterType(node, numberValue, 5060) ) return;
	int msec = (int)( 1000 * param1->getValue().Number() );
	startWaiting(msec);
}

void Executer::startWaiting(int msec)
{
	bStopWaiting = false;
	// call a timer that sets stopWaiting to true when it runs 
	QTimer::singleShot( msec, this, SLOT( slotStopWaiting() ) );
	while (bStopWaiting == false)
	{
		if (bAbort) return; // waits need to be interrupted by the stop action
		if (bPause) startPausing();
		kapp->processEvents();

		// only 10 times per second is enough... else the CPU gets 100% loaded ( not nice :)
		usleep(100000);
	}
}

void Executer::slotStopWaiting()
{
	bStopWaiting = true;
}

void Executer::startPausing()
{
	while (bPause == true)
	{
		if (bAbort) return; // waits need to be interrupted by the stop action
		kapp->processEvents();
		// only 10 times per second is enough... else the CPU gets 100% loaded ( not nice :)
		usleep(100000);
	}
}

void Executer::slotStopPausing()
{
	bPause = false;
}




bool Executer::checkParameterQuantity(TreeNode* node, uint quantity, int errorCode)
{
	if (quantity == 0)
	{
		if (node->size() == 0) return true; // thats easy!
		emit ErrorMsg(node->getToken(), i18n("The %1 command accepts no parameters.").arg( node->getLook() ), errorCode);
		return false;
	}
	
	uint nodeSize = node->size();
	if (nodeSize != 0) // when all parameters are forgotten the parser puts a Unknown/tokEOL param, catch this:
		if (node->firstChild()->getToken().type == tokEOL) nodeSize = 0;
	
	if (nodeSize != quantity)
	{
		if (nodeSize < quantity)
		{
			emit ErrorMsg(node->getToken(), i18n("The %1 command was called with %2 but needs 1 parameter.", "The %1 command was called with %2 but needs %n parameters.", quantity).arg( node->getLook() ).arg(nodeSize), errorCode);
		}
		else
		{
			emit ErrorMsg(node->getToken(), i18n("The %1 command was called with %2 but only accepts 1 parameter.", "The %1 command was called with %2 but only accepts %n parameters.", quantity).arg( node->getLook() ).arg(nodeSize), errorCode);
		}
		return false;
	}
	return true; // if all tests passed
}


bool Executer::checkParameterType(TreeNode* node, int valueType, int errorCode)
{
	uint quantity = node->size();
	uint ii = 1;
	TreeNode::iterator i = node->begin();
	while ( i != node->end() && ii <= quantity )
	{
		if ( (*i)->getValue().Type() != valueType )
		{
			switch (valueType)
			{
				case stringValue:
					if (quantity == 1)
						emit ErrorMsg(node->getToken(), i18n("The %1 command only accepts a string as its parameter.").arg( node->getLook() ), errorCode); 
					else
						emit ErrorMsg(node->getToken(), i18n("The %1 command only accepts strings as its parameters.").arg( node->getLook() ), errorCode);
					break;
				
				case numberValue:
					if (quantity == 1)
						emit ErrorMsg(node->getToken(), i18n("The %1 command only accepts a number as its parameter.").arg( node->getLook() ), errorCode);
					else
						emit ErrorMsg(node->getToken(), i18n("The %1 command only accepts numbers as its parameters.").arg( node->getLook() ), errorCode);
					break;
			}
			return false;
		}
		++i;
		ii++;
	}
	return true; // if all tests passed
}

#include "executer.moc"
