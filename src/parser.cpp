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



Parser::Parser(QTextIStream& in) {
	lexer = new Lexer(in);
	tree = new TreeNode();
}

Parser::~Parser() {
	delete lexer;
}

bool Parser::parse() {
	bNoErrors = true;
	tree = Program(); // keeps the lexer running till finished/error
	// return bNoErrors;        ----> see what happens  WORKS GREAT! parser now allways 'succeeds'
	return true;
}

void Parser::getToken() {
	lookToken = lexer->lex(); // stores a token, obtained though the lexer, in 'lookToken'
	row = lookToken.row;
	col = lookToken.col;
	kdDebug(0)<<"Parser::getToken(), got a token: '"<<lookToken.str<<"', @ ("<<row<<", "<<col<<")"<<endl;
}

TreeNode* Parser::Program() {
	row = 0;
	col = 0;
	TreeNode* program = new TreeNode(programNode, row, col, "program");
	TreeNode* block = new TreeNode(blockNode, row, col, "block");

	getToken();
	
	// this is the main parse loop
	while (lookToken.type != tokEof) { // lookToken.type returns the type of the current token
		kdDebug(0)<<"Parser::entered main parse loop"<<endl;
		block->appendChild( Statement() );
		// runs statement related code, stores the returned TreeNode* in the nodetree
		// note: Statement() allways gets a new token with getToken() before it returns 
	}
	program->appendChild(block);

	Match(tokEof);
	return program;
}

void Parser::Match(int x) {
	if (lookToken.type != x) {
		QString tokStr = "";
		switch(x) {
			case tokIf            : tokStr += "if";           break;
			case tokElse          : tokStr += "else";         break;
			case tokWhile         : tokStr += "while";        break;
			case tokFor           : tokStr += "for";          break;
			case tokTo            : tokStr += "to";           break;
			case tokStep          : tokStr += "step";         break;
			case tokNumber        : tokStr += "number";       break;
			case tokString        : tokStr += "string";       break;
			case tokId            : tokStr += "id";           break;
			case tokProcId        : tokStr += "procedure id"; break;
			case tokBegin         : tokStr += "begin";        break;
			case tokEnd           : tokStr += "end";          break;
		
			case tokOr            : tokStr += "or";           break;
			case tokAnd           : tokStr += "and";          break;
			case tokNot           : tokStr += "not";          break;
	
			case tokGe            : tokStr += ">=";           break;
			case tokGt            : tokStr += ">";            break;
			case tokLe            : tokStr += "<=";           break;
			case tokLt            : tokStr += "<";            break;
			case tokNe            : tokStr += "!=";           break;
			case tokEq            : tokStr += "==";           break;
			case tokAssign        : tokStr += "=";            break;

			case tokReturn        : tokStr += "return";       break;
			case tokBreak         : tokStr += "break";        break;

			case tokForEach       : tokStr += "foreach";      break;
			case tokIn            : tokStr += "in";           break;
		
			case tokRun           : tokStr += "run";          break;
			case tokEof           : tokStr += "end of file";  break;
			case tokError         : tokStr += "error token";  break;
			
			case tokLearn         : tokStr += "learn";        break;
			
			case tokClear         : tokStr += "clear";        break;
			case tokGo            : tokStr += "go";           break;
			case tokGoX           : tokStr += "gox";          break;
			case tokGoY           : tokStr += "goy";          break;
			case tokForward       : tokStr += "forward";      break;
			case tokBackward      : tokStr += "backward";     break;
			case tokDirection     : tokStr += "direction";    break;
			case tokTurnLeft      : tokStr += "turnleft";     break;
			case tokTurnRight     : tokStr += "turnright";    break;
			case tokCenter        : tokStr += "center";       break;
			case tokSetPenWidth   : tokStr += "setpenwidth";  break;
			case tokPenUp         : tokStr += "penup";        break;
			case tokPenDown       : tokStr += "pendown";      break;
			case tokSetFgColor    : tokStr += "setfgcolor";   break;
			case tokSetBgColor    : tokStr += "setbgcolor";   break;
			case tokResizeCanvas  : tokStr += "resizecanvas"; break;
			case tokSpriteShow    : tokStr += "spriteshow";   break;
			case tokSpriteHide    : tokStr += "spritehide";   break;
			case tokSpritePress   : tokStr += "spritepress";  break;
			case tokSpriteChange  : tokStr += "spritechange"; break;

			case tokMessage       : tokStr += "message";      break;
			case tokInputWindow   : tokStr += "inputwindow";  break;
			case tokPrint         : tokStr += "print";        break;
			case tokFontType      : tokStr += "fonttype";     break;
			case tokFontSize      : tokStr += "fontsize";     break;
			case tokRepeat        : tokStr += "repeat";       break;
			case tokRandom        : tokStr += "random";       break;
			case tokWait          : tokStr += "wait";         break;
			case tokWrapOn        : tokStr += "wrapon";       break;
			case tokWrapOff       : tokStr += "wrapoff";      break;
			case tokReset         : tokStr += "reset";        break;

			default               : tokStr += (char)x; break;
		}
		QString key = lexer->translateCommand(tokStr);
		if ( key.isEmpty() ) {
			// if the tokStr cannot be translated it is an unknow command, and thus either
			// a user defined command or an error
			
			// Error( i18n("Expected a command on line %1, found %2").arg(row).arg(lookToken.str), 1010);
			// without this error it might also work, the executer will take care of it
		} else { 
			Error( i18n("Expected '%1' on line %2").arg(key).arg(row), 1010);
		}
	} else {
		getToken(); 
	}
}




