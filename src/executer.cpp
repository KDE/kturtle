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

// This file is originally written by Walter Scheppers, but allmost
// every aspect of it is slightly changed by Cies Breijs.
  
 
#include <unistd.h> // for usleep();

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
	bAbort = false;
	symtable main;
	symbolTables.push(main); // new symbol table for main block
	
	TreeNode::const_iterator i;
	for( i = tree->begin(); i != tree->end(); ++i )
	{
		if (bAbort) return false;
		kapp->processEvents();
		execute(*i);
	
		symbolTables.pop(); //free up stack
	}
	return true;
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
		case stringConstantNode : execConstant(/*node*/);   break; // idem
		
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
	TreeNode::iterator pfrom,pto = funcIds->begin();
	symtable funcSymTable;
	
	for (pfrom = callparams->begin(); pfrom != callparams->end(); ++pfrom )
	{
		if (bAbort) return;
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


//execute a function and expect and get return 
//value from stack
//first child   = function name
//second child  = parameters
void Executer::execRetFunction(TreeNode* node)
{
	execFunction(node);
	if( runStack.size() == 0 )
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
		if (bAbort) return;
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
	Value startVal=startNode->getValue();
	( symbolTables.top() )[ name ] = startVal;
	
	
	execute(stopNode);
	Value stopVal=stopNode->getValue();
	
	if(node->size() == 4 ) //for loop without step part
	{
		bBreak = false;
		for (double d = startVal.Number(); d <= stopVal.Number(); d = d + 1)
		{
			if (bAbort) return;
			kapp->processEvents();
			(symbolTables.top() )[name] = d;
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
		bBreak=false;
		if( (stepVal.Number() >= 0.0) && (startVal.Number() <= stopVal.Number()) )
		{
			for( double d = startVal.Number(); d <= stopVal.Number(); d = d + stepVal.Number() )
			{
				if (bAbort) return;
				kapp->processEvents();
				
				(symbolTables.top() )[name] = d;
				execute( statements );
				if (bBreak || bReturn) break; //jump out loop
			}
		}
		else if ( (stepVal.Number() < 0.0) && (startVal.Number() >= stopVal.Number()) )
		{
			for (double d = startVal.Number(); d >= stopVal.Number(); d = d + stepVal.Number())
			{
				if (bAbort) return;
				kapp->processEvents();
				
				( symbolTables.top() )[name] = d;
				execute(statements);
				if (bBreak || bReturn) break; //jump out loop
			}
		}
		bBreak = false;
	}
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
	// if one of the values to be summed is not a constant they should be concatenated
	if (node->firstChild()->getType() == constantNode && node->secondChild()->getType() == constantNode)
	{
		node->setValue( exec2getValue( node->firstChild() )
		                +
		                exec2getValue( node->secondChild() )
		              );
	}
	else
	{
		node->setValue( exec2getValue( node->firstChild() ).String().append( exec2getValue( node->secondChild() ).String() ) );
		node->setType(stringConstantNode);
	}
}

       
void Executer::execMul(TreeNode* node)
{
	if (node->firstChild()->getType() == stringConstantNode ||
	    node->secondChild()->getType() == stringConstantNode)
	{
		emit ErrorMsg(node->getToken(), i18n("Cannot multiply strings"), 9000);
		return;
	}
	node->setValue( exec2getValue( node->firstChild() )
	                *
	                exec2getValue( node->secondChild() )
	              );
}

       
void Executer::execDiv(TreeNode* node)
{
	if (node->firstChild()->getType() == stringConstantNode ||
	    node->secondChild()->getType() == stringConstantNode)
	{
		emit ErrorMsg(node->getToken(), i18n("Cannot divide strings"), 9000);
		return;
	}
	node->setValue( exec2getValue( node->firstChild() )
	                /
	                exec2getValue( node->secondChild() ) 
	              );
}

       
void Executer::execSub(TreeNode* node)
{
	if (node->firstChild()->getType() == stringConstantNode ||
	    node->secondChild()->getType() == stringConstantNode)
	{
		emit ErrorMsg(node->getToken(), i18n("Cannot subtract strings"), 9000);
		return;
	}
	node->setValue( exec2getValue( node->firstChild() ) 
	                - 
	                exec2getValue( node->secondChild() )
	              );
}

       
void Executer::execLT(TreeNode* node)
{
	node->setValue( (double) ( exec2getValue( node->firstChild() )
	                           <
	                           exec2getValue( node->secondChild() ) )
	              );
}

void Executer::execLE(TreeNode* node)
{
	node->setValue( (double) ( exec2getValue( node->firstChild() )
	                           <=
	                           exec2getValue( node->secondChild() ) )
	              );
}

void Executer::execGT(TreeNode* node)
{
	node->setValue( (double) ( exec2getValue( node->firstChild() )
	                           >
	                           exec2getValue( node->secondChild() ) )
	              );
}

void Executer::execGE(TreeNode* node)
{
	node->setValue( (double) ( exec2getValue( node->firstChild() )
	                           >=
	                           exec2getValue( node->secondChild() ) )
	              );
}


void Executer::execEQ(TreeNode* node)
{
	node->setValue( (double) ( exec2getValue( node->firstChild() )
	                           ==
	                           exec2getValue( node->secondChild() ) )
	              );
}


void Executer::execNE(TreeNode* node)
{
	node->setValue( (double) ( exec2getValue( node->firstChild() )
	                           !=
	                           exec2getValue( node->secondChild() ) )
	              );
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
	node->setValue( (double) (nl || nr) );
}


void Executer::execNot(TreeNode* node)
{
	node->setValue(1 - exec2getValue( node->firstChild() ).Number()); 
}


void Executer::execMinus(TreeNode* node)
{
	node->setValue(-exec2getValue( node->firstChild() ).Number()); 
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
	// check if number of parameters match, or else...
	if (node->size() != 0)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 accepts no parameters.").arg( node->getLook() ), 5040);
		return;
	}
	emit Clear();
}

void Executer::execGo(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 2)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 5050);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	TreeNode* nodeY = node->secondChild();
	execute(nodeX); // executing
	execute(nodeY);
	int x = ROUND2INT(nodeX->getValue().Number()); // converting & rounding to int
	int y = ROUND2INT(nodeY->getValue().Number());
	emit Go(x, y);
}

