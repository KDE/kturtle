/*
 * Copyright (C) 2003 Cies Breijs <cies # showroommama ! nl>
 */

#include <stdlib.h>

#include <qtimer.h>
#include <qlayout.h>

#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kinputdialog.h> 
#include <kkeydialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h> 
#include <kstatusbar.h>
#include <kstdaction.h>
#include <ktextedit.h>
#include <kurl.h>

#include <ktexteditor/clipboardinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/printinterface.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/undointerface.h>
#include <ktexteditor/viewcursorinterface.h>

#include "settings.h"
#include "kturtle.h"

// StatusBar field IDs
#define IDS_INS    0
#define IDS_STATUS      1
#define IDS_STATUS_CLM  3

MainWindow::MainWindow(KTextEditor::Document *document) : editor(0) {
    // set the shell's ui resource file
    if (!document) {
      if ( !(document = KTextEditor::EditorChooser::createDocument(0,"KTextEditor::Document") ) ) {
        KMessageBox::error(this, i18n("A KDE text editor component could not be found!\n"
                                     "Please check your KDE installation."));
        kapp->exit(1);
      }  
    // docList.append(doc);
    }
    doc = document;

    setupCanvas();
    setupEditor();
    setupActions();
    setupStatusBar();
    
    setXMLFile("kturtleui.rc");
    createShellGUI( true );
    //guiFactory()->addClient(editor);
    setMinimumSize(200,200);

    // the initial values
    CurrentFile = "";
    filename2saveAs = "";
    setCaption( i18n("Untitled") );
    picker = 0; // for the colorpickerdialog
    
    // at last; must be sure everything is already set up ;)
    setAutoSaveSettings ("MainWindow Settings");

    // init with more usefull size, stolen from kwite (they stole it from konq)
    if ( !initialGeometrySet() && !kapp->config()->hasGroup("MainWindow Settings") ) {
        resize( 640, 480 );
    }
     
    KConfig *config = kapp->config();
    readConfig(config);
    
    show();
}

MainWindow::~MainWindow() { // The MainWindow destructor
    if ( editor->document() ) {
        delete editor->document();
    }
}


void MainWindow::setupActions() {
    KActionCollection *ac = actionCollection();
    // Set up file actions
    KStdAction::openNew(this, SLOT(slotNewFile()), ac);
    openExAction = new KAction(i18n("Open Examples"), "bookmark_folder", 0, this, SLOT(slotOpenEx()), ac, "open_examples");
    KStdAction::open(this, SLOT(slotOpenFile()), ac);
    m_recentFiles = KStdAction::openRecent(this, SLOT(slotOpen(const KURL&)), ac);
    KStdAction::save(this, SLOT(slotSaveFile()), ac);
    KStdAction::saveAs(this, SLOT(slotSaveAs()), ac);
    run = new KAction(i18n("&Execute Commands"), "gear", 0, this, SLOT( slotExecute() ), ac, "run");
    run->setEnabled(false);
    KStdAction::print(this, SLOT(slotPrint()), ac);
    KStdAction::quit(this, SLOT(slotQuit()), ac);
    // Set up edit actions
    KStdAction::undo(this, SLOT(slotUndo()), ac);
    KStdAction::redo(this, SLOT(slotRedo()), ac);
    KStdAction::cut(this, SLOT(slotCut()), ac);
    KStdAction::copy(this, SLOT(slotCopy()), ac);
    KStdAction::paste(this, SLOT(slotPaste()), ac);
    KStdAction::selectAll(this, SLOT(slotSelectAll()), ac);
    KStdAction::deselect(this, SLOT(slotClearSelection()), ac);
    new KToggleAction(i18n("Toggle Insert"), Key_Insert, this, SLOT(slotToggleInsert()), ac, "set_insert");
    KStdAction::find(this, SLOT(slotFind()), ac);
    KStdAction::findNext(this, SLOT(slotFindNext()), ac);
    KStdAction::findPrev(this, SLOT(slotFindPrevious()), ac);
    KStdAction::replace(this, SLOT(slotReplace()), ac);
    // Set up view actions
    new KToggleAction(i18n("Show &Line Numbers"), 0, Qt::Key_F11, this, SLOT(slotToggleLineNumbers()), ac, "line_numbers");
    m_fullscreen = KStdAction::fullScreen(this, SLOT( slotToggleFullscreen() ), ac, this, "full_screen");
    m_fullscreen->setChecked(b_fullscreen);
    // Set up tools actions
    new KToggleAction(i18n("&Color Picker"), "colorize", 0, this, SLOT(slotColorPicker()), ac, "color_picker");
    new KAction(i18n("&Indent"), "indent", CTRL+Key_I, this, SLOT(slotIndent()), ac, "edit_indent");
    new KAction(i18n("&Unindent"), "unindent", CTRL+SHIFT+Key_I, this, SLOT(slotUnIndent()), ac, "edit_unindent");
    new KAction(i18n("Cl&ean Indentation"), 0, 0, this, SLOT(slotCleanIndent()), ac, "edit_cleanIndent");
    new KAction(i18n("Co&mment"), 0, CTRL+Key_D, this, SLOT(slotComment()), ac, "edit_comment");
    new KAction(i18n("Unc&omment"), 0, CTRL+SHIFT+Key_D, this, SLOT(slotUnComment()), ac, "edit_uncomment");

    //@todo: make the EditorDock hideable, better to do it when on KTextEditor... 
    // (void)new KToggleAction(i18n("&Hide Editor"), 0, 0, this, SLOT(slotToggleHideEditor()),
   
    // setup settings actions
    setStandardToolBarMenuEnabled(true);
    KStdAction::preferences( this, SLOT( slotSettings() ), ac );
    KStdAction::keyBindings( this, SLOT( slotConfigureKeys() ), ac );
    new KAction(i18n("&Configure Editor..."), "configure", 0, this, SLOT(slotEditor()), ac, "set_confdlg");
    KStdAction::configureToolbars( this, SLOT(slotConfigureToolbars()), ac);
    // setup help actions
    ContextHelp = new KAction(0, 0, Key_F1, this, SLOT(slotContextHelp()), ac, "context_help");
    slotContextHelpUpdate(); // this sets the label of this action
}

