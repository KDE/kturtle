/*
    KTurtle, Copyright (C) 2003-04 Cies Breijs <cies # kde ! nl>

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
	: KDialogBase (parent, "errorDialog", false, 0, KDialogBase::Close|KDialogBase::Help|KDialogBase::User1, KDialogBase::Close, true, i18n("Help on &Error") )
{
	setCaption( i18n("Error Dialog") );
	setButtonWhatsThis( KDialogBase::Close, i18n("Closes this Error Dialog") );
	setButtonWhatsThis( KDialogBase::Help, i18n("Click here to read more on this Error Dialog in KTurtle's Handbook.") );
	setButtonTip( KDialogBase::Help, i18n("Click here for help using this Error Dialog") );
	setButtonWhatsThis( KDialogBase::User1, i18n("Click here for help regarding the error you selected in the list. This button will not work when no error is selected.") );
	setButtonTip( KDialogBase::User1, i18n("Click here for help regarding the error you selected.") );
	
	QWidget *baseWidget = new QWidget(this);
	setMainWidget(baseWidget);
	baseLayout = new QVBoxLayout(baseWidget); 
	
	label = new QLabel(baseWidget);
	label->setText( i18n("In this list you find the error(s) that resulted from running your Logo code.\nYou can select an error and click the 'Help on Error' button for help. Good luck!") );
	label->setScaledContents(true);
	baseLayout->addWidget(label);
	
	spacer = new QSpacerItem( 10, 10, QSizePolicy::Minimum, QSizePolicy::Fixed );
	baseLayout->addItem(spacer);
	
	errTable = new QTable(0, 3, baseWidget);
	errTable->setSelectionMode(QTable::SingleRow);
	errTable->setReadOnly(true);
	errTable->setShowGrid(false);
	errTable->setFocusStyle(QTable::FollowStyle);
	errTable->setLeftMargin(0);
	
	errTable->horizontalHeader()->setLabel( 0, i18n("number") );
	errTable->hideColumn(0); // needed to link with the errorData which stores the tokens, codes, etc.
	
	errTable->horizontalHeader()->setLabel( 1, i18n("line") );
	errTable->setColumnWidth(1, baseWidget->fontMetrics().width("88888") );
	
	errTable->horizontalHeader()->setLabel( 2, i18n("description") );
	errTable->setColumnStretchable(2, true);

	baseLayout->addWidget(errTable);
	
	errCount = 1;
}


void ErrorMessage::slotAddError(Token& t, QString s, uint c)
{
	errorData err;
	err.code = c;
	err.tok = t;
	err.msg = s;
	errList.append(err);
	
// 	Token currentToken = err.tok; kdDebug(0)<<"ErrorMessage::slotAddError, got token: '"<<currentToken.look<<"', @ ("<<currentToken.start.row<<", "<<currentToken.start.col<<") - ("<<currentToken.end.row<<", "<<currentToken.end.col<<"), tok-number:"<<currentToken.type<<endl;
	
	errTable->insertRows(0);
	errTable->setText( 0, 0, QString::number(errCount) ); // put the count in a hidden field for reference
	errTable->setText( 0, 1, QString::number(err.tok.start.row) );
	errTable->setText( 0, 2, err.msg );
	
	errCount++;
}


bool ErrorMessage::containsErrors()
{
	if (errTable->numRows() != 0) return false;
	return true;
}

void ErrorMessage::display()
{
	errTable->clearSelection();
	enableButton (KDialogBase::User1, false);
	errTable->sortColumn(0, true, true);
	show();
	connect( errTable, SIGNAL( selectionChanged() ), this, SLOT( updateSelection() ) );
}

void ErrorMessage::updateSelection()
{
	int i = errTable->text( errTable->currentRow(), 0 ).toInt(); // get the hidden errCount value
	errorData err = *errList.at(i - 1);
	emit SetSelection(err.tok.start.row, err.tok.start.col, err.tok.end.row, err.tok.end.col);
	enableButton (KDialogBase::User1, true);
}


#include "errormsg.moc"
