/*
 * Copyright (C) 2003 Cies Breijs <cies # showroommama ! nl>
 */

#include <stdlib.h>

#include <kaccel.h>
#include <kaction.h>
#include <kapp.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kkeydialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h> 
#include <kstatusbar.h>
#include <kstdaction.h>
#include <ktextedit.h>
#include <kurl.h>

#include "settings.h"
#include "kturtle.h"


MainWindow::MainWindow() : KMainWindow( 0, i18n("KTurtle") ) {
    
    setMinimumSize(200,200);
    
    setupActions(); 
    setupCanvas();
    setupEditor();
    setupStatusBar();
    
    // setXMLFile("kturtleui.rc");  //from kwrite
    // createShellGUI(true); // i intend to use a katepart as editor later...
    createGUI("kturtleui.rc");

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
             
    readConfig ();
}

MainWindow::~MainWindow() { // The MainWindow destructor
}

void MainWindow::setupActions() {
    // Set up file menu
    KStdAction::openNew(this, SLOT(slotNewFile()), actionCollection());
    KStdAction::open(this, SLOT(slotOpenFile()), actionCollection());
    /// @todo implement a recent files list
    // recenter = KStdAction::openRecent(this, SLOT(openRecent(int)), actionCollection());
    KStdAction::save(this, SLOT(slotSaveFile()), actionCollection());
    KStdAction::saveAs(this, SLOT(slotSaveFileAs()), actionCollection());
    //
    run = new KAction(i18n("&Execute Commands"), "gear", 0, this, SLOT(slotRun()),
      actionCollection(), "run");
    run->setEnabled(false);
    //
    KStdAction::print(this, SLOT(slotPrint()), actionCollection());
    //
    KStdAction::quit(this, SLOT(slotQuit()), actionCollection());

    // setup edit menu
    KStdAction::undo(this, SLOT(slotUndo()), actionCollection());
    KStdAction::redo(this, SLOT(slotRedo()), actionCollection());
    //
    KStdAction::cut(this, SLOT(slotCut()), actionCollection());
    KStdAction::copy(this, SLOT(slotCopy()), actionCollection());
    KStdAction::paste(this, SLOT(slotPaste()), actionCollection());
    //
    KStdAction::selectAll(this, SLOT(slotSelectAll()), actionCollection());
    //
    KStdAction::find(this, SLOT(slotFind()), actionCollection());
    KStdAction::findNext(this, SLOT(slotFindAgain()), actionCollection());
    KStdAction::replace(this, SLOT(slotReplace()), actionCollection());
    // setup View menu
    m_fullscreen = KStdAction::fullScreen(this, SLOT( slotToggleFullscreen() ), actionCollection(), this, "full_screen");
    m_fullscreen->setChecked(b_fullscreen);
    colorpicker = new KToggleAction(i18n("&Color Picker"), "colorize", 0, this, SLOT(slotColorPicker()),
      actionCollection(), "color_picker");
   ///@todo recent file list, but this needs KURL which will be implemented with KTextEditor 
    // m_recentFiles = KStdAction::openRecent(this, SLOT( slotOpen(const KURL&) ), actionCollection());
    
   ///@todo: make the EditorDock hideable, better to do it when on KTextEditor... 
    // (void)new KToggleAction(i18n("&Hide Editor"), 0, 0, this, SLOT(slotToggleHideEditor()),
    //   actionCollection(), "line_numbers"); 
   ///@todo: linenumbers; probably when going from KTextEdit to KTextEditor::Editor
    // (void)new KToggleAction(i18n("Show &Line Numbers"), 0, 0, this, SLOT(slotLineNumbers()),
    //   actionCollection(), "line_numbers");
    // Going to KTestEditor will also bring -- eventually -- BiDi/RTL text support to us!!!!
    
    // setup Settings menu
    KStdAction::preferences( this, SLOT( slotSettings() ), actionCollection() );
    KStdAction::keyBindings( this, SLOT( slotConfigureKeys() ), actionCollection() );
}

