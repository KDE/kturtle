/*
 * Copyright (C) 2003 Cies Breijs <cies # showroommama ! nl>
 */

#include <qlabel.h>

#include <klocale.h>

#include "colorpicker.h"


ColorPicker::ColorPicker(QWidget *parent) : KDialogBase(parent, "colorpicker",
             false, i18n("Color Picker"), KDialogBase::Close|KDialogBase::Help|KDialogBase::User1, 
             KDialogBase::Close, true ) {

    // for toggling convenience
    connect( this, SIGNAL( finished() ), SLOT( slotEmitVisibility() ) );
    
    // Create the top level page and its layout
    BaseWidget = new QWidget(this);
    setMainWidget(BaseWidget);
    
    // the User1 button:
    setButtonText( KDialogBase::User1, i18n("Insert color code at cursor") );
    connect( this, SIGNAL( user1Clicked() ), SLOT( slotEmitColorCode() ) );
 
    QVBoxLayout *vlayout = new QVBoxLayout(BaseWidget);
    
    vlayout->addSpacing(5); // spacing on top

    // the palette and value selector go into a H-box
    QHBoxLayout *h1layout = new QHBoxLayout(BaseWidget);
    vlayout->addLayout(h1layout);
    
    h1layout->addSpacing(10); // space on the left border
     
    hsSelector = new KHSSelector(BaseWidget); // the color (SH) selector
    hsSelector->setMinimumSize(300, 150);
    h1layout->addWidget(hsSelector);
    connect( hsSelector, SIGNAL( valueChanged(int, int) ), SLOT( slotSelectorChanged(int, int) ) );

    h1layout->addSpacing(5); // space in between
   
    valuePal = new KValueSelector(BaseWidget); // the darkness (V) pal
    valuePal->setFixedWidth(30);
    h1layout->addWidget(valuePal);
    connect( valuePal, SIGNAL( valueChanged(int) ), SLOT( slotPalChanged(int) ) );   
    
    vlayout->addSpacing(15); // space in between the top and the bottom widgets

    // the patch and the codegenerator also go into a H-box
    QHBoxLayout *h2layout = new QHBoxLayout(BaseWidget);
    vlayout->addLayout(h2layout);
    
    h2layout->addSpacing(10); // space on the left border
   
    patch = new KColorPatch(BaseWidget); // the patch (color previewer)
    patch->setFixedSize(50, 50);
    h2layout->addWidget(patch);
   
    h2layout->addSpacing(10); // space in between

    // the label and the codegenerator go in a V-box
    QVBoxLayout *v2layout = new QVBoxLayout(BaseWidget);
    h2layout->addLayout(v2layout); 
    
    copyable = new QLabel(i18n("Color code:"), BaseWidget); // tha label
    v2layout->addWidget(copyable);
        
    colorcode = new QLineEdit(BaseWidget); // the code generator
    colorcode->setReadOnly(true);
    v2layout->addWidget(colorcode);
    connect( colorcode, SIGNAL( selectionChanged() ), SLOT( slotReselect() ) );
    
    h2layout->addSpacing(5); // spacing on the right border
    
    vlayout->addSpacing(10); // spacing on the bottom

    h = g = b = 0; // start with red
    s = v = r = 255;

    slotSelectorChanged(h, s); // update all at once
}

ColorPicker::~ColorPicker() {
}



void ColorPicker::updateSelector() {
    hsSelector->setValues(h, s);
}

void ColorPicker::updatePal() {
    valuePal->setHue(h);
    valuePal->setSaturation(s);
    valuePal->setValue(v);
    valuePal->updateContents();
    valuePal->repaint(false);
}

void ColorPicker::updatePatch() {
    patch->setColor(color);
}

void ColorPicker::updateColorCode() {
    color.getRgb(&r, &g, &b);
    colorcode->setText( QString("%1, %2, %3").arg(r).arg(g).arg(b) );
    colorcode->selectAll();
}

void ColorPicker::slotSelectorChanged(int h_, int s_) {
    h = h_;
    s = s_;
    color.setHsv(h, s, v);

    //updateSelector(); // updated it self allready
    updatePal();
    updatePatch();
    updateColorCode();
}

void ColorPicker::slotPalChanged(int v_) {
    v = v_;
    color.setHsv(h, s, v);

    //updateSelector(); // only needed when H or S changes
    //updatePal(); // updated it self allready
    updatePatch();
    updateColorCode();
}

void ColorPicker::slotReselect() {
    // reselect by selectAll(), but make sure no looping occurs
    disconnect( colorcode, SIGNAL( selectionChanged() ), 0, 0 );
    colorcode->selectAll();
    connect( colorcode, SIGNAL( selectionChanged() ), SLOT( slotReselect() ) );
}

void ColorPicker::slotEmitVisibility() {
    // for toggling convenience
    emit visible(false);
}

void ColorPicker::slotEmitColorCode() {
    // convenience
    emit ColorCode( colorcode->text() );
}


#include "colorpicker.moc"
