/*
 * Copyright (C) 2003 Cies Breijs <cies # showroommama ! nl>
 */

#include <stdlib.h>

#include <qtimer.h>

#include <kaccel.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kfiledialog.h>
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

#include <ktexteditor/editorchooser.h>

#include "settings.h"
#include "kturtle.h"


// StatusBar field IDs
#define KTURTLE_ID_GEN 1

MainWindow::MainWindow(KTextEditor::Document *doc) : editor(0) {
    // set the shell's ui resource file
    if (!doc) {
      if ( !(doc = KTextEditor::EditorChooser::createDocument(0,"KTextEditor::Document") ) ) {
        KMessageBox::error(this, i18n("A KDE text editor component could not be found!\n"
                                     "Please check your KDE installation."));
        kapp->exit(1);
      }  
    // docList.append(doc);
    }

    setupCanvas();
    setupActions();
    setupStatusBar();
   
    setXMLFile("kturtleui.rc");
    createShellGUI( true );
   
    EditorDock = new QDockWindow(this);
    EditorDock->setNewLine(true);
    EditorDock->setFixedExtentWidth(250);
    EditorDock->setFixedExtentHeight(150);
    EditorDock->setResizeEnabled(true);
    EditorDock->setFrameShape(QFrame::ToolBarPanel);
    moveDockWindow(EditorDock, Qt::DockLeft);
    editor = doc->createView (EditorDock, 0L);
    // ei is the editor interface which allows us to access the text in the part
    ei = dynamic_cast<KTextEditor::EditInterface*>( doc );
    EditorDock->setWidget(editor);
    //dynamic_cast<KTextEditor::Document*>(this)->actionCollection()->remove(action("file_quit"));
    //doc->actionCollection()->remove(action("file_quit"));
    
    ///allow to enable run only when some text is written in editor
    connect(editor->document(), SIGNAL(textChanged()), this, SLOT(setRunEnabled()));
    
    guiFactory()->addClient(editor);
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
  
    // Set up file menu
    KStdAction::openNew(this, SLOT(slotNewFile()), actionCollection());
    openExAction = new KAction(i18n("Open Examples"), "bookmark_folder", 0, this, SLOT(slotOpenEx()),
      actionCollection(), "open_examples");
    KStdAction::open(this, SLOT(slotOpenFile()), actionCollection());
    /// @todo implement a recent files list
    // recenter = KStdAction::openRecent(this, SLOT(openRecent(int)), actionCollection());
    KStdAction::save(this, SLOT(slotSaveFile()), actionCollection());
    KStdAction::saveAs(this, SLOT(slotSaveAs()), actionCollection());
    //
    run = new KAction(i18n("&Execute Commands"), "gear", 0, this, SLOT( slotExecute() ),
      actionCollection(), "run");
    run->setEnabled(false);

    //KAction *a = new KAction(i18n("Reloa&d"), "file_reload", KStdAccel::reload(), this, SLOT(reloadFile()), //dynamic_cast<KTextEditor::Document*>(this)->actionCollection(), "file_reload");
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
     m_recentFiles = KStdAction::openRecent(this, SLOT(slotOpen(const KURL&)),
                                         actionCollection());
    
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

// Implementation of most of the items in the File and Edit menus //
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
    slotStatusBar(i18n("New file... Happy coding!"), 1); 
}

void MainWindow::slotSaveFile() {
    // if no filename yet, go get it first
    if ( CurrentFile.isEmpty() && filename2saveAs.isEmpty() ) {
        slotSaveAs();
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
    stream << ei->text();
    file.close();
    
    editor->document()->setModified(false);
    CurrentFile = filestr;
    setCaption(CurrentFile);
    slotStatusBar(i18n("Saved file to: %1").arg(CurrentFile), 1); 
    //@TODO add file in recent files
}

void MainWindow::slotSaveAs() {
       KURL url;
      while(true) {
      url = KFileDialog::getSaveURL( QString(":logo_dir"), QString("*.logo|") +
      i18n("Logo files"), this, i18n("Save logo file...") );
        if (url.isEmpty()) { // when cancelled the KFiledialog?
            return;
        }
        QFile file(url.url());
        if ( file.exists() ) {
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
    filename2saveAs = url.url();
    slotSaveFile();
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
}

void MainWindow::slotQuit() {
    if ( editor->document()->isModified() ) {
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
    KConfig *config = kapp->config();
    writeConfig(config);

    close();
}


static TreeNode::const_iterator ss_it;

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
    slotStatusBar(i18n("Parsing commands..."), 1);
    kapp->processEvents();
    
    string txt = ( ei->text() + "\n" ).latin1(); // the /n is needed for proper parsing
    stringbuf sbuf(txt, ios_base::in);
    istream in(&sbuf);
    Parser parser(in);
    connect( &parser, SIGNAL(ErrorMsg(QString, int, int, int) ), 
             this, SLOT(slotErrorDialog(QString, int, int, int) ) );
    
    // parsing and executing...
    if( parser.parse() ) {
        TreeNode* root = parser.getTree();
        root->showTree(root); // show parsetree  DEBUG OPTION

        slotStatusBar(i18n("Executing commands..."), 1); 
        exe = new Executer(root); // make Executer object, 'exe', and have it point to the root
        connect( exe, SIGNAL( ErrorMsg(QString, int, int, int) ), 
                 this, SLOT( slotErrorDialog(QString, int, int, int) ) );

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
            slotStatusBar(i18n("Done."), 1);
            finishExecution();
        } else {
            slotStatusBar(i18n("Execution aborted."), 1);
            finishExecution();
        }
    } else {
        slotStatusBar(i18n("Parsing failed!"), 1);
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

void MainWindow::readConfig(KConfig *config) {
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
	
	config->setGroup("General Options");
	m_recentFiles->loadEntries(config, "Recent Files");
}

void MainWindow::writeConfig(KConfig *config) {
	config->setGroup("General Options");
	m_recentFiles->saveEntries(config, "Recent Files");
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

void MainWindow::setRunEnabled() {
    run->setEnabled(true);
}

void MainWindow::slotOpenEx() {
    KURL url;
    url.setPath( locate("data", "kturtle/examples/en/"));
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
       if (file.open(IO_ReadWrite)){
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
        slotStatusBar(i18n("Opened file: %1").arg(url.fileName()), 1);
	}
    } else { 
        slotStatusBar(i18n("Opening aborted, nothing opened."), 1);
    }
}

#include "kturtle.moc"
