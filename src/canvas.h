/*
 * Copyright (C) 2003 Cies Breijs <cies # kde ! nl>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef _CANVAS_H_
#define _CANVAS_H_

#include <qcanvas.h>


class Canvas : public QCanvasView
{
	Q_OBJECT

	public:
		Canvas(QWidget *parent = 0, const char *name = 0);
		~Canvas();
		
		QPixmap* canvas2Pixmap();


	public slots:
		void slotClear();
		void slotClearSpriteToo();
		void slotGo(double x, double y);
		void slotGoX(double x);
		void slotGoY(double y);
		void slotForward(double x);
		void slotBackward(double x);
		void slotDirection(double deg);
		void slotTurnLeft(double deg);
		void slotTurnRight(double deg);
		void slotCenter();
		void slotSetPenWidth(int w);
		void slotPenUp();
		void slotPenDown();
		void slotSetFgColor(int r, int g, int b);
		void slotSetBgColor(int r, int g, int b);
		void slotResizeCanvas(int x, int y);
		void slotSpriteShow();
		void slotSpriteHide();
		void slotSpritePress();
		void slotSpriteChange(int x);
	
		void slotPrint(QString text);
		void slotFontType(QString family, QString extra);
		void slotFontSize(int px);
		void slotWrapOn();
		void slotWrapOff();
		void slotReset();


	signals:
		void CanvasResized();


	private:
		void initValues();
		
		void line(double xa, double ya, double xb, double yb);
		void lineShell(double xa, double ya, double xb, double yb);
		bool endlessLoop(QPoint begin, QPoint end);
		bool pointInRange(double px, double py, double xa, double ya, double xb, double yb);
		QPoint offset(int x, int y);
		QPoint translationFactor(double xa, double ya, double xb, double yb);
		
		void loadSpriteFrames(QString name);
		void updateSpritePos();
		void updateSpriteAngle();
		
		QCanvas             *canvas;
		QPixmap              pixmap;
		QCanvasSprite       *sprite;
		QCanvasPixmapArray  *spriteFrames;
		QFont                font;
		double               posX, posY;
		int                  canvasWidth, canvasHeight;
		int                  penWidth;
		double               direction;
		int                  fgR, fgG, fgB;
		bool                 pen;
		bool                 wrap;
		bool                 cutLoop;
		QPoint               prevStartPos3, prevStartPos2, prevStartPos1, prevEndPos3, prevEndPos2, prevEndPos1;
};

#endif // _CANVAS_H_
