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


#include <qpainter.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "settings.h"
#include "canvas.h"

#include <stdlib.h>

const double PI=3.14159265358979323846;

Canvas::Canvas(QWidget *parent, const char *name) : QCanvasView(0, parent, name) {
	// Create a new canvas for this view
	TurtleCanvas = new QCanvas(parent);
	TurtleCanvas->setAdvancePeriod(250);  // refresh-rate in [ms]
	
	// set initial values
	initValues();
	
	// at last we assign the canvas to the view
	setCanvas(TurtleCanvas);
}

Canvas::~Canvas()
{
	delete Sprite;
	delete SpriteFrames;
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
	Dir = PI/2;
	font = QFont("serif", 18);
	// the position
	PosX = CanvasWidth / 2;
	PosY = CanvasHeight / 2;
	// construct the default sprite
	loadSpriteFrames("turtle");
	updateSpritePos();
	updateSpriteAngle();
	Sprite->show();
}

QPixmap* Canvas::Canvas2Pixmap() {
	pixmap = QPixmap( TurtleCanvas->width(), TurtleCanvas->height() );
	QPainter painter(&pixmap);
	TurtleCanvas->drawArea(TurtleCanvas->rect(), &painter);
	return &pixmap;
}


void Canvas::Line(int xa, int ya, int xb, int yb) {
	QCanvasLine* l = new QCanvasLine(TurtleCanvas);
	l->setPoints( xa, ya, xb, yb );
	l->setPen( QPen( QColor(FgR, FgG, FgB), PenWidth, SolidLine ) );
	l->setZ(1);
	l->show();
	kdDebug(0)<<"Line:: xa:"<<xa<<", ya:"<<ya<<", xb:"<<xb<<", yb:"<<yb<<endl;
	if ( Wrap && !TurtleCanvas->onCanvas(xb, yb) ) {
		QPoint translation = TranslationFactor(xa, ya, xb, yb);
		if (translation == QPoint(0, 0) ) {
		kdDebug(0)<<"***********ERRORRR***********"<<endl;
		return;
		}
		kdDebug(0)<<"transX:"<<translation.x()<<",   transY:"<<translation.y()<<endl;
		QPoint t_startPos = QPoint(xa, ya) +
				QPoint(translation.x() * CanvasWidth, translation.y() * CanvasHeight);
		QPoint t_endPos   = QPoint(xb, yb) + 
				QPoint(translation.x() * CanvasWidth, translation.y() * CanvasHeight);
		Line( t_startPos.x(), t_startPos.y(), t_endPos.x(), t_endPos.y() );
		// Line( t_startPos, t_endPos ); // not yet :-)
	}
}

