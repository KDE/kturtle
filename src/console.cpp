/*
	SPDX-FileCopyrightText: 2003-2009 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
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
        baseLayout->setContentsMargins(0, 0, 0, 0);
	baseWidget->setLayout(baseLayout);

	comboBox = new QComboBox(baseWidget);
	comboBox->setEditable(true);
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

	connect(comboBox->lineEdit(), &QLineEdit::returnPressed, this, &Console::run);
	connect(comboBox, &QComboBox::editTextChanged, this, &Console::clearMarkings);
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
	comboBox->setStyleSheet(QLatin1String(""));
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
