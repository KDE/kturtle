/*
 * Copyright (C) 2003 Cies Breijs <cies # showroommama ! nl>
 */

#ifndef _COLORPICKER_H_
#define _COLORPICKER_H_

#include <qlayout.h>
#include <qlineedit.h> 
#include <qslider.h> 
#include <qspinbox.h>

#include <kcolordialog.h>
#include <kdialogbase.h>


class ColorPicker : public KDialogBase
{   Q_OBJECT
  
  public:
    ColorPicker(QWidget *parent);
    virtual ~ColorPicker();
  
  signals:
    void visible(bool); // for toggling convenience
   
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
};

#endif // _COLORPICKER_H_