void MainWindow::setupEditor() {
    EditorDock = new QDockWindow(this);
    EditorDock->setNewLine(true);
    EditorDock->setFixedExtentWidth(250);
    EditorDock->setFixedExtentHeight(150);
    EditorDock->setResizeEnabled(true);
    EditorDock->setFrameShape(QFrame::ToolBarPanel);
    moveDockWindow(EditorDock, Qt::DockLeft);
    editor = doc->createView (EditorDock, 0L);
    // ei is the editor interface which allows us to access the text in the part
    ei = dynamic_cast<KTextEditor::EditInterface*>(doc);
    EditorDock->setWidget(editor);

    // default the highlightstyle to "logo" someday this needs to be i18nized
    KTextEditor::HighlightingInterface *hli = dynamic_cast<KTextEditor::HighlightingInterface*>(doc);
    for(unsigned int i = 0; i < hli->hlModeCount(); i++) {
         if(hli->hlModeName(i) == "logo") {
             hli->setHlMode(i);
         }
    }
    // allow the cursor position to be indicated in the statusbar
    connect(editor, SIGNAL(cursorPositionChanged()), this, SLOT(slotCursor()));
    // allow to enable run only when some text is written in editor
    connect( editor->document(), SIGNAL( textChanged() ), this, SLOT( setRunEnabled() ) );
}

void MainWindow::setupStatusBar() {
    statusBar()->insertItem("", IDS_INS, 0, true);
    statusBar()->insertItem(" ", 2, 0, true);//to add a space, ID is 2
    statusBar()->insertItem("",  IDS_STATUS, 1, false);
    QString linenumber = i18n(" Line: %1 Col: %2 ").arg(1).arg(1);
    statusBar()->insertItem(linenumber,  IDS_STATUS_CLM, 0, true);
    statusBar()->setItemAlignment( IDS_STATUS, AlignLeft);
    statusBar()->show();
    // fill the statusbar
    slotStatusBar(i18n("INS"), IDS_INS); 
    slotStatusBar(i18n("Welcome to KTurtle..."),  IDS_STATUS); // the message part
}

void MainWindow::setupCanvas() {
    BaseWidget = new QWidget(this);
    setCentralWidget(BaseWidget);
    BaseLayout = new QGridLayout(BaseWidget, 0, 0);
    TurtleView = new Canvas(BaseWidget);
    BaseLayout->addWidget(TurtleView, 0, 0, AlignCenter);
    TurtleView->show();
    
    connect( TurtleView, SIGNAL( CanvasResized() ), this, SLOT( slotUpdateCanvas() ) );
}

