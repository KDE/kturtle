/*
	SPDX-FileCopyrightText: 2003-2008 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
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
