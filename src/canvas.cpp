/*
 * Copyright (C) 2003 Cies Breijs <cies # showroommama ! nl>
 */

#include <kdebug.h>

#include "settings.h"
#include "canvas.h"

// Implementation of the canvas //
//
Canvas::Canvas(QWidget *parent, const char *name) : QCanvasView(0, parent, name) {
    // Create a new canvas for this view
    TurtleCanvas = new QCanvas(parent);
    TurtleCanvas->setAdvancePeriod(250);  // refresh-rate in [ms]

    // set initial values
    initValues();

    // at last we assign the canvas to the view
    setCanvas(TurtleCanvas);
}

void Canvas::initValues() {
    // canvas size
    slotResizeCanvas( Settings::canvasWidth(), Settings::canvasHeight() );
    CanvasWidth = Settings::canvasWidth();
    CanvasHeight = Settings::canvasHeight();
    // colors
    TurtleCanvas->setBackgroundColor( QColor(255, 255, 255) ); // background
    FgR = 0; // pencolor (forground)
    FgG = 0;
    FgB = 0;
    // pen, wrap, direction and font
    PenWidth = 0;
    Pen = true;
    Wrap = true;
    Dir = 3.14159265358979323846 / 2;
    font = QFont("serif", 10);
    // the position
    slotCenter();
    // construct the default sprite
//    loadSpriteFrames("logo");
}

void Canvas::Line(int xa, int ya, int xb, int yb, bool repeat) {
    QCanvasLine* i = new QCanvasLine(TurtleCanvas);
    i->setPoints( xa, ya, xb, yb );
    i->setPen( QPen( QColor(FgR, FgB, FgG), PenWidth, SolidLine ) );
    i->setZ(1);
    i->show();
    if ( Wrap && !TurtleCanvas->onCanvas(xb, yb) && repeat == false ) {
        intpair offset = Offset(xb, yb);
        int xi = offset.x;
        int yi = offset.y;
        while ( xi != 0 ) {
            Line( xa - ( xi * CanvasWidth ), ya, 
                  xb - ( xi * CanvasWidth ), yb, true );
            if (xi > 0) { xi--; } else if (xi < 0) { xi++; } // offset.x > 0 ? offset.x-- : offset.x++;
        }
        while ( yi != 0 ) {
            Line( xa, ya - ( yi * CanvasHeight ), 
                  xb, yb - ( yi * CanvasHeight ), true );
            if (yi > 0) { yi--; } else if (yi < 0) { yi++; } // offset.y > 0 ? offset.y-- : offset.y++;
        }
    }
}

intpair Canvas::Offset(int x, int y) {
    // This funktion make is easy to read since deviding int's is a weird thing:
    // int x = 5 / 2,  outputs: x = 2,  with: 5 % 2 = 1 (the rest value) 
    intpair offset;
    if ( x < 0 ) { x = x - CanvasWidth; } 
    if ( y < 0 ) { y = y - CanvasHeight; } 
    offset.x = x / CanvasWidth;
    offset.y = y / CanvasHeight;
    kdDebug(0)<<"  offset start:"<<x<<", "<<y<<"offset end:"<<offset.x<<", "<<offset.y<<endl;
    return offset;
}

void Canvas::loadSpriteFrames(QString name) {
    // read the pixmaps name.0001.png, name.0002.png, ..., name.0035.png: the different rotations
    // #0000 for 0 or 360, #0001 for 10, #0002 for 20, ..., #0018 for 180, etc.
    
    // WARNING if the dir doesnt exists the app will crash!!!
    // This will be fixed in qt3.3 and in the current qt-copy
    QCanvasPixmapArray* SpriteFrames = new QCanvasPixmapArray("/home/cies/logo/.sprites/"+name+".%1.png", 36);
    QCanvasSprite* Sprite = new QCanvasSprite(SpriteFrames, TurtleCanvas);
    Sprite->setZ(1);
//     Sprite->show();
}

// void Canvas::updateSprite(int xa, int ya, int xb, int yb) {
//     QCanvasSprite* i = new QCanvasLine(TurtleCanvas);
// }

// Slots:
void Canvas::slotClear() {
    QCanvasItemList list = canvas()->allItems();
    QCanvasItemList::Iterator it = list.begin();
    for (; it != list.end(); ++it) {
        if ( *it ) {
            delete *it;
        }
    }
}

void Canvas::slotGo(int x, int y) {
    if ( Wrap && !TurtleCanvas->onCanvas(x, y) ) {
        intpair offset = Offset(x, y);
        kdDebug(0)<<"offset:"<<offset.x<<", "<<offset.y<<"  X:"<<x<<", Y:"<<y;
        PosX = x - (offset.x * CanvasWidth);
        PosY = y - (offset.y * CanvasHeight);
	kdDebug(0)<<"   PosXnew:"<<PosX<<", PosYnew:"<<PosY<<endl;
    } else {
        PosX = x;
        PosY = y;
    }
}

void Canvas::slotGoX(int x) {
    if ( Wrap && !TurtleCanvas->onCanvas(x, PosY) ) {
        intpair offset = Offset(x, PosY);
        PosX = x - (offset.x * CanvasWidth);
    } else {
        PosX = x;
    }
}

