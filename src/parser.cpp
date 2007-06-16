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

// This file is originally written by Walter Scheppers, but allmost
// allmost every aspect of it is heavily changed by Cies Breijs.


#include <qstringlist.h>

#include <kdebug.h>
#include <klocale.h>

#include "parser.h"


Parser::Parser(QTextIStream& in)
{
	lexer = new Lexer(in);
	tree = new TreeNode();
}

Parser::~Parser()
{
	delete lexer;
}

void Parser::parse()
{
	tree = Program(); // the first node that keeps the lexer running till finished/error
}

void Parser::getToken()
{
	currentToken = lexer->lex(); // stores a Token, obtained though the lexer, in 'currentToken'
	row = currentToken.start.row; // these will have to leave eventually, all should be passed on by the Token
	col = currentToken.start.col;
	kdDebug(0)<<"Parser::getToken(), got a token: '"<<currentToken.look<<"', @ ("<<currentToken.start.row<<", "<<currentToken.start.col<<") - ("<<currentToken.end.row<<", "<<currentToken.end.col<<"), tok-number:"<<currentToken.type<<endl;
}

TreeNode* Parser::Program()
{
	Token emptyToken;
	emptyToken.type = tokNotSet;
	emptyToken.look = "";
	emptyToken.start.row = 0;
	emptyToken.start.col = 0;
	emptyToken.end.row = 0;
	emptyToken.end.col = 0;

	TreeNode* program = new TreeNode(emptyToken, programNode, "program");
	TreeNode* block = new TreeNode(emptyToken, blockNode, "block");

	getToken();

	// this is the main parse loop
	kdDebug(0)<<"Parser::Program(): entering main parse loop..."<<endl;
	while (currentToken.type != tokEOF) // currentToken.type returns the type of the currentToken
	{
		kdDebug(0)<<"Parser::Program(), [main parse loop]: looking for next statement..."<<endl;
		block->appendChild( Statement() );
		while (currentToken.type == tokEOL) getToken(); // newlines between statements are allowed
		// runs statement related code, stores the returned TreeNode* in the nodetree
		// note: Statement() allways gets a new Token with getToken() before it returns
	}
	program->appendChild(block);
	kdDebug(0)<<"Parser::Program(): leaving main parse loop..."<<endl;

	return program;
}

void Parser::matchToken(int expectedToken)
{
	if (currentToken.type == expectedToken)
	{
		getToken(); // get a new token
		return;
	}
	
	switch (expectedToken)
	{
		case tokEOL:
			Error(currentToken, i18n("Unexpected intruction after the '%1' command, please use only one instruction per line").arg(preservedToken.look), 1010);
			break;
		
		case tokBegin:
			Error(preservedToken, i18n("Expected '['"), 1010);
			break;
		
		case tokTo:
			Error(currentToken, i18n("Expected 'to' after the '%1' command").arg(preservedToken.look), 1010);
			break;
		
		case tokAssign:
			Error(currentToken, i18n("Expected '=' after the '%1' command").arg(preservedToken.look), 1010);
			break;
		
		case tokEnd:
			Error(currentToken, i18n("Expected ']' after the '%1' command").arg(preservedToken.look), 1010);
			break;
		
		case tokUnknown:
			Error(preservedToken, i18n("Expected a name after the '%1' command").arg(preservedToken.look), 1010);
			break;
		
		default:
			Error(currentToken, i18n("UNDEFINED ERROR NR %1: please send this Logo script to the KTurtle developers").arg(expectedToken), 1010);
			break;
	}
}


void Parser::appendParameters(TreeNode* node)
{
	node->appendChild( Expression() ); // append the first papameter
	while (currentToken.type == tokComma)
	{
		matchToken(tokComma); // push through the comma
		if (currentToken.type == tokEOL) return; // catch forgotten expressions, like "go 10, "
		node->appendChild( Expression() );
	}
}


TreeNode* Parser::getId()
{
	TreeNode* n = new TreeNode(currentToken, idNode);
	n->setLook(currentToken.look);
	matchToken(tokUnknown); // Id's are ofcouse not yet known 
	return n;
}



