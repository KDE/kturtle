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

#include "inspector.h"

#include "interpreter/translator.h"  // for getting the translated ArgumentSeparator

#include <QHeaderView>
#include <QTextCharFormat>

#include <kdebug.h>
#include <klocale.h>
#include <KTabWidget>


Inspector::Inspector(QWidget *parent)
	: QFrame(parent)
{
	mainLayout = new QHBoxLayout(this);
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	tabWidget  = new KTabWidget(this);

	variableTab    = new QWidget();
	variableLayout = new QHBoxLayout(variableTab);
	variableTable  = new QTableWidget(variableTab);
	variableTable->setAlternatingRowColors(true);
	variableLayout->addWidget(variableTable);
	tabWidget->addTab(variableTab, i18n("Variables"));

	functionTab    = new QWidget();
	functionLayout = new QHBoxLayout(functionTab);
	functionTable  = new QTableWidget(functionTab);
	functionTable->setAlternatingRowColors(true);
	functionLayout->addWidget(functionTable);
	tabWidget->addTab(functionTab, i18n("Functions"));

	treeTab    = new QWidget();
	treeLayout = new QHBoxLayout(treeTab);
	treeView   = new QTreeWidget(treeTab);
	treeView->header()->setVisible(false);
	treeLayout->addWidget(treeView);
	tabWidget->addTab(treeTab, i18n("Tree"));

	mainLayout->addWidget(tabWidget);

	// our syntax highlighter (this does not do any markings)
	highlighter = new Highlighter();

// 	// the maps used when marking table/tree items later
// 	variableMap = new QHash<QString, QTableWidgetItem*>();
// 	functionMap = new QHash<QString, QTableWidgetItem*>();
// 	treeMap = new QHash<TreeNode*, QTableWidgetItem*>();

	currentlyMarkedTreeItem = 0;

	disable();

	clear();
}

Inspector::~Inspector()
{
	delete highlighter;
}


void Inspector::disable()
{
	variableTable->setEnabled(false);
	functionTable->setEnabled(false);
	treeView->setEnabled(false);
	// enabling happend when the inspector gets filled with data
}

void Inspector::clear()
{
	clearAllMarks();

	// Question: is the code duplication below enough
	// for a subclass-of-QTableWidget based approach?

	variableMap.clear();
	variableTableEmpty = true;
	variableTable->clear();
	QStringList headers;
	headers << i18n("name") << i18n("value") << i18n("type");
	variableTable->setColumnCount(3);
	variableTable->setHorizontalHeaderLabels(headers);
	variableTable->setSelectionMode(QAbstractItemView::SingleSelection);
	variableTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	variableTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	variableTable->verticalHeader()->setVisible(false);
	variableTable->setShowGrid(false);
	variableTable->setRowCount(1);
	QTableWidgetItem* emptyItem;
	emptyItem = new QTableWidgetItem(i18n("No variables"));
	variableTable->setItem(0, 0, emptyItem);
	variableTable->resizeColumnsToContents();

	functionMap.clear();
	functionTableEmpty = true;
	functionTable->clear();
	headers.clear();
	headers << i18n("name") << i18n("parameters");
	functionTable->setColumnCount(2);
	functionTable->setHorizontalHeaderLabels(headers);
	functionTable->setSelectionMode(QAbstractItemView::SingleSelection);
	functionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	functionTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	functionTable->verticalHeader()->setVisible(false);
	functionTable->setShowGrid(false);
	functionTable->setRowCount(1);
	emptyItem = new QTableWidgetItem(i18n("No learned functions"));
	functionTable->setItem(0, 0, emptyItem);
	functionTable->resizeColumnsToContents();

	// Treeview gets cleared in updateTree()
	disable();
}


