/*
 * Copyright (C) 2003 Cies Breijs <cies # showroommama ! nl>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _COLORPICKER_H_
#define _COLORPICKER_H_

#include <qlayout.h>
#include <qlineedit.h> 

#include <kcolordialog.h>


class ColorPicker : public KDialogBase
{   Q_OBJECT
  
  public:
    ColorPicker(QWidget *parent);
    virtual ~ColorPicker();
  
  signals:
    void visible(bool); // for toggling convenience
    void ColorCode(const QString &);
   
  private:
    QWidget            *BaseWidget;
    KHSSelector        *hsSelector;
    KValueSelector     *valuePal;
    QVBoxLayout        *vlayout;
    QHBoxLayout        *h1layout;
    QHBoxLayout        *h2layout;
    KColorPatch        *patch;
    QLabel             *copyable;
    QLineEdit          *colorcode;
    
    QColor             color;
    int h, s, v, r, g, b;
    
    void updateSelector();
    void updatePal();
    void updatePatch();
    void updateColorCode();
      
  protected slots:
    void slotSelectorChanged(int _h, int _s);
    void slotPalChanged(int _v);
    void slotReselect();
    
    void slotEmitVisibility(); // for toggling convenience
    void slotEmitColorCode(); // convenience
};

#endif // _COLORPICKER_H_
