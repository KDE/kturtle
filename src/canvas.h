/*
 * Copyright (C) 2003 Cies Breijs <cies # showroommama ! nl>

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

#include <cmath>


struct BorderPoint {
    int border;
    QPoint point;
};

class Canvas : public QCanvasView
{   Q_OBJECT

  public:
    Canvas(QWidget *parent = 0, const char *name = 0);
    ~Canvas();
    QPixmap* Canvas2Pixmap();
  
  public slots:
    void slotClear();
    void slotClearSpriteToo();
    void slotGo(int x, int y);
    void slotGoX(int x);
    void slotGoY(int y);
    void slotForward(int x);
    void slotBackward(int x);
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
    void Line(int xa, int ya, int xb, int yb);
    QPoint TranslationFactor(int xa, int ya, int xb, int yb);
    bool PointInRange(int px, int py, int xa, int ya, int xb, int yb);
    QPoint Offset(int x, int y);
    void loadSpriteFrames(QString name);
    void updateSpritePos();
    void updateSpriteAngle();
    
    QCanvas             *TurtleCanvas;
    QPixmap              pixmap;
    QCanvasSprite       *Sprite;
    QCanvasPixmapArray  *SpriteFrames;
    QFont                font;
    int PosX, PosY;
    int CanvasWidth, CanvasHeight;
    int PenWidth;
    double Dir;
    int FgR, FgG, FgB;
    bool Pen;
    bool Wrap;
};

#endif // _CANVAS_H_