void MainWindow::setupStatusBar() {
    statusBar()->insertItem("", 0, 0, true);
    statusBar()->insertItem("", 1, 1, false);
    statusBar()->setItemAlignment(1, AlignLeft);
    statusBar()->show();

    // fill the statusbar
    slotStatusBar(i18n("v0.1"), 0); // the version part
    slotStatusBar(i18n("Welcome to KTurtle..."), 1); // the message part
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

void MainWindow::setupEditor() {
    EditorDock = new QDockWindow(this);
    EditorDock->setNewLine(true);
    EditorDock->setFixedExtentWidth(250);
    EditorDock->setFixedExtentHeight(150);
    EditorDock->setResizeEnabled(true);
    EditorDock->setFrameShape(QFrame::ToolBarPanel);
    moveDockWindow(EditorDock, Qt::DockLeft);
    editor = new KTextEdit(EditorDock);
    /// @todo migrate from KTextEdit to KTextEditor::Editor (a 2.0 job?)
    editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    editor->setTextFormat(PlainText);
    /// @todo make the editor look good and internationalize well
    //     probably when going from KTextEdit to KTextEditor::Editor
    EditorDock->setWidget(editor);
    ///allow to enable run only when some text is written in editor
    connect(editor, SIGNAL(textChanged()), this, SLOT(setRunEnabled()));
}




// Implementation of most of the items in the File and Edit menus //
//
// / @todo: most of the Edit functionality (Cut,Copy,Paste,Find,...) is still lacking.
// I will probably implement it when going from KTextEdit to KTextEditor::Editor,
// because I'll have to reimplement it then anyway (it implements itself then actally).
//
void MainWindow::slotNewFile() {
    if ( !editor->isModified() && CurrentFile == "" ) {
        return; // do nothing when nothing is to be done
    }
    if ( editor->isModified() ) {
        int result = KMessageBox::warningContinueCancel( this,
        i18n("The changes you have made to the file you "
             "are currently working are not saved. "
             "By continuing you will lose all the changes you have made."),
        i18n("Unsaved file..."), i18n("Discard changes") );
        if (result != KMessageBox::Continue) {
            return;
        }
    }
    editor->clear();// clear the editor
    TurtleView->slotClear();// clear the view
    CurrentFile = "";
    setCaption( i18n("Untitled") );
    slotStatusBar(i18n("New file... Happy coding!"), 1); 
}

void MainWindow::slotSaveFile() {
    // if no filename yet, go get it first
    if ( CurrentFile.isEmpty() && filename2saveAs.isEmpty() ) {
        slotSaveFileAs();
        // cancelled saveAs? -> return
        if ( filename2saveAs.isEmpty() ) {
            return;
        }
    }
    
    QString filestr = CurrentFile;
    // when coming from SaveAs, then dont use CurrentFile but filename2saveAs
    if ( !filename2saveAs.isEmpty() ) {
        filestr = filename2saveAs;
        filename2saveAs = "";
    }
    
    QFile file(filestr);
    if ( !file.open(IO_WriteOnly) ) {
        KMessageBox::error( this, i18n("Could not write to \"%1\".\n"
            "You might not have permission to change that file, "
            "or some other application is currently using it.").arg( file.name() ),
            i18n("Error saving file...") );
        slotStatusBar(i18n("Error saving file, file NOT saved."), 1); 
        return;
    }
    QTextStream stream(&file);
    stream << editor->text();
    file.close();
    
    editor->setModified(false);
    CurrentFile = filestr;
    setCaption(CurrentFile);
    slotStatusBar(i18n("Saved file to: %1").arg(CurrentFile), 1); 
}

void MainWindow::slotSaveFileAs() {
    /// @todo migrate to KURL, probably when going from KTextEdit to KTextEditor::Editor
    QString filestr; // forward declaration (see end of scope)
    while(true) {
        filestr = KFileDialog::getSaveFileName(QString(":logo_dir"), QString("*.logo|") +
          i18n("Logo files"), this, i18n("Save logo file..."));
        if (filestr.isEmpty()) { // when cancelled the KFiledialog?
            return;
        }
        QFile file(filestr);
        if ( file.exists() ) {
            int result = KMessageBox::warningContinueCancel( this,
                i18n("A file named \"%1\" already exists.\n"
                     "Are you sure you want to overwrite it?").arg(filestr),
                i18n("Overwrite existing file?"), i18n("Overwrite") );
            if (result != KMessageBox::Continue) {
                return;
            }
        }
        break;
    }
    filename2saveAs = filestr;
    slotSaveFile();
}

void MainWindow::slotOpenFile() {
   ///@todo migrate to KURL, probably when going from KTextEdit to KTextEditor::Editor
    QString filestr = KFileDialog::getOpenFileName(QString(":logo_dir"), QString("*.logo|") +
      i18n("Logo files"), this, i18n("Open logo file..."));
    if ( !filestr.isEmpty() ) {
        QFile file(filestr);
        if ( !file.open(IO_ReadOnly) ) {
            return;
        }
        if ( editor->isModified() ) {
            int result = KMessageBox::warningContinueCancel( this,
              i18n("The changes you have made to the file you "
                   "are currently working on (%1) are not saved. "
                   "By continuing you will lose all the changes "
                   "you have made.").arg( filestr ),
              i18n("Unsaved file..."), i18n("Discard changes") );
            if (result != KMessageBox::Continue) {
                return;
            }
        }
        QTextStream stream(&file);
        editor->setText( stream.read() );

        CurrentFile = filestr;
        setCaption(CurrentFile);
        slotStatusBar(i18n("Opened file: %1").arg(CurrentFile), 1);
    } else { 
        slotStatusBar(i18n("Opening aborted, nothing opened."), 1);
    }
}

void MainWindow::slotQuit() {
    if ( editor->isModified() ) {
        slotStatusBar(i18n("Quitting KTurtle..."), 1);
        // make sure the dialog looks good with new -- unnamed -- files.
        int result = KMessageBox::warningYesNoCancel( this,
        i18n("The changes you have made are not saved. "
             "By quitting KTurtle you will lose all the changes "
             "you have made."),
        i18n("Unsaved file..."), i18n("Save"), i18n("Discard changes and Quit") );
        if (result == KMessageBox::Cancel) {
            slotStatusBar(i18n("Quitting aborted."), 1);
            return;
        } else if (result == KMessageBox::Yes) {
            slotSaveFile();
            if ( CurrentFile.isEmpty() ) {
                return; // when saveAs get cancelled or X-ed it should not quit
            }
        }
    }
    // saveOptions();   / @todo: Implement KConfig
    close();
}

void MainWindow::slotRun() {
    allreadyError = false;
    run->setIcon("stop");
    run->setText("Stop Execution");   
    slotStatusBar(i18n("Parsing commands..."), 1);
    
    string txt=editor->text().latin1();///@todo interpreter shouldnt need extra "[" & "]"
    stringbuf sbuf(txt , ios_base::in);
    istream in(&sbuf);
    
    Parser parser(in);
    connect( &parser, SIGNAL(ErrorMsg(QString, int, int, int) ), 
             this, SLOT(slotErrorDialog(QString, int, int, int) ) );
    
    // parsing and executing...
    if( parser.parse() ) {
        TreeNode* root = parser.getTree();
        root->showTree(root); // show parsetree  DEBUG OPTION

        slotStatusBar(i18n("Executing commands..."), 1); 
        Executer exe(root); // make Executer object, 'exe', and have it point to the root
        connect( &exe, SIGNAL(ErrorMsg(QString, int, int, int) ), 
                 this, SLOT(slotErrorDialog(QString, int, int, int) ) );
        connect( &exe, SIGNAL( Finished() ), this, SLOT( slotExecutionFinished() ) );

        // Connect the signals form Executer to the slots from Canvas:
       ///@todo / @todo / @todo Implement new funktions!!
        connect( &exe, SIGNAL( Clear() ),
                 TurtleView, SLOT( slotClear() ) );
        connect( &exe, SIGNAL( Go(int, int) ),
                 TurtleView, SLOT( slotGo(int, int) ) );
        connect( &exe, SIGNAL( GoX(int) ),
                 TurtleView, SLOT( slotGoX(int) ) );
        connect( &exe, SIGNAL( GoY(int) ),
                 TurtleView, SLOT( slotGoY(int) ) );
        connect( &exe, SIGNAL( Forward(int) ),
                 TurtleView, SLOT( slotForward(int) ) );
        connect( &exe, SIGNAL( Backward(int) ),
                 TurtleView, SLOT( slotBackward(int) ) );
        connect( &exe, SIGNAL( Direction(double) ),
                 TurtleView, SLOT( slotDirection(double) ) );
        connect( &exe, SIGNAL( TurnLeft(double) ),
                 TurtleView, SLOT( slotTurnLeft(double) ) );
        connect( &exe, SIGNAL( TurnRight(double) ),
                 TurtleView, SLOT( slotTurnRight(double) ) );
        connect( &exe, SIGNAL( Center() ),
                 TurtleView, SLOT( slotCenter() ) );
        connect( &exe, SIGNAL( SetPenWidth(int) ),
                 TurtleView, SLOT( slotSetPenWidth(int) ) );
        connect( &exe, SIGNAL( PenUp() ),
                 TurtleView, SLOT( slotPenUp() ) );
        connect( &exe, SIGNAL( PenDown() ),
                 TurtleView, SLOT( slotPenDown() ) );
        connect( &exe, SIGNAL( SetFgColor(int, int, int) ),
                 TurtleView, SLOT( slotSetFgColor(int, int, int) ) );
        connect( &exe, SIGNAL( SetBgColor(int, int, int) ),
                 TurtleView, SLOT( slotSetBgColor(int, int, int) ) );
        connect( &exe, SIGNAL( ResizeCanvas(int, int) ),
                 TurtleView, SLOT( slotResizeCanvas(int, int) ) );
        connect( &exe, SIGNAL( SpriteShow() ),
                 TurtleView, SLOT( slotSpriteShow() ) );
        connect( &exe, SIGNAL( SpriteHide() ),
                 TurtleView, SLOT( slotSpriteHide() ) );
        connect( &exe, SIGNAL( SpritePress() ),
                 TurtleView, SLOT( slotSpritePress() ) );
        connect( &exe, SIGNAL( SpriteChange(int) ),
                 TurtleView, SLOT( slotSpriteChange(int) ) );
        connect( &exe, SIGNAL( Print(QString) ),
                 TurtleView, SLOT( slotPrint(QString) ) );
        connect( &exe, SIGNAL( FontType(QString, QString) ),
                 TurtleView, SLOT( slotFontType(QString, QString) ) );
        connect( &exe, SIGNAL( FontSize(int) ),
                 TurtleView, SLOT( slotFontSize(int) ) );
        connect( &exe, SIGNAL( WrapOn() ),
                 TurtleView, SLOT( slotWrapOn() ) );
        connect( &exe, SIGNAL( WrapOff() ),
                 TurtleView, SLOT( slotWrapOff() ) );
        connect( &exe, SIGNAL( Reset() ),
                 TurtleView, SLOT( slotReset() ) );

        exe.run();
        slotStatusBar(i18n("Done."), 1); // this will override all the err-msgs
    } else {
        slotStatusBar(i18n("Parsing failed!"), 1);
        slotExecutionFinished();
    }

    //run->setIcon("gear");

// wait for finished or error signal to change 'stop' icon back to 'gear'
// finished: change button + statusbar
}

void MainWindow::slotExecutionFinished() {
    run->setIcon("gear");
    run->setText( i18n("&Execute Commands") );
    slotStatusBar(i18n("Done."), 1);
}

// / @todo: linenumbers; probably when going from KTextEdit to KTextEditor::Editor
// void MainWindow::slotLineNumbers() {
//     editor->toggleLineNumbersOn();
// }

// Implementation of FullScreen functionality //
//
// switch to showFullScreen() or showNormal()
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
    if(id == 0) { // the version part of the statusbar
        text = " " + text + " "; // help the layout
    } else { // if id is not 0 it must be 1 (error cacher)
        text = " " + text; // help the layout
    }
    statusBar()->changeItem(text, id);
}

