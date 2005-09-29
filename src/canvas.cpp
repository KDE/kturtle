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


 
// Note on this file:
// It contains 200 lines of code just to make sure very long line are drawn correctly
// till a certain extent... Beyond that extent the code just cuts the crap, since use user
// it then probably not doing anything usefull anymore; so he she will not notice the code
// is cheating a bit in order to prevent CPU hogging.
// If anyone has a good fix for this problem, than please test it with these logo scripts:

// # bastard script 1
// reset
// canvassize 350,348
// center
// for x = 1 to 255 [
//   fw x
//   tr x / 65
// ]

// # bastard script 2
// reset
// canvassize 350,350
// center
// for x = 1 to 255 [
//   fw x*x
//   tr x
// ]

// Thanks for looking at the code of KTurtle!

// Updated 21.10.2004 by Kiril Jovchev
//    -- Changed that position is kept in double values.
//       This makes accuracy higher.


// BEGIN includes, defines and constants

#include <cmath>

#include <qpainter.h>

#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "settings.h"
#include "canvas.h"


// this function is used in executer and canvas:
#define ROUND2INT(x) ( (x) >= 0 ? (int)( (x) + .5 ) : (int)( (x) - .5 ) )

const double PI = 3.14159265358979323846;
const double DEG2RAD = 3.14159265358979323846/180;

// END



// BEGIN public methods

Canvas::Canvas(QWidget *parent, const char *name) : QCanvasView(0, parent, name)
{
	// Create a new canvas for this view
	canvas = new QCanvas(parent);
	canvas->setAdvancePeriod(250);  // refresh-rate in [ms]
	
	// set initial values
	initValues();
	
	// at last we assign the canvas to the view
	setCanvas(canvas);
}

Canvas::~Canvas()
{
	delete sprite;
	delete spriteFrames;
}


QPixmap* Canvas::canvas2Pixmap()
{
	pixmap = QPixmap( canvas->width(), canvas->height() );
	QPainter painter(&pixmap);
	canvas->drawArea(canvas->rect(), &painter);
	return &pixmap;
}

// END



// BEGIN public slots

void Canvas::slotClear()
{
	QCanvasItemList list = canvas->allItems();
	QCanvasItemList::Iterator it = list.begin();
	for (; it != list.end(); ++it)
	{
		// kill everything but the turtle (who lives on a seperate layer)
		if ( *it && !( (*it)->z() == 250 ) ) delete *it;
	}
}

void Canvas::slotClearSpriteToo()
{
	QCanvasItemList list = canvas->allItems();
	QCanvasItemList::Iterator it = list.begin();
	for (; it != list.end(); ++it)
	{
		if (*it) delete *it;
	}
}


void Canvas::slotGo(double x, double y)
{
	int intX = ROUND2INT(x);
	int intY = ROUND2INT(y);
	
	if ( wrap && !canvas->onCanvas(intX, intY) )
	{
		QPoint offsetPoint = offset(intX, intY);
		posX = x - ( offsetPoint.x() * canvasWidth );
		posY = y - ( offsetPoint.y() * canvasHeight );
	}
	else
	{
		posX = x;
		posY = y;
	}
	updateSpritePos();
}

void Canvas::slotGoX(double x)
{
	int intX = ROUND2INT(x);
	int intPosY = ROUND2INT(posY);
	if ( wrap && !canvas->onCanvas(intX, intPosY) )
	{
		QPoint offsetPoint = offset(intX, intPosY);
		posX = x - ( offsetPoint.x() * canvasWidth );
	}
	else posX = x;
	updateSpritePos();
}

void Canvas::slotGoY(double y)
{
	int intY = ROUND2INT(y);
	int intPosX = ROUND2INT(posX);
	if ( wrap && !canvas->onCanvas(intPosX, intY) )
	{
		QPoint offsetPoint = offset(intPosX, intY);
		posY = y - ( offsetPoint.y() * canvasHeight );
	}
	else posY = y;
	updateSpritePos();
}

void Canvas::slotForward(double x)
{
	double posXnew = posX + ( x * sin(direction * DEG2RAD) );
	double posYnew = posY - ( x * cos(direction * DEG2RAD) );
	if (pen) lineShell(posX, posY, posXnew, posYnew);
	slotGo(posXnew, posYnew);
}

