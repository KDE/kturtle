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

#include "turtle.h"

#include <cmath>

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>



static const double Pi = 3.14159265358979323846264338327950288419717;
static double TwoPi = 2.0 * Pi;


// static qreal normalizeAngle(qreal angle)
// {
// 	while (angle < 0)     angle += TwoPi;
// 	while (angle > TwoPi) angle -= TwoPi;
// 	return angle;
// }


Turtle::Turtle()
{
	_angle = 0;
	_speed = 0;
// 	startTimer(1000 / 33);
}

QRectF Turtle::boundingRect() const
{
	return QRectF(-18.5, -28.5, 37, 47);
}

QPainterPath Turtle::shape() const
{
	QPainterPath path;
	path.addRect(-18, -28, 36, 46);
	return path;
}

void Turtle::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	painter->setBrush(Qt::darkGreen);
	// Legs
	painter->drawEllipse(-18, -18, 10, 10);
	painter->drawEllipse(  8, -18, 10, 10);
	painter->drawEllipse(-18,   8, 10, 10);
	painter->drawEllipse(  8,   8, 10, 10);
	// Head
	painter->drawEllipse( -7, -28, 14, 14);
	// Body
	painter->drawEllipse(-15, -15, 30, 30);
}


void Turtle::setAngle(double deg) {
	rotate(deg - _angle);
	_angle = deg;
}


// void Turtle::timerEvent(QTimerEvent *)
// {
// 	speed += (-50 + rand() % 100) / 100.0;
// 	setPos(mapToParent(0, -(3 + sin(speed) * 3)));
// }


#include "turtle.moc"
