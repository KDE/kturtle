/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

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

#include "canvas.h"

#include <cmath>

#include <QResizeEvent>

#include <kdebug.h>
#include <klocale.h>
// #include <kmessagebox.h>

// #include "settings.h"


// this function is used in executer and canvas:
#define ROUND2INT(x) ( (x) >= 0 ? (int)( (x) + .5 ) : (int)( (x) - .5 ) )

static const double Pi = 3.14159265358979323846264338327950288419717;
const double DegToRad = Pi / 180.0;

Canvas::Canvas(QWidget *parent) : QGraphicsView(parent)
{
	// create a new scene for this view
	scene = new QGraphicsScene(parent);
	scene->setItemIndexMethod(QGraphicsScene::NoIndex);
	//scene->setSceneRect(-200, -200, 400, 400);  // (-50, -50, 50, 50);

	setCacheMode(CacheBackground);
	setRenderHint(QPainter::Antialiasing);
	setTransformationAnchor(AnchorUnderMouse);
	setResizeAnchor(AnchorViewCenter);
	setMinimumSize(100, 100);

	// foreground pen for drawing
	pen = new QPen();
	
	// font
	textFont = new QFont();
	
	// Canvas area marker
	canvasFrame = new QGraphicsRectItem();
	canvasFrame->setZValue(-10000);
	scene->addItem(canvasFrame);
	

	// the turtle shape
	turtle = new Sprite();
	turtle->setZValue(1);  // above the others
	scene->addItem(turtle);

	// set initial values
	initValues();
	setInteractive(false);

	// at last we assign the scene to the view
	setScene(scene);
}

Canvas::~Canvas()
{
	delete pen;
	delete turtle;
	delete canvasFrame;
	delete textFont;
	delete scene;
}


void Canvas::initValues()
{
// 	QSettings settings("KDE", "KTurtle");
// 	int width  = qMin(qMax(settings.value("canvasWidth",  400).toInt(), 20), 10000);
// 	int height = qMin(qMax(settings.value("canvasHeight", 300).toInt(), 20), 10000);
	scene->setSceneRect(QRectF(0,0,400,400));
	canvasFrame->setBrush(QBrush());
	canvasFrame->setRect(scene->sceneRect());
	fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
	turtle->setPos(200, 200);
	turtle->setAngle(0);
	scene->setBackgroundBrush(QBrush(Qt::white));
	pen->setColor(Qt::black);
	pen->setWidth(1);
	penWidthIsZero = false;
	textColor.setRgb(0, 0, 0) ;
	delete textFont;
	textFont = new QFont();
	//Reset our pen to the default position
	slotPenDown();
}

void Canvas::resizeEvent(QResizeEvent* event)
{
	//kDebug() << ">>>>>>" << event->size();
	//scene->setSceneRect(scene->itemsBoundingRect());
	fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
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
		if ((item->zValue() != 1)&&(item->zValue() !=-10000)) delete item;
	}
}

void Canvas::slotForward(double x)
{
	double x2 = turtle->pos().x() + (x * sin(turtle->angle() * DegToRad));
	double y2 = turtle->pos().y() - (x * cos(turtle->angle() * DegToRad));
	//drawLine(turtle->pos().x(), turtle->pos().y(), x2, y2);
	slotGo(x2, y2);
}

void Canvas::slotBackward(double x)
{
	double x2 = turtle->pos().x() - ( x * sin(turtle->angle() * DegToRad) );
	double y2 = turtle->pos().y() + ( x * cos(turtle->angle() * DegToRad) );
	//drawLine(turtle->pos().x(), turtle->pos().y(), x2, y2);
	slotGo(x2, y2);
}

void Canvas::slotCenter()
{
	slotGo(scene->width()/2, scene->height()/2);
}

void Canvas::slotGo(double x, double y)
{
	drawLine(turtle->pos().x(), turtle->pos().y(), x, y);
	turtle->setPos(x, y);
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

void Canvas::slotPenColor(double r, double g, double b)
{
	pen->setColor(rgbDoublesToColor(r, g, b));
	textColor.setRgb((int)r, (int)g, (int)b);
}

void Canvas::slotCanvasColor(double r, double g, double b)
{
	//scene->setBackgroundBrush(QBrush(rgbDoublesToColor(r, g, b)));
	canvasFrame->setBrush(QBrush(rgbDoublesToColor(r, g, b)));
}

void Canvas::slotCanvasSize(double r, double g)
{
	scene->setSceneRect(QRectF(0,0,r,g));
	canvasFrame->setRect(scene->sceneRect());
	fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void Canvas::slotPrint(const QString& text)
{
	QGraphicsTextItem *ti = new QGraphicsTextItem(text, 0, scene);
// 	ti->setDefaultTextColor(textColor);
	ti->setFont(*textFont);
	ti->rotate(turtle->angle());
	ti->setPos(turtle->pos().x(), turtle->pos().y());
}

void Canvas::slotFontType(const QString& family, const QString& extra)
{
	textFont->setFamily(family);
	textFont->setBold(extra.contains(i18n("bold")) > 0);
	textFont->setItalic(extra.contains(i18n("italic")) > 0);
	textFont->setUnderline(extra.contains(i18n("underline")) > 0);
	textFont->setOverline(extra.contains(i18n("overline")) > 0);
	textFont->setStrikeOut(extra.contains(i18n("strikeout")) > 0);
}

void Canvas::slotReset()
{
	slotClear();
	initValues();
}

void Canvas::wheelEvent(QWheelEvent *event)
{
	scaleView(pow((double)2, -event->delta() / 240.0));
}

void Canvas::scaleView(double scaleFactor)
{
	qreal factor = matrix().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
	if (factor < 0.07 || factor > 100) return;
	scale(scaleFactor, scaleFactor);
}

void Canvas::getX(double& value)
{
	value = turtle->pos().x();
}

void Canvas::getY(double& value)
{
	value = turtle->pos().y();
}

#include "canvas.moc"
