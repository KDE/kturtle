/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

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

#ifndef _CANVAS_H_
#define _CANVAS_H_

#include <QGraphicsView>
#include <QSvgGenerator>

#include "sprite.h"


class Canvas : public QGraphicsView
{
	Q_OBJECT

	public:
		explicit Canvas(QWidget *parent = nullptr);
		~Canvas();

		double turtleAngle() { return turtle->angle(); }
		QImage getPicture();
		void saveAsSvg(const QString&, const QString&);
// 		void scene() { return _scene; }

	public slots:
		void slotClear();
		void slotGo(double x, double y) { turtle->setPos(x, y); };
		void slotGoX(double x) { turtle->setPos(x, turtle->pos().y()); }
		void slotGoY(double y) { turtle->setPos(turtle->pos().x(), y); }
		void slotForward(double x);
		void slotBackward(double x);
		void slotDirection(double deg) { turtle->setAngle(deg); }
		void slotTurnLeft(double deg)  { turtle->setAngle(turtle->angle() - deg); }
		void slotTurnRight(double deg) { turtle->setAngle(turtle->angle() + deg); }
		void slotCenter();
		void slotPenWidth(double width);
		void slotPenUp()   { pen->setStyle(Qt::NoPen); }
		void slotPenDown() { pen->setStyle(Qt::SolidLine); }
		void slotPenColor(double r, double g, double b);
		void slotCanvasColor(double r, double g, double b);
		void slotCanvasSize(double r, double g);
		void slotSpriteShow() { turtle->show(); }
		void slotSpriteHide() { turtle->hide(); }
		void slotPrint(const QString& text);
		void slotFontType(const QString& family, const QString& extra);
		void slotFontSize(double px) { textFont->setPixelSize((int)px); }
		void slotReset();
		void getDirection(double& value);
		void getX(double& value);
		void getY(double& value);

	protected:
		void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;

	private:
		void initValues();
		QColor rgbDoublesToColor(double r, double g, double b);
		void drawLine(double x1, double y1, double x2, double y2);
		void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
		void scaleView(double scaleFactor);

		QGraphicsScene            *_scene;
		QPen                      *pen;
		Sprite                    *turtle;
		QList<QGraphicsLineItem*>  lines;
		QGraphicsLineItem         *line;
		QGraphicsRectItem         *canvasFrame;
		bool                       penWidthIsZero;
		QFont                      *textFont;
		QColor                     textColor;
};


#endif  // _CANVAS_H_