void MainWindow::slotNewFile() {
    if ( !editor->document()->isModified() && CurrentFile == "" ) {
        return; // do nothing when nothing is to be done
    }
    if ( editor->document()->isModified() ) {
        int result = KMessageBox::warningContinueCancel( this,
        i18n("The changes you have made to the file you "
             "are currently working are not saved. "
             "By continuing you will lose all the changes you have made."),
        i18n("Unsaved file..."), i18n("Discard changes") );
        if (result != KMessageBox::Continue) {
            return;
        }
    }
    ei->clear();// clear the editor
    TurtleView->slotClear();// clear the view
    CurrentFile = "";
    setCaption( i18n("Untitled") );
    slotStatusBar(i18n("New file... Happy coding!"), IDS_STATUS); 
}

void MainWindow::slotSaveFile() {  
    QString filestr = CurrentFile;
    // when coming from SaveAs, then dont use CurrentFile but filename2saveAs
    if ( !filename2saveAs.isEmpty() ) {
        filestr = filename2saveAs;
        filename2saveAs = "";
    }
    KURL url = KURL(filestr);
    slotSave(url);
}

void MainWindow::slotSaveAs() {
       KURL url;
      while(true) {
      url = KFileDialog::getSaveURL( QString(":logo_dir"), QString("*.logo|") +
      i18n("Logo files"), this, i18n("Save logo file...") );
        if (url.isEmpty()) { // when cancelled the KFiledialog?
            return;
        }
        if (QFile(url.path()).exists()) { 
            int result = KMessageBox::warningContinueCancel( this,
                i18n("A file named \"%1\" already exists.\n"
                     "Are you sure you want to overwrite it?").arg(url.url()),
                i18n("Overwrite existing file?"), i18n("Overwrite") );
            if (result != KMessageBox::Continue) {
                return;
            }
        }
        break;
    }
    slotSave(url);
}

void MainWindow::slotSave(KURL &url)
{
	if ( !url.isEmpty())
 	 {
		filename2saveAs = url.url();
    		editor->document()->saveAs(url);
    		CurrentFile = url.fileName();
    		setCaption(CurrentFile);
    		slotStatusBar(i18n("Saved file to: %1").arg(CurrentFile),  IDS_STATUS); 
    		m_recentFiles->addURL( url );
		editor->document()->setModified(false);
	}
	else
	{
	  	slotSaveAs();
	}
}


void MainWindow::slotOpenFile() {
      KURL url = KFileDialog::getOpenURL( QString(":logo_dir"), QString("*.logo|") +
      i18n("Logo files"), this, i18n("Open logo file...") );
       if( !url.isEmpty() )
           loadFile(url);
}

void MainWindow::slotOpen(const KURL& url) {
       	if( !url.isEmpty() )
           loadFile(url);
	editor->document()->setModified(false);
}

void MainWindow::slotQuit() {
    if ( editor->document()->isModified() ) {
        slotStatusBar(i18n("Quitting KTurtle..."),  IDS_STATUS);
        // make sure the dialog looks good with new -- unnamed -- files.
        int result = KMessageBox::warningYesNoCancel( this,
        i18n("The changes you have made are not saved. "
             "By quitting KTurtle you will lose all the changes "
             "you have made."),
        i18n("Unsaved file..."), i18n("Save"), i18n("Discard changes and Quit") );
        if (result == KMessageBox::Cancel) {
            slotStatusBar(i18n("Quitting aborted."),  IDS_STATUS);
            return;
        } else if (result == KMessageBox::Yes) {
            slotSaveFile();
            if ( CurrentFile.isEmpty() ) {
                return; // when saveAs get cancelled or X-ed it should not quit
            }
        }
    }
    KConfig *config = kapp->config();
    writeConfig(config);

    close();
}

void MainWindow::slotPrint() {
    dynamic_cast<KTextEditor::PrintInterface*>(doc)->printDialog();
}

void MainWindow::slotExecute() {
    if ( executing ) {
        abortExecution();
    } else {
        startExecution();
    }
}

