/*
 * Copyright (C) 2003 Cies Breijs <cies # showroommama ! nl>
 */

#ifndef _CANVAS_H_
#define _CANVAS_H_

#include <qcanvas.h>
#include <qfont.h>

#include <cmath>

struct intpair {
    int x;
    int y;
};

class Canvas : public QCanvasView
{   Q_OBJECT

  public:
    Canvas(QWidget *parent = 0, const char *name = 0);
  
  public slots:
    void slotClear();
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
    
    QString slotInput(); // should these two be in canvas?????
    QString slotInputWindow();

    void slotPrint(QString text);
    void slotFontType(QString family, QString extra);
    void slotFontSize(int px);
    void slotWrapOn();
    void slotWrapOff();
    void slotReset();
    
  signals:
    void resize();
    
  private:
    void initValues();
    void Line(int xa, int ya, int xb, int yb);
    intpair Offset(int x, int y);
    void loadSpriteFrames(QString name);
    
    QCanvas *TurtleCanvas;
    QCanvasSprite* Sprite;
    QCanvasPixmapArray* SpriteFrames;
    int PosX, PosY;
    int CanvasWidth, CanvasHeight;
    int PenWidth;
    double Dir;
    int FgR, FgG, FgB;
    bool Pen;
    bool Wrap;
    QFont font;
};

#endif // _CANVAS_H_
