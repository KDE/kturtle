/*
	Copyright (C) 2003-2007 Cies Breijs <cies AT kde DOT nl>

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

#include <QHeaderView>

#include <kdebug.h>

#include <klocale.h>

#include "interpreter/translator.h"  // for getting the translated ArgumentSeparator


// // static const int CURSOR_WIDTH = 2;  // in pixels
// // static const int TAB_WIDTH    = 2;  // in character widths


Inspector::Inspector(QWidget *parent)
	: QFrame(parent)
{
	mainLayout = new QHBoxLayout(this);
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	tabWidget  = new QTabWidget(this);

	variableTab    = new QWidget();
	variableLayout = new QHBoxLayout(variableTab);
	variableTable  = new QTableWidget(variableTab);
	variableTable->setAlternatingRowColors(true);
	variableLayout->addWidget(variableTable);
	tabWidget->addTab(variableTab, i18n("&Variables"));

	functionTab    = new QWidget();
	functionLayout = new QHBoxLayout(functionTab);
	functionTable  = new QTableWidget(functionTab);
	functionTable->setAlternatingRowColors(true);
	functionLayout->addWidget(functionTable);
	tabWidget->addTab(functionTab, i18n("&Functions"));

	treeTab    = new QWidget();
	treeLayout = new QHBoxLayout(treeTab);
	treeView   = new QTreeWidget(treeTab);
	treeView->header()->setVisible(false);
	treeLayout->addWidget(treeView);
	tabWidget->addTab(treeTab, i18n("&Tree"));

	mainLayout->addWidget(tabWidget);

	disable();

	clear();
}


Inspector::~Inspector()
{
}

void Inspector::enable()
{
// 	variableTable->setEnabled(true);
// 	functionTable->setEnabled(true);
// 	treeView->setEnabled(true);
}

void Inspector::disable()
{
	variableTable->setEnabled(false);
	functionTable->setEnabled(false);
	treeView->setEnabled(false);
}

void Inspector::clear()
{
	variableTableEmpty = true;
	functionTableEmpty = true;

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

	variableTable->setRowCount(1);
	functionTable->setRowCount(1);

	QTableWidgetItem* emptyItem;
	emptyItem = new QTableWidgetItem(i18n("Nothing to show here"));
	// emptyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	variableTable->setItem(0, 0, emptyItem);
	variableTable->resizeColumnsToContents();

	emptyItem = new QTableWidgetItem(i18n("Nothing to show here"));
	// emptyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
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
	variableTable->setItem(row, 0, nameItem);

	QTableWidgetItem* valueItem;
	valueItem = new QTableWidgetItem(value.string());
	valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	variableTable->setItem(row, 1, valueItem);

	QTableWidgetItem* typeItem;
	switch (value.type()) {
		case Value::Empty: {
			typeItem  = new QTableWidgetItem(QString("empty"));
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
	variableTable->setItem(row, 2, typeItem);
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
	functionTable->resizeColumnsToContents();
	functionTable->setEnabled(true);
}

void Inspector::updateTree(TreeNode* rootNode)
{
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
	if (node->hasChildren()) {
		for (uint i = 0; i < node->childCount(); i++) {
			result->addChild(walkTree(node->child(i)));
		}
	}
	return result;
}

void Inspector::highlightSymbol(const QString& name)
{
}

int Inspector::findVariable(const QString& name)
{
	// This function will search for a specified variable and return the row number of this variable.
	QList<QTableWidgetItem*> matches = variableTable->findItems(name, Qt::MatchExactly);
	QList<QTableWidgetItem*>::iterator i;
	for (i = matches.begin(); i != matches.end(); ++i) {
		if ((*i)->column() == 0) // only check the first column
			return (*i)->row();
	}
	return -1;
}

#include "inspector.moc"