void MainWindow::slotErrorDialog(QString msg, int row, int col, int code) {
    if(allreadyError) { return; } // one error dialog per 'run' is enough... (see next line)
    // allreadyError = true; NO I WANT TO SE ALL ERRORS for the time beeing
    QString line;
    if( row <= 0 || col <= 0 ) {
        line = ".";
    } else {
//         RowCol = QString(" on row %1, column %2.").arg(row).arg(col); // no column, it over informs
         line = QString(" on line %1.").arg(row);
    }
    QString ErrorType;
    if( 1000 <= code < 2000 ) {
        ErrorType = i18n("Parse Error");
    } else if( 3000 <= code < 4000 ) {
        ErrorType = i18n("Internal Error");
    } else if( code >= 5000 ) {
        ErrorType = i18n("Execution Error");
    } else if( code < 1000 ) {
        ErrorType = i18n("Error");
    }
    KMessageBox::detailedSorry( this, msg + line, i18n("Error code: %1").arg(code), ErrorType );
}

void MainWindow::slotUpdateCanvas() {
    // fixes a non updateing bug
    // I tried doing this by connecting Canvas's resized to BaseWidget's update...
    // but i had no luck :(       this worked though
    TurtleView->hide();
    TurtleView->show();
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
    WidthHeightBox = new QGroupBox(i18n("Initial canvas size"), general);
    WidthHeightBox->setGeometry(QRect( 20, 40, 440, 160) );
    kcfg_CanvasWidth = new KIntNumInput(WidthHeightBox, "kcfg_CanvasWidth");
    kcfg_CanvasWidth->setMinValue(1);
    kcfg_CanvasWidth->setGeometry( QRect(270, 50, 130, 30) );
    kcfg_CanvasHeight = new KIntNumInput(WidthHeightBox, "kcfg_CanvasHeight");
    kcfg_CanvasHeight->setMinValue(1);
    kcfg_CanvasHeight->setGeometry( QRect(270, 100, 130, 30) );
    WidthLabel = new QLabel( kcfg_CanvasWidth, i18n("Canvas &width:"), WidthHeightBox );
    WidthLabel->setGeometry( QRect(40, 50, 190, 30) );
    HeightLabel = new QLabel( kcfg_CanvasHeight, i18n("Canvas &height:"), WidthHeightBox );
    HeightLabel->setGeometry( QRect(40, 100, 190, 30) );
    dialog->addPage( general, i18n("General"), "package_settings", i18n("General Settings") );

    // making the filling for the 'Language' settings dept.    
    QWidget *language = new QWidget();
    kcfg_TranslationFilePath = new KURLRequester(language, "kcfg_TranslationFilePath");
    kcfg_TranslationFilePath->setGeometry( QRect(20, 90, 440, 30) );
    TranslationFileLabel = new QLabel(kcfg_TranslationFilePath, i18n("&Translation file:"), language);
    TranslationFileLabel->setGeometry( QRect(20, 40, 200, 30) );
    dialog->addPage( language, i18n("Language"), "locale", i18n("Language Settings") );

    // When the user clicks OK or Apply we want to update our settings.
    connect( dialog, SIGNAL( settingsChanged() ), this, SLOT( slotUpdateSettings() ) );

    // Display the dialog.
    dialog->setInitialSize( QSize(610, 400) );
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

void MainWindow::readConfig() {
	//in case the xml files are not installed. I believe it should quit in that case as the kstandardirs has been 
	//searched for the xml files
        if ( locate("data", "kturtle/data/logokeywords.en_US.xml").isNull() ) {
        	kdDebug(0)<<"--3--"<<endl;
            	slotErrorDialog( i18n("Could not find the command translation file.\n"
                                  "Please go to \"Settings -> Configure KTurtle\" and "
                                  "specify the full name and path to "
                                  "\"logokeywords.en_US.xml\" or any other logokeywords "
                                  "file. Otherwise KTurtle will not operate." ) );
        }
	
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

void MainWindow::setRunEnabled(){
    run->setEnabled(true);
}

#include "kturtle.moc"
