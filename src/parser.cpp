#include <qstring.h>

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

Parser::Parser(istream& in) {
  lexer = new Lexer(in);
  tree = new TreeNode();
}


Parser::~Parser() {
  delete lexer;
}


void Parser::Error(const QString& s, int code) {
  emit ErrorMsg(s, row, col, code);
  //exit(1); // better = throw exception here!
  bNoErrors=false;
}

void Parser::getToken() {
  look = lexer->lex();
  row = lexer->getRow();
  col = lexer->getCol();
}


bool Parser::parse() {
  bNoErrors = true;
  getToken();
  tree = Program();
  return bNoErrors;
}


TreeNode* Parser::getTree() {
  return tree;
}

void Parser::Match(int x) {
  if(look.type != x) {
    string tokStr = "";
    switch(x) {
      case tokIf        : tokStr+="if";           break;
      case tokElse      : tokStr+="else";         break;
      case tokWhile     : tokStr+="while";        break;
      case tokFor       : tokStr+="for";          break;
      case tokTo        : tokStr+="to";           break;
      case tokStep      : tokStr+="step";         break;
      case tokNumber    : tokStr+="number";       break;
      case tokString    : tokStr+="string";       break;
      case tokId        : tokStr+="id";           break;
      case tokProcId    : tokStr+="procedure id"; break;
      case tokBegin     : tokStr+="begin";        break;
      case tokEnd       : tokStr+="end";          break;
    
      case tokOr        : tokStr+="or";      break;
      case tokAnd       : tokStr+="and";     break;
      case tokNot       : tokStr+="not";     break;
  
      case tokGe        : tokStr+=">=";      break;
      case tokGt        : tokStr+=">";       break;
      case tokLe        : tokStr+="<=";      break;
      case tokLt        : tokStr+="<";       break;
      case tokNe        : tokStr+="!=";      break;
      case tokEq        : tokStr+="==";      break;
      case tokAssign    : tokStr+="=";       break;

      case tokReturn    : tokStr+="return";  break;
      case tokBreak     : tokStr+="break";   break;

      case tokForEach   : tokStr+="foreach";   break;
      case tokIn        : tokStr+="in";        break;
    
      case tokRun       : tokStr+="run";          break;
      case tokEof       : tokStr+="end of file";  break;
      case tokError     : tokStr+="error token";  break;
      
      case tokLearn         : tokStr+="learn";        break;
      
      case tokClear         : tokStr+="clear";        break;
      case tokGo            : tokStr+="go";           break;
      case tokGoX           : tokStr+="gox";          break;
      case tokGoY           : tokStr+="goy";          break;
      case tokForward       : tokStr+="forward";      break;
      case tokBackward      : tokStr+="backward";     break;
      case tokDirection     : tokStr+="direction";    break;
      case tokTurnLeft      : tokStr+="turnleft";     break;
      case tokTurnRight     : tokStr+="turnright";    break;
      case tokCenter        : tokStr+="center";       break;
      case tokSetPenWidth   : tokStr+="setpenwidth";  break;
      case tokPenUp         : tokStr+="penup";        break;
      case tokPenDown       : tokStr+="pendown";      break;
      case tokSetFgColor    : tokStr+="setfgcolor";   break;
      case tokSetBgColor    : tokStr+="setbgcolor";   break;
      case tokResizeCanvas  : tokStr+="resizecanvas"; break;
      case tokSpriteShow    : tokStr+="spriteshow";   break;
      case tokSpriteHide    : tokStr+="spritehide";   break;
      case tokSpritePress   : tokStr+="spritepress";  break;
      case tokSpriteChange  : tokStr+="spritechange"; break;

      case tokInput         : tokStr+="input";        break;
      case tokInputWindow   : tokStr+="inputwindow";  break;
      case tokPrint         : tokStr+="print";        break;
      case tokFontType      : tokStr+="fonttype";     break;
      case tokFontSize      : tokStr+="fontsize";     break;
      case tokRepeat        : tokStr+="repeat";       break;
      case tokRandom        : tokStr+="random";       break;
      case tokWait          : tokStr+="wait";         break;
      case tokWrapOn        : tokStr+="wrapon";       break;
      case tokWrapOff       : tokStr+="wrapoff";      break;
      case tokReset         : tokStr+="reset";        break;

      default: tokStr+= (char)x; break;
    }
//     QString QTokStr = tokStr.c_str();
//     Error( i18n("Syntax error, expected") + " '" + QTokStr + "'", 1010);
    QString commandname = lexer->translateCommand(tokStr);
    Error( i18n("Syntax error, expected ") + commandname, 1010);
    } else {
        getToken(); 
    }
}


