/*
    SPDX-FileCopyrightText: 2003-2008 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _COLORPICKER_H_
#define _COLORPICKER_H_

#include <QFrame>
#include <QPainter>
#include <QPaintEvent>
#include <QDialog>

class QLineEdit;
class QSlider;
class QSpinBox;


class ColorPatch : public QFrame
{
	Q_OBJECT

	public:
		explicit ColorPatch(QWidget *parent) : QFrame(parent) {
			setFrameStyle(QFrame::Panel|QFrame::Sunken);
		}
        void setColor(const QColor &c) { col = c; }

	protected:
		void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE {
			QPainter p(this);
			drawFrame(&p);
			p.fillRect(contentsRect()&e->rect(), col);
		}
	
	private:
		QColor col;
};




class ColorPicker : public QDialog
{
	Q_OBJECT

	public:
        explicit ColorPicker(QWidget* parent = nullptr);

	Q_SIGNALS:
		void pasteText(const QString&);

	private Q_SLOTS:
		void updateResult(int r, int g, int b);
		void redChanged(int);
		void greenChanged(int);
		void blueChanged(int);
		void copyProxy();
		void pasteProxy();

	private:
		ColorPatch *colorPatch;
		QLineEdit  *resultBox;

		QSlider *redSlider;
		QSlider *greenSlider;
		QSlider *blueSlider;

		QSpinBox *redSpin;
		QSpinBox *greenSpin;
		QSpinBox *blueSpin;
};

#endif  // _COLORPICKER_H_
