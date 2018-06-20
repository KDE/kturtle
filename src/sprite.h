/*
	Copyright (C) 2003-2008 Cies Breijs <cies AT kde DOT nl>

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

#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <QGraphicsSvgItem>


class Sprite : public QGraphicsSvgItem
{
	Q_OBJECT

	public:
		Sprite();

        double angle() const { return m_angle; }

		void setAngle(double degrees);
		void setSpriteSize(int pixels);

	private:
		double m_angle;
		double m_speed;
};

#endif  // _SPRITE_H_