TreeNode* Parser::FunctionCall(Token maybeFunctionCall)
{
	kdDebug(0)<<"Parser::FunctionCall() [using identifier: '"<<maybeFunctionCall.look<<"']"<<endl;
	TreeNode* fcall = new TreeNode(maybeFunctionCall, functionCallNode);

	TreeNode* paramList = new TreeNode(currentToken, idListNode, "idlist");
	// if (currentToken.type != tokEOL && currentToken.type != tokEOF)
	if (currentToken.type == tokNumber ||
	    currentToken.type == tokString ||
	    currentToken.type == tokUnknown) // only if there is a possible parameter given after the call...
	{
		TreeNode* expr = Expression();
		if (expr->getType() == Unknown) Error(currentToken, i18n("Expected an expression"), 1020);
		else paramList->appendChild(expr);
		while (currentToken.type == tokComma)
		{
			matchToken(tokComma);
			expr = Expression();
			if (expr->getType() == Unknown) Error(currentToken, i18n("Expected an expression"), 1020);
			else paramList->appendChild(expr);
		}
	}
	fcall->appendChild(paramList);

	return fcall;
}


TreeNode* Parser::Factor()
{
	TreeNode* node;
	Token rememberedToken = currentToken;
	switch (currentToken.type)
	{
		case tokBraceOpen:
			matchToken(tokBraceOpen);
			node = Expression();
			matchToken(tokBraceClose);
			break;

		case tokUnknown:
			node = getId();
			if (learnedFunctionList.contains(rememberedToken.look) > 0) // is function call
			{
				delete node;
				node = FunctionCall(rememberedToken);
				node->setType(funcReturnNode); // expect returned value on stack
			}
			break;

		case tokString:
			node = new TreeNode(currentToken, constantNode);
			{ // extra scope to localize the QString 'str'
				QString str = currentToken.look;
				if ( currentToken.look.endsWith("\"") )
				{
					// cut off the quotes and store the value
					str.remove(0, 1).truncate( currentToken.look.length() - 2 );
				}
				else // problems but we need to keep it moving
				{
					str.remove(0, 1); // cut off the first quote only
					Error(currentToken, i18n("String text not properly delimited with a ' \" ' (double quote)"), 1060);
				}
				node->setValue(str);
			}
			matchToken(tokString);
			break;

		case tokNumber:
			node = new TreeNode(currentToken, constantNode);
			node->setValue(currentToken.value);
			matchToken(tokNumber);
			break;

		case tokRun:
			node = ExternalRun();
			break;

		case tokInputWindow:
			node = InputWindow();
			break;

		case tokRandom:
			node = Random();
			break;

		case tokEOL:
			node = new TreeNode(currentToken, Unknown);
			break;

		default:
			QString s = currentToken.look;
			if ( s.isEmpty() || currentToken.type == tokEOF )
			{
				Error(currentToken, i18n("INTERNAL ERROR NR %1: please sent this Logo script to KTurtle developers").arg(1), 1020);
				// if this error occurs the see the Parser::Repeat for the good solution using 'preservedToken'
			}
			else
			{
				Error(currentToken, i18n("Cannot understand '%1', expected an expression after the '%2' command").arg(s).arg(preservedToken.look), 1020);
			}
			node = new TreeNode(currentToken, Unknown);
			getToken();
			break;
	}
	return node;
}


TreeNode* Parser::signedFactor()
{
	// see if there is a tokPlus, tokMinus or tokNot infront of the factor
	TreeNode* node;
	switch (currentToken.type)
	{
		case tokPlus:
			matchToken(tokPlus);
			return Factor();
			break;

		case tokMinus:
			preservedToken = currentToken;
			matchToken(tokMinus);
			node = Factor();
			if (node->getType() == constantNode)
			{
				// in case of just a constant (-3) situation
				Value num = node->getValue();
				num.setNumber( -num.Number() );
				node->setValue(num);
				return node;
			}
			else
			{
				// in case of a variable or other situation (-a)
				TreeNode* minus = new TreeNode(preservedToken, minusNode);
				minus->appendChild(node);
				return minus;
			}
			break;

		case tokNot:
			preservedToken = currentToken;
			matchToken(tokNot);
			node = Factor();
			{ // extra scope needed to localize not_Node
				TreeNode* not_Node = new TreeNode(preservedToken, notNode);
				not_Node->appendChild(node);
				return not_Node;
			}
			break;

		default:
			// fall-through safety
			return Factor();
			break;
	}
}