TreeNode* Parser::Learn() {
	// dummy word
	Match(tokLearn);
	TreeNode* r = Function();
	return r;
}

/*==================================================================
   EBNF for a function 
   <function> := tokId '(' <idlist> ')' <block>

   we can safely use tokId because we require "(" and ")" to mark the params
   to be given after the id when it is called, 
   if this were not the case
   we would have to extend the lexer so that it gives a
   tokFunctionId whenever an id has the same name as a function...
===================================================================*/
TreeNode* Parser::Function() {
	TreeNode* func = new TreeNode(functionNode, row, col, "function");
	TreeNode* idn = getId();

	func->appendChild( idn );
	Match('(');
	func->appendChild( IdList() );    
	Match(')');
	func->appendChild( Block() );
	
	learnedFunctionList.append( idn->getName() );

	return func;
}

TreeNode* Parser::getId(){
	TreeNode* n = new TreeNode(idNode, row, col, "id-node");
	n->setName(lookToken.str);
	Match( tokId );
	return n;  
}

/*
  paramlist is like idlist except it does not only
  except id's it accepts expressions seperated by ','!
  This is used by FunctionCall(). 
*/
TreeNode* Parser::ParamList() {
	TreeNode* ilist = new TreeNode(idListNode, row, col, "ilist");
	//check for empty idlist -> function with no parameters
	if( lookToken.type == ')' ) {
		return ilist;
	}

	//get id's seperated by ','
	ilist->appendChild( Expression() ); //aaah KISS (Keep It Simple Sidney)
	while( lookToken.type == ',' ) {
		Match(',');
		ilist->appendChild( Expression() ); 
	}

	return ilist;
}

/*
  <functioncall> := tokId '(' <paramlist> ')' 
*/
TreeNode* Parser::FunctionCall(const QString& name, uint r, uint c) {
	if (learnedFunctionList.contains(name) == 0) {
		Error( i18n("'%1' is not a Logo command nor a learned command.").arg(name), 1010, r, c);
		TreeNode* errNode = new TreeNode(Unknown, r, c, "name");
		return errNode;
	}
	
	kdDebug(0)<<"Parser::FunctionCall, using identifier: '"<<name<<"'"<<endl;
	TreeNode* fcall = new TreeNode(functionCallNode, r, c, "functioncall");
	//first child contains function name
	
	TreeNode* funcid = new TreeNode(idNode, row, col);
	funcid->setName(name);
	fcall->appendChild(funcid);

	Match('(');
	fcall->appendChild( ParamList() );
	Match(')');

	return fcall;
}


/*---------------------------------------------------------------*/
/* Parse and Translate a Math Factor */
TreeNode* Parser::Factor() {
	TreeNode* n;
	switch(lookToken.type) {
		case '(':       
			Match('(');
			n = Expression();
			Match(')');
			break;

		case tokId:     
			n = getId();
			if(lookToken.type == '(') {       //is function call
				QString name = n->getName();
				delete n;
				n = FunctionCall(name);
				n->setType(funcReturnNode); //expect returned value on stack
			}
			break;
                  
		case tokString:
			n = new TreeNode(stringConstantNode, row, col, "string-constant");
			n->setValue(lookToken.str);
			Match(tokString);
			break;

		case tokNumber:
			n = new TreeNode(constantNode, row, col, "constant");
			n->setValue(lookToken.val);
			Match(tokNumber);
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
			QString s = lookToken.str;
			if ( s.isEmpty() || lookToken.type == tokEof ) {
				Error( i18n("Expected an expression"), 1020 );
			} else {
				Error( i18n("Cannot understand '%1' in expression").arg(s), 1020 );
			}
			n = new TreeNode(Unknown, row, col);
			getToken();
			break;
	}
	return n;
}


