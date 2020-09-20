/*
    SPDX-FileCopyrightText: 2003-2008 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _INSPECTOR_H_
#define _INSPECTOR_H_

#include <QFrame>
#include <QHash>

#include "editor.h"
#include "highlighter.h"
#include "interpreter/value.h"
#include "interpreter/treenode.h"

class QHBoxLayout;
class QTableWidget;
class QTableWidgetItem;
class QTabWidget;
class QTreeWidget;
class QTreeWidgetItem;


class Inspector : public QFrame
{
	Q_OBJECT

	public:
        explicit Inspector(QWidget *parent = nullptr);
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