void MainWindow::startExecution() {
    allreadyError = false;
    executing = true;
    run->setIcon("stop");
    run->setText("Stop Execution");   
    slotStatusBar(i18n("Parsing commands..."),  IDS_STATUS);
    kapp->processEvents();
    
    string txt = ( ei->text() + "\n" ).latin1(); // the /n is needed for proper parsing
    stringbuf sbuf(txt, ios_base::in);
    istream in(&sbuf);
    Parser parser(in);
    connect( &parser, SIGNAL(ErrorMsg(QString, unsigned int, unsigned int, unsigned int) ), 
             this, SLOT(slotErrorDialog(QString, unsigned int, unsigned int, unsigned int) ) );
    
    // parsing and executing...
    if( parser.parse() ) {
        TreeNode* root = parser.getTree();
        root->showTree(root); // show parsetree  DEBUG OPTION

        slotStatusBar(i18n("Executing commands..."),  IDS_STATUS); 
        exe = new Executer(root); // make Executer object, 'exe', and have it point to the root
        connect( exe, SIGNAL( ErrorMsg(QString, unsigned int, unsigned int, unsigned int) ), 
                 this, SLOT( slotErrorDialog(QString, unsigned int, unsigned int, unsigned int) ) );
        connect( exe, SIGNAL( InputDialog(QString&) ), 
                 this, SLOT( slotInputDialog(QString&) ) );
        connect( exe, SIGNAL( MessageDialog(QString) ), 
                 this, SLOT( slotMessageDialog(QString) ) );

        // Connect the signals form Executer to the slots from Canvas:
        connect( exe, SIGNAL( Clear() ),
                 TurtleView, SLOT( slotClear() ) );
        connect( exe, SIGNAL( Go(int, int) ),
                 TurtleView, SLOT( slotGo(int, int) ) );
        connect( exe, SIGNAL( GoX(int) ),
                 TurtleView, SLOT( slotGoX(int) ) );
        connect( exe, SIGNAL( GoY(int) ),
                 TurtleView, SLOT( slotGoY(int) ) );
        connect( exe, SIGNAL( Forward(int) ),
                 TurtleView, SLOT( slotForward(int) ) );
        connect( exe, SIGNAL( Backward(int) ),
                 TurtleView, SLOT( slotBackward(int) ) );
        connect( exe, SIGNAL( Direction(double) ),
                 TurtleView, SLOT( slotDirection(double) ) );
        connect( exe, SIGNAL( TurnLeft(double) ),
                 TurtleView, SLOT( slotTurnLeft(double) ) );
        connect( exe, SIGNAL( TurnRight(double) ),
                 TurtleView, SLOT( slotTurnRight(double) ) );
        connect( exe, SIGNAL( Center() ),
                 TurtleView, SLOT( slotCenter() ) );
        connect( exe, SIGNAL( SetPenWidth(int) ),
                 TurtleView, SLOT( slotSetPenWidth(int) ) );
        connect( exe, SIGNAL( PenUp() ),
                 TurtleView, SLOT( slotPenUp() ) );
        connect( exe, SIGNAL( PenDown() ),
                 TurtleView, SLOT( slotPenDown() ) );
        connect( exe, SIGNAL( SetFgColor(int, int, int) ),
                 TurtleView, SLOT( slotSetFgColor(int, int, int) ) );
        connect( exe, SIGNAL( SetBgColor(int, int, int) ),
                 TurtleView, SLOT( slotSetBgColor(int, int, int) ) );
        connect( exe, SIGNAL( ResizeCanvas(int, int) ),
                 TurtleView, SLOT( slotResizeCanvas(int, int) ) );
        connect( exe, SIGNAL( SpriteShow() ),
                 TurtleView, SLOT( slotSpriteShow() ) );
        connect( exe, SIGNAL( SpriteHide() ),
                 TurtleView, SLOT( slotSpriteHide() ) );
        connect( exe, SIGNAL( SpritePress() ),
                 TurtleView, SLOT( slotSpritePress() ) );
        connect( exe, SIGNAL( SpriteChange(int) ),
                 TurtleView, SLOT( slotSpriteChange(int) ) );
        connect( exe, SIGNAL( Print(QString) ),
                 TurtleView, SLOT( slotPrint(QString) ) );
        connect( exe, SIGNAL( FontType(QString, QString) ),
                 TurtleView, SLOT( slotFontType(QString, QString) ) );
        connect( exe, SIGNAL( FontSize(int) ),
                 TurtleView, SLOT( slotFontSize(int) ) );
        connect( exe, SIGNAL( WrapOn() ),
                 TurtleView, SLOT( slotWrapOn() ) );
        connect( exe, SIGNAL( WrapOff() ),
                 TurtleView, SLOT( slotWrapOff() ) );
        connect( exe, SIGNAL( Reset() ),
                 TurtleView, SLOT( slotReset() ) );

        if ( exe->run() ) {
            slotStatusBar(i18n("Done."),  IDS_STATUS);
            finishExecution();
        } else {
            slotStatusBar(i18n("Execution aborted."),  IDS_STATUS);
            finishExecution();
        }
    } else {
        slotStatusBar(i18n("Parsing failed!"),  IDS_STATUS);
        finishExecution();
    }
}