TreeNode* Parser::signedFactor(){
  TreeNode* sfac; //used by '-' and tokNot
  switch( look.type ){
    case '+':     Match('+');
                  return Factor();
                  break;

    case '-':     Match('-');
                  sfac=Factor();
                  if( sfac->getType() == constantNode ){
                    Number n=sfac->getValue();
                    n.val= - n.val;
                    sfac->setValue( n );
                    return sfac;
                  }
                  else{
                    TreeNode* minus= new TreeNode( minusNode, row, col );
                    minus->setName("minus");
                    minus->appendChild( sfac );
                    return minus;
                  }
                  break;

    case tokNot:  Match(tokNot);
                  sfac=Factor();
                  if( sfac->getType() == constantNode ){
                    Number n=sfac->getValue();
                    n.val= 1 - n.val;
                    sfac->setValue( n );
                    return sfac;
                  }
                  else{
                    TreeNode* n = new TreeNode( notNode, row, col );
                    n->setName("not");
                    n->appendChild( sfac );
                    return n;
                  }
                  break;

    default:      return Factor();
                  break;
  }
}



TreeNode* Parser::getId(){
  TreeNode* n=new TreeNode( idNode, row, col );
  n->setName( look.str );
  Match( tokId );
  return n;  
}


TreeNode* Parser::runFunction() {
  TreeNode* n = new TreeNode( runNode, row, col );
  n->setName("run");
  n->setKey( lexer->translateCommand("run") );
  Match(tokRun);
  n->appendChild( Expression() );
  return n;
}


/*---------------------------------------------------------------*/
/* Parse and Translate a Math Factor */
TreeNode* Parser::Factor() {
  TreeNode* n;
  switch( look.type ) {
    case '(':       Match('(');
                    n=Expression();
                    Match(')');
                    break;

    case tokId:     n=getId();
                    if( look.type == '(' ){       //is function call
                      string name=n->getName();
                      delete n;              
                      n=FunctionCall( name );
                      n->setType(funcReturnNode); //expect returned value on stack
                    }
                    break;
                  
    case tokString: n=new TreeNode( stringConstantNode, row, col );
                    n->setValue( look.str );
                    Match(tokString);
                    break;

    case tokNumber: n=new TreeNode( constantNode, row, col );
                    n->setValue( look.val );
                    Match(tokNumber);
                    break;
    
    case tokRun:    n=runFunction();     break;
    
    case tokInputWindow: n=InputWindow();     break;
    
    case tokRandom: n=Random();     break;
                                    
    default:        Error( i18n("Illegal char in expression"), 1020 );
                    n=new TreeNode( Unknown, row, col );
                    getToken();
                    break;
  }
  return n;
}




/*---------------------------------------------------------------*/
/* identify a multiplicative operator */
bool Parser::isMulOp(token t){
  return 
    (t.type == '*') ||
    (t.type == '/') ||
    (t.type == tokAnd)
  ;
}



/*---------------------------------------------------------------*/
/* Parse and Translate a Math Term */
TreeNode* Parser::Term()
{
  TreeNode* termNode=signedFactor();
  TreeNode* pos=termNode;
  TreeNode* left=NULL;
  TreeNode* right=NULL;
  
  while( isMulOp(look) ){
    left=pos;
    pos=new TreeNode( Unknown, row, col );
    pos->appendChild( left );
        
    switch( look.type ){
      case '*':     Match('*');
                    right = signedFactor();
                    pos->setType( mulNode );
                    pos->setName( "mul" );
                    break;
      
      case '/':     Match('/');
                    right = signedFactor();
                    pos->setType( divNode );
                    pos->setName( "div" );
                    break;
      
      case tokAnd:  Match(tokAnd);
                    right = signedFactor();
                    pos->setType( andNode );
                    pos->setName( "and" );
                    break;
                    
      default:      Error( i18n("Expected '*' or '/'"), 1030 ); 
                    getToken();
                    return pos;
                    break;
    }
    
    if(right!=NULL) pos->appendChild( right );
    termNode=pos;
  }//end while
  
  return termNode;
}


bool Parser::isAddOp(token t){
  return 
    (t.type == '+') ||
    (t.type == '-') ||
    (t.type == tokGt)   ||
    (t.type == tokGe)   ||
    (t.type == tokLt)   ||
    (t.type == tokLe)   ||
    (t.type == tokEq)   ||
    (t.type == tokNe)   ||
    
    (t.type == tokOr )  ||
    (t.type == tokGe)
  ;
}


