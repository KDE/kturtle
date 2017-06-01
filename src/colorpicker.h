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

#ifndef _COLORPICKER_H_
#define _COLORPICKER_H_

#include <QFrame>
#include <QPainter>
#include <QPaintEvent>
#include <QDialog>

class QLineEdit;
class QSlider;
class QSpinBox;


class ColorPatch : public QFrame
{
	Q_OBJECT

	public:
		ColorPatch(QWidget *parent) : QFrame(parent) {
			setFrameStyle(QFrame::Panel|QFrame::Sunken);
		}
		void setColor(QColor c) { col = c; }

	protected:
		void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE {
			QPainter p(this);
			drawFrame(&p);
			p.fillRect(contentsRect()&e->rect(), col);
		}
	
	private:
		QColor col;
};




class ColorPicker : public QDialog
{
	Q_OBJECT

	public:
		ColorPicker(QWidget* parent = 0);

	signals:
		void pasteText(const QString&);

	private slots:
		void updateResult(int r, int g, int b);
		void redChanged(int);
		void greenChanged(int);
		void blueChanged(int);
		void copyProxy();
		void pasteProxy();

	private:
		ColorPatch *colorPatch;
		QLineEdit  *resultBox;

		QSlider *redSlider;
		QSlider *greenSlider;
		QSlider *blueSlider;

		QSpinBox *redSpin;
		QSpinBox *greenSpin;
		QSpinBox *blueSpin;
};

#endif  // _COLORPICKER_H_