void MainWindow::abortExecution() {
    exe->abort();
}

void MainWindow::finishExecution() {
    run->setIcon("gear");
    run->setText( i18n("&Execute Commands") );
    executing = false;
}

void MainWindow::slotErrorDialog(QString msg, unsigned int row, unsigned int col, unsigned int code) {
    if(allreadyError) { return; } // one error dialog per 'run' is enough... (see next line)
    // allreadyError = true; NO I WANT TO SE ALL ERRORS for the time beeing
    QString line;
    if( row <= 0 || col <= 0 ) {
        line = ".";
    } else {
//         RowCol = QString(" on row %1, column %2.").arg(row).arg(col); // no column, it over informs
         line = QString(" on line %1.").arg(row - 1);
    }
    
    // move cursor to the error
    dynamic_cast<KTextEditor::ViewCursorInterface*>(editor)->setCursorPositionReal(row - 2, col);
    
    QString ErrorType;
    if( 1000 <= code || code < 2000 ) {
        ErrorType = i18n("Parse Error");
    } else if( 3000 <= code || code < 4000 ) {
        ErrorType = i18n("Internal Error");
    } else if( code >= 5000 ) {
        ErrorType = i18n("Execution Error");
    } else if( code < 1000 ) {
        ErrorType = i18n("Error");
    }
    KMessageBox::detailedSorry( this, msg + line, i18n("Error code: %1").arg(code) + "\n" +
        i18n("Exact location: %1, %2").arg(row - 1).arg(col), ErrorType );
}

void MainWindow::slotInputDialog(QString& value) {
    value = KInputDialog::getText (i18n("Input"), value);
}

void MainWindow::slotMessageDialog(QString text) {
    KMessageBox::information( this, text, i18n("Message") );
}

void MainWindow::slotUndo() {
    dynamic_cast<KTextEditor::UndoInterface*>(doc)->undo();
}
   
void MainWindow::slotRedo() {
    dynamic_cast<KTextEditor::UndoInterface*>(doc)->redo();
}
   
void MainWindow::slotCut() {
    dynamic_cast<KTextEditor::ClipboardInterface*>(editor)->cut();
}
  
void MainWindow::slotCopy() {
    dynamic_cast<KTextEditor::ClipboardInterface*>(editor)->copy();
}
   
void MainWindow::slotPaste() {
    dynamic_cast<KTextEditor::ClipboardInterface*>(editor)->paste();
}

void MainWindow::slotSelectAll() {
    dynamic_cast<KTextEditor::SelectionInterface*>(doc)->selectAll();
}

void MainWindow::slotClearSelection() {
    dynamic_cast<KTextEditor::SelectionInterface*>(doc)->clearSelection();
}

void MainWindow::slotToggleInsert() {
      KToggleAction *a = dynamic_cast<KToggleAction*>(editor->actionCollection()->action("set_insert"));
      a->activate();
     if (a)
      {
        statusBar()->changeItem(a->isChecked() ? i18n(" OVR ") : i18n(" INS "), IDS_INS); 
     } 
}

void MainWindow::slotFind() {
kdDebug(0)<<"qwdqw"<<endl;
    KAction *a = editor->actionCollection()->action("edit_find");
    a->activate();
}

void MainWindow::slotFindNext() {
    KAction *a = editor->actionCollection()->action("edit_find_next");
    a->activate();
}

void MainWindow::slotFindPrevious() {
    KAction *a = editor->actionCollection()->action("edit_find_prev");
    a->activate();
}

