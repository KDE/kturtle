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


#include "console.h"
#include "editor.h"  // for the error highlight color

#include <QHBoxLayout>
#include <QLabel>
#include <QToolTip>
#include <QWidget>

#include <kdebug.h>

#include <klocale.h>



Console::Console(QWidget* parent)
	: QWidgetAction(parent)
{
	baseWidget = new QWidget(parent);
	QHBoxLayout* baseLayout = new QHBoxLayout();
	baseWidget->setLayout(baseLayout);

	comboBox = new KComboBox(true, baseWidget);
	comboBox->setMinimumWidth(200);
	comboBox->setDuplicatesEnabled(true);
	comboBox->setFont(KGlobalSettings::fixedFont());
	comboBox->setToolTip(i18n("Write a command here and press enter..."));
	comboBox->setWhatsThis(i18n("Console: quickly run single commands -- write a command here and press enter."));

	QLabel* consoleLabel = new QLabel(i18n("Console:"), baseWidget);
	consoleLabel->setBuddy(comboBox);
	consoleLabel->setWhatsThis(comboBox->whatsThis());

	baseLayout->addWidget(consoleLabel);
	baseLayout->addWidget(comboBox);
	setDefaultWidget(baseWidget);

	connect(comboBox, SIGNAL(returnPressed()), this, SLOT(run()));
	connect(comboBox, SIGNAL(editTextChanged(const QString&)), this, SLOT(clearMarkings()));
}

void Console::disable()
{
	comboBox->setEnabled(false);
}

void Console::enable()
{
	comboBox->setEnabled(true);
}

void Console::clearMarkings()
{
	comboBox->setToolTip(i18n("Write a command here and press enter..."));
	comboBox->setStyleSheet("");
	comboBox->setFont(KGlobalSettings::fixedFont());
}


void Console::run()
{
	QString errorMessage = emit execute(comboBox->currentText());
	if (errorMessage.isNull()) return;
	showError(errorMessage);
}

void Console::showError(const QString& msg)
{
	comboBox->setStyleSheet("QComboBox:editable{background:"+ERROR_HIGHLIGHT_COLOR.name()+";}");
	comboBox->setFont(KGlobalSettings::fixedFont());
	QString toolTipText("<p style='white-space:pre'><b>ERROR:</b> " + msg + "</p>");
	comboBox->setToolTip(toolTipText);
	// TODO make a tooltip show for one second showing the errorr
// 	QToolTip::showText(baseWidget->mapToGlobal(baseWidget->pos()) /*+ QPoint(comboBox->width()/2, comboBox->height())*/, toolTipText, comboBox, QRect(comboBox->rect()));
}

#include "console.moc"