void Canvas::slotBackward(double x)
{
	double posXnew = posX - ( x * sin(direction * DEG2RAD) );
	double posYnew = posY + ( x * cos(direction * DEG2RAD) );
	if (pen) lineShell(posX, posY, posXnew, posYnew);
	slotGo(posXnew, posYnew);
}

void Canvas::slotDirection(double deg)
{
	direction = deg;
	updateSpriteAngle();
}

void Canvas::slotTurnLeft(double deg)
{
	direction = direction - deg;
	updateSpriteAngle();
}

void Canvas::slotTurnRight(double deg)
{
	direction = direction + deg;
	updateSpriteAngle();
}

void Canvas::slotCenter()
{
	posX = canvasWidth / 2;
	posY = canvasHeight / 2;
	updateSpritePos();
}

void Canvas::slotSetPenWidth(int w)
{
	if (w == 1) penWidth = 0; // 0 gives 1 pixel lines using fast algorithem
	else penWidth = w;
}

void Canvas::slotPenUp()
{
	pen = false;
}

void Canvas::slotPenDown()
{
	pen = true;
}

void Canvas::slotSetFgColor(int r, int g, int b)
{
	// shouldn't it be checked if: ( 0 =< r, g, b =< 255) ?
	fgR = r;
	fgG = g;
	fgB = b;
}

void Canvas::slotSetBgColor(int r, int g, int b)
{
	canvas->setBackgroundColor( QColor(r, g, b) );
}

void Canvas::slotResizeCanvas(int x, int y)
{
	if (x <= 0 || y <= 0)
	{
		// TODO put error message
		x = 100;
		y = 100;
	}
	canvasWidth = x;
	canvasHeight = y;
	canvas->resize(x, y);
	emit CanvasResized(); 
}


// I'm having major problems with the canvas and qt-3.2
// qt-3.3 will fix it and the supposed fix is allready in qt-copy
// i'll not work any further on sprites, while i dont have qt-3.3 or a fresh qt-copy

void Canvas::slotSpriteShow()
{
	sprite->show();
}

void Canvas::slotSpriteHide()
{
	sprite->hide();
}

void Canvas::slotSpritePress()
{
}

void Canvas::slotSpriteChange(int x)
{
	sprite->setFrame(x);
	sprite->move(ROUND2INT(posX - sprite->width()/2), ROUND2INT(posY - sprite->height()/2));
}

void Canvas::slotPrint(QString text)
{
	QCanvasText* t = new QCanvasText(text, font, canvas);
	// text does not do the wrapping, never... sorry
	t->setColor( QColor(fgR, fgG, fgB) );
	t->move(ROUND2INT(posX), ROUND2INT(posY));
	t->show();
}

void Canvas::slotFontType(QString family, QString extra)
{
	font.setFamily(family);
	font.setBold( extra.contains("bold") > 0 );
	font.setItalic( extra.contains("italic") > 0 );
	font.setUnderline( extra.contains("underline") > 0 );
	font.setOverline( extra.contains("overline") > 0 );
	font.setStrikeOut( extra.contains("strikeout") > 0 );
}

void Canvas::slotFontSize(int px)
{
	font.setPixelSize(px);
}

void Canvas::slotWrapOn()
{
	wrap = true;
}

void Canvas::slotWrapOff()
{
	wrap = false;
}

void Canvas::slotReset()
{
	slotClearSpriteToo();
	initValues();
}

// END



// BEGIN private methods

void Canvas::initValues()
{
	// canvas size
	slotResizeCanvas( Settings::canvasWidth(), Settings::canvasHeight() );
	canvasWidth = Settings::canvasWidth();
	canvasHeight = Settings::canvasHeight();
	// colors
	canvas->setBackgroundColor( QColor(255, 255, 255) ); // background
	fgR = 0; // pencolor (forground)
	fgG = 0;
	fgB = 0;
	// pen, wrap, direction and font
	penWidth = 0;
	pen = true;
	wrap = true;
	direction = 0;
	font = QFont("serif", 18);
	// the position
	posX = canvasWidth / 2;
	posY = canvasHeight / 2;
	// construct the default sprite
	loadSpriteFrames("turtle");
	updateSpritePos();
	updateSpriteAngle();
	sprite->show();
}


void Canvas::lineShell(double xa, double ya, double xb, double yb)
{
	// line can fallback into this function in case of cutLoop == true
	cutLoop = false;
	// Reset the loop detection memory
	prevStartPos3 = prevStartPos2 = prevStartPos1 = prevEndPos3 = prevEndPos2 = prevEndPos1 = QPoint(0, 0);
	// and go!
	line(xa, ya, xb, yb);
}