TreeNode* Parser::signedFactor() {
	TreeNode* sfac; //used by '-' and tokNot
	switch( lookToken.type ){
		case '+':
			Match('+');
			return Factor();
			break;

		case '-':
			Match('-');
			sfac=Factor();
			if(sfac->getType() == constantNode) {
				Number n = sfac->getValue();
				n.val = -n.val;
				sfac->setValue( n );
				return sfac;
			}
			else{
				TreeNode* minus= new TreeNode(minusNode, row, col, "minus");
				minus->appendChild( sfac );
				return minus;
			}
			break;

		case tokNot:
			Match(tokNot);
			sfac = Factor();
			if(sfac->getType() == constantNode) {
				Number n = sfac->getValue();
				n.val = 1 - n.val;
				sfac->setValue(n);
				return sfac;
			}
			else{
				TreeNode* n = new TreeNode(notNode, row, col, "not", lexer->translateCommand("not") );
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

	while ( (lookToken.type == '*') || (lookToken.type == '/') || (lookToken.type == tokAnd) ) {
		// while is is a multiplicative operator do...
		left = pos;
		pos = new TreeNode(Unknown, row, col);
		pos->appendChild(left);

		switch( lookToken.type ) {
			case '*':
				Match('*');
				right = signedFactor();
				pos->setType( mulNode );
				pos->setName( "mul" );
				break;

			case '/':
				Match('/');
				right = signedFactor();
				pos->setType( divNode );
				pos->setName( "div" );
				break;

			case tokAnd:
				Match(tokAnd);
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
		if( right != NULL) {
			pos->appendChild( right );
		}
		termNode = pos;
	}   //end while
	return termNode;
}


bool Parser::isAddOp(token t) {
	return 
		(t.type == '+')   ||
		(t.type == '-')   ||
		(t.type == tokGt) ||
		(t.type == tokGe) ||
		(t.type == tokLt) ||
		(t.type == tokLe) ||
		(t.type == tokEq) ||
		(t.type == tokNe) ||
		
		(t.type == tokOr) ||
		(t.type == tokGe);
}


/*---------------------------------------------------------------*/
/* Parse and Translate an Expression */
TreeNode* Parser::Expression() {
	TreeNode* retExp = Term();
	TreeNode* pos = retExp;
	TreeNode* left = NULL;
	TreeNode* right = NULL;

	while( isAddOp(lookToken) ) {
		left = pos;
		pos = new TreeNode(Unknown, row, col);
		pos->appendChild(left);
		switch(lookToken.type) {
			case '+':
				Match('+');
				right = Term() ;
				pos->setType( addNode );
				pos->setName("add");
				break;

			case '-':
				Match('-');
				right = Term();
				pos->setType( subNode );
				pos->setName("sub");
				break;

			case tokGt:
				Match(tokGt);
				right = Term();
				pos->setType( nodeGT );
				pos->setName(">");
				break;

			case tokLt:
				Match(tokLt);
				right = Term();
				pos->setType( nodeLT );
				pos->setName("<");
				break;

			case tokGe:
				Match(tokGe);
				right = Term();
				pos->setType( nodeGE );
				pos->setName(">=");
				break;

			case tokLe:
				Match(tokLe);
				right = Term();
				pos->setType( nodeLE );
				pos->setName("<=");
				break;
			
			case tokEq:
				Match(tokEq);
				right = Term();
				pos->setType( nodeEQ );
				pos->setName("==");
				break;

			case tokNe:
				Match(tokNe);
				right = Term();
				pos->setType( nodeNE );
				pos->setName("!=");
				break;
			
			case tokOr:
				Match(tokOr);
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


TreeNode* Parser::Assignment(const QString& name) {
	TreeNode* assign = new TreeNode(assignNode, row, col, "assignment");
	
	Match(tokAssign);

	//first child of assign is id or lhv of assignment
	TreeNode* left = new TreeNode(idNode, row, col, "", lexer->translateCommand("") );
	left->setName(name);
	assign->appendChild(left);
	
	//next child is expression or rhv of assignment
	TreeNode* right = Expression();
	assign->appendChild(right);

	return assign;
}


/*
  <while> ::= tokWhile <expression> ( <statement> | <block> )
*/
TreeNode* Parser::While() {
	TreeNode* wNode=new TreeNode(whileNode, row, col, "while", lexer->translateCommand("while") );
	Match(tokWhile);
	wNode->appendChild( Expression() );

	if(lookToken.type == tokBegin){ //while followed by a block
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
TreeNode* Parser::For() {
	TreeNode* fNode=new TreeNode(forNode, row, col, "for loop", lexer->translateCommand("for loop") );
	Match(tokFor);
	fNode->appendChild( getId() ); //loop id
	Match(tokAssign);
	
	fNode->appendChild( Expression() ); //start value expression
	Match(tokTo);
	fNode->appendChild( Expression() ); //stop value expression

	if(lookToken.type == tokStep){
		Match(tokStep);
		fNode->appendChild( Expression() ); //step expression
	}

	if(lookToken.type == tokBegin){ //for followed by a block
		fNode->appendChild( Block() ); 
	} 
	else{ //while followed by single statement
		fNode->appendChild( Statement() );
	}

	return fNode;
}


TreeNode* Parser::ForEach() {
	TreeNode* fNode = new TreeNode(forEachNode, row, col, "foreach loop", lexer->translateCommand("foreach loop") );

	Match(tokForEach);
	fNode->appendChild( Expression() );
	Match(tokIn);
	fNode->appendChild( Expression() );

	if(lookToken.type == tokBegin) { //for followed by a block
		fNode->appendChild( Block() ); 
	} else { //while followed by single statement
		fNode->appendChild( Statement() );
	}

	return fNode;
}


/*
  <if> ::= tokIf <expression> ( <statement> | <block> )
                ( tokElse ( <statement> | <block> ) )?
 
 the expression is in first child
 the first block or statement is in second child
 the else block or statement is in third child
*/
TreeNode* Parser::If() {
	TreeNode* node = new TreeNode(ifNode, row, col, "if", lexer->translateCommand("if") );

	Match(tokIf);
	node->appendChild( Expression() );

	if(lookToken.type == tokDo) {  // skip the 'do'
		getToken(); //next word
	}

	if(lookToken.type == tokBegin) {  //if followed by a block
		node->appendChild( Block() ); 
	} else {   //if followed by single statement
		node->appendChild( Statement() );
	}

	if(lookToken.type == tokElse) { //else part
		Match(tokElse);
		
		if(lookToken.type == tokDo) {  // skip the 'do'
			getToken(); // next word
		}

		if(lookToken.type == tokBegin) {  //else is followed by block
			node->appendChild( Block() );
		} else {
			node->appendChild( Statement() );
		}

	}

	return node;
}


TreeNode* Parser::getString(){
	TreeNode* str = new TreeNode(stringConstantNode, row, col, "string");
	
	str->setStrValue(lookToken.str);
	Match( tokString );
	
	return str;
}

TreeNode* Parser::Return(){
	TreeNode* ret=new TreeNode(returnNode, row, col, "return", lexer->translateCommand("return") );
	
	Match(tokReturn);
	ret->appendChild( Expression() );
	
	return ret;
}

TreeNode* Parser::Break() {
	TreeNode* brk = new TreeNode(breakNode, row, col, "break", lexer->translateCommand("break") );
	Match ( tokBreak );
	
	return brk;
}

TreeNode* Parser::Statement() {
	kdDebug(0)<<"Parser::Statement()"<<endl;
	switch(lookToken.type) {
		case tokLearn         : return Learn();            break;

		case tokIf            : return If();               break;
		case tokFor           : return For();              break;
		case tokForEach       : return ForEach();          break;
		case tokWhile         : return While();            break;
		case tokRun           : return ExternalRun();      break;
		case tokReturn        : return Return();           break;
		case tokBreak         : return Break();            break;
		case tokId            : return Other();            break; //assignment or function call

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
	const QString beginChar = lexer->translateCommand("begin");
	const QString lookTokenChar = lookToken.str;
	if (lookTokenChar == beginChar) {
		Error( i18n("'%1' is expected").arg(beginChar), 1060);
	} else {
		Error( i18n("'%1' is no Logo command").arg(lookTokenChar), 1060); 
	}
	getToken();
	return new TreeNode(Unknown, row, col); // fallback for unknowns
}


TreeNode* Parser::Block() {
	TreeNode* block = new TreeNode(blockNode, row, col, "block");
	
	Match(tokBegin);
	while( (lookToken.type != tokEnd) && (lookToken.type != tokEof) ){
		block->appendChild( Statement() );
	}
	Match(tokEnd);
	
	return block;
}


TreeNode* Parser::IdList() {
	TreeNode* ilist = new TreeNode(idListNode, row, col, "idlist");

	//check for empty idlist -> function with no parameters
	if( lookToken.type == ')' ) return ilist;
	
	//get id's seperated by ','
	ilist->appendChild( getId() );
	while( lookToken.type == ',' ){
		Match(',');
		ilist->appendChild( getId() );
	}

	return ilist;
}


//
//    Turtle Funktions
//

TreeNode* Parser::Clear() {
	TreeNode* node = new TreeNode(ClearNode, row, col, "clear", lexer->translateCommand("clear") );
	getToken();
	return node;
}


TreeNode* Parser::Go() {
	TreeNode* node = new TreeNode(GoNode, row, col, "go", lexer->translateCommand("go") );
	
	getToken();
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}


TreeNode* Parser::GoX() {
	TreeNode* node = new TreeNode(GoXNode, row, col, "gox", lexer->translateCommand("gox") );
	
	getToken();
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}


TreeNode* Parser::GoY() {
	TreeNode* node = new TreeNode(GoYNode, row, col, "goy", lexer->translateCommand("goy") );
	
	getToken();
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}


TreeNode* Parser::Forward() {
	TreeNode* node = new TreeNode(ForwardNode, row, col, "forward", lexer->translateCommand("forward") );
	
	getToken();
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ) {
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}


TreeNode* Parser::Backward() {
	TreeNode* node = new TreeNode(BackwardNode, row, col, "backward", lexer->translateCommand("backward") );
	
	getToken();
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ) {
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}

TreeNode* Parser::Direction() {
	TreeNode* node = new TreeNode(DirectionNode, row, col, "direction", lexer->translateCommand("direction") );
	
	getToken(); //is tokPrint or tokPrintLn
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ) {
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}

TreeNode* Parser::TurnLeft() {
	TreeNode* node = new TreeNode(TurnLeftNode, row, col, "turnleft", lexer->translateCommand("turnleft") );
	
	getToken(); //is tokPrint or tokPrintLn
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}


TreeNode* Parser::TurnRight() {
	TreeNode* node = new TreeNode(TurnRightNode, row, col, "turnright", lexer->translateCommand("turnright") );
	
	getToken(); //is tokPrint or tokPrintLn
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}


TreeNode* Parser::Center() {
	TreeNode* node = new TreeNode(CenterNode, row, col, "center", lexer->translateCommand("center") );
	getToken();
	return node;
}


TreeNode* Parser::SetPenWidth() {
	TreeNode* node = new TreeNode(SetPenWidthNode, row, col, "setpenwidth", lexer->translateCommand("setpenwidth") );
	
	getToken();
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}


TreeNode* Parser::PenUp() {
	TreeNode* node = new TreeNode(PenUpNode, row, col, "penup", lexer->translateCommand("penup") );
	getToken();
	return node;
}


TreeNode* Parser::PenDown() {
	TreeNode* node = new TreeNode(PenDownNode, row, col, "pendown", lexer->translateCommand("pendown") );
	getToken();
	return node;
}


TreeNode* Parser::SetFgColor() {
	TreeNode* node = new TreeNode(SetFgColorNode, row, col, "setfgcolor", lexer->translateCommand("setfgcolor") );
	
	//first expression
	getToken();
	node->appendChild( Expression() );
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}


TreeNode* Parser::SetBgColor() {
	TreeNode* node = new TreeNode(SetBgColorNode, row, col, "setbgcolor", lexer->translateCommand("setbgcolor") );
	
	//first expression
	getToken();
	node->appendChild( Expression() );
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}


TreeNode* Parser::ResizeCanvas() {
	TreeNode* node = new TreeNode(ResizeCanvasNode, row, col, "resizecanvas", lexer->translateCommand("resizecanvas") );
	
	getToken();
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}


TreeNode* Parser::SpriteShow() {
	TreeNode* node = new TreeNode(SpriteShowNode, row, col, "spriteshow", lexer->translateCommand("spriteshow") );
	getToken();
	return node;
}


TreeNode* Parser::SpriteHide() {
	TreeNode* node = new TreeNode(SpriteHideNode, row, col, "spritehide", lexer->translateCommand("spritehide") );
	getToken();
	return node;
}


TreeNode* Parser::SpritePress() {
	TreeNode* node = new TreeNode(SpritePressNode, row, col, "spritepress", lexer->translateCommand("spritepress") );
	getToken();
	return node;
}


TreeNode* Parser::SpriteChange() {
	TreeNode* node = new TreeNode(SpriteChangeNode, row, col, "spritechange", lexer->translateCommand("spritechange") );
	
	getToken();
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}



TreeNode* Parser::Message() {
	TreeNode* node = new TreeNode(MessageNode, row, col, "message", lexer->translateCommand("message") );
	
	getToken();
	node->appendChild( Expression() );
	
	return node;
}

TreeNode* Parser::InputWindow() {
	TreeNode* node = new TreeNode(InputWindowNode, row, col, "inputwindow", lexer->translateCommand("inputwindow") );
	
	getToken();
	node->appendChild( Expression() );
	
	return node;
}


/*
  <print> ::= 'print' ( ( <expression> | <string> ) ',' )+
*/
TreeNode* Parser::Print() {
	TreeNode* node = new TreeNode(printNode, row, col, "print", lexer->translateCommand("print") );
	
	getToken();
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	
	return node;
}

TreeNode* Parser::FontType() {
	TreeNode* node = new TreeNode(FontTypeNode, row, col, "fonttype", lexer->translateCommand("fonttype") );
	
	getToken();

	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}

TreeNode* Parser::FontSize() {
	TreeNode* node = new TreeNode(FontSizeNode, row, col, "fontsize", lexer->translateCommand("fontsize") );
	
	getToken();
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ) {
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}

TreeNode* Parser::Repeat() {
	TreeNode* node = new TreeNode(RepeatNode, row, col, "repeat", lexer->translateCommand("repeat") );
	Match(tokRepeat);
	node->appendChild( Expression() );
	node->appendChild( Block() );
	
// 	if(lookToken.type == tokBegin) { // repeat followed by a block
// 		node->appendChild( Block() ); 
// 	} else { // repeat followed by single statement
// 		node->appendChild( Statement() );
// 	}
	
	return node;
}

TreeNode* Parser::Random() {
	TreeNode* node = new TreeNode(RandomNode, row, col, "random", lexer->translateCommand("random") );
	getToken();
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}

TreeNode* Parser::Wait() {
	TreeNode* node = new TreeNode(WaitNode, row, col, "wait", lexer->translateCommand("wait") );
	getToken();
	
	//first expression
	node->appendChild( Expression() );
	
	//following strings or expressions
	while( lookToken.type == ',' ){
		getToken(); //the comma
		node->appendChild( Expression() );
	}
	return node;
}

TreeNode* Parser::WrapOn() {
	TreeNode* node = new TreeNode(WrapOnNode, row, col, "wrapon", lexer->translateCommand("wrapon") );
	getToken();
	return node;
}

TreeNode* Parser::WrapOff() {
	TreeNode* node = new TreeNode(WrapOffNode, row, col, "wrapoff", lexer->translateCommand("wrapoff") );
	getToken();
	return node;
}

TreeNode* Parser::Reset() {
	TreeNode* node = new TreeNode(ResetNode, row, col, "reset", lexer->translateCommand("reset") );
	getToken();
	return node;
}

TreeNode* Parser::ExternalRun() {
	TreeNode* n = new TreeNode(runNode, row, col, "run", lexer->translateCommand("run") );
	Match(tokRun);
	n->appendChild( Expression() );
	return n;
}

// this is either an assignment or a function call!
TreeNode* Parser::Other() {
	kdDebug(0)<<"Parser::Other()"<<endl;
	token presevedToken = lookToken; // preserve token, else Match() will make sure it gets lost
	Match(tokId);

	if(lookToken.type == tokAssign) {
		return Assignment(presevedToken.str);
	} else {
		return FunctionCall(presevedToken.str, presevedToken.row, presevedToken.col);
	}
}


void Parser::Error(const QString& s, uint code, uint r, uint c) {
	emit ErrorMsg(s, (r == NA ? row : r), (c == NA ? col : c), code);
	bNoErrors = false;
}


#include "parser.moc"