TreeNode* Parser::Term()
{
	TreeNode* termNode = signedFactor();
	TreeNode* pos = termNode;
	TreeNode* left = NULL;
	TreeNode* right = NULL;

	while ( (currentToken.type == tokMul) || (currentToken.type == tokDev) || (currentToken.type == tokAnd) )
	{
		// while is is a multiplicative operator do...
		left = pos;
		pos = new TreeNode(currentToken, Unknown);
		pos->appendChild(left);

		switch (currentToken.type)
		{
			case tokMul:
				matchToken(tokMul);
				right = signedFactor();
				pos->setType(mulNode);
				break;

			case tokDev:
				matchToken(tokDev);
				right = signedFactor();
				pos->setType(divNode);
				break;

			case tokAnd:
				matchToken(tokAnd);
				right = signedFactor();
				pos->setType(andNode);
				break;

			default:
				Error(currentToken, i18n("Expected '*' or '/'"), 1030);
				getToken();
				return pos;
				break;
		}
		if (right != NULL) pos->appendChild(right);
		termNode = pos;
	}
	return termNode;
}


bool Parser::isAddOp(Token t)
{
	return ( (t.type == tokPlus)  ||
	         (t.type == tokMinus) ||
	         (t.type == tokGt)    ||
	         (t.type == tokGe)    ||
	         (t.type == tokLt)    ||
	         (t.type == tokLe)    ||
	         (t.type == tokEq)    ||
	         (t.type == tokNe)    ||
		
	         (t.type == tokOr)    ||
	         (t.type == tokGe) );
}


/*---------------------------------------------------------------*/
/* Parse and Translate an Expression */
TreeNode* Parser::Expression()
{
	TreeNode* retExp = Term(); // preset the base-TreeNode as it eventually will be returned
	TreeNode* pos = retExp;
	TreeNode* left = NULL;
	TreeNode* right = NULL;

	while ( isAddOp(currentToken) )
	{
		left = pos;
		pos = new TreeNode(currentToken, Unknown);
		pos->appendChild(left);
		switch (currentToken.type)
		{
			case tokPlus:
				matchToken(tokPlus);
				right = Term();
				pos->setType(addNode);
				break;

			case tokMinus:
				matchToken(tokMinus);
				right = Term();
				pos->setType(subNode);
				break;

			case tokGt:
				matchToken(tokGt);
				right = Term();
				pos->setType(nodeGT);
				break;

			case tokLt:
				matchToken(tokLt);
				right = Term();
				pos->setType(nodeLT);
				break;

			case tokGe:
				matchToken(tokGe);
				right = Term();
				pos->setType(nodeGE);
				break;

			case tokLe:
				matchToken(tokLe);
				right = Term();
				pos->setType(nodeLE);
				break;

			case tokEq:
				matchToken(tokEq);
				right = Term();
				pos->setType(nodeEQ);
				break;

			case tokNe:
				matchToken(tokNe);
				right = Term();
				pos->setType(nodeNE);
				break;

			case tokOr:
				matchToken(tokOr);
				right = Term();
				pos->setType(orNode);
				break;

			default:
				Error(currentToken, i18n("Expected '*' or '/'"), 1040);
				getToken();
				return pos;
				break;
		}
		if (right != NULL) pos->appendChild(right);
		retExp = pos;
	}
	return retExp;
}


TreeNode* Parser::Assignment(Token t)
{
	TreeNode* node = new TreeNode(t, assignNode);
	matchToken(tokAssign); // match the '='

	// the child is the expression or RHV of assignment
	TreeNode* expr = NULL;
// 	if (currentToken.type == tokUnknown) expr = Other(); // in case of an functioncall
// 	else expr = Expression();               -------> fuctioncalls get caught in Expression() and co.
	
	expr = Expression();
	
	node->appendChild(expr);

	return node;
}


