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
#include <kstdaction.h>
#include <kurlrequester.h>
#include <knuminput.h>

#include "canvas.h"
#include "colorpicker.h"
#include "executer.h"
#include "parser.h"
#include "settings.h"

#include <kparts/mainwindow.h>
#include <ktexteditor/document.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/view.h>


class KRecentFilesAction;

class MainWindow : public KParts::MainWindow
{   Q_OBJECT

  public:
    MainWindow(KTextEditor::Document* = 0L);
    virtual ~MainWindow();
       
    KTextEditor::View *view() const { return editor; }
    KTextEditor::EditInterface* ei;
    
  public slots:
    void slotNewFile();
    void slotOpenFile();
    void slotOpen(const KURL&);
    void slotSaveFile();
    void slotSaveAs();
    // TODO: linenumbers; probably when going from KTextEdit to KTextEditor::Editor
    // void slotLineNumbers();
    void slotQuit();
    void slotExecute();
    
    void slotUndo();
   
    void slotStatusBar(QString text, int place);
    void slotUpdateCanvas();
    
    void slotErrorDialog(QString msg, int row = 0, int col = 0, int code = 0);
    void slotInputDialog(QString& value);
    void slotMessageDialog(QString text);
    
  signals:
    void ResizeCanvas(int x, int y);

  private:
    void setupActions();
    void setupEditor();
    void setupCanvas();
    void setupStatusBar();
    void readConfig(KConfig *config);
    void writeConfig(KConfig *config);
    void startExecution();
    void abortExecution();
    void finishExecution();
    void updateFullScreen();
    
    KTextEditor::Document *doc;
    KTextEditor::View  *editor;
    Canvas             *TurtleView;
    QWidget            *BaseWidget;
    QGridLayout        *BaseLayout;
    QDockWindow        *EditorDock;
    
    QString             CurrentFile;
    QString             filename2saveAs;

    KToggleAction      *m_paShowPath;
    KToggleAction      *m_paShowStatusBar;
    
    KAction            *run;
    KAction            *openExAction;
    KAction            *openFileAction;
    KAction            *newAction;
    KToggleAction      *m_fullscreen;
    KToggleAction      *colorpicker;
    KRecentFilesAction * m_recentFiles;
    
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
    
    Executer           *exe;
    bool                b_fullscreen:1;
    bool                allreadyError;
    bool                executing;
    
  private slots:
    void slotToggleFullscreen();
    void slotSettings();
    void slotUpdateSettings();
    void slotConfigureKeys();
    void slotColorPicker();
    void setRunEnabled();
    void slotOpenEx();
    void loadFile(KURL url);

  protected:
    virtual bool event(QEvent* e);
};


#endif // _KTURTLE_H_
