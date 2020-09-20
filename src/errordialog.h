/*
    SPDX-FileCopyrightText: 2003-2008 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
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

