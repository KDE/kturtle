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


#include <qstringlist.h>
   
#include <kdebug.h>
#include <klocale.h>

#include "parser.h"

/*======================= THE GRAMMAR =============================

  BNF for arithmetic expressions (improved unary minus)

    <expression>    ::= <term> [<addop> <term>]*
    <term>          ::= <signed factor> [<mulop> <signed factor>]*
    <signed factor> ::= [<addop>] <factor>
    <factor>        ::= <integer> | <variable> | (<expression>)



  NOT DONE YET !: 
  BNF for boolean algebra

    <b-expression>::= <b-term> [<orop> <b-term>]*
    <b-term>      ::= <not-factor> [AND <not-factor>]*
    <not-factor>  ::= [NOT] <b-factor>
    <b-factor>    ::= <b-literal> | <b-variable> | (<b-expression>)

=================================================================*/



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
	currentNode = tree;
}

void Parser::getToken() {
	currentToken = lexer->lex(); // stores a token, obtained though the lexer, in 'currentToken'
	row = currentToken.start.row; // these will have to leave eventually, all should be passed on by the token
	col = currentToken.start.col;
	kdDebug(0)<<"Parser::getToken(), got a token: '"<<currentToken.look<<"', @ ("<<currentToken.start.row<<", "<<currentToken.start.col<<"), tok-number:"<<currentToken.type<<endl;
}

TreeNode* Parser::Program() {
	TreeNode* program = new TreeNode(programNode, 0, 0, "program");
	TreeNode* block = new TreeNode(blockNode, 0, 0, "block");

	getToken();
	
	// this is the main parse loop
	kdDebug(0)<<"Parser::Program(), Entering main parse loop..."<<endl;
	while (currentToken.type != tokEOF) { // currentToken.type returns the type of the current token
		kdDebug(0)<<"Parser::Program(), looking for a statement..."<<endl;
		block->appendChild( Statement() );
		while (currentToken.type == tokEOL) getToken(); // newlines between statements are allowed
		// runs statement related code, stores the returned TreeNode* in the nodetree
		// note: Statement() allways gets a new token with getToken() before it returns 
	}
	program->appendChild(block);
	kdDebug(0)<<"Parser::Program(), Left main parse loop..."<<endl;

	matchToken(tokEOF);
	return program;
}

