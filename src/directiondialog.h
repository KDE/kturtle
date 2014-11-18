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

#ifndef _DIRECTIONDIALOG_H_
#define _DIRECTIONDIALOG_H_

#include "interpreter/translator.h"

#include <QSvgRenderer>
#include <QWidget>
#include <QDialog>
#include <QLineEdit>


class QHBoxLayout;
class QMouseEvent;
class QPaintEvent;
class QRadioButton;
class QPushButton;

class KIntSpinBox;
class KComboBox;

class DirectionCanvas : public QWidget
{
	Q_OBJECT
	
	public:
		DirectionCanvas(QWidget* parent = 0);
		void enableGreyTurtle(bool);

	public slots:
		void updateDirections(double previousDeg, double deg);
	
	signals:
		void degreeChanged(double deg);
		void previousDegreeChanged(double deg);

	protected:
		void paintEvent(QPaintEvent *event);
		void mouseMoveEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);

	private:
		double translateMouseCoords(double x, double y);

		double deg;
		double previousDeg;
		QSvgRenderer turtle;
		QSvgRenderer greyTurtle;
		bool greyTurtleEnabled;
};


class DirectionDialog : public KDialog
{
	Q_OBJECT

	public:
		DirectionDialog(double deg, QWidget* parent);
		
		enum Command {
			Turnleft = 0,
			Turnright = 1,
			Direction = 2
		};

	signals:
		void pasteText(const QString&);

	private:
		DirectionCanvas* canvas;

		KComboBox* commandPicker;
		KIntSpinBox* previousDirectionSpin;
		KIntSpinBox* directionSpin;

		QPushButton *copyButton;
		QPushButton *pasteButton;

		QLineEdit* commandBox;

		int currentCommand;  // enum DirectionChooser::Command

		Translator* translator;

		bool skipValueChangedEvent;

		void updateCanvas();
		void updateCommandBox();

	private slots:
		void directionChanged(int value);
		void changeCommand(int command);
		void updateDegrees(double deg);
		void updatePreviousDegrees(double deg);
		void copyProxy();
		void pasteProxy();
};

#endif  // _DIRECTIONDIALOG_H_