TreeNode* Parser::Statement()
{
	kdDebug(0)<<"Parser::Statement()"<<endl;
	while (currentToken.type == tokEOL) getToken(); // statements can allways start on newlines
	switch (currentToken.type)
	{
		case tokLearn         : return Learn();            break;

		case tokIf            : return If();               break;
		case tokFor           : return For();              break;
		case tokForEach       : return ForEach();          break;
		case tokWhile         : return While();            break;
		case tokRun           : return ExternalRun();      break;
		case tokReturn        : return Return();           break;
		case tokBreak         : return Break();            break;
		case tokUnknown       : return Other();            break; //assignment or function call

		case tokClear         : return Clear();            break;
		case tokGo            : return Go();               break;
		case tokGoX           : return GoX();              break;
		case tokGoY           : return GoY();              break;
		case tokForward       : return Forward();          break;
		case tokBackward      : return Backward();         break;
		case tokDirection     : return Direction();        break;
		case tokTurnLeft      : return TurnLeft();         break;
		case tokTurnRight     : return TurnRight();        break;
		case tokCenter        : return Center();           break;
		case tokSetPenWidth   : return SetPenWidth();      break;
		case tokPenUp         : return PenUp();            break;
		case tokPenDown       : return PenDown();          break;
		case tokSetFgColor    : return SetFgColor();       break;
		case tokSetBgColor    : return SetBgColor();       break;
		case tokResizeCanvas  : return ResizeCanvas();     break;
		case tokSpriteShow    : return SpriteShow();       break;
		case tokSpriteHide    : return SpriteHide();       break;
		case tokSpritePress   : return SpritePress();      break;
		case tokSpriteChange  : return SpriteChange();     break;

		case tokPrint         : return Print();            break;
		case tokInputWindow   : return InputWindow();      break;
		case tokMessage       : return Message();          break;
		case tokFontType      : return FontType();         break;
		case tokFontSize      : return FontSize();         break;
		case tokRepeat        : return Repeat();           break;
		case tokRandom        : return Random();           break;
		case tokWait          : return Wait();             break;
		case tokWrapOn        : return WrapOn();           break;
		case tokWrapOff       : return WrapOff();          break;
		case tokReset         : return Reset();            break;
		
		case tokEOF           : return EndOfFile();        break;
		
		case tokEnd           : Error(currentToken, i18n("Cannot understand ']'"), 1050);
		                        getToken();
		                        return new TreeNode(currentToken, Unknown);
		                        break;

		case tokBegin         : Error(currentToken, i18n("Cannot understand '['"), 1050);
		                        getToken();
		                        return new TreeNode(currentToken, Unknown);
		                        break;

		default               : break;
	}
	if (currentToken.type != tokEnd)
	{
		Error(currentToken, i18n("Cannot understand '%1'").arg(currentToken.look), 1060);
	}

	getToken();
	return new TreeNode(currentToken, Unknown); // fall-though for unknowns
}


TreeNode* Parser::Block()
{
	TreeNode* block = new TreeNode(currentToken, blockNode, "block");

	while (currentToken.type == tokEOL) getToken(); // skip newlines
	matchToken(tokBegin);
	while (currentToken.type == tokEOL) getToken(); // skip newlines
	while ( (currentToken.type != tokEnd) && (currentToken.type != tokEOF) )
	{
		block->appendChild( Statement() );
		while (currentToken.type == tokEOL) getToken(); // blocks can have newlines between their statements
	}
	matchToken(tokEnd);
	return block;
}


//
//    Turtle Funktions
//

// Functions that take NO arguments

