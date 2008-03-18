/*
	Copyright (C) 2007 Niels Slot <nielsslot AT gmail DOT com>

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

#include "directiondialog.h"

#include <cmath>

#include <kapplication.h>
#include <kglobalsettings.h>
#include <klocale.h>

#include <QClipboard>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>

#define ROUND2INT(x) ( (x) >= 0 ? (int)( (x) + .5 ) : (int)( (x) - .5 ) )


//BEGIN DirectionCanvas widget

DirectionCanvas::DirectionCanvas(QWidget* parent)
	: QWidget(parent)
{
	setFocusPolicy(Qt::ClickFocus);
	setMinimumSize(230, 200);
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
	turtle.load(QString(":turtle.svg"));
	greyTurtle.load(QString(":turtle_grey.svg"));

	deg = 0;
	previousDeg = 0;
	greyTurtleEnabled = true;
}

void DirectionCanvas::enableGreyTurtle(bool enable)
{
	greyTurtleEnabled = enable;
	update();
}

void DirectionCanvas::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	int side = qMin(width(), height());

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	
	painter.save();
	
	// Place us in the middle of the widget
	painter.translate(width() / 2, height() / 2);

	// Scale the widget to a square of 200 by 200
	painter.scale(side / 200.0, side / 200.0);

	// Draw the ellipse. With a nice border of 10
	painter.drawEllipse(-80, -80, 160, 160);

//	painter.save();
//	// Rotate for the circle lines.
//	painter.rotate(90);
	
	// Draw the lines in the circle
	painter.save();
	for (int i = 0; i < 4; i++) {
		painter.drawLine(0, -80, 0, 80);
		painter.rotate(45);
	}
	painter.restore();

	painter.drawText(-100, -98, 200, 200, Qt::AlignHCenter|Qt::AlignTop, "0");
	painter.drawText(-100, -100, 200, 200, Qt::AlignHCenter|Qt::AlignBottom, "180");
	painter.drawText(-100, -100, 203, 200, Qt::AlignRight|Qt::AlignVCenter, "90");
	painter.drawText(-109, -100, 200, 200, Qt::AlignLeft|Qt::AlignVCenter, "270");
	//painter.restore(); //Rotate back

	painter.save();

// 	// Rotate for the previousDeg pointer
// 	painter.rotate(previousDeg);
// 	painter.setBrush(QColor(0, 180, 0, 128));
// 	painter.drawPie(-20, -88, 40, 30, 60*16, 60*16);

	if (greyTurtleEnabled) {
		// Rotate for the turtle
		painter.rotate(previousDeg);
		painter.setPen(Qt::blue);
		painter.drawLine(0, -80, 0, 0);
		// Draw the turtle 70 by 70, in the middle of the widget
	// 	QRectF greyTurtleRect(-35, -35, 70, 70);
		QRectF greyTurtleRect(-25, -25, 50, 50);
		greyTurtle.render(&painter, greyTurtleRect);
		painter.restore();
		painter.save();
	}

	// Rotate for the turtle
	painter.rotate(deg);
	painter.setPen(Qt::red);
	painter.drawLine(0, -80, 0, 0);
	// Draw the turtle 50 by 50, in the middle of the widget
	QRectF turtleRect(-25, -25, 50, 50);
	turtle.render(&painter, turtleRect);

	painter.restore();
	painter.restore();

	// Draw the widget's border
	painter.setPen(palette().dark().color());
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}

void DirectionCanvas::mouseMoveEvent(QMouseEvent *event)
{
	mousePressEvent(event);
}

void DirectionCanvas::mousePressEvent(QMouseEvent *event)
{
	bool leftOrRight;
	// Only do something if event is caused by the left mouse button
	if (event->buttons() & Qt::LeftButton) {
		leftOrRight = true;  // left
	} else if (event->buttons() & Qt::RightButton) {
		leftOrRight = false;
	} else return;
	
	// 'Translate' the X and Y coordinates so that 
	// 0,0 is in the middle of the widget.
	double trans_x = event->x() - (width() / 2);
	double trans_y = (event->y() - (height() / 2));

	// We now have 4 squares. One four every corner.
	// With a cross in the middle.
	// For every square we calculate a different tangent
	// therefore we have to add of substract a number of degrees

	double tempDeg;

	if (trans_x >= 0 && trans_y >= 0) {
		// Right down
		double arc_tan = trans_y / trans_x;
		tempDeg = 90 + (atan(arc_tan)) * (180/M_PI);
	} else if (trans_x <= 0 && trans_y >= 0) {
		// Left down
		trans_x = trans_x * -1;
		double arc_tan = trans_y / trans_x;
		tempDeg = 270 - (atan(arc_tan)) * (180/M_PI);
	} else if (trans_x >= 0 && trans_y <= 0) {
		// Right up
		trans_y = trans_y * -1;
		double arc_tan = trans_y / trans_x;
		tempDeg = 90 - (atan(arc_tan)) * (180/M_PI);
	} else if (trans_x <= 0 && trans_y <= 0) {
		// Left up
		trans_x = trans_x * -1;
		trans_y = trans_y * -1;
		double arc_tan = trans_y / trans_x;
		tempDeg = 270 + (atan(arc_tan)) * (180/M_PI);
	}

	if (leftOrRight) {
		deg = tempDeg;
		emit degreeChanged(deg);
	} else {
		previousDeg = tempDeg;
		emit previousDegreeChanged(previousDeg);
	}
	update();
}

void DirectionCanvas::changed(double previousDeg, double deg, int cmd)
{
	Q_UNUSED(cmd);
	this->deg = deg;
	this->previousDeg = previousDeg;
	update();
}

//END DirectionCanvas widget


DirectionDialog::DirectionDialog(double deg, QWidget* parent) 
	: KDialog(parent)
{
	skipValueChangedEvent = false;

	if (deg < 0)
		deg = deg + 360;
	else if (deg > 359)
		deg = deg - 360;

	translator = Translator::instance();

	setCaption(i18n("Direction Chooser"));
	setModal(false);
	setButtons(User1);
	setDefaultButton(User1);
	setButtonGuiItem(User1, KGuiItem(i18n("&Quit"), "dialog-close"));
	connect(this, SIGNAL(user1Clicked()), this, SLOT(delayedDestruct()));
	showButtonSeparator(false);

	QWidget* baseWidget = new QWidget(this);
	setMainWidget(baseWidget);

	QVBoxLayout* baseLayout = new QVBoxLayout(baseWidget);
	QHBoxLayout* degreeChooserLayout = new QHBoxLayout(baseWidget);
	baseLayout->addLayout(degreeChooserLayout);

	canvas = new DirectionCanvas(baseWidget);
	connect(canvas, SIGNAL(degreeChanged(double)), this, SLOT(updateDegrees(double)));
	connect(canvas, SIGNAL(previousDegreeChanged(double)), this, SLOT(updatePreviousDegrees(double)));

	degreeChooserLayout->addWidget(canvas);

	QWidget* rightWidget = new QWidget(baseWidget);
	degreeChooserLayout->addWidget(rightWidget);

	QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);

	QLabel* commandPickerLabel = new QLabel(rightWidget);
	commandPickerLabel->setText(i18n("Command &type:"));
	commandPickerLabel->setScaledContents(true);
	rightLayout->addWidget(commandPickerLabel);
	commandPicker = new QComboBox(rightWidget);
	commandPicker->insertItem(Turnleft, translator->default2localized("turnleft"));
	commandPicker->insertItem(Turnright, translator->default2localized("turnright"));
	commandPicker->insertItem(Direction, translator->default2localized("direction"));
	rightLayout->addWidget(commandPicker);
	commandPickerLabel->setBuddy(commandPicker);
	connect(commandPicker, SIGNAL(currentIndexChanged(int)), this, SLOT(changeCommand(int)));

	rightLayout->addStretch();

	QLabel* previousDirectionLabel = new QLabel(rightWidget);
	previousDirectionLabel->setText(i18n("&Previous direction:"));
	previousDirectionLabel->setScaledContents(true);
	rightLayout->addWidget(previousDirectionLabel);
	previousDirectionSpin = new QSpinBox(rightWidget);
	// Use -360 to 720 instead of 0 to 360
	// If 0 to 360 is used, then wrap-around goes from 360 to 0 (which isn't really a step at all)
	// Instead use larger range and then convert it into the 0 to 359 range whenever it is changed.
	previousDirectionSpin->setRange(-360, 720);
	previousDirectionSpin->setWrapping(true);
	previousDirectionSpin->setSingleStep(10);
	previousDirectionSpin->setValue((int)deg);
	rightLayout->addWidget(previousDirectionSpin);
	previousDirectionLabel->setBuddy(previousDirectionSpin);
	connect(previousDirectionSpin, SIGNAL(valueChanged(int)), this, SLOT(directionChanged(int)));

	QLabel* directionLabel = new QLabel(rightWidget);
	directionLabel->setText(i18n("&New direction:"));
	rightLayout->addWidget(directionLabel);
	directionSpin = new QSpinBox(rightWidget);
	// Use -360 to 720 instead of 0 to 360
	// If 0 to 360 is used, then wrap-around goes from 360 to 0 (which isn't really a step at all)
	// Instead use larger range and then convert it into the 0 to 359 range whenever it is changed.
	directionSpin->setRange(-360, 720);
	directionSpin->setWrapping(true);
	directionSpin->setSingleStep(10);
	directionSpin->setValue((int)deg);
	rightLayout->addWidget(directionSpin);
	directionLabel->setBuddy(directionSpin);
	connect(directionSpin, SIGNAL(valueChanged(int)), this, SLOT(directionChanged(int)));

	baseLayout->addSpacing(20);

// 	QVBoxLayout* commandPasteLayout = new QVBoxLayout(baseWidget);
// 	baseLayout->addLayout(commandPasteLayout);
	QHBoxLayout *pasteRowLayout = new QHBoxLayout(baseWidget);
	baseLayout->addLayout(pasteRowLayout);

	pasteRowLayout->addStretch();
	cmdLineEdit = new KLineEdit(rightWidget);
	cmdLineEdit->setMinimumWidth(cmdLineEdit->fontMetrics().width("000000000_360"));
	cmdLineEdit->setReadOnly(true);
	cmdLineEdit->setFont(KGlobalSettings::fixedFont());
	pasteRowLayout->addWidget(cmdLineEdit);
	KPushButton* copyButton = new KPushButton(KIcon("edit-copy"), i18n("&Copy to clipboard"), baseWidget);
	pasteRowLayout->addWidget(copyButton);
	connect(copyButton, SIGNAL(clicked()), this, SLOT(copyProxy()));
	KPushButton* pasteButton = new KPushButton(KIcon("edit-paste"), i18n("&Paste to editor"), baseWidget);
	pasteRowLayout->addWidget(pasteButton);
	connect(pasteButton, SIGNAL(clicked()), this, SLOT(pasteProxy()));
	pasteRowLayout->addStretch();

	baseLayout->addSpacing(10);

	changeCommand(0);
	show();
}

void DirectionDialog::directionChanged(int value)
{
	Q_UNUSED(value);
	// Don't update the canvas when we just updated the	direction spinbox.
	// (only update when the users changes the spinbox)

	// if value is outside of the 0 to 359 range, then move it into that range
	if (previousDirectionSpin->value() < 0) {
		previousDirectionSpin->setValue(previousDirectionSpin->value() + 360);
	} else if (previousDirectionSpin->value() >= 360) {
		previousDirectionSpin->setValue(previousDirectionSpin->value() - 360);
	}

	// if value is outside of the 0 to 359 range, then move it into that range
	if (directionSpin->value() < 0) {
		directionSpin->setValue(directionSpin->value() + 360);
	} else if (directionSpin->value() >= 360) {
		directionSpin->setValue(directionSpin->value() - 360);
	}

	if (skipValueChangedEvent) {
		skipValueChangedEvent = false;
		return;
	}
	updateCanvas();
}

void DirectionDialog::updateCanvas()
{
	//Get the things we need, then update the canvas.
	int previousDir = previousDirectionSpin->value();
	int dir = directionSpin->value();
	canvas->changed(previousDir, dir, cmd);
	//When the canvas is changed, so should the command.
	updateCmdLineEdit();
}

void DirectionDialog::changeCommand(int command)
{
	cmd = command;
	if (cmd == Direction) {
		previousDirectionSpin->setEnabled(false);
		canvas->enableGreyTurtle(false);
	} else {
		previousDirectionSpin->setEnabled(true);
		canvas->enableGreyTurtle(true);
	}
	updateCanvas();
}

void DirectionDialog::updateDegrees(double deg)
{
	// The canvas has changed, update the spinbox and command-LineEdit
	skipValueChangedEvent = true;
	directionSpin->setValue(ROUND2INT(deg));
	updateCmdLineEdit();
}

void DirectionDialog::updatePreviousDegrees(double deg)
{
	// The canvas has changed, update the spinbox and command-LineEdit
	skipValueChangedEvent = true;
	previousDirectionSpin->setValue(ROUND2INT(deg));
	updateCmdLineEdit();
}

void DirectionDialog::updateCmdLineEdit()
{
	// Generate a new value for the command-LineEdit.
	QString output;
	int degree;
	switch (cmd) {
		case Turnleft:
			output.append(translator->default2localized("turnleft"));
			degree = 360 - (directionSpin->value() - previousDirectionSpin->value());
			break;
		case Turnright:
			output.append(translator->default2localized("turnright"));
			degree = directionSpin->value() - previousDirectionSpin->value();
			break;
		case Direction:
			output.append(translator->default2localized("direction"));
			degree = directionSpin->value();
			break;
	}
	if (degree < 0) {
		degree += 360;
	} else if (degree >= 360) {
		degree -= 360;
	}
	output.append(QString(" %1\n").arg(degree));
	cmdLineEdit->setText(output);
}

void DirectionDialog::copyProxy()
{
	KApplication::clipboard()->setText(cmdLineEdit->text());
}

void DirectionDialog::pasteProxy()
{
	emit pasteText(cmdLineEdit->text());
}

#include "directiondialog.moc"