/*---------------------------------------------------------------*/
/* Parse and Translate an Expression */
TreeNode* Parser::Expression()
{
  TreeNode* retExp=Term();
  TreeNode* pos=retExp;
  TreeNode* left=NULL;
  TreeNode* right=NULL;

  while( isAddOp( look ) ){
    left=pos;
    pos=new TreeNode( Unknown, row, col );
    pos->appendChild( left );
      
    switch( look.type ){
      case '+':   Match('+');
                  right = Term() ;
                  pos->setType( addNode );
                  pos->setName("add");
                  break;

      case '-':   Match('-');
                  right = Term();
                  pos->setType( subNode );
                  pos->setName("sub");
                  break;

      case tokGt: Match(tokGt);
                  right = Term();
                  pos->setType( nodeGT );
                  pos->setName(">");
                  break;

      case tokLt: Match(tokLt);
                  right = Term();
                  pos->setType( nodeLT );
                  pos->setName("<");
                  break;

      case tokGe: Match(tokGe);
                  right = Term();
                  pos->setType( nodeGE );
                  pos->setName(">=");
                  break;

      case tokLe: Match(tokLe);
                  right = Term();
                  pos->setType( nodeLE );
                  pos->setName("<=");
                  break;
      
      case tokEq: Match(tokEq);
                  right = Term();
                  pos->setType( nodeEQ );
                  pos->setName("==");
                  break;

      case tokNe: Match(tokNe);
                  right = Term();
                  pos->setType( nodeNE );
                  pos->setName("!=");
                  break;
      
      case tokOr: Match(tokOr);
                  right = Term();
                  pos->setType( orNode );
                  pos->setName( "or" );
                  break;

      default :   Error( i18n("Expected additive operation"), 1040 );
                  getToken();
                  return pos;
                  break;
    }
    if(right!=NULL) pos->appendChild( right );
    retExp=pos;
  }
    
  return retExp;
}


TreeNode* Parser::Assignment( const string& name ){
  TreeNode* assign = new TreeNode( assignNode, row, col );
  assign->setName("assignment");
  
  Match(tokAssign);

  //first child of assign is id or lhv of assignment
  TreeNode* left = new TreeNode( idNode, row, col );
  left->setName(name);
  assign->appendChild(left);
  
  //next child is expression or rhv of assignment
  TreeNode* right = Expression();
  assign->appendChild(right);

  return assign;
}



/*
  paramlist is like idlist except it does not only
  except id's it accepts expressions seperated by ','!
*/
TreeNode* Parser::ParamList() {
  TreeNode* ilist=new TreeNode( idListNode, row, col );
  ilist->setName("ilist");
  //check for empty idlist -> function with no parameters
  if( look.type == ')' ) return ilist;
  
  //get id's seperated by ','
  ilist->appendChild( Expression() ); //aaah KISS (Keep It Simple Sidney)
  while( look.type == ',' ){
    Match(',');
    ilist->appendChild( Expression() ); 
  }

  return ilist;
}



/*
  <functioncall> := tokId '(' <paramlist> ')' 
*/
TreeNode* Parser::FunctionCall( const string& name ) {
  TreeNode* fcall=new TreeNode( functionCallNode, row, col );
  fcall->setName("functioncall");
  //first child contains function name
  TreeNode* funcid= new TreeNode( idNode, row, col );
  funcid->setName( name );
  fcall->appendChild( funcid );
  
  Match( '(' );
  fcall->appendChild( ParamList() );
  Match( ')' );
  
  return fcall;
}


// this is either an assignment or a function call!
TreeNode* Parser::Other(){
  string idname=look.str; 
  Match(tokId);
  
  if( look.type == tokAssign ){
    return Assignment( idname );
  }
  else{
    return FunctionCall( idname );
  }
}




