/*=============================================================================
author        :Walter Schreppers
filename      :treenode.h
description   :A node in an n-ary tree
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
 
#ifndef _TREENODE_H_
#define _TREENODE_H_

#include <list>

#include <qstring.h>

#include "number.h"


// this const is used in executer, treenode and errormsg when uint row/col information is N/A
const uint NA = 999999999;

using namespace std;  // i dont know if this is still needed

//BUGS: prevSibling and nextSibling sometimes segfault and are not optimal, in short, don't use em! :)


enum NodeType { 
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
	ResetNode
};


class TreeNode : public list<TreeNode*> {
	public:
	//constructors/destructor
	TreeNode();
	TreeNode(TreeNode*); //give parent

	// TreeNode(NodeType);
	TreeNode(NodeType,
	         uint row = NA, 
	         uint col = NA,
	         QString name = "<name not set>", 
	         QString key = "<key not set>");

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
	
	uint getRow() const                { return fRow; }
	uint getCol() const                { return fCol; }
	
	void setName(const QString& n)     { fName = n; }
	QString getName() const            { return fName; }

	void setKey(const QString& k)      { fKey = k; }
	QString getKey() const             { return fKey; }
	
	void setValue(const Number&n )     { value = n; }
	void setValue(double d)            { value = d; }
	void setValue(const QString& s)    { value = s; }
	Number getValue() const            { return value; }
	
	void setStrValue(const QString& s) { strValue = s; }
	QString getStrValue() const        { return strValue; }


	
	TreeNode::iterator lookup(); //gives location in parent list as iterator (used by prevSibling and nextSibling)
	bool hasChildren() { return size() != 0; }

	TreeNode& operator= (const TreeNode&);


	protected:
	TreeNode* parent;


	private:
	void destroy(TreeNode*);

	//private locals
	QString    fKey;
	uint       fRow; //for runtime error messages
	uint       fCol;
	QString    fName;
	NodeType   fType;
	Number     value;
	QString    strValue;
};

#endif // _TREENODE_H_



