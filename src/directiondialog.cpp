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
using std::atan;

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288419717
#endif

#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFontDatabase>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QSpinBox>

#include <KGuiItem>
#include <KLocalizedString>
#include <KStandardGuiItem>

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

	painter.save();

	// the gray turtle
	if (greyTurtleEnabled) {
		painter.rotate(previousDeg);
		painter.setPen(Qt::blue);
		painter.drawLine(0, -80, 0, 0);
		QRectF greyTurtleRect(-25, -25, 50, 50);
		greyTurtle.render(&painter, greyTurtleRect);
		painter.restore();
		painter.save();
	}

	// the more healthy looking one
	painter.rotate(deg);
	painter.setPen(Qt::red);
	painter.drawLine(0, -80, 0, 0);
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
	// Only act upon left and right mouse button clicks,
	// then translate the X and Y coordinates so that
	// (0, 0) is in the middle of the widget, sent the
	// signals and update the widget.
	if (event->buttons() & Qt::LeftButton) {
		deg = translateMouseCoords(event->x() - (width() / 2), event->y() - (height() / 2));
		update();
		emit degreeChanged(deg);
	} else if (event->buttons() & Qt::RightButton) {
		previousDeg = translateMouseCoords(event->x() - (width() / 2), event->y() - (height() / 2));
		emit previousDegreeChanged(previousDeg);
		update();
	}
}

void DirectionCanvas::updateDirections(double previousDeg, double deg)
{
	this->deg = deg;
	this->previousDeg = previousDeg;
	update();
}

double DirectionCanvas::translateMouseCoords(double trans_x, double trans_y)
{
	// We now have 4 squares. One four every corner.
	// With a cross in the middle.
	// For every square we calculate a different tangent
	// therefore we have to add of subtract a number of degrees
	double result = 0;
	if (trans_x >= 0 && trans_y >= 0) {
		// Right down
		double arc_tan = trans_y / trans_x;
		result = 90 + (atan(arc_tan)) * (180/M_PI);
	} else if (trans_x <= 0 && trans_y >= 0) {
		// Left down
		trans_x = trans_x * -1;
		double arc_tan = trans_y / trans_x;
		result = 270 - (atan(arc_tan)) * (180/M_PI);
	} else if (trans_x >= 0 && trans_y <= 0) {
		// Right up
		trans_y = trans_y * -1;
		double arc_tan = trans_y / trans_x;
		result = 90 - (atan(arc_tan)) * (180/M_PI);
	} else if (trans_x <= 0 && trans_y <= 0) {
		// Left up
		trans_x = trans_x * -1;
		trans_y = trans_y * -1;
		double arc_tan = trans_y / trans_x;
		result = 270 + (atan(arc_tan)) * (180/M_PI);
	}
	return result;
}

//END DirectionCanvas widget