QPoint Canvas::TranslationFactor(int xa, int ya, int xb, int yb) {
	// this class returns a QPoint which can be used to properly 'wrap' a line
	QPoint CrossPoint[4]; // under wicked circumstance this can happen! (crossing both corners)
	QPoint Translate[4];
	int i = 0;
	if ( ( xb - xa ) == 0 ) {  // check for an infinite direction coefficient
		i++;
		Translate[i] = QPoint(0, 1);
		CrossPoint[i] = QPoint(xa, 0);
		i++;
		Translate[i] = QPoint(0,-1);
		CrossPoint[i] = QPoint(xa, CanvasHeight);
	} else {
		// Here we find out what crossing points the line has with canvas border lines
		float A = (float)( yb - ya ) / (float)( xb - xa );
		int B = ya - (int)( ( A * xa ) );
		int x_sT = (int)( ( -B ) / A );                // A * x_sT + B = 0  =>   x_sT = -B / A 
		int x_sB = (int)( ( CanvasHeight - B ) / A );  // A * x_sB + B = CW  =>   x_sB = (CW - B) / A
		int y_sL = B;                                  // A * 0 + B = y_sL  =>  y_sL = B
		int y_sR = (int)( A * CanvasWidth ) + B;
		kdDebug(0)<<"CB:: rc:"<<A<<", xTop:"<<x_sT<<", xBot:"<<x_sB<<", yLft:"<<y_sL<<", yRft:"<<y_sR<<". "<<endl;
		
		// Here we find out what crossing points are on the borders AND on the lines
		if ( 0 <= x_sT && x_sT <= CanvasWidth && PointInRange(x_sT, 0, xa, ya, xb, yb) ) {
		i++;
		Translate[i] = QPoint(0, 1);
		CrossPoint[i] = QPoint(x_sT, 0);
		}
		if ( 0 <= x_sB && x_sB <= CanvasWidth && PointInRange(x_sB, CanvasHeight, xa, ya, xb, yb) ) {
		i++;
		Translate[i] = QPoint(0,-1);
		CrossPoint[i] = QPoint(x_sB, CanvasHeight);
		} 
		if ( 0 <= y_sL && y_sL <= CanvasHeight && PointInRange(0, y_sL, xa, ya, xb, yb) ) {
		i++;
		Translate[i] = QPoint(1, 0);
		CrossPoint[i] = QPoint(0, y_sL);
		}
		if ( 0 <= y_sR && y_sR <= CanvasHeight && PointInRange(CanvasWidth, y_sR, xa, ya, xb, yb) ) {
		i++;
		Translate[i] = QPoint(-1, 0);
		CrossPoint[i] = QPoint(CanvasWidth, y_sR);
		}
	
		if ( i == 0 ) {
		kdDebug(0)<<"**no border crossings**"<<endl;
		QPoint returnValue = QPoint(0, 0);
		// Here a fallback if the line has no crossings points with any borders.
		// This mostly happens because of unlucky rounding, when this happens the line is nearly
		// crossing a corner of the canvas.
		// This code make sure the line is tranlated back onto the canvas.
		// The -2 and +2 was just something i learnt from examples... I HAVE NO PROOF FOR THIS!
		if ( -2 <= x_sT && x_sT <= (CanvasWidth + 3) && PointInRange(x_sT, 0, xa, ya, xb, yb) ) {
			returnValue = returnValue + QPoint( 0, 1);
		}
		if ( -2 <= x_sB && x_sB <= (CanvasWidth + 3) && PointInRange(x_sB, CanvasHeight, xa, ya, xb, yb) ) {
			returnValue = returnValue + QPoint( 0,-1);
		} 
		if ( -2 <= y_sL && y_sL <= (CanvasHeight + 3) && PointInRange(0, y_sL, xa, ya, xb, yb) ) {
			returnValue = returnValue + QPoint( 1, 0);
		}
		if ( -2 <= y_sR && y_sR <= (CanvasHeight + 3)  && PointInRange(CanvasWidth, y_sR, xa, ya, xb, yb) ) {
			returnValue = returnValue + QPoint(-1, 0);
		}
		
		if ( returnValue == QPoint(0, 0) ) { kdDebug(0)<<"*****Shouldn't happen*****"<<endl; }
		return returnValue;
		}
	}
	
	QPoint returnValue = QPoint(0, 0);
	if ( i == 1 ) { kdDebug(0)<<"***123243455!"<<endl; return Translate[1]; }
	if ( i > 1 )  {
		QPoint endPos(xb, yb);
		int smallestSize = ( QPoint(xa, ya) - endPos ).manhattanLength();
		for ( int ii = 1; ii <= i; ii++ ) {
		int testSize = ( CrossPoint[ii] - endPos ).manhattanLength();
		if ( testSize < smallestSize ) {
			smallestSize = testSize;
			returnValue = Translate[ii];
			kdDebug(0)<<"***heeeeeeeeeeee!"<<endl;
		} else if ( testSize == smallestSize ) {  // this only happens on corners
			kdDebug(0)<<"***ARggg!"<<endl;
			returnValue = QPoint(0, 0);
			if ( xb < 0 ) {
			returnValue = returnValue + QPoint(1, 0);
			} else if ( xb > CanvasWidth ) {
			returnValue = returnValue + QPoint(-1, 0);
			}
			if ( yb < 0 ) {
			returnValue = returnValue + QPoint(0, 1);
			} else if ( yb > CanvasHeight ) {
			returnValue = returnValue + QPoint(0,-1);
			}
			return returnValue;
		}
		}
	kdDebug(0)<<"***yoooo!"<<endl;
	return returnValue;
	}
	kdDebug(0)<<"***nnnooooOOoooOOOoooOOOoooo!"<<endl;
	return returnValue;
}

bool Canvas::PointInRange(int px, int py, int xa, int ya, int xb, int yb) {
	if ( ( ( px >= xa && px <= xb ) || ( px <= xa && px >= xb ) ) &&
		( ( py >= ya && py <= yb ) || ( py <= ya && py >= yb ) ) ) {
		return true;
	}
	return false;
}

QPoint Canvas::Offset(int x, int y) {
	// This funktion make is easy to read since deviding int's is a weird thing:
	// int x = 5 / 2,  outputs: x = 2,  with: 5 % 2 = 1 (the rest value) 
	if ( x < 0 ) { x = x - CanvasWidth; } 
	if ( y < 0 ) { y = y - CanvasHeight; } 
	QPoint offset( x / CanvasWidth, y / CanvasHeight);
	return offset;
}

void Canvas::loadSpriteFrames(QString name) {
	// read the pixmaps name.0001.png, name.0002.png, ..., name.0035.png: the different rotations
	// #0000 for 0 or 360, #0001 for 10, #0002 for 20, ..., #0018 for 180, etc.
	
	// WARNING if the dir doesnt exists the app will crash!!!
	// This will be fixed in qt3.3 and in the current qt-copy
	QPixmap turtlePix = QPixmap(locate("data","kturtle/pics/turtle.0000.png") );
	if ( turtlePix.isNull() ) {
	QString mString = i18n("The turtle picture is not found;\nplease check your installation.");
	KMessageBox::sorry( this, mString, i18n("Error") );
	exit(1);
	}
	QString spritePath = locate("data","kturtle/pics/"+name+".0000.png");
	spritePath.remove(".0000.png");
	SpriteFrames = new QCanvasPixmapArray(spritePath+".%1.png", 36);
	Sprite = new QCanvasSprite(SpriteFrames, TurtleCanvas);
	Sprite->setZ(250);
}