void MainWindow::slotReplace() {
    KAction* a = editor->actionCollection()->action("edit_replace");
    a->activate();
}

void MainWindow::slotToggleLineNumbers() {
    KToggleAction *a = dynamic_cast<KToggleAction*>( editor->actionCollection()->action("view_line_numbers") );
    a->activate();
}

void MainWindow::slotEditor() {
    KAction *a = editor->actionCollection()->action("set_confdlg");
    a->activate();
}
void MainWindow::slotConfigureToolbars() {
    // use the standard toolbar editor
    saveMainWindowSettings( KGlobal::config(), autoSaveGroup() );
    KEditToolbar dlg(actionCollection());
    connect(&dlg, SIGNAL(newToolbarConfig()), this, SLOT(newToolbarConfig()));
    dlg.exec();
}

void MainWindow::slotToggleFullscreen() {
    if (!b_fullscreen) {
       showFullScreen(); // both calls will generate event triggering updateFullScreen()
    } else {
        if( isFullScreen() ) {
            // showNormal() may also do unminimize, unmaximize etc.
            // solution: / @todo previous state should be reminded...
            showNormal();
        }
    }
}

bool MainWindow::event(QEvent* e) {
// excecutes updateFullScreen() after a ShowFullScreen or ShowNormal event got triggered
    if (e->type() == QEvent::ShowFullScreen || e->type() == QEvent::ShowNormal) {
        updateFullScreen();
    }
    return KMainWindow::event(e);
}

// some code that will excecute after a ShowFullScreen or ShowNormal event
void MainWindow::updateFullScreen() {
    if (isFullScreen() == b_fullscreen) {
        return;
    }
    b_fullscreen = isFullScreen();
    if (m_fullscreen) {
        m_fullscreen->setChecked(b_fullscreen);
    }
}

void MainWindow::slotStatusBar(QString text, int id) {
    if(id == IDS_INS) { // the version part of the statusbar
        text = " " + text + " "; // help the layout
    } else { // if id is not 0 it must be 1 (error cacher)
        text = " " + text; // help the layout
    }
    statusBar()->changeItem(text, id);
}

void MainWindow::slotUpdateCanvas() {
    // fixes a non updateing bug
    // I tried doing this by connecting Canvas's resized to BaseWidget's update...
    // but i had no luck :(       this worked though
    TurtleView->hide();
    TurtleView->show();
}

void MainWindow::slotColorPicker() {
    // in the constructor picker is initialised as 0
    // if picker is 0 when this funktion is called a colorpickerdialog is created and connected
    if(picker == 0) {
        picker = new ColorPicker(this);
      if(picker == 0) {
          return; // safety
      }
      connect( picker, SIGNAL( visible(bool) ), colorpicker, SLOT( setChecked(bool) ) );
    }
    // if picker is not 0, there is a colorpickerdialog which only needs to be shown OR hidden
    if( picker->isHidden() ) {
        picker->show();
        colorpicker->setChecked(true);
    } else {
        picker->hide();
        colorpicker->setChecked(false);
    }
}

void MainWindow::slotIndent() {
    KAction *a = editor->actionCollection()->action("tools_indent");
    a->activate();
}

void MainWindow::slotUnIndent() {
    KAction *a = editor->actionCollection()->action("tools_unindent");
    a->activate();
}

void MainWindow::slotCleanIndent() {
    KAction *a = editor->actionCollection()->action("tools_cleanIndent");
    a->activate();
}

void MainWindow::slotComment() {
    KAction *a = editor->actionCollection()->action("tools_comment");
    a->activate();
}

void MainWindow::slotUnComment() {
    KAction *a = editor->actionCollection()->action("tools_uncomment");
    a->activate();
}

