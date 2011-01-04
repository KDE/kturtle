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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
 

#include <kapplication.h>   
#include <kdebug.h>
#include <klocale.h>

#include "dialogs.h"


// BEGIN class ErrorMessage dialog

ErrorMessage::ErrorMessage (TQWidget *parent)
	: KDialogBase (parent, "errorDialog", false, 0, KDialogBase::Close | KDialogBase::Help | KDialogBase::User1, KDialogBase::Close, true, i18n("Help on &Error") )
{
	connect( this, TQT_SIGNAL( user1Clicked() ), this, TQT_SLOT( showHelpOnError() ) );
	connect( this, TQT_SIGNAL( helpClicked() ), this, TQT_SLOT( errorMessageHelp() ) );
	setCaption( i18n("Error Dialog") );
	setButtonWhatsThis( KDialogBase::Close, i18n("Closes this error dialog") );
	setButtonWhatsThis( KDialogBase::Help, i18n("Click here to read more on this error dialog in KTurtle's Handbook.") );
	setButtonTip( KDialogBase::Help, i18n("Click here for help using this error dialog") );
	setButtonWhatsThis( KDialogBase::User1, i18n("Click here for help regarding the error you selected in the list. This button will not work when no error is selected.") );
	setButtonTip( KDialogBase::User1, i18n("Click here for help regarding the error you selected.") );
	
	TQWidget *baseWidget = new TQWidget(this);
	setMainWidget(baseWidget);
	baseLayout = new TQVBoxLayout(baseWidget); 
	
	label = new TQLabel(baseWidget);
	label->setText( i18n("In this list you find the error(s) that resulted from running your Logo code. \nGood luck!") );
	// \nYou can select an error and click the 'Help on Error' button for help.
	label->setScaledContents(true);
	baseLayout->addWidget(label);
	
	spacer = new TQSpacerItem( 10, 10, TQSizePolicy::Minimum, TQSizePolicy::Fixed );
	baseLayout->addItem(spacer);
	
	errTable = new TQTable(0, 3, baseWidget);
	errTable->setSelectionMode(TQTable::SingleRow);
	errTable->setReadOnly(true);
	errTable->setShowGrid(false);
	errTable->setFocusStyle(TQTable::FollowStyle);
	errTable->setLeftMargin(0);
	
	errTable->horizontalHeader()->setLabel( 0, i18n("number") );
	errTable->hideColumn(0); // needed to link with the errorData which stores the tokens, codes, etc.
	
	errTable->horizontalHeader()->setLabel( 1, i18n("line") );
	errTable->setColumnWidth(1, baseWidget->fontMetrics().width("88888") );
	
	errTable->horizontalHeader()->setLabel( 2, i18n("description") );
	errTable->setColumnStretchable(2, true);

	baseLayout->addWidget(errTable);
	
	// Since both help buttons are not working yet. Doc need a section on the Error Dialog!
	enableButton(KDialogBase::Help, false);
	enableButton(KDialogBase::User1, false);
	
	errCount = 1;
}


void ErrorMessage::slotAddError(Token& t, const TQString& s, uint c)
{
	errorData err;
	err.code = c;
	err.tok = t;
	err.msg = s;
	errList.append(err);
	
	Token currentToken = err.tok; kdDebug(0)<<"ErrorMessage::slotAddError(); >> "<<err.msg<<" <<, token: '"<<currentToken.look<<"', @ ("<<currentToken.start.row<<", "<<currentToken.start.col<<") - ("<<currentToken.end.row<<", "<<currentToken.end.col<<"), tok-number:"<<currentToken.type<<endl;
	
	errTable->insertRows(0);
	errTable->setText( 0, 0, TQString::number(errCount) ); // put the count in a hidden field for reference
	errTable->setText( 0, 1, TQString::number(err.tok.start.row) );
	errTable->setText( 0, 2, err.msg );
	
	errCount++;
}


bool ErrorMessage::tqcontainsErrors()
{
	if (errTable->numRows() != 0) return true;
	return false;
}

void ErrorMessage::display()
{
	errTable->clearSelection();
	enableButton (KDialogBase::User1, false);
	errTable->sortColumn(0, true, true);
	show();
	connect( errTable, TQT_SIGNAL( selectionChanged() ), this, TQT_SLOT( updateSelection() ) );
}

void ErrorMessage::updateSelection()
{
	int i = errTable->text( errTable->currentRow(), 0 ).toInt(); // get the hidden errCount value
	currentError = *errList.at(i - 1);
	emit setSelection(currentError.tok.start.row, currentError.tok.start.col, 
	                  currentError.tok.end.row,   currentError.tok.end.col);
// #if 0 // FIXME
// 	if ( tokenTypeNames[currentError.tok.type].isEmpty() ) enableButton(KDialogBase::User1, true);
// 	else                                                   enableButton(KDialogBase::User1, false);
// #endif
}

void ErrorMessage::showHelpOnError()
{
// #if 0 // FIXME
// 	kapp->invokeHelp(tokenTypeNames[currentError.tok.type], "", "");
// #endif
}

void ErrorMessage::errorMessageHelp()
{
	kapp->invokeHelp("anchorname", "", "");
}

// END



// BEGIN class ColorPicker dialog

