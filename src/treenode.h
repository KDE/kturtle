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

   
#ifndef _TREENODE_H_
#define _TREENODE_H_

#include <list>

#include <qstring.h>

#include "lexer.h"
#include "number.h"


// this const is used in executer, treenode and errormsg when uint row/col information is N/A
const uint NA = 999999999;

using namespace std;  // i dont know if this is still needed

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
	stringConstantNode,

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
	
	LineBreakNode

	
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
// 	nodeNumber,
// 	nodeString,
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



class TreeNode : public list<TreeNode*>
{
	public:
	//constructors/destructor
	TreeNode();
	TreeNode(TreeNode*); //give parent

	// TreeNode(NodeType);
	TreeNode(NodeType,
	         uint row = NA, 
	         uint col = NA,
	         QString name = "<name not set>", 
				QString key = "<key not set>");  // temporary

	TreeNode(token, NodeType = Unknown); // this should become the thing!
	TreeNode(NodeType) {}; // temporary!
	
	virtual ~TreeNode();

	//public members
	void init();
	void setParent(TreeNode* p);
	TreeNode* getParent() {return parent;}
	
	virtual void show(int indent = 0);
	void showTree(TreeNode* node, int indent = 0) const;
	
	void appendChild(TreeNode*);
	void appendSibling(TreeNode*); //works only if it has parent set!
	
	TreeNode* firstChild();
	TreeNode* secondChild();
	TreeNode* thirdChild();
	TreeNode* fourthChild();
	TreeNode* fifthChild();
	
	TreeNode* lastChild();    
	TreeNode* nextSibling();   
	TreeNode* prevSibling();
	
	
	
	void setType(NodeType t)           { fType = t; }
	NodeType getType() const           { return fType; }

	void setToken(token t)             { fTok = t; }
	token getToken() const             { return fTok; }
	
	uint getRow() const                { return fTok.start.row; }
	uint getCol() const                { return fTok.start.col; }
	
	void setName(const QString& n)     { fName = n; }
	QString getName() const            { return fName; }
	
	void setValue(const Number& n)     { fTok.value = n; }
	void setValue(double d)            { fTok.value = d; }
	void setValue(const QString& s)    { fTok.value = s; }
	Number getValue() const            { return fTok.value; }
	
	void setLook(const QString& s)     { fTok.look = s; }
	QString getLook() const            { return fTok.look; }


	
	TreeNode::iterator lookup(); //gives location in parent list as iterator (used by prevSibling and nextSibling)
	bool hasChildren() { return size() != 0; }

	TreeNode& operator= (const TreeNode&);


	private:
	void destroy(TreeNode*);

	//private locals
	NodeType   fType;
	token      fTok;
	QString    fName;
	
	protected:
	TreeNode  *parent;
};

#endif // _TREENODE_H_