void MainWindow::slotSettings() {
    // Check if there is already a dialog, if so bring it to the foreground.
    if( KConfigDialog::showDialog("settings") ) {
        return;
    }
    // Create a new dialog with the same name as the above checking code.
    KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self() );
    // making the filling for the 'General' settings dept.
    general = new QWidget();
    QGridLayout *generalLayout = new QGridLayout( general, 1, 1, 11, 6, "generalLayout");
    WidthHeightBox = new QGroupBox( i18n("Initial Canvas Size"), general );
    WidthHeightBox->setColumnLayout(0, Qt::Vertical );
    WidthHeightBox->layout()->setSpacing( 6 );
    WidthHeightBox->layout()->setMargin( 11 );
    QVBoxLayout *WidthHeightBoxLayout = new QVBoxLayout( WidthHeightBox->layout() );
    WidthHeightBoxLayout->setAlignment( Qt::AlignTop );
    QHBoxLayout *layout3 = new QHBoxLayout( 0, 0, 6, "layout3"); 
    QVBoxLayout *layout2 = new QVBoxLayout( 0, 0, 6, "layout2"); 
    WidthLabel = new QLabel( kcfg_CanvasWidth, i18n("Canvas Width:"), WidthHeightBox );
    layout2->addWidget( WidthLabel );
    HeightLabel = new QLabel( kcfg_CanvasHeight, i18n("Canvas Weight:"), WidthHeightBox );
    layout2->addWidget( HeightLabel );
    layout3->addLayout( layout2 );
    QVBoxLayout *layout1 = new QVBoxLayout( 0, 0, 6, "layout1"); 
    kcfg_CanvasWidth = new KIntNumInput( WidthHeightBox, "kcfg_CanvasWidth" );
    kcfg_CanvasWidth->setValue( 400 );
    kcfg_CanvasWidth->setMinValue( 1 );
    kcfg_CanvasWidth->setReferencePoint( 1 );
    layout1->addWidget( kcfg_CanvasWidth );
    kcfg_CanvasHeight = new KIntNumInput( WidthHeightBox, "kcfg_CanvasHeight" );
    kcfg_CanvasHeight->setValue( 300 );
    kcfg_CanvasHeight->setMinValue( 1 );
    kcfg_CanvasHeight->setReferencePoint( 1 );
    layout1->addWidget( kcfg_CanvasHeight );
    layout3->addLayout( layout1 );
    WidthHeightBoxLayout->addLayout( layout3 );
    generalLayout->addWidget( WidthHeightBox, 0, 0 );
    general->resize( QSize(234, 109).expandedTo(minimumSizeHint()) );
    
    dialog->addPage( general, i18n("General"), "package_settings", i18n("General Settings") );

    // making the filling for the 'Language' settings dept.   
    QWidget *language = new QWidget();
    QGridLayout *languageLayout = new QGridLayout( language, 1, 1, 11, 6, "Form1Layout"); 
    QGroupBox *groupBox1 = new QGroupBox( language, "groupBox1" );
    groupBox1->setColumnLayout(0, Qt::Vertical );
    groupBox1->layout()->setSpacing( 6 );
    groupBox1->layout()->setMargin( 11 );
    QGridLayout *groupBox1Layout = new QGridLayout( groupBox1->layout() );
    groupBox1Layout->setAlignment( Qt::AlignTop );
    
    QVBoxLayout *layout4 = new QVBoxLayout( 0, 0, 6, "layout4"); 
    kcfg_TranslationFilePath = new KURLRequester(groupBox1, "kcfg_TranslationFilePath");
    TranslationFileLabel = new QLabel(kcfg_TranslationFilePath, i18n("Select the language of the LOGO commands:"), groupBox1);
    layout4->addWidget( TranslationFileLabel );
    layout4->addWidget( kcfg_TranslationFilePath );
    groupBox1Layout->addLayout( layout4, 0, 0 );
    languageLayout->addWidget( groupBox1, 0, 0 ); 
    language->resize( QSize(373, 80).expandedTo(minimumSizeHint()) );
    TranslationFileLabel->setBuddy( kcfg_TranslationFilePath );
    
    dialog->addPage( language, i18n("Language"), "locale", i18n("Language Settings") );

    // When the user clicks OK or Apply we want to update our settings.
    connect( dialog, SIGNAL( settingsChanged() ), this, SLOT( slotUpdateSettings() ) );

    // Display the dialog.
    dialog->setInitialSize( QSize(550, 300));
    dialog->show();
    // This need to be placed here for the url to show up in the lineedit
    kcfg_TranslationFilePath->setURL(Settings::translationFilePath());
}

void MainWindow::slotUpdateSettings() {
    	connect( this, SIGNAL( ResizeCanvas(int, int) ), TurtleView, SLOT( slotResizeCanvas(int, int) ) );
   	 emit ResizeCanvas( Settings::canvasWidth(), Settings::canvasWidth() );
    	// something with the xml-translation-file
}

void MainWindow::slotConfigureKeys() {
    KKeyDialog::configure(actionCollection(), this);
}

