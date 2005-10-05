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
// most aspects of it are slightly changed by Cies Breijs.


#include <kdebug.h>

#include "treenode.h"


TreeNode::TreeNode()
{
	init();
	fType = Unknown;
}

TreeNode::TreeNode(Token t, NodeType nodeType, QString name)
{
	init();
	fType = nodeType;
	fTok = t;
	if ( !name.isNull() ) fTok.look = name;
	kdDebug(0)<<"TreeNode::TreeNode(); new TreeNode created, with look: '"<<fTok.look<<"', from position ("<<fTok.start.row<<", "<<fTok.start.col<<")."<<endl;
}

TreeNode::TreeNode(TreeNode* p)
{
	init();
	setParent(p);
	fType = Unknown;
	fTok.value = -1;
}

TreeNode::~TreeNode()
{
	destroy(this);
}



void TreeNode::init()
{
	clear();
	parent = NULL;
	// fTok.look (QString) and fTok.value (Value) are properly init'ed when constructed
	fTok.start.row = 0;
	fTok.start.col = 0;
	fTok.end.row = 0;
	fTok.end.col = 0;
}




void TreeNode::appendChild(TreeNode* node)
{
	node->setParent(this);
	push_back(node);
}

void TreeNode::appendSibling(TreeNode* node)
{
	node->parent = parent;
	if (parent != NULL) parent->push_back(node);
}


TreeNode* TreeNode::nextSibling()
{
	if (parent)
	{
		TreeNode::iterator i = lookup();
		if ( i != parent->end() )
		{
			++i;
			// Must check after i has been incremented
			// to make sure i isn't at the end before
			// returning the contained pointer value.
			if ( i != parent->end() ) return *i;
		}
	}
	return NULL;
}

TreeNode* TreeNode::prevSibling()
{
	if (parent)
	{
		TreeNode::iterator i = lookup();
		// Must make sure we aren't at beginning as well
		// or we can crash when decrementing since we shouldn't
		// decrement before the start of the list.
		if ( ( i != parent->end() ) && ( i != parent->begin() ) )
		{
			--i;
			return *i;
		}
	}
	return NULL;
}



TreeNode* TreeNode::firstChild()
{
	if ( !empty() )
	{
		TreeNode::const_iterator child = begin();
		return *child;
	}
	return NULL;
}

TreeNode* TreeNode::secondChild()
{
	if ( !empty() )
	{
		TreeNode::const_iterator child = begin();
		return (*child)->nextSibling();
	}
	return NULL;
}

TreeNode* TreeNode::thirdChild()
{
	if ( !empty() )
	{
		TreeNode* child = secondChild();
		if (child != NULL) return child->nextSibling();
	}
	return NULL;
}

TreeNode* TreeNode::fourthChild()
{
	if ( !empty() )
	{
		TreeNode* child = thirdChild();
		if (child != NULL) return child->nextSibling();
	}
	return NULL;
}

TreeNode* TreeNode::fifthChild()
{
	if ( !empty() )
	{
		TreeNode* child = fourthChild();
		if(child != NULL) return child->nextSibling();
	}
	return NULL;
}

TreeNode* TreeNode::lastChild()
{
	// using a reverse iterator...
	if ( !empty() )
	{
		TreeNode::const_reverse_iterator child = rbegin();
		return *child;
	}
	return NULL;
}




TreeNode::iterator TreeNode::lookup()
{
	if (parent != NULL)
	{
		TreeNode::iterator i = parent->begin();
		while ( (*i != this) && (i != parent->end() ) ) ++i;
		return i;
	}
	return end();
}



TreeNode& TreeNode::operator= (const TreeNode& t)
{
	if (this != &t)
	{
		fType  = t.fType;
		fTok   = t.fTok;
		parent = t.parent;
		
		clear();
		for (TreeNode::const_iterator i = t.begin(); i != t.end(); ++i)
		{
			push_back(*i);
		}
	}
	return *this;
}




// recursively walk through tree and show node names with indentation
void TreeNode::showTree(TreeNode* node, int indent) const
{
	indent++;
	if ( (node != NULL) && (node->size() > 0) )
	{
		for ( TreeNode::const_iterator i = node->begin(); i != node->end(); ++i )
		{
			(*i)->show(indent);
			showTree(*i, indent);
		}
	}
}

void TreeNode::show(int indent)
{
	QString s = "";
	for (int i = 0; i < indent; i++)
	{
		s += ">  ";
	}
	kdDebug(0)<<s<<""<<fTok.look<<" @ ("<<getRow()<<", "<<getCol()<<")"<<endl;
}


//recursively walk down tree and delete every node bottom up
void TreeNode::destroy(TreeNode* node)
{
	if ( (node != NULL) && (node->size() > 0) )
	{
		for ( TreeNode::iterator i = node->begin(); i != node->end(); ++i )
		{
			destroy(*i);
			(*i)->clear(); //free children
			//delete ( *i ); //free mem
		}
	}
}