void Canvas::updateSpritePos() {
	Sprite->move( (double)(PosX - ( Sprite->width() / 2 ) ), (double)(PosY - ( Sprite->height() / 2 ) ), -1 );
}

void Canvas::updateSpriteAngle() {
	// get the Dir back on the 1st circle 
	while (Dir >= 2*PI || Dir < 0) {
		if (Dir >= 2*PI) {
			Dir = Dir - 2*PI;
		}
		if (Dir < 0) {
			Dir = Dir + 2*PI;
		}
	}
	// convert to degrees, fix the direction, divide by 10 (for picnr), and add .5 before casting to int
	int i = (int)( ( ( (-Dir * 180) / PI + 90) / 10 ) + .5 );
	Sprite->setFrame(i);
	updateSpritePos(); // pixmaps of different rotations have different sizes, so refresh
}


// Slots:
void Canvas::slotClear() {
	QCanvasItemList list = canvas()->allItems();
	QCanvasItemList::Iterator it = list.begin();
	for (; it != list.end(); ++it) {
		if ( *it ) {
		if ( !( (*it)->z() == 250 ) ) { // this is the turtle sprite, we dont want to kill him
			delete *it;
		}
		}
	}
}

void Canvas::slotClearSpriteToo() {
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
		QPoint offset = Offset(x, y);
		PosX = x - ( offset.x() * CanvasWidth );
		PosY = y - ( offset.y() * CanvasHeight );
	} else {
		PosX = x;
		PosY = y;
	}
	updateSpritePos();
}

void Canvas::slotGoX(int x) {
	if ( Wrap && !TurtleCanvas->onCanvas(x, PosY) ) {
		QPoint offset = Offset(x, PosY);
		PosX = x - ( offset.x() * CanvasWidth );
	} else {
		PosX = x;
	}
	updateSpritePos();
}

void Canvas::slotGoY(int y) {
	if ( Wrap && !TurtleCanvas->onCanvas(PosX, y) ) {
		QPoint offset = Offset(PosX, y);
		PosY = y - ( offset.y() * CanvasHeight );
	} else {
		PosY = y;
	}
	updateSpritePos();
}

void Canvas::slotForward(int x) {
	float f = (float)x + .5;
	int PosXnew = PosX + (int)(f * cos(Dir) );
	int PosYnew = PosY - (int)(f * sin(Dir) );
	if (Pen) {
		Line(PosX, PosY, PosXnew, PosYnew);
	}
	slotGo(PosXnew, PosYnew);
}

void Canvas::slotBackward(int x) {
	float f = (float)x + .5;
	int PosXnew = PosX - (int)(f * cos(Dir) );
	int PosYnew = PosY + (int)(f * sin(Dir) );
	if (Pen) {
		Line(PosX, PosY, PosXnew, PosYnew);
	}
	slotGo(PosXnew, PosYnew);
}

void Canvas::slotDirection(double deg) {
	Dir = ( -deg + 90 ) * PI / 180;
	updateSpriteAngle();
}

void Canvas::slotTurnLeft(double deg) {
	Dir = Dir + ( deg * PI / 180 );
	updateSpriteAngle();
}

void Canvas::slotTurnRight(double deg) {
	Dir = Dir - ( deg * PI / 180 );
	updateSpriteAngle();
}

void Canvas::slotCenter() {
	PosX = CanvasWidth / 2;
	PosY = CanvasHeight / 2;
	updateSpritePos();
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
	emit CanvasResized(); 
}


// I'm having major problems with the canvas and qt-3.2
// qt-3.3 will fix it and the supposed fix is allready in qt-copy
// i'll not work any further on sprites, while i dont have qt-3.3 or a fresh qt-copy

void Canvas::slotSpriteShow() {
    	Sprite->show();
}

void Canvas::slotSpriteHide() {
    	Sprite->hide();
}

void Canvas::slotSpritePress() {
}

void Canvas::slotSpriteChange(int x) {
    	Sprite->setFrame(x);
    	Sprite->move(PosX - Sprite->width()/2, PosY - Sprite->height()/2);
}

void Canvas::slotPrint(QString text) {
	QCanvasText* t = new QCanvasText(text, font, TurtleCanvas);
	// text does not do the wrapping, never... sorry
	t->setColor( QColor(FgR, FgG, FgB) );
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
    	slotClearSpriteToo();
    	initValues();
}

    
#include "canvas.moc"
