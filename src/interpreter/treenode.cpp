/*
    SPDX-FileCopyrightText: 2003-2006 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "treenode.h"

#include <QDebug>


TreeNode::~TreeNode()
{
	if (childList != nullptr) {
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
	childList = nullptr;
	currentChildIndex = -1;
	_value = nullptr;
}


void TreeNode::appendChild(TreeNode* newChild)
{
	if (childList == nullptr) childList = new ChildList();
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
	if (childList == nullptr) return nullptr;
	if (0 <= i && i < childList->size()) return childList->at(i);
	return nullptr;
}


TreeNode* TreeNode::firstChild()
{
	if (childList == nullptr || childList->isEmpty()) return nullptr;
	currentChildIndex = 0;
	return childList->first();
}

TreeNode* TreeNode::nextChild()
{
	if (childList == nullptr) return nullptr;
	currentChildIndex++;
	return child(currentChildIndex);
}


int TreeNode::findChildIndex(TreeNode* child)
{
	return childList->indexOf(child);
}


TreeNode* TreeNode::nextSibling()
{
	if (_parent == nullptr) return nullptr;
	return _parent->child(_parent->findChildIndex(this)+1);
}


QString TreeNode::toString()
{
	QString str = QLatin1String("");
	showTree(str);
	return str;
}

// recursively walk through tree and show node names with indentation
void TreeNode::showTree(QString& str, int indent)
{
	if (childList == nullptr) return;
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
	QString indentString = QLatin1String("");
	for (int i = 0; i < indent; i++) indentString += QLatin1String("> ");
	str += indentString + _token->look() + QStringLiteral(" @ (%1, %2)-(%3, %4)\n")
		.arg(_token->startRow())
		.arg(_token->startCol())
		.arg(_token->endRow())
		.arg(_token->endCol());
}

