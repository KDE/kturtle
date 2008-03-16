/*
	Copyright (C) 2007 Aleix Pol Gonzalez <aleixpol@gmail.com>

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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QWidget>
#include <QListWidget>
#include <QComboBox>

class Interpreter;

class Console : public QWidget
{
	Q_OBJECT
	public:
		Console(QWidget *parent);
	public slots:
		void addOperation();
		
	signals:
		void execute(const QString &);
	private:
		QComboBox *m_input;
		QListWidget *m_log;
};

#endif
