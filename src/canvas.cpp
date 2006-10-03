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


#include <cmath>

#include <kdebug.h>
#include <klocale.h>
// #include <kmessagebox.h>

// #include "settings.h"
#include "canvas.h"



// this function is used in executer and canvas:
#define ROUND2INT(x) ( (x) >= 0 ? (int)( (x) + .5 ) : (int)( (x) - .5 ) )

static const double Pi = 3.14159265358979323846264338327950288419717;
const double DegToRad = Pi / 180.0;


Canvas::Canvas(QWidget *parent) : QGraphicsView(parent)
{
	// create a new scene for this view
	scene = new QGraphicsScene(parent);
	scene->setItemIndexMethod(QGraphicsScene::NoIndex);

	// foreground pen for drawing
	pen = new QPen();
	
	// the turtle shape
	turtle = new Sprite();
	turtle->setZValue(1);  // above the others
	scene->addItem(turtle);

	// set initial values
	initValues();

	setAlignment(Qt::AlignLeft|Qt::AlignTop);
	setInteractive(false);
	setMaximumSize(QSize((int)scene->width()+20, (int)scene->height()+20));
// 	setMinimumSize(QSize(scene->width(), scene->height()));

	// at last we assign the scene to the view
	setScene(scene);
}

Canvas::~Canvas()
{
	delete pen;
	delete turtle;
	delete scene;
}


void Canvas::initValues()
{
// 	QSettings settings("KDE", "KTurtle");
// 	int width  = qMin(qMax(settings.value("canvasWidth",  400).toInt(), 20), 10000);
// 	int height = qMin(qMax(settings.value("canvasHeight", 300).toInt(), 20), 10000);
	scene->setSceneRect(0, 0, 400, 300);
	turtle->setPos(scene->width()/2, scene->height()/2);
	scene->setBackgroundBrush(QBrush(Qt::white));
	pen->setColor(Qt::black);
	pen->setWidth(1);
	penWidthIsZero = false;
	
	textColor.setRgb(0, 0, 0) ;
}

void Canvas::resizeEvent(QResizeEvent* event)
{
	kDebug() << ">>>>>>" << event->size() << endl;

// 	if (event->newSize() >= scene->sceneRect()) {
// 		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
// 		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
// 	}
// 
// 	if (size().width() > scene->sceneRect().width())
// 		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
// 	else
// 		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
// 
// 	if (size().height() > scene->sceneRect().height())
// 		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
// 	else
// 		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	event->accept();
}

QColor Canvas::rgbDoublesToColor(double r, double g, double b)
{
	return QColor(qMin(qMax((int)r, 0), 255),
	              qMin(qMax((int)g, 0), 255),
	              qMin(qMax((int)b, 0), 255));
}

void Canvas::drawLine(double x1, double y1, double x2, double y2)
{
	if (penWidthIsZero) return;
	QGraphicsLineItem* line = new QGraphicsLineItem(QLineF(x1, y1, x2, y2), 0, scene);
	line->setPen(*pen);
	lines.append(line);
}


void Canvas::slotClear()
{
	QList<QGraphicsItem*> list = scene->items();
	foreach (QGraphicsItem* item, list) {
		// delete all but the turtle (who lives on a separate layer with z-value 1)
		if (item->zValue() != 1) delete item;
	}
}

void Canvas::slotGo(double x, double y)
{
	turtle->setPos(x, y);
}

void Canvas::slotGoX(double x)
{
	turtle->setPos(x, turtle->pos().y());
}

void Canvas::slotGoY(double y)
{
	turtle->setPos(turtle->pos().x(), y);
}

void Canvas::slotForward(double x)
{
	double x2 = turtle->pos().x() + ( x * sin(turtle->angle() * DegToRad) );
	double y2 = turtle->pos().y() - ( x * cos(turtle->angle() * DegToRad) );
	drawLine(turtle->pos().x(), turtle->pos().y(), x2, y2);
	slotGo(x2, y2);
}