void Parser::matchToken(int expectedToken)
{
	if (currentToken.type != expectedToken)
	{
		// do something sensible here when there is an error!
		
// 		QString tokStr = "";
// 		switch(x) {
// 			case tokIf            : tokStr += "if";           break;
// 			case tokElse          : tokStr += "else";         break;
// 			case tokWhile         : tokStr += "while";        break;
// 			case tokFfor           : tokStr += "for";          break;
// 			case tokTo            : tokStr += "to";           break;
// 			case tokStep          : tokStr += "step";         break;
// 			case tokNumber        : tokStr += "number";       break;
// 			case tokString        : tokStr += "string";       break;
// 			case tokId            : tokStr += "id";           break;
// 			case tokProcId        : tokStr += "procedure id"; break;
// 			case tokBegin         : tokStr += "begin";        break;
// 			case tokEnd           : tokStr += "end";          break;
// 		
// 			case tokOr            : tokStr += "or";           break;
// 			case tokAnd           : tokStr += "and";          break;
// 			case tokNot           : tokStr += "not";          break;
// 	
// 			case tokGe            : tokStr += ">=";           break;
// 			case tokGt            : tokStr += ">";            break;
// 			case tokLe            : tokStr += "<=";           break;
// 			case tokLt            : tokStr += "<";            break;
// 			case tokNe            : tokStr += "!=";           break;
// 			case tokEq            : tokStr += "==";           break;
// 			case tokAssign        : tokStr += "=";            break;
// 
// 			case tokReturn        : tokStr += "return";       break;
// 			case tokBreak         : tokStr += "break";        break;
// 
// 			case tokForEach       : tokStr += "foreach";      break;
// 			case tokIn            : tokStr += "in";           break;
// 		
// 			case tokRun           : tokStr += "run";          break;
// 			case tokEOF           : tokStr += "end of file";  break;
// 			case tokError         : tokStr += "error token";  break;
// 			
// 			case tokLearn         : tokStr += "learn";        break;
// 			
// 			case tokClear         : tokStr += "clear";        break;
// 			case tokGo            : tokStr += "go";           break;
// 			case tokGoX           : tokStr += "gox";          break;
// 			case tokGoY           : tokStr += "goy";          break;
// 			case tokForward       : tokStr += "forward";      break;
// 			case tokBackward      : tokStr += "backward";     break;
// 			case tokDirection     : tokStr += "direction";    break;
// 			case tokTurnLeft      : tokStr += "turnleft";     break;
// 			case tokTurnRight     : tokStr += "turnright";    break;
// 			case tokCenter        : tokStr += "center";       break;
// 			case tokSetPenWidth   : tokStr += "setpenwidth";  break;
// 			case tokPenUp         : tokStr += "penup";        break;
// 			case tokPenDown       : tokStr += "pendown";      break;
// 			case tokSetFgColor    : tokStr += "setfgcolor";   break;
// 			case tokSetBgColor    : tokStr += "setbgcolor";   break;
// 			case tokResizeCanvas  : tokStr += "resizecanvas"; break;
// 			case tokSpriteShow    : tokStr += "spriteshow";   break;
// 			case tokSpriteHide    : tokStr += "spritehide";   break;
// 			case tokSpritePress   : tokStr += "spritepress";  break;
// 			case tokSpriteChange  : tokStr += "spritechange"; break;
// 
// 			case tokMessage       : tokStr += "message";      break;
// 			case tokInputWindow   : tokStr += "inputwindow";  break;
// 			case tokPrint         : tokStr += "print";        break;
// 			case tokFontType      : tokStr += "fonttype";     break;
// 			case tokFontSize      : tokStr += "fontsize";     break;
// 			case tokRepeat        : tokStr += "repeat";       break;
// 			case tokRandom        : tokStr += "random";       break;
// 			case tokWait          : tokStr += "wait";         break;
// 			case tokWrapOn        : tokStr += "wrapon";       break;
// 			case tokWrapOff       : tokStr += "wrapoff";      break;
// 			case tokReset         : tokStr += "reset";        break;
// 
// 			default               : tokStr += (char)x; break;
// 		}
// 		QString key = lexer->name2key(tokStr); // translate if possible, else key=tokStr
// 		Error( i18n("Expected %1 on line %2").arg(key).arg( lexer->getRow() ), 1010, lexer->getRow(), lexer->getCol() );
		Error( i18n("Could not match token-number: '%1'").arg(expectedToken), 1010, row, col );
	}
	else
	{
		getToken(); // get a new token
	}
}

TreeNode* Parser::getId()
{
	TreeNode* n = new TreeNode(currentToken, idNode);
	n->setName(currentToken.look);
	matchToken(tokUnknown); // Id's are ofcouse not yet known 
	return n;  
}

/*
  paramlist is like idlist except it does not only
  except id's it accepts expressions seperated by ','!
  This is used by FunctionCall(). 
*/
TreeNode* Parser::ParamList()
{
	TreeNode* ilist = new TreeNode(currentToken, idListNode);
	//check for empty idlist -> function with no parameters
	if( currentToken.type == ')' )
	{
		return ilist;
	}

	TreeNode* expr = Expression();
	if (expr->getType() == Unknown) {
		Error( i18n("Expected an expression"), 1020, ilist->getRow(), ilist->getCol() );
	} else {
		ilist->appendChild( expr );
	}
	
// 	//get id's seperated by ','
// 	ilist->appendChild( Expression() ); //aaah KISS (Keep It Simple Sidney)
	
	while( currentToken.type == ',' )
	{
		matchToken(tokComma);
		// ilist->appendChild( Expression() ); 
		
		expr = Expression();
		if (expr->getType() == Unknown) {
			Error( i18n("Expected an expression"), 1020, ilist->getRow(), ilist->getCol() );
		} else {
			ilist->appendChild( expr );
		}
	}

	return ilist;
}

