/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

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

#include <kdebug.h>

#include <kdialog.h>
#include <klocale.h>

#include "errordialog.h"


ErrorDialog::ErrorDialog(ErrorList* _errorList, QWidget* parent)
	: KDialog(parent), errorList(_errorList)
{
	setCaption(i18n("Errors"));
	setModal(false);
	setButtons(Close | Help);  // | User1);
// 	setButtonGuiItem(User1, i18n("Help on &Error"));
	setDefaultButton(Close);
	showButtonSeparator(false);


	QWidget *baseWidget = new QWidget(this);
	setMainWidget(baseWidget);
	baseLayout = new QVBoxLayout(baseWidget); 
	
	label = new QLabel(baseWidget);
	label->setText(i18n("In this list you find the error(s) that resulted from running your Logo code. \nGood luck!"));
	// \nYou can select an error and click the 'Help on Error' button for help.
	label->setScaledContents(true);
	baseLayout->addWidget(label);
	
	spacer = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed);
	baseLayout->addItem(spacer);
	
	errorTable = new QTableWidget(errorList->count(), 3, baseWidget);
	errorTable->setSelectionMode(QAbstractItemView::SingleSelection);
	errorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
// 	errorTable->setReadOnly(true);
	errorTable->setShowGrid(false);
// 	errorTable->setFocusStyle(QTable::FollowStyle);
// 	errorTable->setLeftMargin(0);
	
// 	errorTable->horizontalHeader()->setLabel(0, i18n("number"));
// 	errorTable->hideColumn(0); // needed to link with the errorData which stores the tokens, codes, etc.
// 	
// 	errorTable->horizontalHeader()->setLabel(1, i18n("line") );
// 	
// 	errorTable->horizontalHeader()->setLabel(2, i18n("description"));
// 	errorTable->setColumnStretchable(2, true);

	QStringList horizontalHeaderTexts;
	horizontalHeaderTexts << i18n("line") << i18n("description") << i18n("code");
	errorTable->setHorizontalHeaderLabels(horizontalHeaderTexts);
	errorTable->setColumnWidth(0, baseWidget->fontMetrics().width("88888"));

	errorTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

	baseLayout->addWidget(errorTable);

	int row = 0;
	foreach (const ErrorMessage &error, *errorList) {
		int col = 0;
		QStringList itemTexts;
		itemTexts << QString::number(error.token().startRow()) << error.text() << QString::number(error.code());
		foreach (const QString &itemText, itemTexts) {
			errorTable->setItem(row, col, new QTableWidgetItem(itemText));
			col++;
		}
		row++;
	}

	errorTable->resizeColumnsToContents();

	connect (errorTable, SIGNAL(itemSelectionChanged()), this, SLOT(selectedErrorChangedProxy()));

	show();
}


void ErrorDialog::selectedErrorChangedProxy()
{
	const Token* t = &errorList->at(errorTable->selectedItems().first()->row()).token();
	emit currentlySelectedError(t->startRow(), t->startCol(), t->endRow(), t->endCol());
}


#include "errordialog.moc"