void Canvas::slotBackward(double x)
{
	double x2 = turtle->pos().x() - ( x * sin(turtle->angle() * DegToRad) );
	double y2 = turtle->pos().y() + ( x * cos(turtle->angle() * DegToRad) );
	drawLine(turtle->pos().x(), turtle->pos().y(), x2, y2);
	slotGo(x2, y2);
}

void Canvas::slotDirection(double deg)
{
	turtle->setAngle(deg);
}

void Canvas::slotTurnLeft(double deg)
{
	turtle->setAngle(turtle->angle() - deg);
}

void Canvas::slotTurnRight(double deg)
{
	turtle->setAngle(turtle->angle() + deg);
}

void Canvas::slotCenter()
{
	slotGo(scene->width()/2, scene->height()/2);
}

void Canvas::slotPenWidth(double width)
{
	int w = qMax(ROUND2INT(width), 0);
	if (w == 0) {
		penWidthIsZero = true;
		return;
	} else {
		penWidthIsZero = false;
		if (w == 1) pen->setWidth(0);
		else        pen->setWidthF(width);
	}
}

void Canvas::slotPenUp()
{
	pen->setStyle(Qt::NoPen);
}

void Canvas::slotPenDown()
{
	pen->setStyle(Qt::SolidLine);
}

void Canvas::slotPenColor(double r, double g, double b)
{
	pen->setColor(rgbDoublesToColor(r, g, b));
	
	textColor.setRgb((int)r, (int)g, (int)b);
}

void Canvas::slotCanvasColor(double r, double g, double b)
{
	scene->setBackgroundBrush(QBrush(rgbDoublesToColor(r, g, b)));
}

void Canvas::slotCanvasSize(double x, double y)
{
	x = x;  // get rid of warning
	y = y;
// 	if (x <= 0 || y <= 0)
// 	{
// 		// TODO put error message
// 		x = 100;
// 		y = 100;
// 	}
// 	canvasWidth = x;
// 	canvasHeight = y;
// 	canvas->resize(x, y);
// 	emit CanvasResized(); 
}

void Canvas::slotSpriteShow()
{
	turtle->show();
}

void Canvas::slotSpriteHide()
{
	turtle->hide();
}

void Canvas::slotPrint(const QString& text)
{
	QGraphicsTextItem *ti = new QGraphicsTextItem(text.mid(1, text.length() - 2), 0, scene);
// 	ti->setDefaultTextColor(textColor);
	ti->setFont(textFont);
	ti->setPos(turtle->pos().x(), turtle->pos().y());
}

void Canvas::slotFontType(const QString& family, const QString& extra)
{
	textFont.setFamily(family);
	textFont.setBold(extra.contains(i18n("bold")) > 0);
	textFont.setItalic(extra.contains(i18n("italic")) > 0);
	textFont.setUnderline(extra.contains(i18n("underline")) > 0);
	textFont.setOverline(extra.contains(i18n("overline")) > 0);
	textFont.setStrikeOut(extra.contains(i18n("strikeout")) > 0);
}

void Canvas::slotFontSize(double px)
{
	textFont.setPixelSize((int)px);
}

void Canvas::slotWrapOn()
{
// 	wrap = true;
}

void Canvas::slotWrapOff()
{
// 	wrap = false;
}

void Canvas::slotReset()
{
	slotClear();
	initValues();
}