TreeNode* Parser::Clear()
{
	TreeNode* node = new TreeNode(currentToken, ClearNode);
	preservedToken = currentToken;
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Center()
{
	TreeNode* node = new TreeNode(currentToken, CenterNode);
	preservedToken = currentToken;
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::PenUp()
{
	TreeNode* node = new TreeNode(currentToken, PenUpNode);
	preservedToken = currentToken;
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::PenDown()
{
	TreeNode* node = new TreeNode(currentToken, PenDownNode);
	preservedToken = currentToken;
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::SpriteShow()
{
	TreeNode* node = new TreeNode(currentToken, SpriteShowNode);
	preservedToken = currentToken;
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::SpriteHide()
{
	TreeNode* node = new TreeNode(currentToken, SpriteHideNode);
	preservedToken = currentToken;
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::SpritePress()
{
	TreeNode* node = new TreeNode(currentToken, SpritePressNode);
	preservedToken = currentToken;
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::WrapOn()
{
	TreeNode* node = new TreeNode(currentToken, WrapOnNode);
	preservedToken = currentToken;
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::WrapOff()
{
	TreeNode* node = new TreeNode(currentToken, WrapOffNode);
	preservedToken = currentToken;
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Reset()
{
	TreeNode* node = new TreeNode(currentToken, ResetNode);
	preservedToken = currentToken;
	getToken();
	matchToken(tokEOL);
	return node;
}




// Functions that take 1 arguments

TreeNode* Parser::GoX()
{
	TreeNode* node = new TreeNode(currentToken, GoXNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}


TreeNode* Parser::GoY()
{
	TreeNode* node = new TreeNode(currentToken, GoYNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}


TreeNode* Parser::Forward()
{
	TreeNode* node = new TreeNode(currentToken, ForwardNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}


TreeNode* Parser::Backward()
{
	TreeNode* node = new TreeNode(currentToken, BackwardNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Direction()
{
	TreeNode* node = new TreeNode(currentToken, DirectionNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::TurnLeft()
{
	TreeNode* node = new TreeNode(currentToken, TurnLeftNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::TurnRight()
{
	TreeNode* node = new TreeNode(currentToken, TurnRightNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::SetPenWidth()
{
	TreeNode* node = new TreeNode(currentToken, SetPenWidthNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Message()
{
	TreeNode* node = new TreeNode(currentToken, MessageNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::InputWindow()
{
	TreeNode* node = new TreeNode(currentToken, InputWindowNode);
	preservedToken = currentToken;
	getToken();
	node->appendChild( Expression() );
	// matchToken(tokEOL);  this command can return values so can be used as expression/parameter
	return node;
}

TreeNode* Parser::SpriteChange()
{
	TreeNode* node = new TreeNode(currentToken, SpriteChangeNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::FontType()
{
	TreeNode* node = new TreeNode(currentToken, FontTypeNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::FontSize()
{
	TreeNode* node = new TreeNode(currentToken, FontSizeNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Wait()
{
	TreeNode* node = new TreeNode(currentToken, WaitNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::ExternalRun()
{
	TreeNode* node = new TreeNode(currentToken, runNode);
	preservedToken = currentToken;
	getToken();
	node->appendChild( Expression() );
	// matchToken(tokEOL);  this command can return values so can be used as expression/parameter
	return node;
}



// Functions that take 2 arguments

TreeNode* Parser::Go()
{
	TreeNode* node = new TreeNode(currentToken, GoNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::ResizeCanvas()
{
	TreeNode* node = new TreeNode(currentToken, ResizeCanvasNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Random()
{
	TreeNode* node = new TreeNode(currentToken, RandomNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	// matchToken(tokEOL);  this command can return values so can be used as expression/parameter
	return node;
}




// Functions that take 3 arguments

TreeNode* Parser::SetFgColor()
{
	TreeNode* node = new TreeNode(currentToken, SetFgColorNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::SetBgColor()
{
	TreeNode* node = new TreeNode(currentToken, SetBgColorNode);
	preservedToken = currentToken;
	getToken();
	appendParameters(node);
	matchToken(tokEOL);
	return node;
}




// Weirdo's (learn, execution controllers, print, and Other()s)

TreeNode* Parser::Learn()
{
	preservedToken = currentToken;
	matchToken(tokLearn); // skip the 'dummy' command
	TreeNode* func = new TreeNode(currentToken, functionNode);
	getToken(); // get the token after the function's name

	TreeNode* idList = new TreeNode(currentToken, idListNode, "idlist");
	if (currentToken.type != tokBegin)
	{
		if (currentToken.type == tokUnknown) idList->appendChild( getId() );
		else
		{
			Error(currentToken, "Expected a parameter name or a '[' after the learn command.", 3030);
			getToken(); // this recovers from the error
		}
		
		while (currentToken.type == tokComma)
		{
			matchToken(tokComma);
			idList->appendChild( getId() );
		}
	}
	func->appendChild(idList);

	learnedFunctionList.append( func->getLook() ); // publish the function

	func->appendChild( Block() );

	return func;
}


TreeNode* Parser::If()
{
	TreeNode* node = new TreeNode(currentToken, ifNode);
	preservedToken = currentToken;
	matchToken(tokIf);

	node->appendChild( Expression() );

	if (currentToken.type == tokDo) getToken(); // skip dummy word 'do'

	if (currentToken.type == tokBegin) node->appendChild( Block() ); // if followed by a block
	else node->appendChild( Statement() ); // if followed by single statement

	while (currentToken.type == tokEOL) getToken(); // allow the else keyword to be on later lines

	if (currentToken.type == tokElse) // else part
	{
		matchToken(tokElse);
		while (currentToken.type == tokEOL) getToken();
		if (currentToken.type == tokDo) getToken(); // next word

		if(currentToken.type == tokBegin) node->appendChild( Block() ); // else is followed by block
		else node->appendChild( Statement() );
	}

	return node;
}


TreeNode* Parser::While()
{
	TreeNode* node = new TreeNode(currentToken, whileNode);
	preservedToken = currentToken;
	matchToken(tokWhile);
	node->appendChild( Expression() );
	node->appendChild( Block() );
	return node;
}


TreeNode* Parser::For()
{
	TreeNode* fNode = new TreeNode(currentToken, forNode);
	preservedToken = currentToken;
	matchToken(tokFor);
	fNode->appendChild( getId() ); // loop id
	matchToken(tokAssign);
	
	fNode->appendChild( Expression() ); // start value expression
	matchToken(tokTo);
	fNode->appendChild( Expression() ); // stop value expression

	if (currentToken.type == tokStep)
	{
		matchToken(tokStep);
		fNode->appendChild( Expression() ); //step expression
	}

	while (currentToken.type == tokEOL) getToken(); // newlines are allowed
	if (currentToken.type == tokBegin) fNode->appendChild( Block() ); // for followed by a block
	else fNode->appendChild( Statement() ); // while followed by single statement

	return fNode;
}


TreeNode* Parser::Repeat()
{
	TreeNode* node = new TreeNode(currentToken, RepeatNode);
	preservedToken = currentToken; // preserve token, else Match() will make sure it gets lost
	matchToken(tokRepeat);
	node->appendChild( Expression() );
	node->appendChild( Block() );
	return node;
}



TreeNode* Parser::ForEach()
{
	TreeNode* fNode = new TreeNode(currentToken, forEachNode);
	preservedToken = currentToken;
	matchToken(tokForEach);

	fNode->appendChild( Expression() );
	matchToken(tokIn);
	fNode->appendChild( Expression() );

	if (currentToken.type == tokBegin) fNode->appendChild( Block() ); // for followed by a block
	else fNode->appendChild( Statement() ); // while followed by single statement

	return fNode;
}

TreeNode* Parser::Print()
{
	TreeNode* node = new TreeNode(currentToken, printNode);
	preservedToken = currentToken; // preserve token, else Match() will make sure it gets lost
	getToken();
	if (currentToken.type == tokEOL) return node; // print called without expressions
	node->appendChild( Expression() ); // first expression
	// following strings or expressions
	while (currentToken.type == tokComma)
	{
		getToken(); // the comma
		node->appendChild( Expression() );
	}
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Return()
{
	TreeNode* ret = new TreeNode(currentToken, returnNode);
	matchToken(tokReturn);

	ret->appendChild( Expression() );

	return ret;
}

TreeNode* Parser::Break()
{
	TreeNode* brk = new TreeNode(currentToken, breakNode);
	matchToken(tokBreak);

	return brk;
}

TreeNode* Parser::EndOfFile()
{
	TreeNode* node = new TreeNode(currentToken, EndOfFileNode);
	return node;
}

TreeNode* Parser::Other()
{
	// this is either an assignment or a function call!
	kdDebug(0)<<"Parser::Other()"<<endl;
	Token rememberedToken = currentToken; // preserve token, else Match() will make sure it gets lost
	matchToken(tokUnknown);

	if (currentToken.type == tokAssign) return Assignment(rememberedToken);
	else if (learnedFunctionList.contains(rememberedToken.look) > 0)
	{
		TreeNode* node;
		node = FunctionCall(rememberedToken);
// 		node->setType(funcReturnNode);
		return node;
	}
	
	Error(rememberedToken, i18n("'%1' is neither a Logo command nor a learned command.").arg(rememberedToken.look), 1020);
	TreeNode* errNode = new TreeNode(rememberedToken, Unknown);
	
	// skip the rest of the line
	while (currentToken.type != tokEOL) getToken();
	
	return errNode;
}


void Parser::Error(Token& t, const QString& s, uint code)
{
	emit ErrorMsg(t, s, code);
}


#include "parser.moc"
