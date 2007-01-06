/*
	Copyright (C) 2003-2007 Cies Breijs <cies AT kde DOT nl>

	This program is free software; you can redistribute it and/or
	modify it under the terms of version 2 of the GNU General Public
	License as published by the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the Free
	Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.
*/



#include <klocale.h>

#include <kdebug.h>


#include "interpreter/translator.h"  // for getting the translated ArgumentSeparator
#include "inspector.h"


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
	variableLayout->addWidget(variableTable);
	tabWidget->addTab(variableTab, i18n("&Variables"));

	QStringList headers;
	headers << i18n("name") << i18n("value") << i18n("type");
	variableTable->setColumnCount(3);
	variableTable->setHorizontalHeaderLabels(headers);

	functionTab    = new QWidget();
	functionLayout = new QHBoxLayout(functionTab);
	functionTable  = new QTableWidget(functionTab);
	functionLayout->addWidget(functionTable);
	tabWidget->addTab(functionTab, i18n("&Learned Functions"));

// 	QStringList headers;
	headers.clear();
	headers << i18n("name") << i18n("parameters");
	functionTable->setColumnCount(2);
	functionTable->setHorizontalHeaderLabels(headers);

	treeTab    = new QWidget();
	treeLayout = new QHBoxLayout(treeTab);
	treeView   = new QTreeWidget(treeTab);
	treeLayout->addWidget(treeView);
	tabWidget->addTab(treeTab, i18n("Code &Tree"));

	mainLayout->addWidget(tabWidget);

	clear();
}


Inspector::~Inspector()
{
}


void Inspector::clear()
{
}


void Inspector::updateVariable(const QString& name, const Value& value)
{
	variableTable->insertRow(0);
	variableTable->setItem(0, 0, new QTableWidgetItem(name));
	variableTable->setItem(0, 1, new QTableWidgetItem(value.string()));
	QTableWidgetItem* typeItem;
	switch (value.type()) {
		case Value::Empty:
			typeItem  = new QTableWidgetItem(QString("<qt><i>%1</i></qt>").arg(i18n("empty")));
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
	variableTable->setItem(0, 2, typeItem);
}

void Inspector::updateFunction(const QString& name, const QStringList& parameters)
{
	functionTable->insertRow(0);
	functionTable->setItem(0, 0, new QTableWidgetItem(name));
	QString paramList = parameters.join(Translator::instance()->default2localized(QString(",")));
	functionTable->setItem(0, 1, new QTableWidgetItem(paramList));
}

void Inspector::updateTree(TreeNode* rootNode)
{
}

void Inspector::highlightSymbol(const QString& name)
{
}


#include "inspector.moc"
