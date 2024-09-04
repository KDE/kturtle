/*
    SPDX-FileCopyrightText: 2003-2006 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "canvas.h"

#include <cmath>

#include <QResizeEvent>
#include <QSvgGenerator>
#include <QWheelEvent>
#include <QtMath>

#include <KLocalizedString>

int kTurtleZValue = 1;
int kCanvasFrameZValue = -10000;
int kCanvasMargin = 20;

Canvas::Canvas(QWidget *parent)
    : QGraphicsView(parent)
{
    // create a new scene for this view
    _scene = new QGraphicsScene(parent);
    //_scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    //_scene->setSceneRect(-200, -200, 400, 400);  // (-50, -50, 50, 50);

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
    canvasFrame->setZValue(kCanvasFrameZValue);
    _scene->addItem(canvasFrame);

    // the turtle shape
    turtle = new Sprite();
    turtle->setZValue(kTurtleZValue); // above the others
    _scene->addItem(turtle);

    // set initial values
    initValues();
    setInteractive(false);

    // at last we assign the scene to the view
    setScene(_scene);
}

Canvas::~Canvas()
{
    delete pen;
    delete turtle;
    delete canvasFrame;
    delete textFont;
    delete _scene;
}

void Canvas::initValues()
{
    _scene->setSceneRect(QRectF(0, 0, 400, 400));
    canvasFrame->setBrush(QBrush());
    canvasFrame->setRect(_scene->sceneRect());
    fitInView(_scene->sceneRect().adjusted(kCanvasMargin * -1, kCanvasMargin * -1, kCanvasMargin, kCanvasMargin), Qt::KeepAspectRatio);
    turtle->setPos(200, 200);
    turtle->setAngle(0);
    _scene->setBackgroundBrush(QBrush(Qt::white));
    pen->setColor(Qt::black);
    pen->setWidth(1);
    penWidthIsZero = false;
    textColor.setRgb(0, 0, 0);
    delete textFont;
    textFont = new QFont();
    // Reset our pen to the default position
    slotPenDown();
    // Show turtle, might have been hidden in the last run
    slotSpriteShow();
}

void Canvas::resizeEvent(QResizeEvent *event)
{
    fitInView(_scene->sceneRect().adjusted(kCanvasMargin * -1, kCanvasMargin * -1, kCanvasMargin, kCanvasMargin), Qt::KeepAspectRatio);
    event->accept();
}

QColor Canvas::rgbDoublesToColor(double r, double g, double b)
{
    return QColor(qMin(qMax(static_cast<int>(r), 0), 255), qMin(qMax(static_cast<int>(g), 0), 255), qMin(qMax(static_cast<int>(b), 0), 255));
}

void Canvas::drawLine(double x1, double y1, double x2, double y2)
{
    if (penWidthIsZero)
        return;
    QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(x1, y1, x2, y2), nullptr);
    _scene->addItem(line);
    line->setPen(*pen);
    lines.append(line);
}

void Canvas::slotClear()
{
    const QList<QGraphicsItem *> list = _scene->items();
    for (QGraphicsItem *item : list) {
        // delete all but the turtle (who lives on a separate layer with z-value 1)
        if ((item->zValue() != kTurtleZValue) && (item->zValue() != kCanvasFrameZValue))
            delete item;
    }
}

void Canvas::slotForward(double x)
{
    double x2 = turtle->pos().x() + (x * std::sin(qDegreesToRadians(turtle->angle())));
    double y2 = turtle->pos().y() - (x * std::cos(qDegreesToRadians(turtle->angle())));
    drawLine(turtle->pos().x(), turtle->pos().y(), x2, y2);
    slotGo(x2, y2);
}

void Canvas::slotBackward(double x)
{
    double x2 = turtle->pos().x() - (x * std::sin(qDegreesToRadians(turtle->angle())));
    double y2 = turtle->pos().y() + (x * std::cos(qDegreesToRadians(turtle->angle())));
    drawLine(turtle->pos().x(), turtle->pos().y(), x2, y2);
    slotGo(x2, y2);
}

void Canvas::slotCenter()
{
    slotGo(_scene->width() / 2, _scene->height() / 2);
}

void Canvas::slotPenWidth(double width)
{
    int w = qMax(static_cast<int>(round(width)), 0);
    if (w == 0) {
        penWidthIsZero = true;
        return;
    } else {
        penWidthIsZero = false;
        if (w == 1)
            pen->setWidth(0);
        else
            pen->setWidthF(width);
    }
}

void Canvas::slotPenColor(double r, double g, double b)
{
    pen->setColor(rgbDoublesToColor(r, g, b));
    textColor.setRgb(static_cast<int>(r), static_cast<int>(g), static_cast<int>(b));
}

void Canvas::slotCanvasColor(double r, double g, double b)
{
    //_scene->setBackgroundBrush(QBrush(rgbDoublesToColor(r, g, b)));
    canvasFrame->setBrush(QBrush(rgbDoublesToColor(r, g, b)));
}

void Canvas::slotCanvasSize(double r, double g)
{
    _scene->setSceneRect(QRectF(0, 0, r, g));
    canvasFrame->setRect(_scene->sceneRect());
    fitInView(_scene->sceneRect(), Qt::KeepAspectRatio);
}

void Canvas::slotPrint(const QString &text)
{
    QGraphicsTextItem *ti = new QGraphicsTextItem(text, nullptr);
    _scene->addItem(ti);
    // 	ti->setDefaultTextColor(textColor);
    ti->setFont(*textFont);
    ti->setTransform(QTransform().rotate(turtle->angle()), true);
    ti->setPos(turtle->pos().x(), turtle->pos().y());
    ti->setDefaultTextColor(textColor);
}

void Canvas::slotFontType(const QString &family, const QString &extra)
{
    textFont->setFamily(family);
    textFont->setBold(extra.contains(i18n("bold")));
    textFont->setItalic(extra.contains(i18n("italic")));
    textFont->setUnderline(extra.contains(i18n("underline")));
    textFont->setOverline(extra.contains(i18n("overline")));
    textFont->setStrikeOut(extra.contains(i18n("strikeout")));
}

void Canvas::slotReset()
{
    slotClear();
    initValues();
}

void Canvas::wheelEvent(QWheelEvent *event)
{
    scaleView(std::pow(static_cast<double>(2.0), -event->angleDelta().y() / 240.0));
}

void Canvas::scaleView(double scaleFactor)
{
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.07 || factor > 100)
        return;
    scale(scaleFactor, scaleFactor);
}

void Canvas::getX(double &value)
{
    value = turtle->pos().x();
}

void Canvas::getY(double &value)
{
    value = turtle->pos().y();
}

void Canvas::getDirection(double &value)
{
    value = fmod(turtle->angle(), 360);
}

QImage Canvas::getPicture()
{
    QImage png(sceneRect().size().toSize(), QImage::Format_RGB32);
    // create a painter to draw on the image
    QPainter p(&png);
    p.setRenderHint(QPainter::Antialiasing); // antialiasing like our Canvas
    _scene->render(&p);
    p.end();
    return png;
}

void Canvas::saveAsSvg(const QString &title, const QString &fileName)
{
    Q_UNUSED(title);
    // it would have been nicer if this method didn't needed to be passed a filename..
    // but otherwise some QBuffer, QByteArray, etc. thing had to be set up.
    QSvgGenerator generator;
    generator.setFileName(fileName);
    generator.setSize(_scene->sceneRect().size().toSize());
    generator.setViewBox(_scene->sceneRect());
    generator.setTitle(title);
    //	generator.setDescription(i18n("Created with KTurtle %1 -- %2").arg(version).arg(website));
    // create a painter to draw on the image
    QPainter p(&generator);
    // 	p.setRenderHint(QPainter::Antialiasing);  // antialiasing like our Canvas

    bool spriteWasVisible = turtle->isVisible();

    slotSpriteHide(); // hide the sprite as it draws really ugly (especially when Qt < 4.5)
    _scene->render(&p);

    if (spriteWasVisible)
        slotSpriteShow();
    p.end();
}

#include "moc_canvas.cpp"
