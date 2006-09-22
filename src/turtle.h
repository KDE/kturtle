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

#ifndef _TURTLE_H_
#define _TURTLE_H_

#include <QGraphicsItem>
#include <QObject>

class Turtle : public QObject, public QGraphicsItem
{
	Q_OBJECT

	public:
		Turtle();

		QRectF boundingRect() const;
		QPainterPath shape() const;
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

		double angle() { return _angle; }
		void setAngle(double angle);

// 	protected:
// 		void timerEvent(QTimerEvent *event);

	private:
		double _angle;
		double _speed;
};

#endif  // _TURTLE_H_
