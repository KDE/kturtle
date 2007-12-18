/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the Free
	Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.
*/

#include "sprite.h"

#include <math.h>

#include <QSvgRenderer>


const int SPRITE_SIZE = 30;

Sprite::Sprite()
	: QGraphicsSvgItem(":turtle.svg")
{
	_angle = 0;	
	_speed = 0;

	setAngle(0);
}

void Sprite::setSpriteSize(int size_in_pixels)
{
	int w = renderer()->defaultSize().width();
	int h = renderer()->defaultSize().height();
	
	if (size_in_pixels <= 0 || w <= 0 || h <= 0) return;
	
	qreal s = ((qreal)size_in_pixels) / ((w > h) ? w : h);
	
	scale(s, s);
}

void Sprite::setAngle(double deg)
{
	resetMatrix();
	setSpriteSize(SPRITE_SIZE);
	
	// Default rotation is done with the top-left corner of the SVG as the rotation point,
	// but we want to the rotation to be around the SVG's center...
	// This is why this "translation" is needed before the actual rotation.
	translate(
		renderer()->defaultSize().width()  * cos((deg-135) * M_PI/180) * sqrt(2)/2,
		renderer()->defaultSize().height() * sin((deg-135) * M_PI/180) * sqrt(2)/2
	);
	rotate(deg);
	_angle = deg;
}


#include "sprite.moc"

