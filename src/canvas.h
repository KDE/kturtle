/*
    SPDX-FileCopyrightText: 2003-2006 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
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
		~Canvas() override;

		double turtleAngle() { return turtle->angle(); }
		QImage getPicture();
		void saveAsSvg(const QString&, const QString&);
// 		void scene() { return _scene; }

	public Q_SLOTS:
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