void Canvas::slotGoY(int y) {
    if ( Wrap && !TurtleCanvas->onCanvas(PosX, y) ) {
        intpair offset = Offset(PosX, y);
        PosY = y - (offset.y * CanvasHeight);
    } else {
        PosY = y;
    }
}

void Canvas::slotForward(int x) {
    int PosXnew = PosX + (int)( x * cos(Dir) );
    int PosYnew = PosY - (int)( x * sin(Dir) );
    if (Pen) {
        Line(PosX, PosY, PosXnew, PosYnew);
    }
    slotGo(PosXnew, PosYnew);
//     if ( Wrap && !TurtleCanvas->onCanvas(PosXnew, PosYnew) ) {
//         intpair offset = Offset(PosXnew, PosYnew);
//         PosX = PosXnew - (offset.x * CanvasWidth);
//         PosY = PosYnew - (offset.y * CanvasHeight);
//     } else {
//         PosX = PosXnew;
//         PosY = PosYnew;
//     }
}

void Canvas::slotBackward(int x) {
    int PosXnew = PosX - (int)( x * cos(Dir) );
    int PosYnew = PosY + (int)( x * sin(Dir) );
    if (Pen) {
        Line(PosX, PosY, PosXnew, PosYnew);
    }
    slotGo(PosXnew, PosYnew);
//     if ( Wrap && !TurtleCanvas->onCanvas(PosXnew, PosYnew) ) {
//         intpair offset = Offset(PosXnew, PosYnew);
//         PosX = PosXnew - (offset.x * CanvasWidth);
//         PosY = PosYnew - (offset.y * CanvasHeight);
//     } else {
//         PosX = PosXnew;
//         PosY = PosYnew;
//     }
}

void Canvas::slotDirection(double deg) {
    Dir = ( -deg + 90 ) * 3.14159265358979323846 / 180;
}

void Canvas::slotTurnLeft(double deg) {
    Dir = Dir + ( deg * 3.14159265358979323846 / 180 );
}

void Canvas::slotTurnRight(double deg) {
    Dir = Dir - ( deg * 3.14159265358979323846 / 180 );
}

void Canvas::slotCenter() {
    PosX = CanvasWidth / 2;
    PosY = CanvasHeight / 2;
}

void Canvas::slotSetPenWidth(int w) {
    if ( w == 1 ) {
        PenWidth = 0; // 0 gives 1 pixel lines using fast algorithem
    } else {
        PenWidth = w;
    }
}

void Canvas::slotPenUp() {
    Pen = false;
}

void Canvas::slotPenDown() {
    Pen = true;
}

void Canvas::slotSetFgColor(int r, int g, int b) {
    // shouldn't it be checked if: ( 0 =< r, g, b =< 255) ?
    FgR = r;
    FgG = g;
    FgB = b;
}

void Canvas::slotSetBgColor(int r, int g, int b) {
    TurtleCanvas->setBackgroundColor( QColor(r, g, b) );
}

void Canvas::slotResizeCanvas(int x, int y) {
    if ( x <= 0 || y <= 0 ) {
        // TODO put error message
        x = 100;
        y = 100;
    }
    CanvasWidth = x;
    CanvasHeight = y;
    TurtleCanvas->resize(x, y);
    emit resize(); 
}


// I'm having major problems with the canvas and qt-3.2
// qt-3.3 will fix it and the supposed fix is allready in qt-copy
// i'll not work any further on sprites, while i dont have qt-3.3 or a fresh qt-copy

void Canvas::slotSpriteShow() {
//     Sprite->show();
}

void Canvas::slotSpriteHide() {
//     Sprite->hide();
}

void Canvas::slotSpritePress() {
}

void Canvas::slotSpriteChange(int x) {
//     Sprite->setFrame(x);
//     Sprite->move(PosX - Sprite->width()/2, PosY - Sprite->height()/2);
}



QString Canvas::slotInput() {} // should these two be in canvas?????
QString Canvas::slotInputWindow() {}

void Canvas::slotPrint(QString text) {
    QCanvasText* t = new QCanvasText(text, font, TurtleCanvas);
    // text does not do the wrapping, never... sorry
    t->setColor( QColor(FgR, FgB, FgG) );
    t->move(PosX, PosY);
    t->show();
}

void Canvas::slotFontType(QString family, QString extra) {
    font.setFamily(family);
    font.setBold( extra.contains("bold") > 0 );
    font.setItalic( extra.contains("italic") > 0 );
    font.setUnderline( extra.contains("underline") > 0 );
    font.setOverline( extra.contains("overline") > 0 );
    font.setStrikeOut( extra.contains("strikeout") > 0 );
}

void Canvas::slotFontSize(int px) {
    font.setPixelSize(px);
}

void Canvas::slotWrapOn() {
    Wrap = true;
}

void Canvas::slotWrapOff() {
    Wrap = false;
}

void Canvas::slotReset() {
    slotClear();
    initValues();
}

    
#include "canvas.moc"
