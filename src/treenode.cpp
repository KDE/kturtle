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


#include <kdebug.h>

#include "treenode.h"


void TreeNode::init()
{
	clear();
	parent = NULL;
	fTok.value = 0;
	// fTok.string = "";
	fTok.look = "";
	fName = "<name not set>"; // generic/internal command name
	fTok.look = "<fTok.look not set>";
	fTok.start.row = 0;
	fTok.start.col = 0;
	fTok.end.row = 0;
	fTok.end.col = 0;
}

TreeNode::TreeNode()
{
	init();
	fType = Unknown;
}

// TreeNode::TreeNode(NodeType t) {   /// to be removed (also in .h)
// 	init();
// 	fType = t;
// }

TreeNode::TreeNode(token t, NodeType nodeType)
{
	init();
	fType = nodeType;
	fTok = t;
	fName = fTok.look;
// 	fName = name;
// 	fKey = key;
	kdDebug(0)<<">>> new TreeNode created;  look: '"<<fTok.look<<"', from position ("<<fTok.start.row<<", "<<fTok.start.col<<")."<<endl;
}

TreeNode::TreeNode(NodeType t, uint row, uint col, QString name, QString key)
{
	init();
	fType = t;
	fTok.start.row = row;
	fTok.start.col = col;
	fName = name;
	kdDebug(0)<<">>> new TreeNode created;  name: '"<<fName<<"', pos: ("<<row<<", "<<col<<")."<<endl;
}

TreeNode::TreeNode(TreeNode* p)
{
	clear();
	setParent(p);
	fTok.value = -1;
	fTok.look = "";
	fType = Unknown;
	fTok.start.row = 0;
	fTok.start.col = 0;
	fName = "<name not set>";
}


void TreeNode::setParent(TreeNode* p)
{
	parent = p;
}


TreeNode::~TreeNode() {
	destroy(this);
}


//recursively walk down tree and delete every node bottom up
void TreeNode::destroy(TreeNode* node)
{
	if( (node != NULL) && (node->size() > 0) ) {
		for( TreeNode::iterator i = node->begin(); i != node->end(); ++i ) {
			destroy(*i);
			//kdDebug()<<"deleting:"<<(*i)->getName()<<endl;
			(*i)->clear(); //free children
			//delete ( *i ); //free mem
		}
	}
}



TreeNode& TreeNode::operator= (const TreeNode& t)
{
	if (this != &t)
	{
		fType = t.fType;
		fTok = t.fTok;
		fName = t.fName;
		parent = t.parent;
		
		this->clear();
		for (TreeNode::const_iterator i = t.begin(); i != t.end(); ++i)
		{
			this->push_back(*i);
		}
	}
	return *this;
}
		



// recursively walk through tree and show node names with indentation
void TreeNode::showTree(TreeNode* node, int indent) const {
	indent++;
	if( (node != NULL) && (node->size() > 0) ) {
		for( TreeNode::const_iterator i = node->begin(); i != node->end(); ++i ) {
			(*i)->show(indent);
			showTree(*i, indent);
		}
	}
}

void TreeNode::show(int indent) {
	QString s = "";
	for (int i = 0; i < indent; i++) {
		s += ">  ";
	}
	kdDebug(0)<<"NodeTree:"<<s<<""<<getName()<<" @ ("<<getRow()<<", "<<getCol()<<")"<<endl;
}


void TreeNode::appendChild(TreeNode* node) {
	node->setParent(this);
	push_back(node);
}

void TreeNode::appendSibling(TreeNode* node) {
	node->parent = parent;
	if(parent != NULL) {
		parent->push_back(node);
	}
}

TreeNode::iterator TreeNode::lookup() {
	if(parent != NULL){
		TreeNode::iterator i = parent->begin();
		while( (*i != this) && (i != parent->end() ) ) {
			++i;
		}
		return i;
	}
	return end();
}



//returns the nextSibling
TreeNode* TreeNode::nextSibling() {
	if (parent) {
		TreeNode::iterator i = lookup();
		if ( i!=parent->end() ) {
			++i;
			// must check after i has been incremented
			// to make sure i isn't at the end before
			// returning the contained pointer value
			if ( i!=parent->end() ) {
				return *i;
			}
		}
	}
	return NULL;
}



TreeNode* TreeNode::prevSibling() {
	if (parent) {
		TreeNode::iterator i = lookup();
		// Must make sure we aren't at beginning as well
		// or we can crash when decrementing since we shouldn't
		// decrement before the start of the list
		if ( ( i!=parent->end() ) && ( i!=parent->begin() ) ) {
			--i;
			return *i;
		}
	}
	return NULL;
}




// returns first child of a node
TreeNode* TreeNode::firstChild() {
	// Must make sure we aren't empty first!!!
	if ( !empty() ) {
		TreeNode::const_iterator child = begin();
		return *child;
	}
	return NULL;
}


TreeNode* TreeNode::secondChild() {
	// Must make sure we aren't empty first!!!
	if ( !empty() ) {
		TreeNode::const_iterator child = begin();
		return (*child)->nextSibling();
	}
	return NULL;
}



TreeNode* TreeNode::thirdChild() {
	// Must make sure we aren't empty first!!!
	if ( !empty() ) {
		TreeNode* child = secondChild();
		if(child!=NULL) return child->nextSibling();
	}
	return NULL;
}

TreeNode* TreeNode::fourthChild() {
	// Must make sure we aren't empty first!!!
	if ( !empty() ) {
		TreeNode* child = thirdChild();
		if(child!=NULL) return child->nextSibling();
	}
	return NULL;
}

TreeNode* TreeNode::fifthChild() {
	// Must make sure we aren't empty first!!!
	if ( !empty() ) {
		TreeNode* child = fourthChild();
		if(child!=NULL) return child->nextSibling();
	}
	return NULL;
}



//returns last child of a node
TreeNode* TreeNode::lastChild() {
	// Must make sure we aren't empty first and use rbegin() and
	// a reverse iterator...
	if ( !empty() ) {
		TreeNode::const_reverse_iterator child= rbegin();
		return *child;
	}
	return NULL;
}
