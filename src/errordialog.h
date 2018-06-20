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

#ifndef _ERRORDIALOG_H_
#define _ERRORDIALOG_H_

#include <QDialog>

#include "interpreter/errormsg.h"

class QDialogButtonBox;
class QLabel;
class QSpacerItem;
class QTableWidget;
class QVBoxLayout;


class ErrorDialog : public QDialog
{
	Q_OBJECT

	public:
        explicit ErrorDialog(QWidget* parent = nullptr);

		void setErrorList(ErrorList*);
		void clear();

	signals:
		void currentlySelectedError(int, int, int, int);

	public slots:
		void enable();
		void disable();

	private slots:
		void selectedErrorChangedProxy();
		void helpRequested();

	private:
		ErrorList      *errorList;
		QTableWidget   *errorTable;
		QVBoxLayout    *baseLayout;
		QLabel         *label;
		QSpacerItem    *spacer;
		QDialogButtonBox *m_buttonBox;
};

#endif  // _ERRORDIALOG_H_

