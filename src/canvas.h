/*
	Copyright (C) 2003-2006 Cies Breijs <cies # kde ! nl>

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


#ifndef _CANVAS_H_
#define _CANVAS_H_

#include <QtGui>

#include "turtle.h"


class Canvas : public QGraphicsView
{
	Q_OBJECT

	public:
		Canvas(QWidget *parent = 0);
		~Canvas();
		
// 		QPixmap* canvas2Pixmap();

// 		QSize sizeHint() { return QSize(scene->width(), scene->height()); }
// 		QSize minimunSizeHint() { return QSize(200, 200); } // do it with set


	public slots:
		void slotClear();
		void slotGo(double x, double y);
		void slotGoX(double x);
		void slotGoY(double y);
		void slotForward(double x);
		void slotBackward(double x);
		void slotDirection(double deg);
		void slotTurnLeft(double deg);
		void slotTurnRight(double deg);
		void slotCenter();
		void slotPenWidth(double width);
		void slotPenUp();
		void slotPenDown();
		void slotPenColor(double r, double g, double b);
		void slotCanvasColor(double r, double g, double b);
		void slotCanvasSize(double x, double y);
		void slotSpriteShow();
		void slotSpriteHide();
		void slotPrint(QString text);
		void slotFontType(QString family, QString extra);
		void slotFontSize(double px);
		void slotWrapOn();
		void slotWrapOff();
		void slotReset();
// 
// 
// 	signals:
// 		void resized();

	protected:
		void resizeEvent(QResizeEvent* event);


	private:
		void initValues();
		QColor rgbDoublesToColor(double r, double g, double b);
		void drawLine(double x1, double y1, double x2, double y2);

// 		void line(double xa, double ya, double xb, double yb);
// 		void updateTurtlePosition();
// 		void updateTurtleAngle();
		
		QGraphicsScene *scene;
		QPen *pen;
		Turtle *turtle;
		QList<QGraphicsLineItem*> lines;
		QGraphicsLineItem *line;
		bool penWidthIsZero;
};

#endif // _CANVAS_H_