DirectionDialog::DirectionDialog(double deg, QWidget* parent)
	: QDialog(parent)
{
	skipValueChangedEvent = false;

	while (deg < 0 || deg > 359) {
		if (deg < 0)
			deg = deg + 360;
		else if (deg > 359)
			deg = deg - 360;
	}

	translator = Translator::instance();

	setWindowTitle(i18n("Direction Chooser"));
	setModal(false);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

	QWidget *mainWidget = new QWidget(this);
	mainLayout->addWidget(mainWidget);

	QWidget* baseWidget = new QWidget(this);
	mainLayout->addWidget(baseWidget);

	QVBoxLayout* baseLayout = new QVBoxLayout;
    baseLayout->setMargin(0);
    baseWidget->setLayout( baseLayout );
	QHBoxLayout* degreeChooserLayout = new QHBoxLayout;
	baseLayout->addLayout(degreeChooserLayout);

	canvas = new DirectionCanvas(baseWidget);
	mainLayout->addWidget(canvas);
	connect(canvas, &DirectionCanvas::degreeChanged, this, &DirectionDialog::updateDegrees);
	connect(canvas, &DirectionCanvas::previousDegreeChanged, this, &DirectionDialog::updatePreviousDegrees);

	degreeChooserLayout->addWidget(canvas);

	QWidget* rightWidget = new QWidget(baseWidget);
	mainLayout->addWidget(rightWidget);
	degreeChooserLayout->addWidget(rightWidget);

	QVBoxLayout* rightLayout = new QVBoxLayout(rightWidget);

	// command picker
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
	connect(commandPicker, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DirectionDialog::changeCommand);

	rightLayout->addStretch();

	// direction
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
	connect(previousDirectionSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DirectionDialog::directionChanged);

	// previous direction
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
	connect(directionSpin, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DirectionDialog::directionChanged);

	baseLayout->addSpacing(20);

	// commandBox and copy/paste buttons
	QHBoxLayout *pasteRowLayout = new QHBoxLayout;
	baseLayout->addLayout(pasteRowLayout);
	pasteRowLayout->addStretch();
	commandBox = new QLineEdit(rightWidget);
	commandBox->setReadOnly(true);
	commandBox->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	commandBox->setMinimumWidth(commandBox->fontMetrics().width("000000000_360"));
	pasteRowLayout->addWidget(commandBox);
	QPushButton* copyButton = new QPushButton(QIcon::fromTheme("edit-copy"), i18n("&Copy to clipboard"), baseWidget);
	mainLayout->addWidget(copyButton);
	pasteRowLayout->addWidget(copyButton);
	connect(copyButton, &QPushButton::clicked, this, &DirectionDialog::copyProxy);
	QPushButton* pasteButton = new QPushButton(QIcon::fromTheme("edit-paste"), i18n("&Paste to editor"), baseWidget);
	mainLayout->addWidget(pasteButton);
	pasteRowLayout->addWidget(pasteButton);
	connect(pasteButton, &QPushButton::clicked, this, &DirectionDialog::pasteProxy);
	pasteRowLayout->addStretch();

	baseLayout->addSpacing(10);

	QDialogButtonBox *buttonBox = new QDialogButtonBox();
	QPushButton *user1Button = new QPushButton;
	buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &DirectionDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &DirectionDialog::reject);
	mainLayout->addWidget(buttonBox);
	user1Button->setDefault(true);
	KGuiItem::assign(user1Button, KStandardGuiItem::close());
	connect(user1Button, &QPushButton::clicked, this, &DirectionDialog::close);

	changeCommand(0);
	show();
}

void DirectionDialog::directionChanged(int value)
{
	Q_UNUSED(value);

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

	// Don't update the canvas when we just updated the direction spinbox.
	// (only update when the users changes the spinbox)
	if (skipValueChangedEvent) {
		skipValueChangedEvent = false;
		return;
	}
	updateCanvas();
	updateCommandBox();
}

void DirectionDialog::updateCanvas()
{
	// Get the things we need, then update the canvas.
	int previousDir = previousDirectionSpin->value();
	int dir = directionSpin->value();
	canvas->updateDirections(previousDir, dir);
}

void DirectionDialog::changeCommand(int command)
{
	currentCommand = command;
	if (currentCommand == Direction) {
		previousDirectionSpin->setEnabled(false);
		canvas->enableGreyTurtle(false);
	} else {
		previousDirectionSpin->setEnabled(true);
		canvas->enableGreyTurtle(true);
	}
	updateCanvas();
	updateCommandBox();
}

void DirectionDialog::updateDegrees(double deg)
{
	// The canvas has changed, update the spinbox and command-LineEdit
	skipValueChangedEvent = true;
	directionSpin->setValue(ROUND2INT(deg));
	updateCommandBox();
}

void DirectionDialog::updatePreviousDegrees(double deg)
{
	// The canvas has changed, update the spinbox and commandBox
	skipValueChangedEvent = true;
	previousDirectionSpin->setValue(ROUND2INT(deg));
	updateCommandBox();
}

void DirectionDialog::updateCommandBox()
{
	// Generate a new value for the commandBox.
	QString output;
	int degree = 0;
	switch (currentCommand) {
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
	commandBox->setText(output);
}

void DirectionDialog::copyProxy()
{
	QApplication::clipboard()->setText(commandBox->text());
}

void DirectionDialog::pasteProxy()
{
	emit pasteText(commandBox->text());
}
