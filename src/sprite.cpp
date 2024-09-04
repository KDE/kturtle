/*
    SPDX-FileCopyrightText: 2003-2008 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sprite.h"

#include <math.h>

#include <QSvgRenderer>

const int SPRITE_SIZE = 30;

Sprite::Sprite()
    : QGraphicsSvgItem(QStringLiteral(":turtle.svg"))
{
    m_angle = 0;
    m_speed = 0;

    setAngle(0);
}

void Sprite::setSpriteSize(int size)
{
    int w = renderer()->defaultSize().width();
    int h = renderer()->defaultSize().height();

    if (size <= 0 || w <= 0 || h <= 0)
        return;

    qreal s = (static_cast<qreal>(size)) / ((w > h) ? w : h);

    setTransform(QTransform::fromScale(s, s), true);
}

void Sprite::setAngle(double degrees)
{
    resetTransform();
    setSpriteSize(SPRITE_SIZE);

    // Default rotation is done with the top-left corner of the SVG as the rotation point,
    // but we want to the rotation to be around the SVG's center...
    // This is why this "translation" is needed before the actual rotation.
    QTransform transform =
        QTransform::fromTranslate(renderer()->defaultSize().width() * cos((degrees - 135) * M_PI / 180) * sqrt(static_cast<double>(2.0)) / 2,
                                  renderer()->defaultSize().height() * sin((degrees - 135) * M_PI / 180) * sqrt(static_cast<double>(2.0)) / 2);
    transform.rotate(degrees);
    setTransform(transform, true);
    m_angle = degrees;

    // TODO: Check if the update can be done more efficiently
    update();
}

#include "moc_sprite.cpp"