/*
  <functioncall> := tokId '(' <paramlist> ')' 
*/
TreeNode* Parser::FunctionCall(const QString& name, uint r, uint c) {
	if (learnedFunctionList.contains(name) == 0) {
		Error( i18n("'%1' is neither a Logo command nor a learned command.").arg(name), 1010, r, c);
		TreeNode* errNode = new TreeNode(Unknown, r, c, "name");
		return errNode;
	}
	
	kdDebug(0)<<"Parser::FunctionCall, using identifier: '"<<name<<"'"<<endl;
	TreeNode* fcall = new TreeNode(functionCallNode, r, c, "functioncall");
	//first child contains function name
	
	TreeNode* funcid = new TreeNode(idNode, row, col);
	funcid->setName(name);
	fcall->appendChild(funcid);

	matchToken(tokBraceOpen);
	fcall->appendChild( ParamList() );
	matchToken(tokBraceClose);

	return fcall;
}


/*---------------------------------------------------------------*/
/* Parse and Translate a Math Factor */
TreeNode* Parser::Factor()
{
	TreeNode* n;
	switch(currentToken.type)
	{
		case tokBraceOpen:
			matchToken(tokBraceOpen);
			n = Expression();
			matchToken(tokBraceClose);
			break;

		case tokUnknown:     
			n = getId();
			if (currentToken.type == tokBraceOpen) { //is function call
				QString name = n->getName();
				delete n;
				n = FunctionCall(name);
				n->setType(funcReturnNode); //expect returned value on stack
			}
			break;
                  
		case tokString:
			n = new TreeNode(stringConstantNode, row, col, "string-constant");
			n->setValue(currentToken.look);
			matchToken(tokString);
			break;

		case tokNumber:
			n = new TreeNode(constantNode, row, col, "constant");
			n->setValue(currentToken.value);
			matchToken(tokNumber);
			break;
    
		case tokRun:
			n = ExternalRun();
			break;

		case tokInputWindow:
			n = InputWindow();
			break;

		case tokRandom:
			n = Random();
			break;

		default:
			QString s = currentToken.look;
			if ( s.isEmpty() || currentToken.type == tokEOF ) {
				kdDebug(0)<<"EXPECTED AN EXPRESSION"<<endl;
				Error( i18n("DEBUG: This message should be avoided, see the Parser::Repeat for the good solution"), 1020, currentNode->getRow(), currentNode->getRow() );
			} else {
				Error( i18n("Cannot understand '%1' in expression").arg(s), 1020 );
			}
			n = new TreeNode(Unknown, row, col);
			getToken();
			break;
	}
	return n;
}


TreeNode* Parser::signedFactor()
{
	TreeNode* sfac; //used by '-' and tokNot
	switch( currentToken.type )
	{
		case tokPlus:
			matchToken(tokPlus);
			return Factor();
			break;

		case tokMinus:
			matchToken(tokMinus);
			sfac = Factor();
			if(sfac->getType() == constantNode)
			{
				Number n = sfac->getValue();
				n.val = -n.val;
				sfac->setValue( n );
				return sfac;
			}
			else
			{
				TreeNode* minus = new TreeNode(minusNode, row, col, "minus");
				minus->appendChild(sfac);
				return minus;
			}
			break;

		case tokNot:
			matchToken(tokNot);
			sfac = Factor();
			if (sfac->getType() == constantNode)
			{
				Number n = sfac->getValue();
				n.val = 1 - n.val;
				sfac->setValue(n);
				return sfac;
			}
			else
			{
				TreeNode* n = new TreeNode(notNode, row, col, "not", lexer->name2key("not") );
				n->appendChild(sfac);
				return n;
			}
			break;

		default:
			return Factor();
			break;
	}
}



