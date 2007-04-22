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

#ifndef _VALUEACTION_H_
#define _VALUEACTION_H_

#include <QAction>



// extends an QAction to remember a simple QString and to pass that QString in its triggered signal
class ValueAction : public QAction
{
	Q_OBJECT

	public:
		ValueAction(const QString& text, const QString& value = QString(), QWidget* parent = 0);

	signals:
		void triggered(const QString&, bool);

	private slots:
		void triggerProxy(bool checked) { emit triggered(m_value, checked); }

	private:
		QString m_value;
};

#endif  // _VALUEACTION_H_

