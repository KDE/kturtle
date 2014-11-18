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

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <QAction>
#include <QComboBox>


class Console : public QAction
{
	Q_OBJECT

	public:
		Console(QWidget *parent);

		void disable();
		void enable();

		void showError(const QString&);

	public slots:
		void run();
		void clearMarkings();
		void executeActionTriggered();

	signals:
		QString execute(const QString&);

	private:
		QComboBox *comboBox;
		QWidget   *baseWidget;
};

#endif  // _CONSOLE_H_