void MainWindow::slotContextHelp() {
}

void MainWindow::slotContextHelpUpdate() {
    QString keyword = i18n("<no keyword>");
    
    ContextHelp->setText( i18n("Help on:") + " " + keyword );
}

void MainWindow::readConfig(KConfig *config) {
        if ( Settings::translationFilePath().isNull() ) {
        	kdDebug(0)<<"--3--"<<endl;
            	slotErrorDialog( i18n("Could not find the command translation file.\n"
                                  "Please go to \"Settings -> Configure KTurtle\" and "
                                  "specify the full name and path to "
                                  "\"logokeywords.en_US.xml\" or any other logokeywords "
                                  "file. Otherwise KTurtle will not operate." ) );
        }
	
	config->setGroup("General Options");
	m_recentFiles->loadEntries(config, "Recent Files");
}

void MainWindow::writeConfig(KConfig *config) {
	config->setGroup("General Options");
	m_recentFiles->saveEntries(config, "Recent Files");
}

void MainWindow::setRunEnabled() {
    run->setEnabled(true);
}

void MainWindow::slotOpenEx() {
    //the program scans in kturtle/examples/ to see what languages data is found
    QStringList m_languages;
    QStringList dirs = KGlobal::dirs()->findDirs("data", "kturtle/examples/");
    for (QStringList::Iterator it = dirs.begin(); it != dirs.end(); ++it ) {
	QDir dir(*it);
	m_languages += dir.entryList(QDir::Dirs, QDir::Name);
    }	
    m_languages.remove(m_languages.find("."));
    m_languages.remove(m_languages.find(".."));
    kdDebug() << m_languages << endl;
    //see what language the user has in his settings
    KConfigBase *globalConf = KGlobal::config();
    globalConf->setGroup("Locale");
    QString userLanguage = globalConf->readEntry("Language");
    userLanguage = userLanguage.left(5);
   //default language is one of the logokeyword or en
   QString defaultLanguage;
   for (QStringList::Iterator it = m_languages.begin(); it != m_languages.end(); ++it ) {
	QString m_language(*it);
	if (m_language == userLanguage || m_language.left(2) == userLanguage) {
		defaultLanguage = m_language;
		break;
	}
	else
  		defaultLanguage = "en_US";
    }    
    kdDebug() << defaultLanguage << endl;
    KURL url;
    url.setPath( locate("data", "kturtle/examples/"+defaultLanguage+"/"));
    url = KFileDialog::getOpenURL( url.path() ,
                       QString("*.logo|") + i18n("Logo Examples files"), this,
                       i18n("Open logo example file..."));
    loadFile(url);
}

void MainWindow::loadFile(KURL url) {
    m_recentFiles->addURL( url );
    QString myFile = url.path();
    if ( !myFile.isEmpty() ) {
        QFile file(myFile);
        //if ( !file.open(IO_ReadWrite) ) {
        //  return;
        // }
        if ( file.open(IO_ReadWrite) ) {
            if ( editor->document()->isModified() ) {
                int result = KMessageBox::warningContinueCancel( this,
                  i18n("The changes you have made to the file you "
                        "are currently working on (%1) are not saved. "
                        "By continuing you will lose all the changes "
                        "you have made.").arg( myFile ),
                  i18n("Unsaved file..."), i18n("Discard changes") );
                if (result != KMessageBox::Continue) {
                    return;
                }
            }
        QTextStream stream(&file);
        ei->setText(stream.read());
        file.close();
        CurrentFile = myFile;
        setCaption(url.fileName());
        slotStatusBar(i18n("Opened file: %1").arg(url.fileName()),  IDS_STATUS);
	editor->document()->setModified(false);
        }
    } else { 
        slotStatusBar(i18n("Opening aborted, nothing opened."),  IDS_STATUS);
    }
}

void MainWindow::slotCursor() {
  	uint cursorLine;
  	uint cursorCol;
 	 dynamic_cast<KTextEditor::ViewCursorInterface*>(editor)->cursorPositionReal(&cursorLine, &cursorCol);
  	QString linenumber = i18n(" Line: %1 Col: %2 ").arg(cursorLine+1).arg(cursorCol+1);
  	statusBar()->changeItem(linenumber, IDS_STATUS_CLM);
}


#include "kturtle.moc"
