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

#include "colorpicker.h"
#include "interpreter/translator.h"  // for getting the translated ArgumentSeparator

#include <QHBoxLayout>
#include <QClipboard>
#include <QFontMetrics>
#include <QLabel>
#include <QVBoxLayout>

#include <kapplication.h>
#include <klocale.h>
#include <kpushbutton.h>


ColorPicker::ColorPicker(QWidget* parent) 
	: KDialog(parent)
{
	setCaption(i18n("Color Picker"));
	setModal(false);
	setButtons(User1);
	setDefaultButton(User1);
	setButtonGuiItem(User1, KGuiItem(i18n("&Quit"), "dialog-close"));
	connect(this, SIGNAL(user1Clicked()), this, SLOT(hide()));
	showButtonSeparator(false);

	QWidget* baseWidget = new QWidget(this);
	setMainWidget(baseWidget);

	QVBoxLayout* baseLayout = new QVBoxLayout(baseWidget);

	QGridLayout* gridLayout = new QGridLayout(baseWidget);
	baseLayout->addLayout(gridLayout);

	redSlider = new QSlider(Qt::Horizontal, this);
	redSlider->setMaximum(255);
	redSpin = new QSpinBox(this);
	redSpin->setMaximum(255);
	QLabel* redLabel = new QLabel(i18n("Amount red:"));
	redLabel->setBuddy(redSpin);
	gridLayout->addWidget(redLabel, 0, 0);
	gridLayout->addWidget(redSlider, 0, 1);
	gridLayout->addWidget(redSpin, 0, 2);
	connect(redSlider, SIGNAL(valueChanged(int)), redSpin, SLOT(setValue(int)));
	connect(redSpin, SIGNAL(valueChanged(int)), redSlider, SLOT(setValue(int)));
	connect(redSpin, SIGNAL(valueChanged(int)), this, SLOT(redChanged(int)));


	greenSlider = new QSlider(Qt::Horizontal, this);
	greenSlider->setMaximum(255);
	greenSpin = new QSpinBox(this);
	greenSpin->setMaximum(255);
	QLabel* greenLabel = new QLabel(i18n("Amount green:"));
	greenLabel->setBuddy(greenSpin);
	gridLayout->addWidget(greenLabel, 1, 0);
	gridLayout->addWidget(greenSlider, 1, 1);
	gridLayout->addWidget(greenSpin, 1, 2);
	connect(greenSlider, SIGNAL(valueChanged(int)), greenSpin, SLOT(setValue(int)));
	connect(greenSpin, SIGNAL(valueChanged(int)), greenSlider, SLOT(setValue(int)));
	connect(greenSpin, SIGNAL(valueChanged(int)), this, SLOT(greenChanged(int)));

	blueSlider = new QSlider(Qt::Horizontal, this);
	blueSlider->setMaximum(255);
	blueSpin = new QSpinBox(this);
	blueSpin->setMaximum(255);
	QLabel* blueLabel = new QLabel(i18n("Amount blue:"));
	blueLabel->setBuddy(blueSpin);
	gridLayout->addWidget(blueLabel, 2, 0);
	gridLayout->addWidget(blueSlider, 2, 1);
	gridLayout->addWidget(blueSpin, 2, 2);
	connect(blueSlider, SIGNAL(valueChanged(int)), blueSpin, SLOT(setValue(int)));
	connect(blueSpin, SIGNAL(valueChanged(int)), blueSlider, SLOT(setValue(int)));
	connect(blueSpin, SIGNAL(valueChanged(int)), this, SLOT(blueChanged(int)));

	baseLayout->addSpacing(20);

	colorPatch = new ColorPatch(this);
	colorPatch->setMinimumWidth(100);
	colorPatch->setMinimumHeight(60);
	baseLayout->addWidget(colorPatch);

	baseLayout->addSpacing(20);

	QHBoxLayout* resultLayout = new QHBoxLayout(baseWidget);
	resultLayout->addStretch();

	baseLayout->addLayout(resultLayout);
	resultBox = new KLineEdit(this);
	resultBox->setReadOnly(true);
	resultBox->setFont(KGlobalSettings::fixedFont());
	int width = QFontMetrics(KGlobalSettings::fixedFont()).width("255, 255, 255_000");
	resultBox->setMinimumWidth(width);
	resultBox->setMaximumWidth(width);
	resultLayout->addWidget(resultBox);

	KPushButton* copyButton = new KPushButton(KIcon("edit-copy"), i18n("&Copy to clipboard"), baseWidget);
	resultLayout->addWidget(copyButton);
	connect(copyButton, SIGNAL(clicked()), this, SLOT(copyProxy()));

	KPushButton* pasteButton = new KPushButton(KIcon("edit-paste"), i18n("&Paste to editor"), baseWidget);
	resultLayout->addWidget(pasteButton);
	connect(pasteButton, SIGNAL(clicked()), this, SLOT(pasteProxy()));

	resultLayout->addStretch();

	updateResult(0, 0, 0);
}

void ColorPicker::redChanged(int value)
{
	updateResult(value, greenSlider->value(), blueSlider->value());
}

void ColorPicker::greenChanged(int value)
{
	updateResult(redSlider->value(), value, blueSlider->value());
}

void ColorPicker::blueChanged(int value)
{
	updateResult(redSlider->value(), greenSlider->value(), value);
}

void ColorPicker::updateResult(int r, int g, int b)
{
	QString separator(Translator::instance()->default2localized(QString(",")));
	resultBox->setText(QString("%2%1 %3%1 %4 \n").arg(separator).arg(r).arg(g).arg(b));
	colorPatch->setColor(QColor(r, g, b));
	colorPatch->repaint();
}

void ColorPicker::copyProxy()
{
	KApplication::clipboard()->setText(resultBox->text());
}

void ColorPicker::pasteProxy()
{
	emit pasteText(resultBox->text());
}


#include "colorpicker.moc"
