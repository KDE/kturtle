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
    void slotSave(KURL &);
    void slotPrint();
    void slotQuit();
    /// monitor wether to start the execution or to abort it
    void slotExecute();
    
    void slotUndo();
    void slotRedo();
    void slotCut();
    void slotCopy();
    void slotPaste();
    void slotSelectAll();
    void slotClearSelection();
    void slotToggleInsert();
    void slotFind();
    void slotFindNext();
    void slotFindPrevious();
    void slotReplace();
    /// show/hide the line numbers, from KTextEditor
    void slotToggleLineNumbers();
    /// get the cursor position indicated in the statusbar
    void slotCursor();

    void slotIndent();
    void slotUnIndent();
    void slotCleanIndent();
    void slotComment();
    void slotUnComment();
    void slotEditor();
    void slotConfigureToolbars();
    void slotContextHelp();
    void slotContextHelpUpdate();

    void slotStatusBar(QString text, int place);
    void slotUpdateCanvas();
    
    void slotErrorDialog(QString msg, unsigned int row = 0, unsigned int col = 0, unsigned int code = 0);
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
    /// start parsing the text in the editor
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
    KAction            *ContextHelp;
    
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
     /// Standard full-screen mode
    void slotToggleFullscreen();
    void slotSettings();
    void slotUpdateSettings();
    void slotConfigureKeys();
    void slotColorPicker();
    /// The 'run' action is available when there is some text in the editor
    void setRunEnabled();
    /// Open the files that are in the examples folders
    void slotOpenEx();
    /// Load a logo file and display it in the editor
    void loadFile(KURL url);

  protected:
    virtual bool event(QEvent* e);
};


#endif // _KTURTLE_H_
