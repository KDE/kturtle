/*
	Copyright (C) 2003-2009 Cies Breijs <cies AT kde DOT nl>

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
#include "editor.h"  // only for the error highlight color value

#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QFontDatabase>
#include <QLabel>
#include <QLineEdit>

#include <KLocalizedString>


Console::Console(QWidget* parent)
	: QWidgetAction(parent)
{
	baseWidget = new QWidget(parent);
	QHBoxLayout* baseLayout = new QHBoxLayout();
        baseLayout->setMargin(0);
	baseWidget->setLayout(baseLayout);

	comboBox = new QComboBox(baseWidget);
	comboBox->setMinimumWidth(200);
	comboBox->setDuplicatesEnabled(true);
	comboBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
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
	comboBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

void Console::run()
{
	QString errorMessage = emit execute(comboBox->currentText());
	if (errorMessage.isNull()) {
		comboBox->clearEditText();
		return;
	}
	showError(errorMessage);
}

void Console::showError(const QString& msg)
{
	comboBox->setStyleSheet("QComboBox:editable{background:" + ERROR_HIGHLIGHT_COLOR.name() + ";}");
	comboBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	QString toolTipText(i18n("<p style='white-space:pre'><b>ERROR:</b> %1</p>", msg));
	comboBox->setToolTip(toolTipText);
}

void Console::executeActionTriggered()
{
	QLineEdit* lineEdit = comboBox->lineEdit();
	if (!lineEdit)
		return;
	QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier, QChar('\n'));
	QApplication::sendEvent(lineEdit, &event);
}
