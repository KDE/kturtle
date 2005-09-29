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


#include <qlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qtable.h>
#include <qvaluelist.h>

#include <kdialogbase.h>
#include <kcolordialog.h>

#include "token.h"


// BEGIN ErrorMessage dialog HEADER

struct errorData
{
	uint    code;
	Token   tok;
	QString msg;
};


class ErrorMessage : public KDialogBase
{
	Q_OBJECT
	
	public:
		ErrorMessage(QWidget *parent);
		~ErrorMessage() {}
	
		bool containsErrors();

		void display();

	public slots:
		void slotAddError(Token&, const QString&, uint code);
		void updateSelection();

	private slots:
		void showHelpOnError();
		void errorMessageHelp();

	signals:
		void setCursor(uint row, uint column);
		void setSelection(uint, uint, uint, uint);

	private:
		typedef QValueList<errorData> errorList;
		errorList     errList;
		QTable       *errTable;
		uint          errCount;
		errorData     currentError;

	protected:
		QDialog      *dialog;
		QVBoxLayout  *baseLayout;
		QLabel       *label;
		QSpacerItem  *spacer;
};

// END



// BEGIN ColorPicker dialog HEADER

class ColorPicker : public KDialogBase
{
	Q_OBJECT
  
	public:
		ColorPicker(QWidget *parent);
		virtual ~ColorPicker() {}


	signals:
		void visible(bool); // for toggling convenience
		void ColorCode(QString);


	private:
		void updateSelector();
		void updatePal();
		void updatePatch();
		void updateColorCode();
		
		QWidget            *BaseWidget;
		KHSSelector        *hsSelector;
		KValueSelector     *valuePal;
		QVBoxLayout        *vlayout;
		QHBoxLayout        *h1layout;
		QHBoxLayout        *h2layout;
		KColorPatch        *patch;
		QLabel             *copyable;
		QLineEdit          *colorcode;
		QColor              color;
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
		RestartOrBack (QWidget *parent);
		~RestartOrBack() {}

	protected:
		QVBoxLayout  *baseLayout;
		QLabel       *label;
};

// END


#endif // _DIALOGS_H_