void Executer::execGoX(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 1)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 5060);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	execute(nodeX); // executing
	int x = ROUND2INT(nodeX->getValue().Number()); // converting & rounding to int
	emit GoX(x);
}

void Executer::execGoY(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 1)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 5070);
		return;
	}
	TreeNode* nodeY = node->firstChild(); // getting
	execute(nodeY); // executing
	int y = ROUND2INT(nodeY->getValue().Number()); // converting & rounding to int
	emit GoY(y);
}

void Executer::execForward(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 1)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 5080);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	execute(nodeX); // executing
	int x = ROUND2INT(nodeX->getValue().Number()); // converting & rounding to int
	emit Forward(x);
}

void Executer::execBackward(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 1)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 5090);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	execute(nodeX); // executing
	int x = ROUND2INT(nodeX->getValue().Number()); // converting & rounding to int
	emit Backward(x);
}

void Executer::execDirection(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 1)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 6000);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	execute(nodeX); // executing
	double x = (nodeX->getValue().Number()); // converting to double
	emit Direction(x);  // why the FUCK doesnt this work??
}

void Executer::execTurnLeft(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 1)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 6010);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	execute(nodeX); // executing
	double x = (nodeX->getValue().Number()); // converting to double
	emit TurnLeft(x);
}

void Executer::execTurnRight(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 1)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 6020);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	execute(nodeX); // executing
	double x = (nodeX->getValue().Number()); // converting to double
	emit TurnRight(x);
}

void Executer::execCenter(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 0)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 accepts no parameters.").arg( node->getLook() ), 6030);
		return;
	}
	emit Center();
}

void Executer::execSetPenWidth(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 1)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 6040);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	execute(nodeX); // executing
	int x = ROUND2INT(nodeX->getValue().Number()); // converting & rounding to int
	if (x < 1)
	{
		emit ErrorMsg(node->getToken(), i18n("The parameter of %1 must be smaller than 1.").arg( node->getLook() ), 6050);
		return;    
	}
	emit SetPenWidth(x);
}

