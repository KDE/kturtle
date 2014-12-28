/*
	Copyright (C) 2003-2008 Cies Breijs <cies AT kde DOT nl>

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

#ifndef _INSPECTOR_H_
#define _INSPECTOR_H_

#include <QHash>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>

#include "editor.h"
#include "highlighter.h"
#include "interpreter/value.h"
#include "interpreter/treenode.h"

class QTabWidget;

class Inspector : public QFrame
{
	Q_OBJECT

	public:
		Inspector(QWidget *parent = 0);
		~Inspector();

		void clear();


	public slots:
		void updateVariable(const QString& name, const Value& value);
		void updateFunction(const QString& name, const QStringList& parameters);
		void updateTree(TreeNode* rootNode);

		void markVariable(const QString&);
		void markFunction(const QString&);
		void markTreeNode(TreeNode*);

		void clearAllMarks();

		void disable();


	private:
		int findVariable(const QString& name);
		QTreeWidgetItem* walkTree(TreeNode* node);

		void clearTreeMark();

		Highlighter  *highlighter;

		// map the names of the variables/functions to their respective items in the tabelwidget
		QHash<QString, QTableWidgetItem*> variableMap;
		QHash<QString, QTableWidgetItem*> functionMap;
		// map the treenodes to their respective items in the treewidget
		QHash<TreeNode*, QTreeWidgetItem*> treeMap;

		QHBoxLayout  *mainLayout;
		QTabWidget   *tabWidget;

		QWidget      *variableTab;
		QHBoxLayout  *variableLayout;
		QTableWidget *variableTable;

		QWidget      *functionTab;
		QHBoxLayout  *functionLayout;
		QTableWidget *functionTable;

		QWidget      *treeTab;
		QHBoxLayout  *treeLayout;
		QTreeWidget  *treeView;

		QBrush previousTreeBackground;
		QTreeWidgetItem *currentlyMarkedTreeItem;

		bool         variableTableEmpty;
		bool         functionTableEmpty;
};


#endif  // _INSPECTOR_H_
