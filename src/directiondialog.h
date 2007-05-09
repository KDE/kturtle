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

#ifndef _DIRECTIONDIALOG_H_
#define _DIRECTIONDIALOG_H_

#include <QHBoxLayout>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QRadioButton>
#include <QSpinBox>
#include <QSvgRenderer>
#include <QWidget>

#include <kdialog.h>

#include "interpreter/translator.h"

class DirectionCanvas : public QWidget
{
	Q_OBJECT
	
	public:
		DirectionCanvas(QWidget* parent = 0);

	public slots:
		void changed(double previousDeg, double deg, int cmd);
	
	signals:
		void degreeChanged(double deg);

	protected:
		void paintEvent(QPaintEvent *event);
		void mousePressEvent(QMouseEvent *event);

	private:
		int cmd;
		double deg;
		double previousDeg;
		QSvgRenderer turtle;
};


class DirectionDialog : public KDialog
{
	Q_OBJECT

	public:
		DirectionDialog(double deg, QWidget* parent);
		
		enum Command {
			Left,
			Right,
			Direction
		};

		QString command() { return cmdLineEdit->text(); }

	private:
		QHBoxLayout* baseLayout;
		DirectionCanvas* canvas;

		QSpinBox* previousDirectionSpin;
		QSpinBox* directionSpin;

		QRadioButton *leftRadio;
		QRadioButton *rightRadio;
		QRadioButton *directionRadio;

		QLineEdit* cmdLineEdit;

		Command cmd;

		Translator* translator;

		bool skipValueChangedEvent;

		void updateCanvas();
		void updateCmdLineEdit();

	private slots:
		void directionChanged(int value);

		void changeCommand(bool checked);

		void updateDegrees(double deg);
};

#endif // _DIRECTIONDIALOG_H_
