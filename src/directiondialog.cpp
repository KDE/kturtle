/*
	Copyright (C) 2007 Niels Slot <nielsslot AT gmail DOT com>

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

#include "directiondialog.h"

#include <cmath>

#include <klocale.h>

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

DirectionCanvas::DirectionCanvas(QWidget* parent)
: QWidget(parent)
{
	setFocusPolicy(Qt::ClickFocus);
	setMinimumSize(200, 200);
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
	turtle.load(QString(":turtle.svg"));

	deg = 0;
	previousDeg = 0;
}

void DirectionCanvas::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	int side = qMin(width(), height());

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	
	painter.save();
	
	//Place us in the middle of the widget
	painter.translate(width() / 2, height() / 2);

	//Scale the widget to a square of 200 by 200
	painter.scale(side / 200.0, side / 200.0);

	//Draw the ellipse. With a nice border of 10
	painter.drawEllipse(-80, -80, 160, 160);

	//painter.save();
	//Rotate for the circle lines.
	//painter.rotate(90);
	
	//Draw the lines in the circle
	painter.save();
	for(int i = 0; i < 4; i++) {
		painter.drawLine(0, -80, 0, 80);
		painter.rotate(45);
	}
	painter.restore();

	painter.drawText(-100, -100, 200, 20, Qt::AlignHCenter, "0");
	painter.drawText(-100, 80, 200, 20, Qt::AlignHCenter, "180");
	//painter.restore(); //Rotate back

	painter.save();
	//Rotate for the previousDeg pointer
	painter.rotate(previousDeg);

	painter.setBrush(QColor(0, 180, 0, 128));
	painter.drawPie(-20, -88, 40, 30, 60*16, 60*16);
	painter.restore();

	painter.save();
	painter.rotate(deg); //Rotate for the turtle

	painter.setPen(Qt::red);
	painter.drawLine(0, -80, 0, 0);

	//Draw the turtle 50 by 50, in the middle of the widget
	QRectF pos(-25,-25,50,50);
	turtle.render(&painter, pos);
	painter.restore();

	painter.restore();

	//Draw the widget's border
	painter.setPen(palette().dark().color());
	painter.setBrush(Qt::NoBrush);
	painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}

void DirectionCanvas::mousePressEvent(QMouseEvent *event)
{
	//'Translate' the X and Y coordinates so that 
	// 0,0 is in the middle of the widget.
	double trans_x = event->x() - (width() / 2);
	double trans_y = (event->y() - (height() / 2));

	//We now have 4 squares. One four every corner.
	//With a cross in the middle.
	//For every square we calculate a different tangent
	//therefore we have to add of substract a number of degrees

	if(trans_x>=0 && trans_y>=0) {
		//Right down
		double arc_tan = trans_y / trans_x;
		deg = 90+(atan(arc_tan))*(180/M_PI);
	}else if(trans_x<=0 && trans_y>=0) {
		//Left down
		trans_x = trans_x * -1;
		double arc_tan = trans_y / trans_x;
		deg = 270-(atan(arc_tan))*(180/M_PI);
	}else if(trans_x>=0 && trans_y<=0) {
		//Right up
		trans_y = trans_y * -1;
		double arc_tan = trans_y / trans_x;
		deg = 90-(atan(arc_tan))*(180/M_PI);
	}else if(trans_x<=0 && trans_y<=0) {
		//Left up
		trans_x = trans_x * -1;
		trans_y = trans_y * -1;
		double arc_tan = trans_y / trans_x;
		deg = 270+(atan(arc_tan))*(180/M_PI);
	}

	emit degreeChanged(deg);
	update();
}


void DirectionCanvas::changed(double previousDeg, double deg, int cmd)
{
	this->deg = deg;
	this->previousDeg = previousDeg;
	this->cmd = cmd;
	update();
}

DirectionDialog::DirectionDialog(double deg, QWidget* parent) 
: KDialog(parent)
{
	skipValueChangedEvent = false;

	if(deg<0)
		deg = deg + 360;
	else if(deg>359)
		deg = deg - 360;

	translator = Translator::instance();

	setCaption(i18n("Set a direction"));
	setModal(false);
	setButtons(Ok | Cancel);
	setDefaultButton(Ok);
	showButtonSeparator(false);

	QWidget *baseWidget = new QWidget(this);
	setMainWidget(baseWidget);

	baseLayout = new QHBoxLayout(baseWidget);

	canvas = new DirectionCanvas(baseWidget);
	connect(canvas, SIGNAL(degreeChanged(double)),
		this, SLOT(updateDegrees(double)));	

	baseLayout->addWidget(canvas);

	QWidget* rightWidget = new QWidget(baseWidget);
	baseLayout->addWidget(rightWidget);
	
	//Order matters...
	cmdLineEdit = new KLineEdit(translator->default2localized("turnleft") + " 0", rightWidget);
	cmdLineEdit->setReadOnly(true);

	QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);

	QLabel* previousDirection = new QLabel(rightWidget);
	previousDirection->setText(i18n("Previous direction"));
	previousDirection->setScaledContents(true);
	rightLayout->addWidget(previousDirection);

	previousDirectionSpin = new QSpinBox(rightWidget);
	// Use -360 to 720 instead of 0 to 360
	// If 0 to 360 is used, then wrap-around goes from 360 to 0 (which isn't really a step at all)
	// Instead use larger range and then convert it into the 0 to 359 range whenever it is changed.
	previousDirectionSpin->setRange(-360, 720);
	previousDirectionSpin->setWrapping(true);
	previousDirectionSpin->setSingleStep(10);
	previousDirectionSpin->setValue((int)deg);
	rightLayout->addWidget(previousDirectionSpin);

	connect(previousDirectionSpin, SIGNAL(valueChanged(int)),
		this, SLOT(directionChanged(int)));

	QLabel* directionLabel = new QLabel(rightWidget);
	directionLabel->setText(i18n("Direction"));
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

	connect(directionSpin, SIGNAL(valueChanged(int)),
		this, SLOT(directionChanged(int)));

	QGroupBox *commandBox = new QGroupBox(i18n("Command"), rightWidget);

	leftRadio = new QRadioButton(i18n("Turnleft"), commandBox);
	connect(leftRadio, SIGNAL(toggled(bool)),
            this, SLOT(changeCommand(bool)));

	rightRadio = new QRadioButton(i18n("Turnright"), commandBox);
	connect(rightRadio, SIGNAL(toggled(bool)),
            this, SLOT(changeCommand(bool)));

	directionRadio = new QRadioButton(i18n("Direction"), commandBox);
	connect(directionRadio, SIGNAL(toggled(bool)),
            this, SLOT(changeCommand(bool)));

	leftRadio->setChecked(true);
	cmd = Left;

	QVBoxLayout *commandBoxLayout = new QVBoxLayout(commandBox);
	commandBoxLayout->addWidget(leftRadio);
	commandBoxLayout->addWidget(rightRadio);
	commandBoxLayout->addWidget(directionRadio);
	commandBox->setLayout(commandBoxLayout);

	rightLayout->addWidget(commandBox);

	rightLayout->addWidget(cmdLineEdit);

	rightLayout->addStretch();

	show();
}

void DirectionDialog::directionChanged(int value)
{
	Q_UNUSED(value);
	//Don't update the canvas when we just
	// updated the	direction spinbox.
	// (Only update when the users changes the spinbox.)

	// if value is outside of the 0 to 359 range, then move it into that range
	if(previousDirectionSpin->value() < 0) {
		previousDirectionSpin->setValue(previousDirectionSpin->value() + 360);
	}
	else if(previousDirectionSpin->value() >= 360) {
		previousDirectionSpin->setValue(previousDirectionSpin->value() - 360);
	}

	// if value is outside of the 0 to 359 range, then move it into that range
	if(directionSpin->value() < 0) {
		directionSpin->setValue(directionSpin->value() + 360);
	}
	else if(directionSpin->value() >= 360) {
		directionSpin->setValue(directionSpin->value() - 360);
	}

	if(skipValueChangedEvent) {
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

void DirectionDialog::changeCommand(bool checked)
{
	//Another command has been selected through the radio
	// buttons by the user. Update cmd and the canvas.
	if(!checked)
		return;
	
	if(leftRadio->isChecked()) {
		cmd = Left;
	} else if(rightRadio->isChecked()) {
		cmd = Right;
	} else if(directionRadio->isChecked()) {
		cmd = Direction;
	}
	
	updateCanvas();
}

void DirectionDialog::updateDegrees(double deg)
{
	//The canvas has changed, update the spinbox and command-LineEdit
	skipValueChangedEvent = true;
	directionSpin->setValue(ROUND2INT(deg));
	updateCmdLineEdit();
}

void DirectionDialog::updateCmdLineEdit()
{
	//Generate a new value for the command-LineEdit.
	QString output;
	int degree;
	switch(cmd) {
		case Left : {
			output.append(translator->default2localized("turnleft"));
			degree = 360 - (directionSpin->value() - previousDirectionSpin->value());
		}
		break;
		case Right : {
			output.append(translator->default2localized("turnright"));
			degree = directionSpin->value() - previousDirectionSpin->value();
		}
		break;
		case Direction : {
			output.append(translator->default2localized("direction"));
			degree = directionSpin->value();
		}
		break;
	}

	if(degree < 0) {
		degree += 360;
	} else if(degree >= 360) {
		degree -= 360;
	}
	
	output.append(QString(" %1\n").arg(degree));
	cmdLineEdit->setText(output);
}

#include "directiondialog.moc"
