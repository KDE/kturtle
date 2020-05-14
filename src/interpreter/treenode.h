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

#ifndef _TREENODE_H_
#define _TREENODE_H_

#include <QList>


#include "token.h"
#include "value.h"



/**
 * @short A node in the pointer based node tree structure.
 *
 * The TreeNode object is a node in the node tree as created by the Parser.
 * It can be executed by the Executer.
 *
 * The list of pointers to the children of the TreeNode is only created
 * when it is needed.
 * Each TreeNode has a list of pointers to its children, a pointer to its
 * parent, a pointer to one Token, and can also have a pointer to a Value.
 *
 * @author Cies Breijs
 */
class TreeNode
{
	public:
		/**
		 * @short Constructor.
		 * Initialses the TreeNode.
		 * @param token pointer to Token that the TreeNode is associated with.
		 */
		explicit TreeNode(Token* token)                  { init(0, token); }

		/**
		 * @short Destructor.
		 * This deletes the Value and the Token associated with this TreeNode.
		 * The childList auto-deletes all the children, which means that deleting
		 * the root node deletes the whole tree.
		 */
		virtual ~TreeNode();



		/** @returns the pointer to the parent TreeNode. @see setParent() */
		TreeNode* parent()                      { return _parent; }

		/** @returns the pointer to associated Token. @see setToken() */
		Token*    token()                       { return _token; }

		/** @returns the pointer to associated Value. @see setValue() @see setNullValue() */
		Value*    value()                       { if (_value == 0) _value = new Value(); return _value; }



		/** Sets the pointer to the parent to @p parent. @see parent() */
		void      setParent(TreeNode* parent)   { _parent = parent; }

		/** Sets the pointer to the associated token to @p token. @see token() and @see TreeNode() */
		void      setToken(Token* token)        { _token = token; }

		/** Sets the pointer to the associated value to @p value. @see setNullValue() @see value() */
		void      setValue(Value value)         { delete _value; _value = new Value(value); }

		/** Sets the pointer to the associated value to zero. @see setValue() @see value() */
		void      setNullValue()                {  delete _value; _value = 0;  } // appears Empty (see value())



		/** @returns TRUE is the TreeNode has an associated Value. @see value @see setValue */
		bool      hasValue() const              { return _value != 0; }

		/** @returns TRUE is the TreeNode has children. @see childCount @see appendChild */
		bool      hasChildren() const           { if (childList == 0) return false; else return !childList->isEmpty(); }

		/** @returns the amount of children. @see appendChild @see hasChildren */
		uint      childCount() const            { if (childList == 0) return 0; else return childList->size(); }

		/**
		 * Appends the pointer to the TreeNode @p newChild to the childList and
		 * sets the child's parent to this. @see childCount @see hasChildren
		 */
		void      appendChild(TreeNode* newChild);



		/** @returns the pointer to child number @p i (zero if the child does not exists). This does not change the current child. */
		TreeNode* child(int i);

		/** @returns the pointer to the first child (zero if the child does not exists), and set the current child to the first */
		TreeNode* firstChild();

		/** @returns the pointer to the next child (zero if the child does not exists), and set the current child to the next */
		TreeNode* nextChild();


		/** @returns the pointer to the next sibling; the next child of the parent (zero if the next sibling does not exists) */
		TreeNode* nextSibling();


		/** @returns the node tree, starting from 'this' node, as a multi line string */
		QString toString();



	private:
		/// Initializes a the TreeNode. Called by the constructor.
		void init(TreeNode* parent, Token* token);

		/** Calls itself, @ref show(), and @ref showTree() for each of the children */
		virtual void show(QString& str, int indent = 0) const;

		/** Prints an indented string describing itself to de debug info */
		void showTree(QString& str, int indent = 0);

		int findChildIndex(TreeNode* child);
	
		/// typedef for the ChildList.
		typedef QList<TreeNode*>         ChildList;

		/// The childList, contains pointers to the children of this node.
		ChildList                       *childList;

		/// Keeps track of the index of the current child. Zero when no child list is available.
		int                              currentChildIndex;



		/// The pointer to the parent of this TreeNode.
		TreeNode                        *_parent;

		/// The pointer to the token associated with this TreeNode (cannot be zero).
		Token                           *_token;

		/// The pointer to the value associated with this TreeNode (can be zero).
		Value                           *_value;
};

#endif  // _TREENODE_H_