// 
// // END
// 
// 
// 
// // BEGIN private methods
// 
// void Canvas::initValues()
// {
// 	// canvas size
// 	slotResizeCanvas( Settings::canvasWidth(), Settings::canvasHeight() );
// 	canvasWidth = Settings::canvasWidth();
// 	canvasHeight = Settings::canvasHeight();
// 	// colors
// 	canvas->setBackgroundColor( QColor(255, 255, 255) ); // background
// 	fgR = 0; // pencolor (forground)
// 	fgG = 0;
// 	fgB = 0;
// 	// pen, wrap, direction and font
// 	penWidth = 0;
// 	pen = true;
// 	wrap = true;
// 	direction = 0;
// 	font = QFont("serif", 18);
// 	// the position
// 	posX = canvasWidth / 2;
// 	posY = canvasHeight / 2;
// 	// construct the default sprite
// 	loadSpriteFrames("turtle");
// 	updateSpritePos();
// 	updateSpriteAngle();
// 	sprite->show();
// }
// 
// 
// void Canvas::lineShell(double xa, double ya, double xb, double yb)
// {
// 	// line can fallback into this function in case of cutLoop == true
// 	cutLoop = false;
// 	// Reset the loop detection memory
// 	prevStartPos3 = prevStartPos2 = prevStartPos1 = prevEndPos3 = prevEndPos2 = prevEndPos1 = QPoint(0, 0);
// 	// and go!
// 	line(xa, ya, xb, yb);
// }
// 
// void Canvas::line(double xa, double ya, double xb, double yb)
// {
// 	Q3CanvasLine* l = new Q3CanvasLine(canvas);
// 	int intXa = ROUND2INT(xa);
// 	int intYa = ROUND2INT(ya);
// 	int intXb = ROUND2INT(xb);
// 	int intYb = ROUND2INT(yb);	
// 	l->setPoints(intXa, intYa, intXb, intYb);
// 	l->setPen( QPen(QColor(fgR, fgG, fgB), penWidth, Qt::SolidLine) );
// 	l->setZ(1);
// 	l->show();
// 	// kdDebug(0)<<"Canvas::line(); xa:"<<xa<<", ya:"<<ya<<", xb:"<<xb<<", yb:"<<yb<<endl;
// 	if ( wrap && !canvas->onCanvas( ROUND2INT(xb), ROUND2INT(yb) ) )
// 	{
// 		if (endlessLoop( QPoint(intXa, intYa), QPoint(intXb, intYb) ) == true) // detect for endless loop
// 		{
// 			slotCenter();
// 			kdDebug(0)<<"Canvas::line(): ENDLESS LOOP DETECTED, BROKE THE LOOP"<<endl;
// 			cutLoop = true;
// 			return;
// 		}
// 		
// 		QPoint translation = translationFactor(xa, ya, xb, yb);
// 		if (translation == QPoint(0, 0) )
// 		{
// 			// this could never happen in theory
// 			kdDebug(0)<<"Canvas::line(): ***********ERRORRR***********"<<endl;
// 			return;
// 		}
// 		// kdDebug(0)<<"Canvas::line(); translate by: <<translation<<endl;
// 		line (xa + translation.x() * canvasWidth, ya + translation.y() * canvasHeight, 
// 		      xb + translation.x() * canvasWidth, yb + translation.y() * canvasHeight);
// 		if (cutLoop == true)
// 		{
// 			// kdDebug(0)<<"Canvas::line(): cutLoop is set to TRUE!  ABORT LINE MISSION"<<endl;
// 			return;
// 		}
// 	}
// }
// 
// bool Canvas::endlessLoop(QPoint begin, QPoint end)
// {
// 	// kdDebug(0)<<"prevStartPos3: "<<prevStartPos3<<", prevStartPos2: "<<prevStartPos2<<", prevStartPos1: "<<prevStartPos1<<", prevStartPos0: "<<begin<<", prevEndPos3: "<<prevEndPos3<<", prevEndPos2: "<<prevEndPos2<<", prevEndPos1: "<<prevEndPos1<<", prevEndPos0: "<<end<<endl;
// 	if ( prevStartPos2 == begin && prevStartPos3 == prevStartPos1 &&
// 	     prevEndPos2   == end   && prevEndPos3   == prevEndPos1 )
// 	{
// 		// this is to break the horrible endless loop bug that I cannot fix...
// 		// I need more simple reproductions of this bug to really find it
// 		// for now I say it is something with QCanvas but I'm likely wrong on thisone
// 		// kdDebug(0)<<"Canvas::endlessLoop TRUE!!"<<endl;
// 		return true;
// 	}
// 	else
// 	{
// 		// kdDebug(0)<<"Canvas::endlessLoop FASLE!!"<<endl;
// 		prevStartPos3 = prevStartPos2;
// 		prevStartPos2 = prevStartPos1;
// 		prevStartPos1 = begin;
// 		prevEndPos3 = prevEndPos2;
// 		prevEndPos2 = prevEndPos1;
// 		prevEndPos1 = end;
// 		return false;
// 	}
// 	return false; // fallback will not be used
// }
// 
// bool Canvas::pointInRange(double px, double py, double xa, double ya, double xb, double yb)
// {
// 	if ( ( ( px >= xa && px <= xb ) || ( px <= xa && px >= xb ) ) &&
// 	     ( ( py >= ya && py <= yb ) || ( py <= ya && py >= yb ) ) ) return true;
// 	return false;
// }
// 
// QPoint Canvas::offset(int x, int y)
// {
// 	// This funktion makes is easy to read since deviding int's is a weird thing:
// 	// int x = 5 / 2,  outputs: x = 2,  with: 5 % 2 = 1 (the rest value) 
// 	if (x < 0) x = x - canvasWidth;
// 	if (y < 0) y = y - canvasHeight;
// 	QPoint offsetPoint( x / canvasWidth, y / canvasHeight);
// 	return offsetPoint;
// }
// 
// QPoint Canvas::translationFactor(double xa, double ya, double xb, double yb)
// {
// 	// this class returns a QPoint which can be used to properly 'wrap' a line
// 	QPoint crossPoint[4]; // under wicked circumstances we can need this
// 	                      // namely when crossing both corners, we have 4 bordercrossings
// 	QPoint translate[4];
// 	int i = 0;
// 	if ( (xb - xa) > -0.00001 && (xb - xa) < 0.00001 ) // check for an infinite tangent (direction coefficient)
// 	{
// 		i++;
// 		translate[i]  = QPoint( 0, 1 );
// 		crossPoint[i] = QPoint( ROUND2INT(xa), 0 );
// 		i++;
// 		translate[i]  = QPoint(0,-1 );
// 		crossPoint[i] = QPoint( ROUND2INT(xa), canvasHeight );
// 	}
// 	else
// 	{
// 		// Here we find out what crossing points the line has with canvas border lines (lines are ENDLESS here)
// 		
// 		// f(t) == (yb - ya) / (xb - xa) * t + ya - (A * xa) == A*t + B
// 		double A    = (yb - ya) / (xb - xa);
// 		double B    = ya - (A * xa);
// 		
// 		double x_sT = -B / A;                              // A * x_sT + B == 0   =>  x_sT == -B / A 
// 		double x_sB = ( (double)(canvasHeight) - B ) / A;  // A * x_sB + B == CH  =>  x_sB == (CH - B) / A
// 		double y_sL = B;                                   // A * 0 + B == y_sL   =>  y_sL == B
// 		double y_sR = ( A * (double)(canvasWidth) ) + B;   // A * CW + B == y_sR
// 		// kdDebug(0)<<"Canvas::translationFactor; rc:"<<A<<", xTop:"<<x_sT<<", xBot:"<<x_sB<<", yLft:"<<y_sL<<", yRft:"<<y_sR<<". "<<endl;
// 		
// 		// Here we find out what crossing points are on the borders AND on the linePIECES
// 		// pointInRange only checks wether the crossing point of the ENDLESS line is on the line PIECE.
// 		if ( 0 <= x_sT && x_sT <= canvasWidth && pointInRange(x_sT, 0, xa, ya, xb, yb) )
// 		{
// 			i++;
// 			translate[i]  = QPoint( 0, 1 );
// 			crossPoint[i] = QPoint( ROUND2INT(x_sT), 0 );
// 		}
// 		if ( 0 <= x_sB && x_sB <= canvasWidth && pointInRange(x_sB, canvasHeight, xa, ya, xb, yb) )
// 		{
// 			i++;
// 			translate[i]  = QPoint( 0,-1 );
// 			crossPoint[i] = QPoint( ROUND2INT(x_sB), ROUND2INT(canvasHeight) );
// 		} 
// 		if ( 0 <= y_sL && y_sL <= canvasHeight && pointInRange(0, y_sL, xa, ya, xb, yb) )
// 		{
// 			i++;
// 			translate[i]  = QPoint( 1, 0 );
// 			crossPoint[i] = QPoint( 0, ROUND2INT(y_sL) );
// 		}
// 		if ( 0 <= y_sR && y_sR <= canvasHeight && pointInRange(canvasWidth, y_sR, xa, ya, xb, yb) )
// 		{
// 			i++;
// 			translate[i]  = QPoint(-1, 0 );
// 			crossPoint[i] = QPoint( ROUND2INT(canvasWidth), ROUND2INT(y_sR) );
// 		}
// 	
// 		if ( i == 0 )
// 		{
// 			// kdDebug(0)<<"Canvas::translationFactor:  FIRST NO BORDER CROSSINGS DETECTED"<<endl;
// 			QPoint returnValue = QPoint(0, 0); // initiate the returnValue
// 			// Here a fallback if the line has no crossings points with any borders.
// 			// This mostly happens because of unlucky rounding, when this happens the line is nearly
// 			// crossing a corner of the canvas.
// 			// This code make sure the line is tranlated back onto the canvas.
// 			// The -2 and +2 was just something I learned from examples... I HAVE NO PROOF FOR THIS!
// 			// This, luckily, almost never happens.
// 			if ( -2 <= x_sT && x_sT <= (canvasWidth + 2) && pointInRange(x_sT, 0, xa, ya, xb, yb) )
// 			{
// 				returnValue = returnValue + QPoint(0, 1);
// 			}
// 			if ( -2 <= x_sB && x_sB <= (canvasWidth + 2) && pointInRange(x_sB, canvasHeight, xa, ya, xb, yb) )
// 			{
// 				returnValue = returnValue + QPoint(0,-1);
// 			} 
// 			if ( -2 <= y_sL && y_sL <= (canvasHeight + 2) && pointInRange(0, y_sL, xa, ya, xb, yb) )
// 			{
// 				returnValue = returnValue + QPoint(1, 0);
// 			}
// 			if ( -2 <= y_sR && y_sR <= (canvasHeight + 2)  && pointInRange(canvasWidth, y_sR, xa, ya, xb, yb) )
// 			{
// 				returnValue = returnValue + QPoint(-1, 0);
// 			}
// 		
// 			if ( returnValue == QPoint(0, 0) )
// 			{
// 				// kdDebug(0)<<"Canvas::translationFactor:  *****This shouldn't happen (1) *****"<<endl;
// 				// and this doesn't happen, that why +3 and -3 are ok values and the code above works.
// 			}
// 			return returnValue;
// 		}
// 	}
// 	
// 	QPoint returnValue = QPoint(0, 0); // a new returnValue QPoint gets inited
// 	if (i == 1)
// 	{
// 		// only one border crossing, this is normal when the start point
// 		// is within the canvas and no corners are crossed
// 		// kdDebug(0)<<"***only one border crossing!"<<endl;
// 		return translate[1];
// 	}
// 	if (i > 1) 
// 	{
// 		// more than one border crossing starting point if of the canvas
// 		// we now have to find out which crossing occurs 'first' to know how to translate the line
// 		// NOTE2SELF: the line does NOT have to start on the canvas!!
// 		QPoint endPos( ROUND2INT(xb), ROUND2INT(yb));
// 		int smallestSize = ( QPoint( ROUND2INT(xa), ROUND2INT(ya) ) - endPos ).manhattanLength();
// 		// smallestSize is initiated to the total size of the line
// 		for (int ii = 1; ii <= i; ii++)
// 		{
// 			int testSize = ( crossPoint[ii] - endPos ).manhattanLength(); // size till the crosspoint
// 			if (testSize < smallestSize) // if testSize is smaller then...
// 			{
// 				smallestSize = testSize;       // ...it becomes smallestSize
// 				returnValue = translate[ii];
// 				// and the returnValue is updated to the corresponing translaton factors
// 				// kdDebug(0)<<"Canvas::translationFactor: UPDATED"<<endl;
// 			}
// 			else if (testSize == smallestSize) // this only happens on corners
// 			{
// 				// kdDebug(0)<<"Canvas::translationFactor: CORNER EXCEPTION"<<endl;
// 				returnValue = QPoint(0, 0);
// 				if      (xb < 0)            returnValue = returnValue + QPoint( 1, 0);
// 				else if (xb > canvasWidth)  returnValue = returnValue + QPoint(-1, 0);
// 				
// 				if      (yb < 0)            returnValue = returnValue + QPoint( 0, 1);
// 				else if (yb > canvasHeight) returnValue = returnValue + QPoint( 0,-1);
// 				
// 				return returnValue;
// 			}
// 		}
// 		// kdDebug(0)<<"Canvas::translationFactor:  NOT RETURNED YET SO DOING IT NOW"<<endl;
// 		return returnValue;
// 	}
// 	// kdDebug(0)<<"Canvas::translationFactor:  *****This shouldn't happen (3) *****"<<endl;
// 	return returnValue;
// }
// 
// 
// 
// // Sprite related methods:
// 
// void Canvas::loadSpriteFrames(QString name)
// {
// 	// read the pixmaps name.0001.png, name.0002.png, ..., name.0035.png: the different rotations
// 	// #0000 for 0 or 360, #0001 for 10, #0002 for 20, ..., #0018 for 180, etc.
// 	
// 	// WARNING if the dir doesn't exists the app will crash!!!
// 	// This will be fixed in qt3.3 and in the current qt-copy
// 	QPixmap turtlePix = QPixmap(locate("data","kturtle/pics/turtle.0000.png") );
// 	if ( turtlePix.isNull() )
// 	{
// 		KMessageBox::sorry( this,
// 			i18n("The turtle picture could not be found. "
// 			     "Please check your installation."), i18n("Error") );
// 		return;
// 	}
// 	QString spritePath = locate("data","kturtle/pics/"+name+".0000.png");
// 	spritePath.remove(".0000.png");
// 	spriteFrames = new Q3CanvasPixmapArray(spritePath+".%1.png", 36);
// 	sprite = new Q3CanvasSprite(spriteFrames, canvas);
// 	sprite->setZ(250);
// }
// 
// void Canvas::updateSpritePos()
// {
// 	sprite->move( posX - ( sprite->width() / 2 ), posY - ( sprite->height() / 2 ), -1 );
// }
// 
// void Canvas::updateSpriteAngle()
// {
// 	// get the direction back on the 1st circle 
// 	while (direction < 0 || direction >= 360)
// 	{
// 		if (direction >= 360)  direction = direction - 360;
// 		if (direction <  0)    direction = direction + 360;
// 	}
// 	// convert to degrees, fix the direction, divide by 10 (for picnr), and round
// 	int i = (int) ( direction / 10 );
// 	// kdDebug(0)<<"########## direction = "<<direction<<";  int i = "<< i << endl;
// 	// int i = (int) ( ( ( (-direction * 180) / PI ) / 10) + 0.000000001 );
// 	sprite->setFrame(i);
// 	updateSpritePos(); // pixmaps of different rotations have different sizes, so refresh
// }
// 


#include "canvas.moc"