void Executer::execPenUp(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 0)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 accepts no parameters.").arg( node->getLook() ), 6060);
		return;
	}
	emit PenUp();
}

void Executer::execPenDown(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 0)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 accepts no parameters.").arg( node->getLook() ), 6070);
		return;
	}
	emit PenDown();
}

void Executer::execSetFgColor(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 3)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 6080);
		return;
	}
	TreeNode* nodeR = node->firstChild(); // getting
	TreeNode* nodeG = node->secondChild();
	TreeNode* nodeB = node->thirdChild();
	execute(nodeR); // executing
	execute(nodeG);
	execute(nodeB);
	int r = ROUND2INT(nodeR->getValue().Number()); // converting & rounding to int
	int g = ROUND2INT(nodeG->getValue().Number());
	int b = ROUND2INT(nodeB->getValue().Number());
	if ( ( r < 0 || g < 0 || b < 0 ) || ( r > 255 || g > 255 || b > 255 ) )
	{
		emit ErrorMsg(node->getToken(), i18n("The parameters of function %1 must be within range: 0 to 255.").arg( node->getLook() ), 6090);
		return;
	}
	emit SetFgColor(r, g, b);
}

void Executer::execSetBgColor(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 3)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 7000);
		return;
	}
	TreeNode* nodeR = node->firstChild(); // getting
	TreeNode* nodeG = node->secondChild();
	TreeNode* nodeB = node->thirdChild();
	execute(nodeR); // executing
	execute(nodeG);
	execute(nodeB);
	int r = ROUND2INT(nodeR->getValue().Number()); // converting & rounding to int
	int g = ROUND2INT(nodeG->getValue().Number());
	int b = ROUND2INT(nodeB->getValue().Number());
	if ( ( r < 0 || g < 0 || b < 0 ) || ( r > 255 || g > 255 || b > 255 ) )
	{
		emit ErrorMsg(node->getToken(), i18n("The parameters of function %1 must be within range: 0 to 255.").arg( node->getLook() ), 7010);
		return;    
	}
	emit SetBgColor(r, g, b);
}

void Executer::execResizeCanvas(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 2)
	{
		QString funcname = node->getLook();
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 7020);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	TreeNode* nodeY = node->secondChild();
	execute(nodeX); // executing
	execute(nodeY);
	int x = ROUND2INT(nodeX->getValue().Number()); // converting & rounding to int
	int y = ROUND2INT(nodeY->getValue().Number());
	if ( ( x < 1 || y < 1 ) || ( x > 10000 || y > 10000 ) )
	{
		emit ErrorMsg(node->getToken(), i18n("The parameters of function %1 must be within range: 1 to 10000.").arg( node->getLook() ), 7030);
		return;    
	}
	emit ResizeCanvas(x, y);
}

void Executer::execSpriteShow(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 0)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 accepts no parameters.").arg( node->getLook() ), 7040);
		return;
	}
	emit SpriteShow();
}

void Executer::execSpriteHide(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 0)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 accepts no parameters.").arg( node->getLook() ), 7050);
		return;
	}
	emit SpriteHide();
}

void Executer::execSpritePress(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 0)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 accepts no parameters.").arg( node->getLook() ), 7060);
		return;
	}
	emit SpritePress();
}

void Executer::execSpriteChange(TreeNode* node)
{
	// check if number of parameters match, or else...
	if( node->size() != 1 )
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 7070);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	execute(nodeX); // executing
	int x = (int)(nodeX->getValue().Number()); // converting & rounding to int
	emit SpriteChange(x);
}



void Executer::execMessage(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 1)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 7070);
		return;
	}
	emit MessageDialog(node->firstChild()->getValue().String());
}


void Executer::execInputWindow(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 1)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 7070);
		return;
	}
	QString value = node->firstChild()->getValue().String();
	emit InputDialog(value);
	bool ok = true;
	value.toFloat(&ok); // to see if the value from the InpDialog is a float
	if (value == "")
	{
		node->setValue(""); // this prevents a crash :)
	}
	else if (ok)
	{
		node->setValue(value);
		node->setType(constantNode);
	}
	else
	{
		node->setValue(value);
		node->setType(stringConstantNode);
	}
}