void Inspector::updateVariable(const QString& name, const Value& value)
{
	// Check if the variable has already been added to the table
	int row = findVariable(name);
	if (row == -1) {
		// We are dealing with a new variable
		if (variableTableEmpty) {  // Check whether we have to add a new row
			variableTableEmpty = false;
		} else {
			variableTable->insertRow(0);
		}
		row = 0;
	}

	QTableWidgetItem* nameItem;
	nameItem = new QTableWidgetItem(name);
	nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	QTextCharFormat* format = highlighter->formatForStatement(name);
	nameItem->setFont(format->font());
	nameItem->setForeground(format->foreground());
	variableTable->setItem(row, 0, nameItem);
	variableMap[name] = nameItem;

	QTableWidgetItem* valueItem;
	valueItem = new QTableWidgetItem(value.string());
	valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
// TODO varialble coloring when tokens are available in the inspector!
// 	format = highlighter->formatForStatement(value.string());
// 	valueItem->setFont(format->font());
// 	valueItem->setForeground(format->foreground());

	QTableWidgetItem* typeItem;
	switch (value.type()) {
		case Value::Empty: {
                        typeItem  = new QTableWidgetItem(i18nc("undefined type of a variable","empty"));
			QFont font = typeItem->font();
			font.setItalic(true);
			typeItem->setFont(font);
			}
			break;
		case Value::Bool:
			typeItem  = new QTableWidgetItem(i18n("boolean"));
			break;
		case Value::Number:
			typeItem  = new QTableWidgetItem(i18n("number"));
			break;
		case Value::String:
			typeItem  = new QTableWidgetItem(i18n("string"));
			break;
		default:
			// should never happen
			typeItem  = new QTableWidgetItem("ERROR! please report to KTurtle developers");
			break;
	}
	typeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	variableTable->setItem(row, 1, valueItem);
	variableTable->setItem(row, 2, typeItem);

	variableTable->sortItems(0);
	variableTable->resizeColumnsToContents();
	variableTable->setEnabled(true);
}

void Inspector::updateFunction(const QString& name, const QStringList& parameters)
{
	// When there is already a the 'Nothing to show' line re-use that one and don't add another
	if (functionTableEmpty) {
		functionTableEmpty = false;
	} else {
		functionTable->insertRow(0);
	}

	QTableWidgetItem* nameItem = new QTableWidgetItem(name);
	nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	functionTable->setItem(0, 0, nameItem);
	functionMap[name] = nameItem;

	QTableWidgetItem* paramItem;
	if (parameters.empty()) {
		paramItem = new QTableWidgetItem(i18n("None"));
		QFont font = paramItem->font();
		font.setItalic(true);
		paramItem->setFont(font);
	} else {
		QString paramList = parameters.join(Translator::instance()->default2localized(QString(",")));
		paramItem = new QTableWidgetItem(paramList);
	}
	
	paramItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	functionTable->setItem(0, 1, paramItem);
	functionTable->sortItems(0);
	functionTable->resizeColumnsToContents();
	functionTable->setEnabled(true);
}

void Inspector::updateTree(TreeNode* rootNode)
{
	treeMap.clear();
	treeView->clear();
	QTreeWidgetItem* rootItem = walkTree(rootNode);
	foreach (QTreeWidgetItem* item, rootItem->takeChildren()) {
		treeView->addTopLevelItem(item);
	}
	delete rootItem;
	treeView->expandAll();
	treeView->setEnabled(true);
}

QTreeWidgetItem* Inspector::walkTree(TreeNode* node)
{
	QTreeWidgetItem* result = new QTreeWidgetItem();
	result->setText(0, node->token()->look());
	QTextCharFormat* format = highlighter->tokenToFormat(node->token());
	if (format != 0) {
		result->setForeground(0, format->foreground());
		QFont font(KGlobalSettings::fixedFont());
		font.setBold(format->font().bold());
		result->setFont(0, font);
	}
	treeMap[node] = result;
	if (node->hasChildren()) {
		for (uint i = 0; i < node->childCount(); i++) {
			result->addChild(walkTree(node->child(i)));
		}
	}
	return result;
}

int Inspector::findVariable(const QString& name)
{
	QTableWidgetItem* item = variableMap[name];
	if (item == 0) return -1;
	return item->row();

// old implementation before we had a variableMap

// 	// This function will search for a specified variable and return the row number of this variable.
// 	QList<QTableWidgetItem*> matches = variableTable->findItems(name, Qt::MatchExactly);
// 	QList<QTableWidgetItem*>::iterator i;
// 	for (i = matches.begin(); i != matches.end(); ++i) {
// 		if ((*i)->column() == 0) // only check the first column
// 			return (*i)->row();
// 	}
// 	return -1;
}

void Inspector::markVariable(const QString& name)
{
	kDebug() << variableMap[name]->row();
}

void Inspector::markFunction(const QString& name)
{
	kDebug() << functionMap[name]->row();
}

void Inspector::markTreeNode(TreeNode* node)
{
// 	kDebug() << treeMap[node]->text(0);
	clearTreeMark();
	currentlyMarkedTreeItem = treeMap[node];
	previousTreeBackground = currentlyMarkedTreeItem->background(0);
	currentlyMarkedTreeItem->setBackground(0, QBrush(WORD_HIGHLIGHT_COLOR));
}

void Inspector::clearTreeMark()
{
	if (currentlyMarkedTreeItem == 0) return;
	currentlyMarkedTreeItem->setBackground(0, previousTreeBackground);
	currentlyMarkedTreeItem = 0;
}

void Inspector::clearAllMarks()
{
	clearTreeMark();
}

#include "inspector.moc"