/*
  <while> ::= tokWhile <expression> ( <statement> | <block> )
*/
TreeNode* Parser::While() {
  TreeNode* wNode=new TreeNode( whileNode, row, col );
  wNode->setName("while");
  wNode->setKey( lexer->translateCommand("while") );
  Match(tokWhile);
  wNode->appendChild( Expression() );
  
  if(look.type == tokBegin){ //while followed by a block
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
TreeNode* Parser::For(){
  TreeNode* fNode=new TreeNode( forNode, row, col );
  fNode->setName("for loop");
  fNode->setKey( lexer->translateCommand("for loop") );
  Match(tokFor);
  fNode->appendChild( getId() ); //loop id
  Match(tokAssign);
  
  fNode->appendChild( Expression() ); //start value expression
  Match(tokTo);
  fNode->appendChild( Expression() ); //stop value expression

  if( look.type == tokStep ){
    Match(tokStep);
    fNode->appendChild( Expression() ); //step expression
  }
    
  if(look.type == tokBegin){ //for followed by a block
    fNode->appendChild( Block() ); 
  } 
  else{ //while followed by single statement
    fNode->appendChild( Statement() );
  }
  
  return fNode;
}


TreeNode* Parser::ForEach(){
  TreeNode* fNode = new TreeNode( forEachNode, row, col );
  fNode->setName("foreach loop");
  fNode->setKey( lexer->translateCommand("foreach loop") );

  Match(tokForEach);
  fNode->appendChild( Expression() );
  Match(tokIn);
  fNode->appendChild( Expression() );
  
  if(look.type == tokBegin) { //for followed by a block
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
  TreeNode* node=new TreeNode( ifNode, row, col );
  node->setName("if");
  node->setKey( lexer->translateCommand("if") );
  
  Match(tokIf);
  node->appendChild( Expression() );

  if(look.type == tokDo) {  // skip the 'do'
    getToken(); //next word
  }
    
  if(look.type == tokBegin) {  //if followed by a block
    node->appendChild( Block() ); 
  } else {   //if followed by single statement
    node->appendChild( Statement() );
  }
  
  if(look.type == tokElse) { //else part
    Match(tokElse);
    
    if(look.type == tokDo) {  // skip the 'do'
      getToken(); // next word
    }
    
    if(look.type == tokBegin) {  //else is followed by block
      node->appendChild( Block() );
    } else {
      node->appendChild( Statement() );
    }
  
  }
  
  return node;
}


TreeNode* Parser::getString(){
  TreeNode* str = new TreeNode( stringConstantNode, row, col );
  str->setName("string");
  
  str->setStrValue( look.str );
  Match( tokString );
  
  return str;
}

TreeNode* Parser::Return(){
  TreeNode* ret=new TreeNode( returnNode, row, col );
  ret->setName("return");
  
  Match( tokReturn );
  ret->appendChild( Expression() );
  
  return ret;
}

TreeNode* Parser::Break() {
  TreeNode* brk = new TreeNode( breakNode, row, col );
  brk->setName("break");
  Match ( tokBreak );
  
  return brk;
}

TreeNode* Parser::Statement() {
  switch(look.type) {
    case tokLearn         : return Learn();               break;
  
    case tokIf            : return If();               break;
    case tokFor           : return For();              break;
    case tokForEach       : return ForEach();          break;
    case tokWhile         : return While();            break;
    case tokRun           : return runFunction();      break;
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
    case tokInput         : return Input();            break;
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
                            return new TreeNode(Unknown, row, col );
                            break;
                                            
    default               : break;    
  }
  QString qstr = look.str.c_str();
  Error( i18n("Incorrect statement: ") + qstr, 1060); 
  getToken();
  return new TreeNode( Unknown, row, col );
}


TreeNode* Parser::Block() {
  TreeNode* block = new TreeNode( blockNode, row, col );
  block->setName("block");
  
  Match(tokBegin);
  while( (look.type != tokEnd) && (look.type != tokEof) ){
    block->appendChild( Statement() );
  }
  Match(tokEnd);
  
  return block;
}


TreeNode* Parser::IdList(){
  TreeNode* ilist=new TreeNode( idListNode, row, col );
  ilist->setName("idlist");

  //check for empty idlist -> function with no parameters
  if( look.type == ')' ) return ilist;
  
  //get id's seperated by ','
  ilist->appendChild( getId() );
  while( look.type == ',' ){
    Match(',');
    ilist->appendChild( getId() );
  }

  return ilist;
}


/*==================================================================
   EBNF for a function 
   <function> := tokId '(' <idlist> ')' <block>

   we can safely use tokId because we require ( .. )
   to be given after the id when it is called, 
   if this were not the case
   we would have to extend the lexer so that it gives a
   tokFunctionId whenever an id has the same name as a function...
===================================================================*/
TreeNode* Parser::Function() {
  TreeNode* func = new TreeNode( functionNode, row, col );
  TreeNode* idn = getId();
  func->setName("function");
  
  func->appendChild( idn );
  Match('(');
  func->appendChild( IdList() );    
  Match(')');
  func->appendChild( Block() );

  return func;
}

TreeNode* Parser::Program() {
  TreeNode* program = new TreeNode( programNode, row, col );
  program->setName("program");  
  TreeNode* block = new TreeNode( blockNode, row, col );
  block->setName("block");

  while( /*(look.type != tokEnd) &&*/ (look.type != tokEof) ) {
    block->appendChild( Statement() );
  }
  program->appendChild( block );

  Match(tokEof);
  return program;
}


//
//    Turtle Funktions
//

TreeNode* Parser::Clear() {
    TreeNode* node=new TreeNode( ClearNode, row, col );
    node->setName("clear");
    node->setKey( lexer->translateCommand("clear") );
    getToken();
    return node;
}


TreeNode* Parser::Go() {
  TreeNode* node=new TreeNode( GoNode, row, col );
  node->setName("go");
  node->setKey( lexer->translateCommand("go") );
  
  getToken();
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}


TreeNode* Parser::GoX() {
  TreeNode* node=new TreeNode( GoXNode, row, col );
  node->setName("gox");
  node->setKey( lexer->translateCommand("gox") );
  
  getToken();
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}


TreeNode* Parser::GoY() {
  TreeNode* node=new TreeNode( GoYNode, row, col );
  node->setName("goy");
  node->setKey( lexer->translateCommand("goy") );
  
  getToken();
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}


TreeNode* Parser::Forward() {
  TreeNode* node=new TreeNode( ForwardNode, row, col );
  node->setName("forward");
  node->setKey( lexer->translateCommand("forward") );
  
  getToken();
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ) {
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}


TreeNode* Parser::Backward() {
  TreeNode* node=new TreeNode( BackwardNode, row, col );
  node->setName("backward");
  node->setKey( lexer->translateCommand("backward") );
  
  getToken();
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ) {
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}

TreeNode* Parser::Direction() {
  TreeNode* node=new TreeNode( DirectionNode, row, col );
  node->setName("direction");
  node->setKey( lexer->translateCommand("direction") );
  
  getToken(); //is tokPrint or tokPrintLn
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ) {
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}

TreeNode* Parser::TurnLeft() {
  TreeNode* node=new TreeNode( TurnLeftNode, row, col );
  node->setName("turnleft");
  node->setKey( lexer->translateCommand("turnleft") );
  
  getToken(); //is tokPrint or tokPrintLn
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}


TreeNode* Parser::TurnRight() {
  TreeNode* node=new TreeNode( TurnRightNode, row, col );
  node->setName("turnright");
  node->setKey( lexer->translateCommand("turnright") );
  
  getToken(); //is tokPrint or tokPrintLn
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}


TreeNode* Parser::Center() {
    TreeNode* node=new TreeNode( CenterNode, row, col );
    node->setName("center");
    node->setKey( lexer->translateCommand("center") );
    getToken();
    return node;
}


TreeNode* Parser::SetPenWidth() {
  TreeNode* node=new TreeNode( SetPenWidthNode, row, col );
  node->setName("setpenwidth");
  node->setKey( lexer->translateCommand("setpenwidth") );
  
  getToken();
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}


TreeNode* Parser::PenUp() {
    TreeNode* node=new TreeNode( PenUpNode, row, col );
    node->setName("penup");
    node->setKey( lexer->translateCommand("penup") );
    getToken();
    return node;
}


TreeNode* Parser::PenDown() {
    TreeNode* node=new TreeNode( PenDownNode, row, col );
    node->setName("pendown");
    node->setKey( lexer->translateCommand("pendown") );
    getToken();
    return node;
}


TreeNode* Parser::SetFgColor() {
  TreeNode* node=new TreeNode( SetFgColorNode, row, col );
  node->setName("setfgcolor");
  node->setKey( lexer->translateCommand("setfgcolor") );
  
  //first expression
  getToken();
  node->appendChild( Expression() );
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}


TreeNode* Parser::SetBgColor() {
  TreeNode* node = new TreeNode( SetBgColorNode, row, col );
  node->setName("setbgcolor");
  node->setKey( lexer->translateCommand("setbgcolor") );
  
  //first expression
  getToken();
  node->appendChild( Expression() );
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}


TreeNode* Parser::ResizeCanvas() {
  TreeNode* node=new TreeNode( ResizeCanvasNode, row, col );
  node->setName("resizecanvas");
  node->setKey( lexer->translateCommand("resizecanvas") );
  
  getToken();
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}


TreeNode* Parser::SpriteShow() {
    TreeNode* node=new TreeNode( SpriteShowNode, row, col );
    node->setName("spriteshow");
    node->setKey( lexer->translateCommand("spriteshow") );
    getToken();
    return node;
}


TreeNode* Parser::SpriteHide() {
    TreeNode* node=new TreeNode( SpriteHideNode, row, col );
    node->setName("spritehide");
    node->setKey( lexer->translateCommand("spritehide") );
    getToken();
    return node;
}


TreeNode* Parser::SpritePress() {
    TreeNode* node=new TreeNode( SpritePressNode, row, col );
    node->setName("spritepress");
    node->setKey( lexer->translateCommand("spritepress") );
    getToken();
    return node;
}


TreeNode* Parser::SpriteChange() {
  TreeNode* node=new TreeNode( SpriteChangeNode, row, col );
  node->setName("spritechange");
  node->setKey( lexer->translateCommand("spritechange") );
  
  getToken();
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}



TreeNode* Parser::Input() {
  TreeNode* node=new TreeNode( inputNode, row, col );
  node->setName("input");
  node->setKey( lexer->translateCommand("input") );
  
  Match( tokInput ); 
  node->appendChild( getId() ); 
  
  return node;
}

TreeNode* Parser::InputWindow() {
  TreeNode* node=new TreeNode( InputWindowNode, row, col );
  node->setName("inputwindow");
  node->setKey( lexer->translateCommand("inputwindow") );
  
  getToken();
  
  //node->appendChild( getId() ); 
  node->appendChild( Expression() );
  
  return node;
}


/*
  <print> ::= 'print' ( ( <expression> | <string> ) ',' )+
*/
TreeNode* Parser::Print(){
  TreeNode* node=new TreeNode( printNode, row, col );
  node->setName("print");
  node->setKey( lexer->translateCommand("print") );
  
  getToken();
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  
  return node;
}

TreeNode* Parser::FontType() {
  TreeNode* node=new TreeNode( FontTypeNode, row, col );
  node->setName("fonttype");
  node->setKey( lexer->translateCommand("fonttype") );
  
  getToken();
 
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}

TreeNode* Parser::FontSize() {
  TreeNode* node=new TreeNode( FontSizeNode, row, col );
  node->setName("fontsize");
  node->setKey( lexer->translateCommand("fontsize") );
  
  getToken();
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}

TreeNode* Parser::Repeat() {
  TreeNode* node=new TreeNode( RepeatNode, row, col );
  node->setName("repeat");
  node->setKey( lexer->translateCommand("repeat") );
  Match(tokRepeat);
  node->appendChild( Expression() );
  
  if(look.type == tokBegin) { // repeat followed by a block
    node->appendChild( Block() ); 
  } else { // repeat followed by single statement
    node->appendChild( Statement() );
  }
  
  return node;
}

TreeNode* Parser::Random() {
  TreeNode* node=new TreeNode( RandomNode, row, col );
  node->setName("random");
  node->setKey( lexer->translateCommand("random") );
  
  getToken();
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}

TreeNode* Parser::Wait() {
  TreeNode* node=new TreeNode( WaitNode, row, col );
  node->setName("wait");
  node->setKey( lexer->translateCommand("wait") );
  
  getToken();
  
  //first expression
  node->appendChild( Expression() );
  
  //following strings or expressions
  while( look.type == ',' ){
    getToken(); //the comma
    node->appendChild( Expression() );
  }
  return node;
}

TreeNode* Parser::WrapOn() {
    TreeNode* node=new TreeNode( WrapOnNode, row, col );
    node->setName("wrapon");
    node->setKey( lexer->translateCommand("wrapon") );
    getToken();
    return node;
}

TreeNode* Parser::WrapOff() {
    TreeNode* node=new TreeNode( WrapOffNode, row, col );
    node->setName("wrapoff");
    node->setKey( lexer->translateCommand("wrapoff") );
    getToken();
    return node;
}

TreeNode* Parser::Reset() {
    TreeNode* node=new TreeNode( ResetNode, row, col );
    node->setName("reset");
    node->setKey( lexer->translateCommand("reset") );
    getToken();
    return node;
}


TreeNode* Parser::Learn() {
    // dummy word
    Match(tokLearn);
    TreeNode* r = Function();
    return r;
}



/*
  //obsolete
  string Parser::toString( number n ){
    ostringstream os;
    os<<n;
    return os.str();
  }
*/

#include "parser.moc"