void Executer::execPrint(TreeNode* node)
{
	TreeNode::iterator i;
	QString str = "";
	for (i = node->begin(); i != node->end(); ++i)
	{
		execute( *i ); //execute expression
		str = str + (*i)->getValue().String();
	}
	emit Print(str);
}

void Executer::execFontType(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 2 || node->size() != 1)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 7070);
		return;
	}
	if (node->firstChild()->getType() == stringConstantNode ||
	    node->secondChild()->getType() == stringConstantNode)
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 only accepts strings as parameters.").arg( node->getLook() ), 9000);
		return;
	}
	QString extra;
	if (node->size() == 2)
	{
		QString extra = node->secondChild()->getValue().String();
	}
	QString family = node->firstChild()->getValue().String();
	emit FontType(family, extra);
}

void Executer::execFontSize(TreeNode* node)
{
	// check if number of parameters match, or else...
	if( node->size() != 1 )
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 5060);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	execute(nodeX); // executing
	int px = ROUND2INT(nodeX->getValue().Number()); // converting & rounding to int
	if ( px < 0 || px > 350 )
	{
		emit ErrorMsg(node->getToken(), i18n("The parameters of function %1 must be within range: 0 to 350.").arg( node->getLook() ), 5065);
		return;    
	}
	emit FontSize(px);
}

void Executer::execRepeat(TreeNode* node)
{
	TreeNode* value = node->firstChild();
	TreeNode* statements = node->secondChild();
	
	bBreak=false;
	execute( value );
	for( int i = ROUND2INT( value->getValue().Number()); i > 0; i-- )
	{
		if (bAbort) return;
		kapp->processEvents();
		
		execute( statements );
		if (bBreak || bReturn) break; //jump out loop
	}
	bBreak = false;
}

void Executer::execRandom(TreeNode* node)
{
	// check if number of parameters match, or else...
	if( node->size() != 2 )
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 5050);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	TreeNode* nodeY = node->secondChild();
	execute(nodeX); // executing
	execute(nodeY);
	float x = nodeX->getValue().Number();
	float y = nodeY->getValue().Number();
	
	float r = (float)( KApplication::random() ) / RAND_MAX;
	Value value;
	value = (double)( r * ( y - x ) ) + x;
	node->setValue(value);
	return;
} 


void Executer::execWait(TreeNode* node)
{
	// check if number of parameters match, or else...
	if (node->size() != 1) 
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 was called with wrong number of parameters.").arg( node->getLook() ), 5090);
		return;
	}
	TreeNode* nodeX = node->firstChild(); // getting
	execute(nodeX); // executing
	float sec = nodeX->getValue().Number();
	startWaiting(sec);
}

void Executer::startWaiting(float sec)
{
	bStopWaiting = false;
	int msec = (int)( sec * 1000 ); // convert
	// call a timer that sets stopWaiting to true when it runs 
	QTimer::singleShot( msec, this, SLOT( slotStopWaiting() ) );
	while (bStopWaiting == false)
	{
		if (bAbort) return;
		kapp->processEvents();

		// only 10 times per second is enough... else the CPU gets 100% loaded ( not nice :)
		usleep(100000);
	}
}

void Executer::slotStopWaiting()
{
    bStopWaiting = true;
}

void Executer::execWrapOn(TreeNode* node)
{
	// check if number of parameters match, or else...
	if( node->size() != 0 )
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 accepts no parameters.").arg( node->getLook() ), 6060);
		return;
	}
	emit WrapOn();
}

void Executer::execWrapOff(TreeNode* node)
{
	// check if number of parameters match, or else...
	if( node->size() != 0 )
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 accepts no parameters.").arg( node->getLook() ), 6060);
		return;
	}
	emit WrapOff();
}

void Executer::execReset(TreeNode* node)
{
	// check if number of parameters match, or else...
	if( node->size() != 0 )
	{
		emit ErrorMsg(node->getToken(), i18n("The function %1 accepts no parameters.").arg( node->getLook() ), 6060);
		return;
	}
	emit Reset();
}


#include "executer.moc"
