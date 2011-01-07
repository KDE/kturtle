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

 
// This is one class for all custom dialogs that KTurtle uses.
// Right now that are:
//   - ErrorMessage dialog, used to display the users coding errors
//   - ColorPicker dialog, used to easily pick color-codes and put them in the code
//   - RestartOrBack dialog, used when executing in fullscreenmode


#ifndef _DIALOGS_H_
#define _DIALOGS_H_


#include <tqlayout.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqstring.h>
#include <tqtable.h>
#include <tqvaluelist.h>

#include <kdialogbase.h>
#include <kcolordialog.h>

#include "token.h"


// BEGIN ErrorMessage dialog HEADER

struct errorData
{
	uint    code;
	Token   tok;
	TQString msg;
};


class ErrorMessage : public KDialogBase
{
	Q_OBJECT
	
	public:
		ErrorMessage(TQWidget *parent);
		~ErrorMessage() {}
	
		bool containsErrors();

		void display();

	public slots:
		void slotAddError(Token&, const TQString&, uint code);
		void updateSelection();

	private slots:
		void showHelpOnError();
		void errorMessageHelp();

	signals:
		void setCursor(uint row, uint column);
		void setSelection(uint, uint, uint, uint);

	private:
		typedef TQValueList<errorData> errorList;
		errorList     errList;
		TQTable       *errTable;
		uint          errCount;
		errorData     currentError;

	protected:
		TQDialog      *dialog;
		TQVBoxLayout  *baseLayout;
		TQLabel       *label;
		TQSpacerItem  *spacer;
};

// END



// BEGIN ColorPicker dialog HEADER

class ColorPicker : public KDialogBase
{
	Q_OBJECT
  
	public:
		ColorPicker(TQWidget *parent);
		virtual ~ColorPicker() {}


	signals:
		void visible(bool); // for toggling convenience
		void ColorCode(TQString);


	private:
		void updateSelector();
		void updatePal();
		void updatePatch();
		void updateColorCode();
		
		TQWidget            *BaseWidget;
		KHSSelector        *hsSelector;
		KValueSelector     *valuePal;
		TQVBoxLayout        *vlayout;
		TQHBoxLayout        *h1layout;
		TQHBoxLayout        *h2layout;
		KColorPatch        *patch;
		TQLabel             *copyable;
		TQLineEdit          *colorcode;
		TQColor              color;
		int                 h, s, v, r, g, b;


	private slots:
		void slotColorPickerHelp();


	protected slots:
		void slotSelectorChanged(int _h, int _s);
		void slotPalChanged(int _v);
		void slotReselect();
		
		void slotEmitVisibility(); // for toggling convenience
		void slotEmitColorCode(); // convenience
};

// END



// BEGIN RestartOrBack dialog HEADER

class RestartOrBack : public KDialogBase
{
	Q_OBJECT
	
	public:
		RestartOrBack (TQWidget *parent);
		~RestartOrBack() {}

	protected:
		TQVBoxLayout  *baseLayout;
		TQLabel       *label;
};

// END


#endif // _DIALOGS_H_
