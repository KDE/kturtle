/*
 * Copyright (C) 2003 Cies Breijs <cies # showroommama ! nl>
 */

#ifndef _KTURTLE_H_
#define _KTURTLE_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qdockwindow.h>
#include <qdom.h>
#include <qfile.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtextstream.h> 
#include <qwidget.h>

#include <kaction.h> 
#include <kmainwindow.h>
#include <ktextedit.h>
#include <kurlrequester.h>
#include <knuminput.h>

#include "canvas.h"
#include "colorpicker.h"
#include "executer.h"
#include "parser.h"
#include "settings.h"


class MainWindow : public KMainWindow
{   Q_OBJECT

  public:
    MainWindow();
    virtual ~MainWindow();
       
  public slots:
    void slotNewFile();
    void slotOpenFile();
    void slotSaveFile();
    void slotSaveFileAs();
    // TODO: linenumbers; probably when going from KTextEdit to KTextEditor::Editor
    // void slotLineNumbers();
    void slotQuit();
    void slotRun();
    void slotExecutionFinished();
    
    void slotStatusBar(QString text, int place);
    void slotUpdateCanvas();
    
    void slotErrorDialog(QString msg, int row = 0, int col = 0, int code = 0);
    
  signals:
    void ResizeCanvas(int x, int y);

  private:
    void setupActions();
    void setupCanvas();
    void setupEditor();
    void setupStatusBar();
    void readConfig();
    
    KTextEdit          *editor;
    Canvas             *TurtleView;
    QWidget            *BaseWidget;
    QGridLayout        *BaseLayout;
    QDockWindow        *EditorDock;
    
    QString             CurrentFile;
    QString             filename2saveAs;
    
    KAction            *run;
    KToggleAction      *m_fullscreen;
    KToggleAction      *colorpicker;
    // KRecentFilesAction *m_recentFiles; // TODO waiting on KURL (KURL wainting on KTextEditor)
    bool                b_fullscreen:1;
    void updateFullScreen();
    
    QWidget            *general;
    QWidget            *language;
    QGroupBox          *WidthHeightBox;
    QLabel             *WidthLabel;
    QLabel             *HeightLabel;
    KIntNumInput       *kcfg_CanvasWidth;
    KIntNumInput       *kcfg_CanvasHeight;
    KURLRequester      *kcfg_TranslationFilePath;
    QLabel             *TranslationFileLabel;
    ColorPicker        *picker;
    
    bool allreadyError;
  
  private slots:
    void slotToggleFullscreen();
    void slotSettings();
    void slotUpdateSettings();
    void slotConfigureKeys();
    void slotColorPicker();
    void setRunEnabled();

  protected:
    virtual bool event(QEvent* e);
};

#endif // _KTURTLE_H_
