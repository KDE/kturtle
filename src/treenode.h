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
// every aspect of it is slightly changed by Cies Breijs.

   
#ifndef _TREENODE_H_
#define _TREENODE_H_

#include <list>

#include <qstring.h>

#include "lexer.h"
#include "token.h"
#include "value.h"


using namespace std;  // needed because treenode inherites from std::list

//BUGS: prevSibling and nextSibling sometimes segfault and are not optimal, in short, don't use em! :)

enum NodeType
{
	Unknown = -1,
	programNode,
	functionNode,
	functionCallNode,
	funcReturnNode,
	returnNode,
	breakNode,
	idListNode,
	blockNode,
	forNode,
	forEachNode,
	whileNode,
	ifNode,
	assignNode,
	expressionNode,
	idNode,
	constantNode,

	addNode,
	mulNode,
	divNode,
	subNode,
	minusNode,
	
	nodeGE,
	nodeGT,
	nodeLE,
	nodeLT,
	nodeNE,
	nodeEQ,

	andNode,
	orNode,
	notNode,

	runNode,

	ClearNode,
	GoNode,
	GoXNode,
	GoYNode,
	ForwardNode,
	BackwardNode,
	DirectionNode,
	TurnLeftNode,
	TurnRightNode,
	CenterNode,
	SetPenWidthNode,
	PenUpNode,
	PenDownNode,
	SetFgColorNode,
	SetBgColorNode,
	ResizeCanvasNode,
	SpriteShowNode,
	SpriteHideNode,
	SpritePressNode,
	SpriteChangeNode,

	MessageNode,
	InputWindowNode,
	printNode,
	FontTypeNode,
	FontSizeNode,
	RepeatNode,
	RandomNode,
	WaitNode,
	WrapOnNode,
	WrapOffNode,
	ResetNode,
	
	LineBreakNode,
	EndOfFileNode

	
//              HOPEFULLY THIS LIST CAN ONCE LOOK LIKE THIS:
// 	nodeProgram, // this frirst block has no corresponding token
// 	nodeFunction,
// 	nodeFunctionCall,
// 	nodeReturnFunction,
// 	nodeParameterList,
// 	nodeBlock,
// 	nodeExpression,
// 	nodeId, // for assignments
// 
// 	nodeNotSet = -1, // inittial type of all tokens
// 	nodeUnknown = 0, // when no token was found, a tokUnknown is often a variable, function or error
// 	
// 	nodeIf,
// 	nodeElse,
// 	nodeWhile,
// 	nodeFor,
// 	nodeTo,
// 	nodeStep,
// 	nodeForEach,
// 	nodeIn,
// 	nodeBreak,
// 	nodeReturn,
// 
// 	nodeBegin,
// 	nodeEnd,
// 
// // 	nodeNumber,  --> constantNode
// // 	nodeString,
// 
// 	nodeAssign,
// 	
// 	nodeAnd,
// 	nodeOr,
// 	nodeNot,
// 	
// 	nodeEq,
// 	nodeNe,
// 	nodeGe,
// 	nodeGt,
// 	nodeLe,
// 	nodeLt,
// 	
// 	// nodeBraceOpen,
// 	// nodeBraceClose,
// 	// nodePlus,
// 	nodeAdd, // different from TokenType
// 	nodeSub, // different from TokenType
// 	nodeMul,
// 	nodeDev,
// 	nodeMinus, // different meaning from TokenType
// 	
// 	// nodeComma,
// 	// nodeEOL,
// 	// nodeEOF,
// 	// nodeError,
// 
// 	nodeLearn,
// 	
// 	nodeClear,
// 	nodeGo,
// 	nodeGoX,
// 	nodeGoY,
// 	nodeForward,
// 	nodeBackward,
// 	nodeDirection,
// 	nodeTurnLeft,
// 	nodeTurnRight,
// 	nodeCenter,
// 	nodeSetPenWidth,
// 	nodePenUp,
// 	nodePenDown,
// 	nodeSetFgColor,
// 	nodeSetBgColor,
// 	nodeResizeCanvas,
// 	nodeSpriteShow,
// 	nodeSpriteHide,
// 	nodeSpritePress,
// 	nodeSpriteChange,
// 	nodeMessage,
// 	nodeInputWindow,
// 	nodePrint,
// 	nodeFontType,
// 	nodeFontSize,
// 	nodeRepeat,
// 	nodeRandom,
// 	nodeWait,
// 	nodeWrapOn,
// 	nodeWrapOff,
// 	nodeReset,
// 	nodeRun,
// 
// 	// nodeDo
};



class TreeNode : public list<TreeNode*> /*, public Value  <-- maybe oneday */
{
	public:
		TreeNode(); // used for creation of the first node called 'tree', in the contructor of the parser 
		TreeNode( TreeNode* ); // give parent
		TreeNode( Token, NodeType = Unknown, QString = QString() );
		virtual ~TreeNode();
	
		void init();
	
		void appendChild(TreeNode*);
		void appendSibling(TreeNode*); // works only if it has parent set!
	
		TreeNode* firstChild();
		TreeNode* secondChild();
		TreeNode* thirdChild();
		TreeNode* fourthChild();
		TreeNode* fifthChild();
	
		TreeNode* lastChild();    
		TreeNode* nextSibling();   
		TreeNode* prevSibling();
	
		void      setParent(TreeNode* p)     { parent = p; }
		TreeNode* getParent() const          { return parent; }
		
		void      setToken(Token t)          { fTok = t; }
		Token&    getToken()                 { return fTok; }
		
		void      setType(NodeType t)        { fType = t; }
		NodeType  getType() const            { return fType; }
		
		void      setLook(const QString& s)  { fTok.look = s; }
		QString   getLook() const            { return fTok.look; }
		
		void      setValue(const Value& n)   { fTok.value = n; }
		void      setValue(double d)         { fTok.value = d; }
		void      setValue(const QString& s) { fTok.value = s; }
		void      setValue(bool b)           { fTok.value.setBool(b); }
		Value     getValue() const           { return fTok.value; }
		
		uint      getRow() const             { return fTok.start.row; }
		uint      getCol() const             { return fTok.start.col; }
	
		bool hasChildren() const             { return size() != 0; }
		TreeNode::iterator lookup(); // gives location in parent list as iterator (used by prevSibling and nextSibling)
	
		TreeNode& operator=(const TreeNode&);
	
		virtual void show(int indent = 0);
		void showTree(TreeNode* node, int indent = 0) const;
	
	
	private:
		void destroy(TreeNode*);
	
		NodeType     fType;
		Token        fTok;
	
	
	protected:
		TreeNode    *parent;
};

#endif // _TREENODE_H_