ColorPicker::ColorPicker(TQWidget *parent)
	: KDialogBase(parent, "colorpicker", false, i18n("Color Picker"), KDialogBase::Close | KDialogBase::Help | KDialogBase::User1, KDialogBase::Close, true )
{
	// connect to help
	connect( this, TQT_SIGNAL( helpClicked() ), TQT_SLOT( slotColorPickerHelp() ) );
	
	// for toggling convenience
	connect( this, TQT_SIGNAL( finished() ), TQT_SLOT( slotEmitVisibility() ) );
    
	// Create the top level page and its tqlayout
	BaseWidget = new TQWidget(this);
	setMainWidget(BaseWidget);
    
	// the User1 button:
	setButtonText( KDialogBase::User1, i18n("Insert Color Code at Cursor") );
	connect( this, TQT_SIGNAL( user1Clicked() ), TQT_SLOT( slotEmitColorCode() ) );
 
	TQVBoxLayout *vtqlayout = new TQVBoxLayout(BaseWidget);
    
	vtqlayout->addSpacing(5); // spacing on top

	// the palette and value selector go into a H-box
	TQHBoxLayout *h1tqlayout = new TQHBoxLayout(BaseWidget);
	vtqlayout->addLayout(h1tqlayout);
    
	h1tqlayout->addSpacing(10); // space on the left border
     
	hsSelector = new KHSSelector(BaseWidget); // the color (SH) selector
	hsSelector->setMinimumSize(300, 150);
	h1tqlayout->addWidget(hsSelector);
	connect( hsSelector, TQT_SIGNAL( valueChanged(int, int) ), TQT_SLOT( slotSelectorChanged(int, int) ) );

	h1tqlayout->addSpacing(5); // space in between
   
	valuePal = new KValueSelector(BaseWidget); // the darkness (V) pal
	valuePal->setFixedWidth(30);
	h1tqlayout->addWidget(valuePal);
	connect( valuePal, TQT_SIGNAL( valueChanged(int) ), TQT_SLOT( slotPalChanged(int) ) );   
    
	vtqlayout->addSpacing(15); // space in between the top and the bottom widgets

	// the patch and the codegenerator also go into a H-box
	TQHBoxLayout *h2tqlayout = new TQHBoxLayout(BaseWidget);
	vtqlayout->addLayout(h2tqlayout);
    
	h2tqlayout->addSpacing(10); // space on the left border
   
	patch = new KColorPatch(BaseWidget); // the patch (color previewer)
	patch->setFixedSize(50, 50);
	h2tqlayout->addWidget(patch);
   
	h2tqlayout->addSpacing(10); // space in between

	// the label and the codegenerator go in a V-box
	TQVBoxLayout *v2tqlayout = new TQVBoxLayout(BaseWidget);
	h2tqlayout->addLayout(v2tqlayout); 
    
	copyable = new TQLabel(i18n("Color code:"), BaseWidget); // tha label
	v2tqlayout->addWidget(copyable);
        
	colorcode = new TQLineEdit(BaseWidget); // the code generator
	colorcode->setReadOnly(true);
	v2tqlayout->addWidget(colorcode);
	connect( colorcode, TQT_SIGNAL( selectionChanged() ), TQT_SLOT( slotReselect() ) );
    
	h2tqlayout->addSpacing(5); // spacing on the right border
    
	vtqlayout->addSpacing(10); // spacing on the bottom

	h = g = b = 0; // start with red
	s = v = r = 255;

	slotSelectorChanged(h, s); // update all at once
}



void ColorPicker::updateSelector()
{
	hsSelector->setValues(h, s);
}

void ColorPicker::updatePal()
{
	valuePal->setHue(h);
	valuePal->setSaturation(s);
	valuePal->setValue(v);
	valuePal->updateContents();
	valuePal->tqrepaint(false);
}

void ColorPicker::updatePatch()
{
	patch->setColor(color);
}

void ColorPicker::updateColorCode()
{
	color.getRgb(&r, &g, &b);
	colorcode->setText( TQString("%1, %2, %3").arg(r).arg(g).arg(b) );
	colorcode->selectAll();
}

void ColorPicker::slotSelectorChanged(int h_, int s_)
{
	h = h_;
	s = s_;
	color.setHsv(h, s, v);

	//updateSelector(); // updated it self allready
	updatePal();
	updatePatch();
	updateColorCode();
}

void ColorPicker::slotPalChanged(int v_)
{
	v = v_;
	color.setHsv(h, s, v);

	//updateSelector(); // only needed when H or S changes
	//updatePal(); // updated it self allready
	updatePatch();
	updateColorCode();
}

void ColorPicker::slotReselect()
{
	// reselect by selectAll(), but make sure no looping occurs
	disconnect( colorcode, TQT_SIGNAL( selectionChanged() ), 0, 0 );
	colorcode->selectAll();
	connect( colorcode, TQT_SIGNAL( selectionChanged() ), TQT_SLOT( slotReselect() ) );
}

void ColorPicker::slotEmitVisibility()
{
	// for toggling convenience
	emit visible(false);
}

void ColorPicker::slotEmitColorCode()
{
	// convenience
	emit ColorCode( colorcode->text() );
}

void ColorPicker::slotColorPickerHelp()
{
	kapp->invokeHelp("tools-color-picker", "", "");
}

// END



// BEGIN class RestartOrBack dialog

RestartOrBack::RestartOrBack (TQWidget *parent)
	: KDialogBase (parent, "rbDialog", true, 0, KDialogBase::User1 | KDialogBase::User2, KDialogBase::User2, false, i18n("&Restart"), i18n("&Back") )
{
	setPlainCaption( i18n("Finished Execution") );
	setButtonWhatsThis( KDialogBase::User1, i18n("Click here to restart the current logo program.") );
	setButtonWhatsThis( KDialogBase::User2, i18n("Click here to switch back to the edit mode.") );
	TQWidget *baseWidget = new TQWidget(this);
	setMainWidget(baseWidget);
	baseLayout = new TQVBoxLayout(baseWidget);
	
	label = new TQLabel(baseWidget);
	label->setText( i18n("Execution was finished without errors.\nWhat do you want to do next?") );
	label->setScaledContents(true);
	baseLayout->addWidget(label);
	disableResize();
}

// END


#include "dialogs.moc"
