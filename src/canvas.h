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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifndef _CANVAS_H_
#define _CANVAS_H_

#include <qcanvas.h>


struct BorderPoint
{
	int border;
	QPoint point;
};


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
		void slotForward(int x);
		void slotBackward(int x);
		void slotdirectionection(double deg);
		void slotTurnLeft(double deg);
		void slotTurnRight(double deg);
		void slotCenter();
		void slotSetpenWidth(int w);
		void slotpenUp();
		void slotpenDown();
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
		void slotwrapOn();
		void slotwrapOff();
		void slotReset();


	signals:
		void CanvasResized();


	private:
		void initValues();
		
		void line(int xa, int ya, int xb, int yb);
		void lineShell(int xa, int ya, int xb, int yb);
		bool endlessLoop(QPoint begin, QPoint end);
		bool pointInRange(int px, int py, int xa, int ya, int xb, int yb);
		QPoint offset(int x, int y);
		QPoint translationFactor(int xa, int ya, int xb, int yb);
		
		void loadSpriteFrames(QString name);
		void updateSpritePos();
		void updateSpriteAngle();
		
		QCanvas             *TurtleCanvas;
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