/*---------------------------------------------------------------*/
/* Parse and Translate a Math Term */
TreeNode* Parser::Term() {
	TreeNode* termNode = signedFactor();
	TreeNode* pos = termNode;
	TreeNode* left = NULL;
	TreeNode* right = NULL;

	while ( (currentToken.type == tokMul) || (currentToken.type == tokDev) || (currentToken.type == tokAnd) ) {
		// while is is a multiplicative operator do...
		left = pos;
		pos = new TreeNode(Unknown, row, col);
		pos->appendChild(left);

		switch (currentToken.type)
		{
			case tokMul:
				matchToken(tokMul);
				right = signedFactor();
				pos->setType( mulNode );
				pos->setName( "mul" );
				break;

			case tokDev:
				matchToken(tokDev);
				right = signedFactor();
				pos->setType( divNode );
				pos->setName( "div" );
				break;

			case tokAnd:
				matchToken(tokAnd);
				right = signedFactor();
				pos->setType( andNode );
				pos->setName( "and" );
				break;

			default:
				Error( i18n("Expected '*' or '/'"), 1030 ); 
				getToken();
				return pos;
				break;
		}
		if( right != NULL)
		{
			pos->appendChild(right);
		}
		termNode = pos;
	}   //end while
	return termNode;
}


bool Parser::isAddOp(token t) {
	return (
		(t.type == tokPlus)  ||
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
TreeNode* Parser::Expression() {
	TreeNode* retExp = Term();
	TreeNode* pos = retExp;
	TreeNode* left = NULL;
	TreeNode* right = NULL;

	while( isAddOp(currentToken) ) {
		left = pos;
		pos = new TreeNode(Unknown, row, col);
		pos->appendChild(left);
		switch(currentToken.type) {
			case tokPlus:
				matchToken(tokPlus);
				right = Term() ;
				pos->setType( addNode );
				pos->setName("add");
				break;

			case tokMinus:
				matchToken(tokMinus);
				right = Term();
				pos->setType( subNode );
				pos->setName("sub");
				break;

			case tokGt:
				matchToken(tokGt);
				right = Term();
				pos->setType( nodeGT );
				pos->setName(">");
				break;

			case tokLt:
				matchToken(tokLt);
				right = Term();
				pos->setType( nodeLT );
				pos->setName("<");
				break;

			case tokGe:
				matchToken(tokGe);
				right = Term();
				pos->setType( nodeGE );
				pos->setName(">=");
				break;

			case tokLe:
				matchToken(tokLe);
				right = Term();
				pos->setType( nodeLE );
				pos->setName("<=");
				break;
			
			case tokEq:
				matchToken(tokEq);
				right = Term();
				pos->setType( nodeEQ );
				pos->setName("==");
				break;

			case tokNe:
				matchToken(tokNe);
				right = Term();
				pos->setType( nodeNE );
				pos->setName("!=");
				break;
			
			case tokOr:
				matchToken(tokOr);
				right = Term();
				pos->setType( orNode );
				pos->setName( "or" );
				break;

			default:
				Error( i18n("Expected additive operation"), 1040 );
				getToken();
				return pos;
				break;
		}
		if (right != NULL) pos->appendChild(right);
		retExp = pos;
	}
	return retExp;
}


TreeNode* Parser::Assignment(const QString& name)
{
	TreeNode* assign = new TreeNode(assignNode, row, col, "assignment");
	
	matchToken(tokAssign);

	//first child of assign is id or lhv of assignment
	TreeNode* left = new TreeNode(idNode, row, col, "", lexer->name2key("") );
	left->setName(name);
	assign->appendChild(left);
	
	//next child is expression or rhv of assignment
	TreeNode* right = Expression();
	assign->appendChild(right);

	return assign;
}


TreeNode* Parser::getString()
{
	TreeNode* str = new TreeNode(stringConstantNode, row, col, "string");
	
	str->setLook(currentToken.look);
	matchToken(tokString);
	
	return str;
}

TreeNode* Parser::Statement() {
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
		case tokEnd           : break; //caught by Block

		case tokBegin         : Error( i18n("Begin without matching end"), 1050 );
		                        getToken();
		                        return new TreeNode(Unknown, row, col);
		                        break;

		default               : break;    
	}
// 	const QString beginChar = lexer->name2key("begin");
// 	const QString currentTokenChar = currentToken.look;
// 	if (currentTokenChar == beginChar) {
// 		Error( i18n("'%1' is expected").arg(beginChar), 1060);
// 	} else {
// 		Error( i18n("'%1' is not a Logo command").arg(currentTokenChar), 1060); 
// 	}
	
	if (currentToken.type != tokEnd)
	{
		Error( i18n("'%1' is not a Logo command").arg(currentToken.look), 1060);
	}
	
	getToken();
	return new TreeNode(Unknown, row, col); // fallback for unknowns
}


TreeNode* Parser::Block()
{
	TreeNode* block = new TreeNode(blockNode, row, col, "block");
	
	matchToken(tokBegin);
	while ( (currentToken.type != tokEnd) && (currentToken.type != tokEOF) )
	{
		block->appendChild( Statement() );
		while (currentToken.type == tokEOL) getToken(); // blocks can have newlines between their statements
	}
	matchToken(tokEnd);
	return block;
}


TreeNode* Parser::IdList()
{
	TreeNode* ilist = new TreeNode(idListNode, row, col, "idlist");

	//check for empty idlist -> function with no parameters
	if( currentToken.type == tokBraceOpen ) return ilist;
	
	//get id's seperated by ','
	ilist->appendChild( getId() );
	while (currentToken.type == tokComma)
	{
		matchToken(tokComma);
		ilist->appendChild( getId() );
	}

	return ilist;
}


//
//    Turtle Funktions
//

// Functions that take NO arguments

TreeNode* Parser::Clear()
{
	TreeNode* node = new TreeNode(currentToken, ClearNode);
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Center()
{
	TreeNode* node = new TreeNode(currentToken, CenterNode);
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::PenUp()
{
	TreeNode* node = new TreeNode(currentToken, PenUpNode);
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::PenDown()
{
	TreeNode* node = new TreeNode(currentToken, PenDownNode);
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::SpriteShow()
{
	TreeNode* node = new TreeNode(currentToken, SpriteShowNode);
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::SpriteHide()
{
	TreeNode* node = new TreeNode(currentToken, SpriteHideNode);
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::SpritePress()
{
	TreeNode* node = new TreeNode(currentToken, SpritePressNode);
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::WrapOn()
{
	TreeNode* node = new TreeNode(currentToken, WrapOnNode);
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::WrapOff()
{
	TreeNode* node = new TreeNode(currentToken, WrapOffNode);
	getToken();
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Reset()
{
	TreeNode* node = new TreeNode(currentToken, ResetNode);
	getToken();
	matchToken(tokEOL);
	return node;
}




// Functions that take 1 arguments

TreeNode* Parser::GoX()
{
	TreeNode* node = new TreeNode(currentToken, GoXNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}


TreeNode* Parser::GoY()
{
	TreeNode* node = new TreeNode(currentToken, GoYNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}


TreeNode* Parser::Forward()
{
	TreeNode* node = new TreeNode(currentToken, ForwardNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}


TreeNode* Parser::Backward()
{
	TreeNode* node = new TreeNode(currentToken, BackwardNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Direction()
{
	TreeNode* node = new TreeNode(currentToken, DirectionNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::TurnLeft()
{
	TreeNode* node = new TreeNode(currentToken, TurnLeftNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::TurnRight()
{
	TreeNode* node = new TreeNode(currentToken, TurnRightNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::SetPenWidth()
{
	TreeNode* node = new TreeNode(currentToken, SetPenWidthNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Message()
{
	TreeNode* node = new TreeNode(currentToken, MessageNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::InputWindow()
{
	TreeNode* node = new TreeNode(currentToken, InputWindowNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::SpriteChange()
{
	TreeNode* node = new TreeNode(currentToken, SpriteChangeNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::FontType()
{
	TreeNode* node = new TreeNode(currentToken, FontTypeNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::FontSize()
{
	TreeNode* node = new TreeNode(currentToken, FontSizeNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Random()
{
	TreeNode* node = new TreeNode(currentToken, RandomNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // first expression
	matchToken(tokComma);
	node->appendChild( Expression() ); // second expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Wait()
{
	TreeNode* node = new TreeNode(currentToken, WaitNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::ExternalRun()
{
	TreeNode* node = new TreeNode(currentToken, runNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // get the expression
	matchToken(tokEOL);
	return node;
}



// Functions that take 2 arguments

TreeNode* Parser::Go()
{
	TreeNode* node = new TreeNode(currentToken, GoNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // first expression
	matchToken(tokComma);
	node->appendChild( Expression() ); // second expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::ResizeCanvas()
{
	TreeNode* node = new TreeNode(currentToken, ResizeCanvasNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // first expression
	matchToken(tokComma);
	node->appendChild( Expression() ); // second expression
	matchToken(tokEOL);
	return node;
}




// Functions that take 3 arguments

TreeNode* Parser::SetFgColor()
{
	TreeNode* node = new TreeNode(currentToken, SetFgColorNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // first expression
	matchToken(tokComma);
	node->appendChild( Expression() ); // second expression
	matchToken(tokComma);
	node->appendChild( Expression() ); // third expression
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::SetBgColor()
{
	TreeNode* node = new TreeNode(currentToken, SetBgColorNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // first expression
	matchToken(tokComma);
	node->appendChild( Expression() ); // second expression
	matchToken(tokComma);
	node->appendChild( Expression() ); // third expression
	matchToken(tokEOL);
	return node;
}




// Weirdo's (learn, execution controllers, print, and Other()s)

/*==================================================================
   EBNF for a function 
   <function> := tokId '(' <idlist> ')' <block>

   we can safely use tokId because we require "(" and ")" to mark the params
   to be given after the id when it is called, 
   if this were not the case
   we would have to extend the lexer so that it gives a
   tokFunctionId whenever an id has the same name as a function...
===================================================================*/
TreeNode* Parser::Learn()
{
	matchToken(tokLearn); // dummy word
	TreeNode* func = new TreeNode(currentToken, functionNode);
	TreeNode* idn = getId();

	func->appendChild( idn );
	matchToken(tokBraceOpen);
	func->appendChild( IdList() );
	matchToken(tokBraceClose);
	func->appendChild( Block() );
	
	learnedFunctionList.append( idn->getName() );

	return func;
}

TreeNode* Parser::Repeat() {
	TreeNode* node = new TreeNode(currentToken, RepeatNode);
	currentNode = node;
	matchToken(tokRepeat);
	node->appendChild( Expression() );
	node->appendChild( Block() );
	return node;
}

/*
  <if> ::= tokIf <expression> ( <statement> | <block> )
                ( tokElse ( <statement> | <block> ) )?
 
 the expression is in first child
 the first block or statement is in second child
 the else block or statement is in third child
*/
TreeNode* Parser::If()
{
	TreeNode* node = new TreeNode(ifNode, row, col, "if", lexer->name2key("if") );
	matchToken(tokIf);
	currentNode = node;
	
	node->appendChild( Expression() );

	if(currentToken.type == tokDo) {  // skip the 'do'
		getToken(); //next word
	}

	if(currentToken.type == tokBegin) {  //if followed by a block
		node->appendChild( Block() ); 
	} else {   //if followed by single statement
		node->appendChild( Statement() );
	}

	if(currentToken.type == tokElse) { //else part
		matchToken(tokElse);
		
		if(currentToken.type == tokDo) {  // skip the 'do'
			getToken(); // next word
		}

		if(currentToken.type == tokBegin) {  //else is followed by block
			node->appendChild( Block() );
		} else {
			node->appendChild( Statement() );
		}

	}

	return node;
}

/*
  <while> ::= tokWhile <expression> ( <statement> | <block> )
 */
TreeNode* Parser::While()
{
	TreeNode* wNode = new TreeNode(whileNode, row, col, "while", lexer->name2key("while") );
	matchToken(tokWhile);
	currentNode = wNode;
	wNode->appendChild( Expression() );

	if(currentToken.type == tokBegin) //while followed by a block
	{
		wNode->appendChild( Block() ); 
	} 
	else{ //while followed by single statement
		wNode->appendChild( Statement() );
	}

	return wNode;
}

/*
  <for> ::= tokFor <id>'='<expression> tokTo <expression> (<tokStep> <expression>)? ( <statement> | <block> )
  for loops with step have 5 children
  for loops without step have 4 children
*/
TreeNode* Parser::For()
{
	TreeNode* fNode = new TreeNode(forNode, row, col, "for loop", lexer->name2key("for loop") );
	matchToken(tokFor);
	currentNode = fNode;
	fNode->appendChild( getId() ); //loop id
	matchToken(tokAssign);
	
	fNode->appendChild( Expression() ); //start value expression
	matchToken(tokTo);
	fNode->appendChild( Expression() ); //stop value expression

	if (currentToken.type == tokStep)
	{
		matchToken(tokStep);
		fNode->appendChild( Expression() ); //step expression
	}

	if (currentToken.type == tokBegin)
	{ //for followed by a block
		fNode->appendChild( Block() ); 
	} 
	else
	{ //while followed by single statement
		fNode->appendChild( Statement() );
	}

	return fNode;
}

TreeNode* Parser::ForEach() {
	TreeNode* fNode = new TreeNode(forEachNode, row, col, "foreach loop", lexer->name2key("foreach loop") );
	matchToken(tokForEach);
	currentNode = fNode;
	
	fNode->appendChild( Expression() );
	matchToken(tokIn);
	fNode->appendChild( Expression() );

	if(currentToken.type == tokBegin) { //for followed by a block
		fNode->appendChild( Block() ); 
	} else { //while followed by single statement
		fNode->appendChild( Statement() );
	}

	return fNode;
}

TreeNode* Parser::Print()
{
	TreeNode* node = new TreeNode(currentToken, printNode);
	currentNode = node;
	getToken();
	node->appendChild( Expression() ); // first expression
	// following strings or expressions
	while(currentToken.type == tokComma)
	{
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	matchToken(tokEOL);
	return node;
}

TreeNode* Parser::Return()
{
	TreeNode* ret = new TreeNode(returnNode, row, col, "return", lexer->name2key("return") );
	matchToken(tokReturn);
	currentNode = ret;
	
	ret->appendChild( Expression() );
	
	return ret;
}

TreeNode* Parser::Break()
{
	TreeNode* brk = new TreeNode(breakNode, row, col, "break", lexer->name2key("break") );
	matchToken(tokBreak);
	
	return brk;
}

TreeNode* Parser::Other()
{  // this is either an assignment or a function call!
	kdDebug(0)<<"Parser::Other()"<<endl;
	token presevedToken = currentToken; // preserve token, else Match() will make sure it gets lost
	matchToken(tokUnknown);

	if(currentToken.type == tokAssign)
	{
		return Assignment(presevedToken.look);
	} else {
		return FunctionCall(presevedToken.look, presevedToken.start.row, presevedToken.start.col);
	}
}


void Parser::Error(const QString& s, uint code, uint r, uint c) {
	emit ErrorMsg(s, (r == NA ? row : r), (c == NA ? col : c), code);
}


#include "parser.moc"
