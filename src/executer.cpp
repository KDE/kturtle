/*=============================================================================
author        :Walter Schreppers
filename      :executer.h
description   :Execute a parse tree.
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
 
#include <unistd.h> // for usleep();

#include <qtimer.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>

#include "executer.h"


Executer::Executer(TreeNode* tree) {
  this->tree = tree;
  functionTable.clear();
  bBreak = false;
  bReturn = false;
}

Executer::~Executer() {
  emit Finished();
}

bool Executer::run() {
  bBreak = false;
  bReturn = false;
  bAbort = false;
  symtable main;
  symbolTables.push( main ); //new symbol table for main block

  TreeNode::const_iterator i;
  for( i = tree->begin(); i != tree->end(); ++i ){
    if (bAbort) { return false; }
    kapp->processEvents();
    execute( *i );
  
    symbolTables.pop(); //free up stack
  }
  return true;
}

void Executer::abort() {
  // The next line is within all loops of the Executer
  //     if(bAbort) { return; }
  // mostly next to
  //     kapp->processEvents();
  bAbort = true;
}


void Executer::execute(TreeNode* node) {
  switch( node->getType() ) {
    case blockNode          : execBlock( node );        break;
    case forNode            : execFor( node );          break;
    case forEachNode        : execForEach( node );      break;
    case whileNode          : execWhile( node );        break;
    case ifNode             : execIf( node );           break;
    case assignNode         : execAssign( node );       break;
    case expressionNode     : execExpression( node );   break;
    case idNode             : execId( node );           break;
    case constantNode       : execConstant( node );     break; 
    case stringConstantNode : execConstant( node );     break; //for now handle same as ordinary constant
    
    case addNode            : execAdd( node );          break;
    case mulNode            : execMul( node );          break;
    case divNode            : execDiv( node );          break;
    case subNode            : execSub( node );          break;
    case minusNode          : execMinus( node );        break;
        
    case nodeGE             : execGE( node );           break; 
    case nodeGT             : execGT( node );           break;
    case nodeLE             : execLE( node );           break;
    case nodeLT             : execLT( node );           break;
    case nodeNE             : execNE( node );           break;
    case nodeEQ             : execEQ( node );           break;
    
    case andNode            : execAnd( node );          break;
    case orNode             : execOr( node );           break;
    case notNode            : execNot( node );          break;
    
    case functionNode       : createFunction( node );   break;
    case functionCallNode   : execFunction( node );     break;
    case funcReturnNode     : execRetFunction( node );  break;
    case returnNode         : execReturn( node );       break;
    case breakNode          : execBreak( node );        break;
    
    case runNode            : execRun( node );          break;
    
    case ClearNode          : execClear( node );        break;
    case GoNode             : execGo( node );           break;
    case GoXNode            : execGoX( node );          break;
    case GoYNode            : execGoY( node );          break;
    case ForwardNode        : execForward( node );      break;
    case BackwardNode       : execBackward( node );     break;
    case DirectionNode      : execDirection( node );    break;
    case TurnLeftNode       : execTurnLeft( node );     break;
    case TurnRightNode      : execTurnRight( node );    break;
    case CenterNode         : execCenter( node );       break;
    case SetPenWidthNode    : execSetPenWidth( node );  break;
    case PenUpNode          : execPenUp( node );        break;
    case PenDownNode        : execPenDown( node );      break;
    case SetFgColorNode     : execSetFgColor( node );   break;
    case SetBgColorNode     : execSetBgColor( node );   break;
    case ResizeCanvasNode   : execResizeCanvas( node ); break;
    case SpriteShowNode     : execSpriteShow( node );   break;
    case SpriteHideNode     : execSpriteHide( node );   break;
    case SpritePressNode    : execSpritePress( node );  break;
    case SpriteChangeNode   : execSpriteChange( node ); break;

    case MessageNode        : execMessage( node );        break;
    case InputWindowNode    : execInputWindow( node );  break;
    case printNode          : execPrint( node );        break;
    case FontTypeNode       : execFontType( node );     break;
    case FontSizeNode       : execFontSize( node );     break;
    case RepeatNode         : execRepeat( node );       break;
    case RandomNode         : execRandom( node );       break;
    case WaitNode           : execWait( node );         break;
    case WrapOnNode         : execWrapOn( node );       break;
    case WrapOffNode        : execWrapOff( node );      break;
    case ResetNode          : execReset( node );        break;
    
    default:
        QString nodename = node->getName();
        emit ErrorMsg( i18n("Found unsupported node named '%1' in the tree.").arg(nodename), 0, 0, 3000 );
    break;
  }  
}


void Executer::createFunction( TreeNode* node ) {
    QString funcname = node->firstChild()->getName();
    functionTable[funcname] = node; //store for later use
}


//execute a function
//first child   = function name
//second child  = parameters
void Executer::execFunction( TreeNode* node ) {
  QString funcname = node->firstChild()->getName();

  //locate function node  
  functable::iterator p = functionTable.find( funcname );
  if ( p == functionTable.end() ) {
    QString f = funcname;
    emit ErrorMsg( i18n("Call to undefined function: %1.").arg(f), 0, 0, 5010);
    return;
  }
  
  TreeNode* funcnode    = p->second;
  TreeNode* funcIds     = funcnode->secondChild();
  TreeNode* callparams  = node->secondChild();
    
  //check if number of parameters match
  if ( callparams->size() != funcIds->size() ) {
    QString f = funcname;
    emit ErrorMsg( i18n("Call to function '%1' with wrong number of parameters.").arg(f), 0, 0, 5020);
    return;
  }

  //pass parameters to function
  //by adding them to it's symboltable and setting the values
  TreeNode::iterator pfrom,pto = funcIds->begin();
  symtable funcSymTable;
  
  for (pfrom = callparams->begin(); pfrom != callparams->end(); ++pfrom ){
    if (bAbort) { return; }
    kapp->processEvents();
    
    //execute the parameters which can be expressions
    execute( *pfrom ); 
    
    QString idname=(*pto)->getName();
    funcSymTable[idname]= (*pfrom)->getValue();
    ++pto;
  
  }
  
  symbolTables.push(funcSymTable); //use new symboltable for current function
  
  //execute function statement block
  bReturn = false; //set to true when return is called
  execute( funcnode->thirdChild() );
  bReturn = false; //function execution done
  
  symbolTables.pop(); //release function symboltable    
}


//execute a function and expect and get return 
//value from stack
//first child   = function name
//second child  = parameters
void Executer::execRetFunction( TreeNode* node ) {
  execFunction( node );
  if( runStack.size() == 0 ) {
    emit ErrorMsg( i18n("Function %1 did not return a value.").arg( node->getKey() ), 0, 0, 5030);
    return;
  }
  node->setValue( runStack.top() ); //set return val
  runStack.pop(); //remove from stack
}


void Executer::execReturn( TreeNode* node ) {
  execute( node->firstChild() ); //execute return expression
  runStack.push( node->firstChild()->getValue() );
  bReturn = true; //notify blocks of return
}


void Executer::execBreak( TreeNode* node ) {
  bBreak = true; //stops loop block execution
}


void Executer::execBlock( TreeNode* node ) {
  //execute all statements in block
  TreeNode::iterator i;
  for( i = node->begin(); i != node->end(); ++i ){
    if (bAbort) { return; }
    kapp->processEvents();
    
    execute( *i );

    if( bReturn || bBreak){
      break; //jump out of block
    }
    
  }
}


void Executer::execForEach( TreeNode* node ) {
  //cout<<"sorry dude not implemented yet"<<endl;
  TreeNode* expr1      = node->firstChild();
  TreeNode* expr2      = node->secondChild();
  TreeNode* statements = node->thirdChild();
  
  execute( expr1 );
  execute( expr2 );
  
  QString expStr1 = expr1->getValue().strVal;
  QString expStr2 = expr2->getValue().strVal;
  
  bBreak = false;
  
  int i = expStr2.contains(expStr1, false);
  for( ; i > 0; i-- ) {
    if (bAbort) { return; }
    kapp->processEvents();
    
    execute( statements );
    if( bBreak || bReturn ) break; //jump out loop
  }
  bBreak = false;
}



void Executer::execFor( TreeNode* node ) {
  TreeNode* id = node->firstChild();
  TreeNode* startNode = node->secondChild();
  TreeNode* stopNode = node->thirdChild();
  TreeNode* statements = node->fourthChild();
  
  QString name = id->getName();

  execute(startNode);
  //assign startval to id
  Number startVal=startNode->getValue();
  ( symbolTables.top() )[ name ] = startVal;
  
  
  execute(stopNode);
  Number stopVal=stopNode->getValue();
  
  if(node->size() == 4 ){ //for loop without step part
    bBreak=false;
    for( double d = startVal.val; d <= stopVal.val; d = d + 1 ) {
      if (bAbort) { return; }
      kapp->processEvents();
      (symbolTables.top() )[name] = d;
      execute( statements );
      if( bBreak || bReturn ) break; //jump out loop
    }
    bBreak = false;
  } else { //for loop with step part
    TreeNode* step  = node->fourthChild();
    statements = node->fifthChild();
    
    execute(step);
    Number stepVal = step->getValue();
    bBreak=false;
    if( (stepVal.val >= 0.0) && (startVal.val <= stopVal.val) ) {
      for( double d = startVal.val; d <= stopVal.val; d = d + stepVal.val ) {
        if (bAbort) { return; }
        kapp->processEvents();
        
        (symbolTables.top() )[name] = d;
        execute( statements );
        if( bBreak || bReturn ) break; //jump out loop
      }
    } else if( (stepVal.val < 0.0) && (startVal.val >= stopVal.val) ) {
      for( double d = startVal.val; d >= stopVal.val; d = d + stepVal.val ) {
        if (bAbort) { return; }
        kapp->processEvents();
        
        (symbolTables.top() )[name] = d;
        execute( statements );
        if( bBreak || bReturn ) break; //jump out loop
      }    
    }
    bBreak = false;
  }

}



void Executer::execWhile( TreeNode* node ) {
  TreeNode* condition = node->firstChild();
  TreeNode* statements = node->secondChild();

  bBreak = false;
  execute( condition );
  while( condition->getValue().val != 0 ) {
    if (bAbort) { return; }
    kapp->processEvents();
    
    execute( statements );
    //if( bBreak || bReturn ) break; //jump out loop
    execute( condition );
  }
  bBreak = false;
}

     
void Executer::execIf( TreeNode* node ) {
  TreeNode* condition = node->firstChild();
  TreeNode* ifblok = node->secondChild();

  //determine if there is an else part
  if( node->size() == 2 ){ //no else
    execute( condition );
    if( condition->getValue().val != 0 ){
      execute( ifblok );
    }   
  } else { //else part given
    TreeNode* elseblok = node->thirdChild();
    execute( condition );
    if( condition->getValue().val != 0 ){
      execute( ifblok );
    } else {
      execute( elseblok );
    }
  }
}


   
void Executer::execAssign( TreeNode* node ){
  TreeNode* var  = node->firstChild();
  TreeNode* expr = node->secondChild();

  execute( expr );
  ( symbolTables.top() )[ var->getName() ] = expr->getValue();
}

    
void Executer::execExpression( TreeNode* node ){
  cerr<<"execExpression is not implemented, because it should not be needed!"<<endl;
  // cies calls it debug info
}


void Executer::execId( TreeNode* node ){
  node->setValue( ( symbolTables.top() )[ node->getName() ] );
}

void Executer::execConstant( TreeNode* node ){
  //do nothing, value is already set
}

Number Executer::getVal( TreeNode* node ){
  execute( node );
  return node->getValue();
}

        
void Executer::execAdd( TreeNode* node ) {
  node->setValue( getVal( node->firstChild() )
                  + 
                  getVal( node->secondChild() ) );
}

       
void Executer::execMul( TreeNode* node ) {
  if( node->firstChild()->getType() == stringConstantNode || node->secondChild()->getType() == stringConstantNode ) {
    emit ErrorMsg( i18n("Cannot multiply strings."), 0, 0, 9000);
    return;
  }
  
  node->setValue( getVal( node->firstChild() )
                  * 
                  getVal( node->secondChild() ) );
}

       
void Executer::execDiv( TreeNode* node ) {
  if( node->firstChild()->getType() == stringConstantNode || node->secondChild()->getType() == stringConstantNode ) {
    emit ErrorMsg( i18n("Cannot divide strings."), 0, 0, 9000);
    return;
  }
  node->setValue( getVal( node->firstChild() )
                  / 
                  getVal( node->secondChild() ) );
}

       
void Executer::execSub( TreeNode* node ) {
  if( node->firstChild()->getType() == stringConstantNode || node->secondChild()->getType() == stringConstantNode ) {
    emit ErrorMsg( i18n("Cannot substract strings."), 0, 0, 9000);
    return;
  }
  node->setValue( getVal( node->firstChild() )
                  - 
                  getVal( node->secondChild() ) );
}

       
void Executer::execLT( TreeNode* node ) {
  node->setValue( (double) (
                  getVal( node->firstChild() )
                  < 
                  getVal( node->secondChild() ) 
                  ) 
                );
}

void Executer::execLE( TreeNode* node ){
  node->setValue( (double) (
                  getVal( node->firstChild() )
                  <= 
                  getVal( node->secondChild() ) 
                  ) 
                );
}

void Executer::execGT( TreeNode* node ){
  node->setValue( (double) (
                  getVal( node->firstChild() )
                  >
                  getVal( node->secondChild() ) 
                  ) 
                );
}

void Executer::execGE( TreeNode* node ){
  node->setValue( (double) (
                  getVal( node->firstChild() )
                  >=
                  getVal( node->secondChild() ) 
                  ) 
                );
}


void Executer::execEQ( TreeNode* node ){
  node->setValue( (double) (
                  getVal( node->firstChild() )
                  ==
                  getVal( node->secondChild() ) 
                  ) 
                );
}


void Executer::execNE( TreeNode* node ){
  node->setValue( (double) (
                  getVal( node->firstChild() )
                  !=
                  getVal( node->secondChild() ) 
                  ) 
                );
}


  
void Executer::execAnd( TreeNode* node ){
  bool nl = getVal( node->firstChild() ).val != 0;
  bool nr = getVal( node->secondChild() ).val != 0;
  node->setValue( (double) (nl && nr) );
}

       
void Executer::execOr( TreeNode* node ){
  bool nl = getVal( node->firstChild() ).val != 0;
  bool nr = getVal( node->secondChild() ).val != 0;
  node->setValue( (double) (nl || nr) );
}


void Executer::execNot( TreeNode* node ){
  node->setValue( 1 - getVal( node->firstChild() ).val ); 
}


void Executer::execMinus( TreeNode* node ){
  node->setValue( - getVal( node->firstChild() ).val ); 
}


QString Executer::runCommand( const QString& command ){
  FILE *pstream;
  
  if ( ( pstream = popen( command, "r" ) ) == NULL ) {
    return "";
  }
  
  QString Line;
  char buf[100];
  
  while( fgets(buf, sizeof(buf), pstream) !=NULL) {
    if (bAbort) { return ""; }
    kapp->processEvents();
    
    Line += buf;
  }
  pclose(pstream);

  return Line;
}


void Executer::execRun( TreeNode* node ) {
  QString cmd = getVal( node->firstChild() ).strVal;
  node->setValue( runCommand(cmd) );
}


void Executer::execClear( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 0 ) {
        emit ErrorMsg( i18n("The function %1 accepts no parameters.").arg( node->getKey() ), 0, 0, 5040);
        return;
    }
    emit Clear();
}

void Executer::execGo( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 2 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 5050);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    TreeNode* nodeY = node->secondChild();
    execute(nodeX); // executing
    execute(nodeY);
    int x = (int)(nodeX->getValue().val + 0.5); // converting & rounding to int
    int y = (int)(nodeY->getValue().val + 0.5);
    emit Go(x, y);
}

void Executer::execGoX( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 5060);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    execute(nodeX); // executing
    int x = (int)(nodeX->getValue().val + 0.5); // converting & rounding to int
    emit GoX(x);
}

void Executer::execGoY( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 5070);
        return;
    }
    TreeNode* nodeY = node->firstChild(); // getting
    execute(nodeY); // executing
    int y = (int)(nodeY->getValue().val + 0.5); // converting & rounding to int
    emit GoY(y);
}

void Executer::execForward( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
//        QString f = node->getKey();
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 5080);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    execute(nodeX); // executing
    int x = (int)(nodeX->getValue().val + 0.5); // converting & rounding to int
    emit Forward(x);
}

void Executer::execBackward( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 5090);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    execute(nodeX); // executing
    int x = (int)(nodeX->getValue().val + 0.5); // converting & rounding to int
    emit Backward(x);
}

void Executer::execDirection( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 6000);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    execute(nodeX); // executing
    double x = (nodeX->getValue().val); // converting to double
    emit Direction(x);  // why the FUCK doesnt this work??
}

void Executer::execTurnLeft( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 6010);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    execute(nodeX); // executing
    double x = (nodeX->getValue().val); // converting to double
    emit TurnLeft(x);
}

void Executer::execTurnRight( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 6020);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    execute(nodeX); // executing
    double x = (nodeX->getValue().val); // converting to double
    emit TurnRight(x);
}

void Executer::execCenter( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 0 ) {
        emit ErrorMsg( i18n("The function %1 accepts no parameters.").arg( node->getKey() ), 0, 0, 6030);
        return;
    }
    emit Center();
}

void Executer::execSetPenWidth( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 6040);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    execute(nodeX); // executing
    int x = (int)(nodeX->getValue().val + 0.5); // converting & rounding to int
    if( x < 1 ) {
        emit ErrorMsg( i18n("The parameter of %1 must be smaller than 1.").arg( node->getKey() ), 0, 0, 6050);
        return;    
    }
    emit SetPenWidth(x);
}

void Executer::execPenUp( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 0 ) {
        emit ErrorMsg( i18n("The function %1 accepts no parameters.").arg( node->getKey() ), 0, 0, 6060);
        return;
    }
    emit PenUp();
}

void Executer::execPenDown( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 0 ) {
        emit ErrorMsg( i18n("The function %1 accepts no parameters.").arg( node->getKey() ), 0, 0, 6070);
        return;
    }
    emit PenDown();
}

void Executer::execSetFgColor( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 3 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 6080);
        return;
    }
    TreeNode* nodeR = node->firstChild(); // getting
    TreeNode* nodeG = node->secondChild();
    TreeNode* nodeB = node->thirdChild();
    execute(nodeR); // executing
    execute(nodeG);
    execute(nodeB);
    int r = (int)(nodeR->getValue().val + 0.5); // converting & rounding to int
    int g = (int)(nodeG->getValue().val + 0.5);
    int b = (int)(nodeB->getValue().val + 0.5);
    if( ( r < 0 || g < 0 || b < 0 ) || ( r > 255 || g > 255 || b > 255 ) ) {
        emit ErrorMsg( i18n("The parameters of function %1 must be within range: 0 to 255.").arg( node->getKey() ), 0, 0, 6090);
        return;
    }
    emit SetFgColor(r, g, b);
}

void Executer::execSetBgColor( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 3 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 7000);
        return;
    }
    TreeNode* nodeR = node->firstChild(); // getting
    TreeNode* nodeG = node->secondChild();
    TreeNode* nodeB = node->thirdChild();
    execute(nodeR); // executing
    execute(nodeG);
    execute(nodeB);
    int r = (int)(nodeR->getValue().val + 0.5); // converting & rounding to int
    int g = (int)(nodeG->getValue().val + 0.5);
    int b = (int)(nodeB->getValue().val + 0.5);
    if( ( r < 0 || g < 0 || b < 0 ) || ( r > 255 || g > 255 || b > 255 ) ) {
        emit ErrorMsg( i18n("The parameters of function %1 must be within range: 0 to 255.").arg( node->getKey() ), 0, 0, 7010);
        return;    
    }
    emit SetBgColor(r, g, b);
}

void Executer::execResizeCanvas( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 2 ) {
        QString funcname = node->getName();
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 7020);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    TreeNode* nodeY = node->secondChild();
    execute(nodeX); // executing
    execute(nodeY);
    int x = (int)(nodeX->getValue().val + 0.5); // converting & rounding to int
    int y = (int)(nodeY->getValue().val + 0.5);
    if( ( x < 1 || y < 1 ) || ( x > 10000 || y > 10000 ) ) {
        emit ErrorMsg( i18n("The parameters of function %1 must be within range: 1 to 10000.").arg( node->getKey() ), 0, 0, 7030);
        return;    
    }
    emit ResizeCanvas(x, y);
}

void Executer::execSpriteShow( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 0 ) {
        emit ErrorMsg( i18n("The function %1 accepts no parameters.").arg( node->getKey() ), 0, 0, 7040);
        return;
    }
    emit SpriteShow();
}

void Executer::execSpriteHide( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 0 ) {
        emit ErrorMsg( i18n("The function %1 accepts no parameters.").arg( node->getKey() ), 0, 0, 7050);
        return;
    }
    emit SpriteHide();
}

void Executer::execSpritePress( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 0 ) {
        emit ErrorMsg( i18n("The function %1 accepts no parameters.").arg( node->getKey() ), 0, 0, 7060);
        return;
    }
    emit SpritePress();
}

void Executer::execSpriteChange( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 7070);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    execute(nodeX); // executing
    int x = (int)(nodeX->getValue().val); // converting & rounding to int
    emit SpriteChange(x);
}



void Executer::execMessage( TreeNode* node ){
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 7070);
        return;
    }
    emit MessageDialog( node->firstChild()->getValue().strVal );
}


void Executer::execInputWindow( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 7070);
        return;
    }
    QString value = node->firstChild()->getValue().strVal;
    emit InputDialog( value );
    if (value == "") {
        node->setValue(""); // this prevents a crash :)
    } else {
        node->setValue( value.latin1() );
    }
}

void Executer::execPrint( TreeNode* node ) {
  TreeNode::iterator i;
  QString str = "";
  for( i = node->begin(); i != node->end(); ++i ) {
    execute( *i ); //execute expression
    str = str + (*i)->getValue().strVal;
  }
  emit Print(str);
}

void Executer::execFontType( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 2 || node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 7070);
        return;
    }
    if( node->firstChild()->getType() == stringConstantNode ||
        node->secondChild()->getType() == stringConstantNode ) {
        emit ErrorMsg( i18n("The function %1 only accepts strings as parameters.").arg( node->getKey() ), 0, 0, 9000);
        return;
    }
    QString extra;
    if( node->size() == 2 ) {
        QString extra = node->secondChild()->getValue().strVal;
    }
    QString family = node->firstChild()->getValue().strVal;
    emit FontType(family, extra);
}

void Executer::execFontSize( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 5060);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    execute(nodeX); // executing
    int px = (int)(nodeX->getValue().val + 0.5); // converting & rounding to int
    if( ( px < 0 || px > 350 ) ) {
        emit ErrorMsg( i18n("The parameters of function %1 must be within range: 0 to 350.").arg( node->getKey() ), 0, 0, 5065);
        return;    
    }
    emit FontSize(px);
}

void Executer::execRepeat( TreeNode* node ) {
  TreeNode* value = node->firstChild();
  TreeNode* statements = node->secondChild();

  bBreak=false;
  execute( value );
  for( int i = (int)( value->getValue().val + 0.5 ); i > 0; i-- ) {
    if (bAbort) { return; }
    kapp->processEvents();
    
    execute( statements );
    if( bBreak || bReturn ) break; //jump out loop
  }
  bBreak=false;
}

void Executer::execRandom( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 2 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 5050);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    TreeNode* nodeY = node->secondChild();
    execute(nodeX); // executing
    execute(nodeY);
    float x = nodeX->getValue().val;
    float y = nodeY->getValue().val;
    
    float r = (float)( KApplication::random() ) / RAND_MAX;
    Number value;
    value = (double)( r * ( y - x ) ) + x;
    node->setValue(value);
    return;
} 


void Executer::execWait( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 1 ) {
        emit ErrorMsg( i18n("The function %1 was called with wrong number of parameters.").arg( node->getKey() ), 0, 0, 5090);
        return;
    }
    TreeNode* nodeX = node->firstChild(); // getting
    execute(nodeX); // executing
    float sec = nodeX->getValue().val;
    startWaiting(sec);
}

void Executer::startWaiting(float sec) {
    bStopWaiting = false;
    int msec = (int)( sec * 1000 ); // convert
    // call a timer that sets stopWaiting to true when it runs 
    QTimer::singleShot( msec, this, SLOT( slotStopWaiting() ) );
    while (bStopWaiting == false) {
        if (bAbort) { return; }
        kapp->processEvents();

        // only 10 times per second is enough... else the CPU gets 100% loaded ( not nice :)
        usleep(100000);
    }
}

void Executer::slotStopWaiting() {
    bStopWaiting = true;
}

void Executer::execWrapOn( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 0 ) {
        emit ErrorMsg( i18n("The function %1 accepts no parameters.").arg( node->getKey() ), 0, 0, 6060);
        return;
    }
    emit WrapOn();
}

void Executer::execWrapOff( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 0 ) {
        emit ErrorMsg( i18n("The function %1 accepts no parameters.").arg( node->getKey() ), 0, 0, 6060);
        return;
    }
    emit WrapOff();
}

void Executer::execReset( TreeNode* node ) {
    // check if number of parameters match, or else...
    if( node->size() != 0 ) {
        emit ErrorMsg( i18n("The function %1 accepts no parameters.").arg( node->getKey() ), 0, 0, 6060);
        return;
    }
    emit Reset();
}


#include "executer.moc"
