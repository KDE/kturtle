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

#include <kdebug.h> 
#include <klocale.h>

#include "errormsg.h"


ErrorMessage::ErrorMessage (QWidget *parent)
			: KDialogBase (parent, "errorDialog", false, 0, KDialogBase::Close|KDialogBase::Help|KDialogBase::User1, KDialogBase::Close, true, i18n("Help on &Error") ) {
	setCaption( i18n("Error Dialog") );
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
	
	errTable->horizontalHeader()->setLabel( 0, i18n("line") );
	errTable->setColumnWidth(0, baseWidget->fontMetrics().width("88888") );
	
	errTable->horizontalHeader()->setLabel( 1, i18n("column") );
	errTable->hideColumn(1); // overinforms, but info could be needed later on

	errTable->horizontalHeader()->setLabel( 2, i18n("code") );
	errTable->hideColumn(2); // overinforms, but info could be needed later on
	// errTable->setColumnWidth(2, baseWidget->fontMetrics().width("88888") );
	
	errTable->horizontalHeader()->setLabel( 3, i18n("description") );
	errTable->setColumnStretchable(3, true);

	baseLayout->addWidget(errTable);
}


void ErrorMessage::slotAddError(QString msg, uint row, uint col, uint code) { 
	errTable->insertRows(0);
	QString dash = i18n("-"); // NA means not available
	
	if (row == NA)
		errTable->setText( 0, 0, dash );
	else
		errTable->setText( 0, 0, QString::number(row) ); 
	
	if (col == NA)
		errTable->setText( 0, 1, dash );
	else
		errTable->setText( 0, 1, QString::number(col) );

	if (code== NA)
		errTable->setText( 0, 2, dash );
	else
		errTable->setText( 0, 2, QString::number(code) );
	
	errTable->setText( 0, 3, msg );
	kdDebug(0)<<"ErrorTable entry: "<<row<<", "<<col<<", "<<code<<", "<<msg<<"."<<endl;
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
	errTable->sortColumn(0, true, true);
	show();
	connect( errTable, SIGNAL( selectionChanged() ), this, SLOT( updateSelection() ) );
}

void ErrorMessage::updateSelection() {
	int row = errTable->currentRow();
	int line = errTable->text(row, 0).toInt() + 1;
	emit SetCursor( line, errTable->text(row, 1).toInt() );
	enableButton (KDialogBase::User1, true);
}


#include "errormsg.moc"