void Canvas::line(double xa, double ya, double xb, double yb)
{
	QCanvasLine* l = new QCanvasLine(canvas);
	int intXa = ROUND2INT(xa);
	int intYa = ROUND2INT(ya);
	int intXb = ROUND2INT(xb);
	int intYb = ROUND2INT(yb);	
	l->setPoints(intXa, intYa, intXb, intYb);
	l->setPen( QPen(QColor(fgR, fgG, fgB), penWidth, SolidLine) );
	l->setZ(1);
	l->show();
	// kdDebug(0)<<"Canvas::line(); xa:"<<xa<<", ya:"<<ya<<", xb:"<<xb<<", yb:"<<yb<<endl;
	if ( wrap && !canvas->onCanvas( ROUND2INT(xb), ROUND2INT(yb) ) )
	{
		if (endlessLoop( QPoint(intXa, intYa), QPoint(intXb, intYb) ) == true) // detect for endless loop
		{
			slotCenter();
			kdDebug(0)<<"Canvas::line(): ENDLESS LOOP DETECTED, BROKE THE LOOP"<<endl;
			cutLoop = true;
			return;
		}
		
		QPoint translation = translationFactor(xa, ya, xb, yb);
		if (translation == QPoint(0, 0) )
		{
			// this could never happen in theory
			kdDebug(0)<<"Canvas::line(): ***********ERRORRR***********"<<endl;
			return;
		}
		// kdDebug(0)<<"Canvas::line(); translate by: <<tranlation<<endl;
		line (xa + translation.x() * canvasWidth, ya + translation.y() * canvasHeight, 
		      xb + translation.x() * canvasWidth, yb + translation.y() * canvasHeight);
		if (cutLoop == true)
		{
			// kdDebug(0)<<"Canvas::line(): cutLoop is set to TRUE!  ABORT LINE MISSION"<<endl;
			return;
		}
	}
}

bool Canvas::endlessLoop(QPoint begin, QPoint end)
{
	// kdDebug(0)<<"prevStartPos3: "<<prevStartPos3<<", prevStartPos2: "<<prevStartPos2<<", prevStartPos1: "<<prevStartPos1<<", prevStartPos0: "<<begin<<", prevEndPos3: "<<prevEndPos3<<", prevEndPos2: "<<prevEndPos2<<", prevEndPos1: "<<prevEndPos1<<", prevEndPos0: "<<end<<endl;
	if ( prevStartPos2 == begin && prevStartPos3 == prevStartPos1 &&
	     prevEndPos2   == end   && prevEndPos3   == prevEndPos1 )
	{
		// this is to break the horrible endless loop bug that i cannot fix...
		// i need more simple reproductions of this bug to really find it
		// for now i say it is something with QCanvas but i'm likely wrong on thisone
		// kdDebug(0)<<"Canvas::endlessLoop TRUE!!"<<endl;
		return true;
	}
	else
	{
		// kdDebug(0)<<"Canvas::endlessLoop FASLE!!"<<endl;
		prevStartPos3 = prevStartPos2;
		prevStartPos2 = prevStartPos1;
		prevStartPos1 = begin;
		prevEndPos3 = prevEndPos2;
		prevEndPos2 = prevEndPos1;
		prevEndPos1 = end;
		return false;
	}
	return false; // fallback will not be used
}

bool Canvas::pointInRange(double px, double py, double xa, double ya, double xb, double yb)
{
	if ( ( ( px >= xa && px <= xb ) || ( px <= xa && px >= xb ) ) &&
	     ( ( py >= ya && py <= yb ) || ( py <= ya && py >= yb ) ) ) return true;
	return false;
}

QPoint Canvas::offset(int x, int y)
{
	// This funktion makes is easy to read since deviding int's is a weird thing:
	// int x = 5 / 2,  outputs: x = 2,  with: 5 % 2 = 1 (the rest value) 
	if (x < 0) x = x - canvasWidth;
	if (y < 0) y = y - canvasHeight;
	QPoint offsetPoint( x / canvasWidth, y / canvasHeight);
	return offsetPoint;
}

