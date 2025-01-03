/*
    SPDX-FileCopyrightText: 2007 Niels Slot <nielsslot AT gmail DOT com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _DIRECTIONDIALOG_H_
#define _DIRECTIONDIALOG_H_

#include "interpreter/translator.h"

#include <QDialog>
#include <QSvgRenderer>
#include <QWidget>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;

class DirectionCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit DirectionCanvas(QWidget *parent = nullptr);
    void enableGreyTurtle(bool);

public Q_SLOTS:
    void updateDirections(double previousDeg, double deg);

Q_SIGNALS:
    void degreeChanged(double deg);
    void previousDegreeChanged(double deg);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    double translateMouseCoords(double x, double y);

    double deg;
    double previousDeg;
    QSvgRenderer turtle;
    QSvgRenderer greyTurtle;
    bool greyTurtleEnabled;
};

class DirectionDialog : public QDialog
{
    Q_OBJECT

public:
    DirectionDialog(double deg, QWidget *parent);

    enum Command {
        Turnleft = 0,
        Turnright = 1,
        Direction = 2
    };

Q_SIGNALS:
    void pasteText(const QString &);

private:
    DirectionCanvas *canvas;

    QComboBox *commandPicker;
    QSpinBox *previousDirectionSpin;
    QSpinBox *directionSpin;

    QPushButton *copyButton;
    QPushButton *pasteButton;

    QLineEdit *commandBox;

    int currentCommand; // enum DirectionChooser::Command

    Translator *translator;

    bool skipValueChangedEvent;

    void updateCanvas();
    void updateCommandBox();

private Q_SLOTS:
    void directionChanged(int value);
    void changeCommand(int command);
    void updateDegrees(double deg);
    void updatePreviousDegrees(double deg);
    void copyProxy();
    void pasteProxy();
};

#endif // _DIRECTIONDIALOG_H_
