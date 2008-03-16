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

#include "console.h"
#include <QLayout>
#include <QLineEdit>
#include <QDebug>

Console::Console(QWidget * parent)
	: QWidget(parent)
{
	QVBoxLayout *consoleLayout = new QVBoxLayout(this);
	m_log = new QListWidget(this);
	m_input = new QComboBox(this);
	m_input->setEditable(true);
	
	consoleLayout->addWidget(m_log);
	consoleLayout->addWidget(m_input);
	
	connect(m_input->lineEdit(), SIGNAL(returnPressed()), this, SLOT(addOperation()));
}

void Console::addOperation()
{
	qDebug() << "adding " << m_input->lineEdit()->text();
	m_log->addItem(m_input->lineEdit()->text());
	m_input->lineEdit()->selectAll();
	
	emit execute(m_input->lineEdit()->text());
}

#include "console.moc"