QPoint Canvas::translationFactor(double xa, double ya, double xb, double yb)
{
	// this class returns a QPoint which can be used to properly 'wrap' a line
	QPoint crossPoint[4]; // under wicked circumstances we can need this
	                      // namely when crossing both corners, we have 4 bordercrossings
	QPoint translate[4];
	int i = 0;
	if ( (xb - xa) > -0.00001 && (xb - xa) < 0.00001 ) // check for an infinite tangent (direction coefficient)
	{
		i++;
		translate[i]  = QPoint( 0, 1 );
		crossPoint[i] = QPoint( ROUND2INT(xa), 0 );
		i++;
		translate[i]  = QPoint(0,-1 );
		crossPoint[i] = QPoint( ROUND2INT(xa), canvasHeight );
	}
	else
	{
		// Here we find out what crossing points the line has with canvas border lines (lines are ENDLESS here)
		
		// f(t) == (yb - ya) / (xb - xa) * t + ya - (A * xa) == A*t + B
		double A    = (yb - ya) / (xb - xa);
		double B    = ya - (A * xa);
		
		double x_sT = -B / A;                              // A * x_sT + B == 0   =>  x_sT == -B / A 
		double x_sB = ( (double)(canvasHeight) - B ) / A;  // A * x_sB + B == CH  =>  x_sB == (CH - B) / A
		double y_sL = B;                                   // A * 0 + B == y_sL   =>  y_sL == B
		double y_sR = ( A * (double)(canvasWidth) ) + B;   // A * CW + B == y_sR
		// kdDebug(0)<<"Canvas::translationFactor; rc:"<<A<<", xTop:"<<x_sT<<", xBot:"<<x_sB<<", yLft:"<<y_sL<<", yRft:"<<y_sR<<". "<<endl;
		
		// Here we find out what crossing points are on the borders AND on the linePIECES
		// pointInRange only checks wether the crossing point of the ENDLESS line is on the line PIECE.
		if ( 0 <= x_sT && x_sT <= canvasWidth && pointInRange(x_sT, 0, xa, ya, xb, yb) )
		{
			i++;
			translate[i]  = QPoint( 0, 1 );
			crossPoint[i] = QPoint( ROUND2INT(x_sT), 0 );
		}
		if ( 0 <= x_sB && x_sB <= canvasWidth && pointInRange(x_sB, canvasHeight, xa, ya, xb, yb) )
		{
			i++;
			translate[i]  = QPoint( 0,-1 );
			crossPoint[i] = QPoint( ROUND2INT(x_sB), ROUND2INT(canvasHeight) );
		} 
		if ( 0 <= y_sL && y_sL <= canvasHeight && pointInRange(0, y_sL, xa, ya, xb, yb) )
		{
			i++;
			translate[i]  = QPoint( 1, 0 );
			crossPoint[i] = QPoint( 0, ROUND2INT(y_sL) );
		}
		if ( 0 <= y_sR && y_sR <= canvasHeight && pointInRange(canvasWidth, y_sR, xa, ya, xb, yb) )
		{
			i++;
			translate[i]  = QPoint(-1, 0 );
			crossPoint[i] = QPoint( ROUND2INT(canvasWidth), ROUND2INT(y_sR) );
		}
	
		if ( i == 0 )
		{
			// kdDebug(0)<<"Canvas::translationFactor:  FIRST NO BORDER CROSSINGS DETECTED"<<endl;
			QPoint returnValue = QPoint(0, 0); // initiate the returnValue
			// Here a fallback if the line has no crossings points with any borders.
			// This mostly happens because of unlucky rounding, when this happens the line is nearly
			// crossing a corner of the canvas.
			// This code make sure the line is tranlated back onto the canvas.
			// The -2 and +2 was just something i learnt from examples... I HAVE NO PROOF FOR THIS!
			// This, luckily, allmost never happens.
			if ( -2 <= x_sT && x_sT <= (canvasWidth + 2) && pointInRange(x_sT, 0, xa, ya, xb, yb) )
			{
				returnValue = returnValue + QPoint(0, 1);
			}
			if ( -2 <= x_sB && x_sB <= (canvasWidth + 2) && pointInRange(x_sB, canvasHeight, xa, ya, xb, yb) )
			{
				returnValue = returnValue + QPoint(0,-1);
			} 
			if ( -2 <= y_sL && y_sL <= (canvasHeight + 2) && pointInRange(0, y_sL, xa, ya, xb, yb) )
			{
				returnValue = returnValue + QPoint(1, 0);
			}
			if ( -2 <= y_sR && y_sR <= (canvasHeight + 2)  && pointInRange(canvasWidth, y_sR, xa, ya, xb, yb) )
			{
				returnValue = returnValue + QPoint(-1, 0);
			}
		
			if ( returnValue == QPoint(0, 0) )
			{
				// kdDebug(0)<<"Canvas::translationFactor:  *****This shouldn't happen (1) *****"<<endl;
				// and this doesnt happen, that why +3 and -3 are ok values and the code above works.
			}
			return returnValue;
		}
	}
	
	QPoint returnValue = QPoint(0, 0); // a new returnValue QPoint gets inited
	if (i == 1)
	{
		// only one border crossing, this is normal when the start point
		// is within the canvas and no corners are crossed
		// kdDebug(0)<<"***only one border crossing!"<<endl;
		return translate[1];
	}
	if (i > 1) 
	{
		// more than one border crossing starting point if of the canvas
		// we now have to find out which crossing occurs 'first' to know how to translate the line
		// NOTE2SELF: the line does NOT have to start on the canvas!!
		QPoint endPos( ROUND2INT(xb), ROUND2INT(yb));
		int smallestSize = ( QPoint( ROUND2INT(xa), ROUND2INT(ya) ) - endPos ).manhattanLength();
		// smallestSize is initiated to the total size of the line
		for (int ii = 1; ii <= i; ii++)
		{
			int testSize = ( crossPoint[ii] - endPos ).manhattanLength(); // size till the crosspoint
			if (testSize < smallestSize) // if testSize is smaller then...
			{
				smallestSize = testSize;       // ...it becomes smallestSize
				returnValue = translate[ii];
				// and the returnValue is updated to the corresponing translaton factors
				// kdDebug(0)<<"Canvas::translationFactor: UPDATED"<<endl;
			}
			else if (testSize == smallestSize) // this only happens on corners
			{
				// kdDebug(0)<<"Canvas::translationFactor: CORNER EXCEPTION"<<endl;
				returnValue = QPoint(0, 0);
				if      (xb < 0)            returnValue = returnValue + QPoint( 1, 0);
				else if (xb > canvasWidth)  returnValue = returnValue + QPoint(-1, 0);
				
				if      (yb < 0)            returnValue = returnValue + QPoint( 0, 1);
				else if (yb > canvasHeight) returnValue = returnValue + QPoint( 0,-1);
				
				return returnValue;
			}
		}
		// kdDebug(0)<<"Canvas::translationFactor:  NOT RETURNED YET SO DOING IT NOW"<<endl;
		return returnValue;
	}
	// kdDebug(0)<<"Canvas::translationFactor:  *****This shouldn't happen (3) *****"<<endl;
	return returnValue;
}



