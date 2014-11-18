/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the Free
	Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.
*/

#include "treenode.h"

#include <QDebug>


TreeNode::~TreeNode()
{
	if (childList != 0) {
		// this should work here:
		qDeleteAll(*childList);
		childList->clear();

		// but this is nicer (and probably less optimal):
		// while (!list.isEmpty()) delete list.takeFirst();
	}
	delete _value;
	delete _token;
}


void TreeNode::init(TreeNode* parent, Token* token)
{
	setParent(parent);
	setToken(token);
	childList = 0;
	currentChildIndex = -1;
	_value = 0;
}


void TreeNode::appendChild(TreeNode* newChild)
{
	if (childList == 0) childList = new ChildList();
	newChild->setParent(this);
	childList->append(newChild);
// // // 		QString out = QString("TreeNode::appendChild(): \"%5\" [%6] @ (%1,%2)-(%3,%4) to parent '%7'")
// // // 			.arg(newChild->token()->startRow())
// // // 			.arg(newChild->token()->startCol())
// // // 			.arg(newChild->token()->endRow())
// // // 			.arg(newChild->token()->endCol())
// // // 			.arg(newChild->token()->look())
// // // 			.arg(newChild->token()->type())
// // // 			.arg(_token->look());
// 	//qDebug() << "TreeNode::appendChild():" << newChild->token()->look() << " on line" << newChild->token()->startRow() << "to parent" << _token->look();
}



TreeNode* TreeNode::child(int i)
{
	if (childList == 0) return 0;
	if (0 <= i && i < childList->size()) return childList->at(i);
	return 0;
}


TreeNode* TreeNode::firstChild()
{
	if (childList == 0 || childList->isEmpty()) return 0;
	currentChildIndex = 0;
	return childList->first();
}

TreeNode* TreeNode::nextChild()
{
	if (childList == 0) return 0;
	currentChildIndex++;
	return child(currentChildIndex);
}


int TreeNode::findChildIndex(TreeNode* child)
{
	return childList->indexOf(child);
}


TreeNode* TreeNode::nextSibling()
{
	if (_parent == 0) return 0;
	return _parent->child(_parent->findChildIndex(this)+1);
}


QString TreeNode::toString()
{
	QString str = "";
	showTree(str);
	return str;
}

// recursively walk through tree and show node names with indentation
void TreeNode::showTree(QString& str, int indent)
{
	if (childList == 0) return;
	indent++;
	TreeNode* node;
	for (int i = 0; i < childList->size(); i++) {
		node = childList->at(i);
		node->show(str, indent);
		node->showTree(str, indent);
	}
}

void TreeNode::show(QString& str, int indent) const
{
	QString indentString = "";
	for (int i = 0; i < indent; i++) indentString += "> ";
	str += indentString + _token->look() + QString(" @ (%1, %2)-(%3, %4)\n")
		.arg(_token->startRow())
		.arg(_token->startCol())
		.arg(_token->endRow())
		.arg(_token->endCol());
}

