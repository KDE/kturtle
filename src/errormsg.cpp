/*
 * KTurtle, Copyright (C) 2003-04 Cies Breijs <cies # kde ! nl>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <klocale.h>

#include "errormsg.h"


/// THE NEW GENERATION ERROR DIALOG is here, now the errors need to be proper classified.

ErrorMessage::ErrorMessage (QWidget *parent)
			: KDialogBase (parent, "errorDialog", false, 0, KDialogBase::Close|KDialogBase::Help|KDialogBase::User1, KDialogBase::Close, true, i18n("Help on Error") ) {
	setCaption( i18n("caption") );
	setButtonWhatsThis( KDialogBase::Close, i18n("Closes this Error Dialog") );
	setButtonWhatsThis( KDialogBase::Help, i18n("Click here to read more on this Error Dialog in KTurtle's Handbook.") );
	setButtonTip( KDialogBase::Help, i18n("Click here for help using this Error Dialog") );
	setButtonWhatsThis( KDialogBase::User1, i18n("Click here for help regarding the error you selected in the list. This button will not work when no error is selected.") );
	setButtonTip( KDialogBase::User1, i18n("Click here for help regarding the error you selected.") );
	
	QWidget *baseWidget = new QWidget( this );
	setMainWidget(baseWidget);
	baseLayout = new QVBoxLayout(baseWidget); 
	
	label = new QLabel(baseWidget);
	label->setText( i18n("In this list you find the error(s) that resulted from running your Logo code.\nYou can select an error and click the 'Help on Error' button for help. Good luck!") );
	label->setScaledContents(true);
	baseLayout->addWidget(label);
	
	spacer = new QSpacerItem( 10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed );
	baseLayout->addItem(spacer);
	
	errTable = new QTable(0, 4, baseWidget);
	errTable->setSelectionMode(QTable::SingleRow);
	errTable->setReadOnly(true);
	errTable->setShowGrid(false);
	errTable->setFocusStyle(QTable::FollowStyle);
	errTable->setLeftMargin(0);
	connect( errTable, SIGNAL( selectionChanged() ), this, SLOT( updateSelection() ) );
	
	errTable->horizontalHeader()->setLabel( 0, i18n("line") );
	errTable->setColumnWidth(0, baseWidget->fontMetrics().width("88888") );
	
	errTable->horizontalHeader()->setLabel( 1, i18n("column") );
	errTable->hideColumn(1); // overinforms, but info could be needed later on

	errTable->horizontalHeader()->setLabel( 2, i18n("code") );
	errTable->setColumnWidth(2, baseWidget->fontMetrics().width("88888") );
	
	errTable->horizontalHeader()->setLabel( 3, i18n("description") );
	errTable->setColumnStretchable(3, true);

	baseLayout->addWidget(errTable);
}


void ErrorMessage::slotAddError(QString msg, uint row, uint col, uint code) {
	errTable->insertRows(0);
	errTable->setText( 0, 0, QString("%1").arg(row) ); // stupid way to convert ints to QStrings
	errTable->setText( 0, 1, QString("%1").arg(col) );
	errTable->setText( 0, 2, QString("%1").arg(code) );
	errTable->setText( 0, 3, QString("%1").arg(msg) );
}


bool ErrorMessage::containsErrors() {
	if (errTable->numRows() != 0) {
		return false;
	}
	return true;
}

void ErrorMessage::display() {
	errTable->clearSelection();
	enableButton (KDialogBase::User1, false);
	show();
}

void ErrorMessage::updateSelection() {
	int row = errTable->currentRow();
	emit SetCursor( errTable->text(row, 0).toInt(), errTable->text(row, 1).toInt() );
	enableButton (KDialogBase::User1, true);
}