// Sprite related methods:

void Canvas::loadSpriteFrames(QString name)
{
	// read the pixmaps name.0001.png, name.0002.png, ..., name.0035.png: the different rotations
	// #0000 for 0 or 360, #0001 for 10, #0002 for 20, ..., #0018 for 180, etc.
	
	// WARNING if the dir doesnt exists the app will crash!!!
	// This will be fixed in qt3.3 and in the current qt-copy
	QPixmap turtlePix = QPixmap(locate("data","kturtle/pics/turtle.0000.png") );
	if ( turtlePix.isNull() )
	{
		KMessageBox::sorry( this,
			i18n("The turtle picture could not be found. "
			     "Please check your installation."), i18n("Error") );
		return;
	}
	QString spritePath = locate("data","kturtle/pics/"+name+".0000.png");
	spritePath.remove(".0000.png");
	spriteFrames = new QCanvasPixmapArray(spritePath+".%1.png", 36);
	sprite = new QCanvasSprite(spriteFrames, canvas);
	sprite->setZ(250);
}

void Canvas::updateSpritePos()
{
	sprite->move( posX - ( sprite->width() / 2 ), posY - ( sprite->height() / 2 ), -1 );
}

void Canvas::updateSpriteAngle()
{
	// get the direction back on the 1st circle 
	while (direction < 0 || direction >= 360)
	{
		if (direction >= 360)  direction = direction - 360;
		if (direction <  0)    direction = direction + 360;
	}
	// convert to degrees, fix the direction, divide by 10 (for picnr), and round
	int i = (int) ( direction / 10 );
	// kdDebug(0)<<"########## direction = "<<direction<<";  int i = "<< i << endl;
	// int i = (int) ( ( ( (-direction * 180) / PI ) / 10) + 0.000000001 );
	sprite->setFrame(i);
	updateSpritePos(); // pixmaps of different rotations have different sizes, so refresh
}

// END

    
#include "canvas.moc"
