/*
 * KTurtle, Copyright (C) 2003-04 Cies Breijs <cies # kde ! nl>

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


// BEGIN includes and defines

#include <stdlib.h>

#include <qregexp.h>
#include <qpainter.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

#include <kapplication.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kimageio.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <ksavefile.h>
#include <kstatusbar.h>

#include <ktexteditor/clipboardinterface.h>
#include <ktexteditor/cursorinterface.h>
#include <ktexteditor/editorchooser.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/highlightinginterface.h>
#include <ktexteditor/printinterface.h>
#include <ktexteditor/selectioninterface.h>
#include <ktexteditor/undointerface.h>
#include <ktexteditor/viewcursorinterface.h>

#include "settings.h"
#include "kturtle.h"

// StatusBar field IDs
#define IDS_INS         0
#define IDS_STATUS      1
#define IDS_STATUS_CLM  3
#define IDS_LANG        5

// END



// BEGIN constructor and destructor

MainWindow::MainWindow(KTextEditor::Document *document) : editor(0) {
	// the initial values
	CurrentFile = "";
	filename2saveAs = "";
	setCaption( i18n("Untitled") );
	picker = 0; // for the colorpickerdialog
	executing = false;
	b_fullscreen = false;

	// set the shell's ui resource file
	if (!document) {
		if ( !(document = KTextEditor::EditorChooser::createDocument(0,"KTextEditor::Document") ) ) {
			KMessageBox::error(this, i18n("A KDE text-editor component could not be found;\n"
					"please check your KDE installation."));
			kapp->exit(1);
		}
		// docList.append(doc);
	}
	doc = document;

	setupCanvas();
	setupEditor();

	setupStatusBar();
	setupActions();
	createShellGUI( true );
	setMinimumSize(200,200);

	// init with more usefull size, stolen from kwite (they stole it from konq)
	if ( !initialGeometrySet() && !kapp->config()->hasGroup("MainWindow Settings") ) {
		resize( 640, 480 );
	}

	KConfig *config = kapp->config();
	readConfig(config);
}

MainWindow::~MainWindow() {  // The MainWindow destructor
	delete editor->document();
}

// END



// BEGIN setup's (actions, editor, statusbar, canvas)

void MainWindow::setupActions() {
	KActionCollection *ac = actionCollection();
	// File menu
	KStdAction::openNew(this, SLOT(slotNewFile()), ac);
	openExAction = new KAction(i18n("Open Examples..."), "bookmark_folder", CTRL+Key_E, this, SLOT(slotOpenEx()), ac, "open_examples");
	KStdAction::open(this, SLOT(slotOpenFile()), ac);
	m_recentFiles = KStdAction::openRecent(this, SLOT(slotOpen(const KURL&)), ac);
	KStdAction::save(this, SLOT(slotSaveFile()), ac);
	KStdAction::saveAs(this, SLOT(slotSaveAs()), ac);
	new KAction(i18n("Save &Canvas..."), 0, 0, this, SLOT(slotSaveCanvas()), ac, "save_canvas");
	run = new KAction(i18n("&Execute Commands"), "gear", ALT+Key_Return, this, SLOT( slotExecute() ), ac, "run");
	stop = new KAction(i18n("Stop E&xecution"), "stop", Key_Escape, this, SLOT( slotAbortExecution() ), ac, "stop");
	stop->setEnabled(false);
	KStdAction::print(this, SLOT(slotPrint()), ac);
	KStdAction::quit(this, SLOT(close()), ac);
	// Edit actions
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
	// View actions
	new KToggleAction(i18n("Show &Line Numbers"), 0, Key_F11, this, SLOT(slotToggleLineNumbers()), ac, "line_numbers");
	m_fullscreen = KStdAction::fullScreen(this, SLOT( slotToggleFullscreen() ), ac, this, "full_screen");
	m_fullscreen->setChecked(b_fullscreen);

	// Tools actions
	colorpicker = new KToggleAction(i18n("&Color Picker"), "colorize", ALT+Key_C, this, SLOT(slotColorPicker()), ac, "color_picker");
	new KAction(i18n("&Indent"), "indent", CTRL+Key_I, this, SLOT(slotIndent()), ac, "edit_indent");
	new KAction(i18n("&Unindent"), "unindent", CTRL+SHIFT+Key_I, this, SLOT(slotUnIndent()), ac, "edit_unindent");
	new KAction(i18n("Cl&ean Indentation"), 0, 0, this, SLOT(slotCleanIndent()), ac, "edit_cleanIndent");
	new KAction(i18n("Co&mment"), 0, CTRL+Key_D, this, SLOT(slotComment()), ac, "edit_comment");
	new KAction(i18n("Unc&omment"), 0, CTRL+SHIFT+Key_D, this, SLOT(slotUnComment()), ac, "edit_uncomment");
	// Settings actions
	KStdAction::preferences( this, SLOT( slotSettings() ), ac );
	new KAction(i18n("&Configure Editor..."), "configure", 0, this, SLOT(slotEditor()), ac, "set_confdlg");
	// Help actions
	ContextHelp = new KAction(0, 0, Key_F2, this, SLOT(slotContextHelp()), ac, "context_help");
	slotContextHelpUpdate(); // this sets the label of this action
	setXMLFile("kturtleui.rc");
	setupGUI();
}

void MainWindow::setupEditor() {
	EditorDock = new QDockWindow(this);
	EditorDock->setNewLine(true);
	EditorDock->setFixedExtentWidth(250);
	EditorDock->setFixedExtentHeight(150);
	EditorDock->setResizeEnabled(true);
	EditorDock->setFrameShape(QFrame::ToolBarPanel);
	QWhatsThis::add( EditorDock, i18n( "This is the code editor, here you type the Logo commands to instruct the turtle. You can also open an existing Logo program with File->Open Examples... or File->Open." ) );
	moveDockWindow(EditorDock, Qt::DockLeft);
	editor = doc->createView (EditorDock, 0L);
	// ei is the editor interface which allows us to access the text in the part
	ei = dynamic_cast<KTextEditor::EditInterface*>(doc);
	EditorDock->setWidget(editor);

	// default the highlightstyle to "logo" using the needed i18n
	kdDebug(0)<<"The HighllightStyle for the Code Editor: "<<Settings::logoLanguage()<<endl;
	slotSetHighlightstyle( Settings::logoLanguage() );

	// allow the cursor position to be indicated in the statusbar
	connect(editor, SIGNAL(cursorPositionChanged()), this, SLOT(slotCursorStatusBar()));
	// and update the context help menu item
	connect(editor, SIGNAL(cursorPositionChanged()), this, SLOT(slotContextHelpUpdate()));
}

void MainWindow::setupStatusBar() {
	// statusBar()->insertItem(" ", 2, 0, true);//to add a space, ID is 2
	statusBar()->insertItem("", IDS_STATUS, 1, false);
	statusBar()->setItemAlignment(IDS_STATUS, AlignLeft);
	statusBar()->insertItem("", IDS_LANG, 1, false);
	statusBar()->setItemAlignment(IDS_LANG, AlignLeft);
	statusBar()->insertItem("", IDS_STATUS_CLM, 0, true);
	statusBar()->insertItem("", IDS_INS, 0, true);
	// fill the statusbar
	slotStatusBar(i18n("Welcome to KTurtle..."),  IDS_STATUS); // the message part
	slotStatusBar(i18n(" Line: %1 Column: %2 ").arg(1).arg(1), IDS_STATUS_CLM);
	slotStatusBar(i18n(" INS "), IDS_INS);
}

void MainWindow::setupCanvas() {
	BaseWidget = new QWidget(this);
	setCentralWidget(BaseWidget);
	BaseLayout = new QGridLayout(BaseWidget, 0, 0);
	TurtleView = new Canvas(BaseWidget);
	BaseLayout->addWidget(TurtleView, 0, 0, AlignCenter);
	QWhatsThis::add( TurtleView, i18n( "This is the canvas, here the turtle draws a picture." ) );
	TurtleView->show();
	connect( TurtleView, SIGNAL( CanvasResized() ), this, SLOT( slotUpdateCanvas() ) );
}

// END



// BEGIN staturbar related functions

void MainWindow::slotStatusBar(QString text, int id) {
	if(id == IDS_INS) { // the version part of the statusbar
		text = " " + text + " "; // help the layout
	} else { // if id is not 0 it must be 1 (error cacher)
		text = " " + text; // help the layout
	}
	statusBar()->changeItem(text, id);
}

void MainWindow::slotCursorStatusBar() {
	uint cursorLine;
	uint cursorCol;
	dynamic_cast<KTextEditor::ViewCursorInterface*>(editor)->cursorPositionReal(&cursorLine, &cursorCol);
	QString linenumber = i18n(" Line: %1 Column: %2 ").arg(cursorLine+1).arg(cursorCol+1);
	statusBar()->changeItem(linenumber, IDS_STATUS_CLM);
}

// END



// BEGIN file menu related fuctions (new, open, save()as, image, print, quit)

void MainWindow::slotNewFile() {
	if ( !editor->document()->isModified() && CurrentFile == "" ) {
		return; // do nothing when nothing is to be done
	}
	if ( editor->document()->isModified() ) {
		int result = KMessageBox::warningContinueCancel( this,
		i18n("The changes you have made to the file you "
		"are currently working are not saved. "
		"By continuing you will lose all the changes you have made."),
		i18n("Unsaved File"), i18n("&Discard Changes") );
		if (result != KMessageBox::Continue) {
			return;
		}
	}
	ei->clear();// clear the editor
	TurtleView->slotClear();// clear the view
	editor->document()->setModified(false);
	CurrentFile = "";
	setCaption( i18n("Untitled") );
	slotStatusBar(i18n("New file... Happy coding!"), IDS_STATUS);
}

void MainWindow::loadFile(KURL url) {
	m_recentFiles->addURL( url );
	QString myFile = url.path();
	if ( !myFile.isEmpty() ) {
		QFile file(myFile);
		//if ( !file.open(IO_ReadWrite) ) {
		//  return;
		// }
		if ( file.open(IO_ReadOnly) ) {
			if ( editor->document()->isModified() ) {
				int result = KMessageBox::warningContinueCancel( this,
				i18n("The changes you have made to the file you "
				"are currently working on (%1) are not saved. "
				"By continuing you will lose all the changes "
				"you have made.").arg( myFile ),
				i18n("Unsaved File"), i18n("&Discard Changes") );
				if (result != KMessageBox::Continue) {
					return;
				}
			}
			QTextStream stream(&file);
			stream.setEncoding(QTextStream::UnicodeUTF8);
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

void MainWindow::slotOpenFile() {
	slotNewFile();
	KURL url = KFileDialog::getOpenURL( QString(":logo_dir"), QString("*.logo|") +
	i18n("Logo Files"), this, i18n("Open Logo File") );
	if( !url.isEmpty() ) {
		loadFile(url);
	}
	editor->document()->setModified(false);
}

void MainWindow::slotOpen(const KURL& url) {
	slotNewFile();
	if( !url.isEmpty() ) {
		loadFile(url);
	}
	editor->document()->setModified(false);
}

void MainWindow::slotOpenEx() {
	KURL url;
	url.setPath( locate("data", "kturtle/examples/" + Settings::logoLanguage() + "/"));
	url = KFileDialog::getOpenURL( url.path(),
		QString("*.logo|") + i18n("Logo Examples Files"), this,
		i18n("Open Logo Example File") );
	loadFile(url);
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

void MainWindow::slotSave(KURL &url) {
	if ( !url.isEmpty() ) {
		filename2saveAs = url.url();
		QString mString=ei->text(); //get the text
		ei->setText(mString.utf8());//convert it to utf8
		editor->document()->saveAs(url);
		loadFile(url);//reload the file as utf8 otherwise display weird chars
		CurrentFile = url.fileName();
		setCaption(CurrentFile);
		slotStatusBar(i18n("Saved file to: %1").arg(CurrentFile),  IDS_STATUS);
		m_recentFiles->addURL( url );
		editor->document()->setModified(false);
	} else {
		slotSaveAs();
	}
}

void MainWindow::slotSaveAs() {
	KURL url;
	while(true) {
		url = KFileDialog::getSaveURL( QString(":logo_dir"), QString("*.logo|") +
		i18n("Logo Files"), this, i18n("Save Logo File") );
		if (url.isEmpty()) { // when cancelled the KFiledialog?
			return;
		}
		if (QFile(url.path()).exists()) {
			int result = KMessageBox::warningContinueCancel ( this,
			i18n("A file named \"%1\" already exists;\n"
			"are you sure you want to overwrite it?").arg( url.url() ),
			i18n("Overwrite Existing File?"), i18n("&Overwrite") );
			if (result != KMessageBox::Continue) {
				return;
			}
		}
		break;
	}
	slotSave(url);
}

void MainWindow::slotSaveCanvas() {
	KURL url;
	while(true) {
		url = KFileDialog::getSaveURL( QString(":logo_dir"), QString("*.png|") +
		i18n("Pictures"), this, i18n("Save Canvas as Picture") );
		if (url.isEmpty()) { // when cancelled the KFiledialog?
			return;
		}
		if (QFile(url.path()).exists()) {
			int result = KMessageBox::warningContinueCancel( this,
				i18n("A picture named \"%1\" already exists;\n"
				"are you sure you want to overwrite it?").arg(url.url()),
				i18n("Overwrite Existing Picture?"), i18n("&Overwrite") );
			if (result != KMessageBox::Continue) {
				return;
			}
		}
		break;
		}
	QString type( KImageIO::type(url.path()) );
	if ( type.isNull() ) {
		type = "PNG";
	}
	bool ok = false;
	QPixmap* pixmap = TurtleView->Canvas2Pixmap();
	if ( url.isLocalFile() ) {
		KSaveFile saveFile( url.path() );
		if ( saveFile.status() == 0 ) {
			if ( pixmap->save( saveFile.file(), type.latin1() ) ) {
				ok = saveFile.close();
			}
		}
	}
	if ( !ok ) {
		kdWarning() << "KTurtle was unable to save the canvas drawing" << endl;

		QString caption = i18n("Unable to Save Image");
		QString text = i18n("KTurtle was unable to save the image to\n%1.").arg(url.prettyURL());
		KMessageBox::error(this, text, caption);
	}
	slotStatusBar(i18n("Saved canvas to: %1").arg(url.fileName()),  IDS_STATUS);
}

void MainWindow::slotPrint() {
	int result = KMessageBox::questionYesNoCancel( this,
	i18n("Do you want to print the Logo code or the canvas?"),
	i18n("Part to print"), i18n("Print &Logo Code"), i18n("Print &Canvas") );
	if (result == KMessageBox::Yes) {
		dynamic_cast<KTextEditor::PrintInterface*>(doc)->printDialog();
		return;
	}
	if (result == KMessageBox::No) {
		KPrinter printer;
		if ( printer.setup(this) ) {
			QPainter painter(&printer);
			QPixmap *CanvasPic = TurtleView->Canvas2Pixmap();
			painter.drawPixmap(0, 0, *CanvasPic);
		}
		return;
	}
	slotStatusBar(i18n("Printing aborted."),  IDS_STATUS);
}

bool MainWindow::queryClose() {
	if ( editor->document()->isModified() ) {
		slotStatusBar(i18n("Quitting KTurtle..."),  IDS_STATUS);
		// make sure the dialog looks good with new -- unnamed -- files.
		int result = KMessageBox::warningYesNoCancel( this,
		i18n("The changes you have made are not saved. "
			"By quitting KTurtle you will lose all the changes "
			"you have made."),
		i18n("Unsaved File"), i18n("&Save"), i18n("Discard Changes And &Quit") );
		if (result == KMessageBox::Cancel) {
			slotStatusBar(i18n("Quitting aborted."),  IDS_STATUS);
			return false;
		} else if (result == KMessageBox::Yes) {
			slotSaveFile();
			if ( CurrentFile.isEmpty() ) {
				return false; // when saveAs get cancelled or X-ed it should not quit
			}
		}
	}
	KConfig *config = kapp->config();
	config->setGroup("General Options");
	m_recentFiles->saveEntries(config, "Recent Files");
	config->sync();
	return true;
}

// END



// BEGIN run related functions

void MainWindow::slotExecute() {
	if ( executing ) {
		slotAbortExecution();
	} else {
		startExecution();
	}
}

void MainWindow::startExecution() {
	allreadyError = false;
	executing = true;

	// check if execution should be runnin 'fullscreen'
	if (b_fullscreen) {
		slotShowEditor();
		statusBar()->hide();
		// menuBar()->hide();   <--- seems pretty hard to hide the menubar
		toolBar()->hide();
	}
	// maybe the fullscreen funktion should be a screen ocupying widget with the canvas in the middle and a button that only shows 1 sec after the execution is finished. (what to do with endless loop scripts?) 

	run->setEnabled(false);
	stop->setEnabled(true);

	slotStatusBar(i18n("Parsing commands..."),  IDS_STATUS);
	kdDebug(0)<<"############## PARSING STARTED ##############"<<endl;

	kapp->processEvents();

	QString txt = ei->text() + "\n"; // the /n is needed for proper parsing
	QTextIStream in(&txt);
	errMsg = new ErrorMessage(this);
	Parser parser(in);
	connect( &parser, SIGNAL(ErrorMsg(QString, uint, uint, uint) ),
		errMsg, SLOT(slotAddError(QString, uint, uint, uint) ) );
	connect( errMsg, SIGNAL(SetCursor(uint, uint) ),
		this, SLOT(slotSetCursorPos(uint, uint) ) );

	// parsing and executing...
	if( parser.parse() ) {
		TreeNode* root = parser.getTree();
		kdDebug(0)<<"############## PARSING FINISHED ##############"<<endl;
		root->showTree(root); // show parsetree  DEBUG OPTION

		slotStatusBar(i18n("Executing commands..."),  IDS_STATUS);
		kdDebug(0)<<"############## EXECUTION STARTED ##############"<<endl;
		exe = new Executer(root); // make Executer object, 'exe', and have it point to the root
		connect( exe, SIGNAL( ErrorMsg(QString, uint, uint, uint) ),
			errMsg, SLOT( slotAddError(QString, uint, uint, uint) ) );
		connect( exe, SIGNAL( InputDialog(QString&) ),
			this, SLOT( slotInputDialog(QString&) ) );
		connect( exe, SIGNAL( MessageDialog(QString) ),
			this, SLOT( slotMessageDialog(QString) ) );

		// Connect the signals form Executer to the slots from Canvas:
		connect( exe, SIGNAL( Clear() ), TurtleView, SLOT( slotClear() ) );
		connect( exe, SIGNAL( Go(int, int) ), TurtleView, SLOT( slotGo(int, int) ) );
		connect( exe, SIGNAL( GoX(int) ), TurtleView, SLOT( slotGoX(int) ) );
		connect( exe, SIGNAL( GoY(int) ), TurtleView, SLOT( slotGoY(int) ) );
		connect( exe, SIGNAL( Forward(int) ), TurtleView, SLOT( slotForward(int) ) );
		connect( exe, SIGNAL( Backward(int) ), TurtleView, SLOT( slotBackward(int) ) );
		connect( exe, SIGNAL( Direction(double) ), TurtleView, SLOT( slotDirection(double) ) );
		connect( exe, SIGNAL( TurnLeft(double) ), TurtleView, SLOT( slotTurnLeft(double) ) );
		connect( exe, SIGNAL( TurnRight(double) ), TurtleView, SLOT( slotTurnRight(double) ) );
		connect( exe, SIGNAL( Center() ), TurtleView, SLOT( slotCenter() ) );
		connect( exe, SIGNAL( SetPenWidth(int) ), TurtleView, SLOT( slotSetPenWidth(int) ) );
		connect( exe, SIGNAL( PenUp() ), TurtleView, SLOT( slotPenUp() ) );
		connect( exe, SIGNAL( PenDown() ), TurtleView, SLOT( slotPenDown() ) );
		connect( exe, SIGNAL( SetFgColor(int, int, int) ), TurtleView, SLOT( slotSetFgColor(int, int, int) ) );
		connect( exe, SIGNAL( SetBgColor(int, int, int) ), TurtleView, SLOT( slotSetBgColor(int, int, int) ) );
		connect( exe, SIGNAL( ResizeCanvas(int, int) ), TurtleView, SLOT( slotResizeCanvas(int, int) ) );
		connect( exe, SIGNAL( SpriteShow() ), TurtleView, SLOT( slotSpriteShow() ) );
		connect( exe, SIGNAL( SpriteHide() ), TurtleView, SLOT( slotSpriteHide() ) );
		connect( exe, SIGNAL( SpritePress() ), TurtleView, SLOT( slotSpritePress() ) );
		connect( exe, SIGNAL( SpriteChange(int) ), TurtleView, SLOT( slotSpriteChange(int) ) );
		connect( exe, SIGNAL( Print(QString) ), TurtleView, SLOT( slotPrint(QString) ) );
		connect( exe, SIGNAL( FontType(QString, QString) ), TurtleView, SLOT( slotFontType(QString, QString) ) );
		connect( exe, SIGNAL( FontSize(int) ), TurtleView, SLOT( slotFontSize(int) ) );
		connect( exe, SIGNAL( WrapOn() ), TurtleView, SLOT( slotWrapOn() ) );
		connect( exe, SIGNAL( WrapOff() ), TurtleView, SLOT( slotWrapOff() ) );
		connect( exe, SIGNAL( Reset() ), TurtleView, SLOT( slotReset() ) );

		if ( exe->run() ) {
			slotStatusBar(i18n("Done."),  IDS_STATUS);
			finishExecution();
		} else {
			slotStatusBar(i18n("Execution aborted."),  IDS_STATUS);
			finishExecution();
		}
		delete exe;
	} else {
		kdDebug(0)<<"############## PARSING FAILED ##############"<<endl;
		slotStatusBar(i18n("Parsing failed."),  IDS_STATUS);
		finishExecution();
	}

	if (errMsg->containsErrors() == false) {
		errMsg->display();
	}
}

void MainWindow::slotAbortExecution() {
	exe->abort();
}

void MainWindow::finishExecution() {
	kdDebug(0)<<"############## EXECUTION FINISHED ##############"<<endl;
	run->setEnabled(true);
	stop->setEnabled(false);
	executing = false;

	// show the editor, menu- and statusbar
	if (b_fullscreen) {
		if(!b_editorShown) {
			slotShowEditor();
		}
		statusBar()->show();
		// menuBar()->hide();   <--- seems pretty hard to hide the menubar
		toolBar()->show();
	}
}

// some other parts of the main window that need to be connected with the mainwindow

void MainWindow::slotInputDialog(QString& value) {
	value = KInputDialog::getText (i18n("Input"), value);
}

void MainWindow::slotMessageDialog(QString text) {
	KMessageBox::information( this, text, i18n("Message") );
}

// END



// BEGIN editor connections (undo, redo, cut, copy, paste, cursor, selections, find, replace, linenumbers etc.)

void MainWindow::slotEditor() {
	KAction *a = editor->actionCollection()->action("set_confdlg");
	a->activate();
}

void MainWindow::slotShowEditor() {
	if ( EditorDock->isHidden() ) {
		EditorDock->show();
		b_editorShown = true;
	} else {
		EditorDock->hide();
		b_editorShown = false;
	}
	// showEditor->setChecked(b_editorShown); // DEPRICATED
}

void MainWindow::slotSetHighlightstyle(QString langCode) {
	KTextEditor::HighlightingInterface *hli = dynamic_cast<KTextEditor::HighlightingInterface*>(doc);
	for(uint i = 0; i < hli->hlModeCount(); i++) {
		if(hli->hlModeName(i) == langCode) {
			hli->setHlMode(i);
			return;
		}
	} // and the fallback:
	for(uint i = 0; i < hli->hlModeCount(); i++) {
		if(hli->hlModeName(i) == "en_US") {
			hli->setHlMode(i);
		}
	}
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

void MainWindow::slotFind() {
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

void MainWindow::slotToggleInsert() {
	KToggleAction *a = dynamic_cast<KToggleAction*>(editor->actionCollection()->action("set_insert"));
	a->activate();
	if (a) {
		statusBar()->changeItem(a->isChecked() ? i18n(" OVR ") : i18n(" INS "), IDS_INS);
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

void MainWindow::slotToggleLineNumbers() {
	KToggleAction *a = dynamic_cast<KToggleAction*>( editor->actionCollection()->action("view_line_numbers") );
	a->activate();
}

void MainWindow::slotInsertText(QString str) {
	uint StartLine, StartCol, EndLine, EndCol;
	dynamic_cast<KTextEditor::ViewCursorInterface*>(editor)->cursorPositionReal(&StartLine, &StartCol);
	dynamic_cast<KTextEditor::EditInterface*>(doc)->insertText(StartLine, StartCol, str);
	dynamic_cast<KTextEditor::ViewCursorInterface*>(editor)->cursorPositionReal(&EndLine, &EndCol);
	dynamic_cast<KTextEditor::SelectionInterface*>(doc)->setSelection(StartLine, StartCol, EndLine, EndCol);
}

void MainWindow::slotSetCursorPos(uint row, uint col) {
	dynamic_cast<KTextEditor::ViewCursorInterface*>(editor)->setCursorPositionReal(row-2, col);
	kdDebug(0)<<"Cursor set to: ("<<row-2<<", "<<col<<")"<<endl;
}
// END



// BEGIN fullscreen functions

void MainWindow::slotToggleFullscreen() {
	if (!b_fullscreen) {
		showFullScreen(); // both calls will generate event triggering updateFullScreen()
	} else {
		if( isFullScreen() ) {
			showNormal();
		}
	}
}

bool MainWindow::event(QEvent* e) {
	// executes updateFullScreen() after a ShowFullScreen or ShowNormal event got triggered
	if (e->type() == QEvent::ShowFullScreen || e->type() == QEvent::ShowNormal) {
		updateFullScreen();
	}
	return KMainWindow::event(e);
}

void MainWindow::updateFullScreen() {
	if (isFullScreen() == b_fullscreen) {
		return;
	}
	b_fullscreen = isFullScreen();
	if (m_fullscreen) {
	m_fullscreen->setChecked(b_fullscreen);
	}
}

// END



// BEGIN configuration related functions

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

	WidthLabel = new QLabel( kcfg_CanvasWidth, i18n("Canvas width:"), WidthHeightBox );
	layout2->addWidget( WidthLabel );
	HeightLabel = new QLabel( kcfg_CanvasHeight, i18n("Canvas height:"), WidthHeightBox );
	layout2->addWidget( HeightLabel );
	layout3->addLayout( layout2 );

	layout3->addLayout( layout1 );
	WidthHeightBoxLayout->addLayout( layout3 );
	QLabel* WidthHeightLabel = new QLabel(i18n("You need to restart before these settings have effect"), WidthHeightBox);
	WidthHeightBoxLayout->addWidget( WidthHeightLabel );
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

	kcfg_LanguageComboBox = new KComboBox(groupBox1, "kcfg_LanguageComboBox");
	kcfg_LanguageComboBox->setEditable(false);
	QStringList LogoLanguageList = Settings::logoLanguageList();
	// Add the full language names to the items
	for ( QStringList::Iterator it = LogoLanguageList.begin(); it != LogoLanguageList.end(); ++it ) {
		*it = KGlobal::locale()->twoAlphaToLanguageName( (*it).left(2) ) + " (" + *it + ")";
	}
	kcfg_LanguageComboBox->insertStringList(LogoLanguageList);
	layout4->addWidget( kcfg_LanguageComboBox );
	
	LanguageLabel = new QLabel(kcfg_LanguageComboBox, i18n("Select the language for the Logo commands:"), groupBox1);
	layout4->addWidget( LanguageLabel );
	LanguageLabel->setBuddy( kcfg_LanguageComboBox );

	groupBox1Layout->addLayout( layout4, 0, 0 );
	languageLayout->addWidget( groupBox1, 0, 0 );
	language->resize( QSize(373, 80).expandedTo(minimumSizeHint()) );

	dialog->addPage( language, i18n("Language"), "locale", i18n("Language Settings") );

	// When the user clicks OK or Apply we want to update our settings.
	connect( dialog, SIGNAL( settingsChanged() ), this, SLOT( slotUpdateSettings() ) );

	// Display the dialog is there where errors.
	dialog->setInitialSize( QSize(550, 300) );
	dialog->show();
}

void MainWindow::slotUpdateSettings() {
	// get the selected language as a language code
	QString selectedLogoLanguage = kcfg_LanguageComboBox->currentText().section( "(", -1, -1 ).remove(")");
	// update the settings
	Settings::setLogoLanguage( selectedLogoLanguage );
	Settings::setLanguageComboBox( kcfg_LanguageComboBox->currentItem() );
	Settings::writeConfig();
	// set the HLstyle
	slotSetHighlightstyle( selectedLogoLanguage );
	// set the statusbar to display the language as just updated
	// TODO maybe this language name can be more pretty by not using ".left(2)", ie "American English" would than be possible... [if this is possible this should be fixed at more places.]
	KConfig entry(locate("locale", "all_languages"));
	entry.setGroup(Settings::logoLanguage().left(2));
	statusBar()-> changeItem(i18n("Command language: ")+entry.readEntry("Name"), IDS_LANG);
}

void MainWindow::readConfig(KConfig *config) {
	config->setGroup("General Options");
	m_recentFiles->loadEntries(config, "Recent Files");
	KConfig entry(locate("locale", "all_languages"));
	entry.setGroup(Settings::logoLanguage().left(2));
	statusBar()-> changeItem(i18n("Command language: ")+entry.readEntry("Name"), IDS_LANG);
}

// END



// BEGIN help related functions

void MainWindow::slotContextHelp() {
// somehow the 'anchor' parameter of invokeHelp is not working correcctly
// this should most likely be filed as a bug in KHelpCenter...
// The other option is to Just use the code from kapplication.cpp:2051
// and compose the helpUrl:
// "help:/"+name()+"/"+"{reference,container,glorrery,etc.}"+"#"+translate(helpKeyword);
// our self... dessision/chat with the creators of KH.C. needed :))
// Ooh... and we also want a DCOPmethod to close the sidebar since it over-informs...
// Ooh2... we must probably put the code to translate commands in a seperate .cpp/.h since
// we want to use it in several places (i.e. here too)

// IDEA!!:  put all the keyword in a i18n("...") this will make translating them a lot easier!!!
// MAYBE THIS IS ALSO POSSIBLE FOR THE INTERPRETER!!!!

	kdDebug(0)<<"helpKeyword = "<<helpKeyword<<endl;
	kapp->invokeHelp(helpKeyword, "", "");

// Firts attempt for making smarter use of the helpcenter:
//
// 	QString url;
// 	if ( helpKeyword == i18n("<no keyword>") ) {
// 		kdWarning() << "Could not launch help, nothing under cursor." << endl;
// 	} else if ( helpKeyword == i18n("<number>") ) {
// 		url = QString("help:/kturtle/index.html");
// 	} else if ( helpKeyword == i18n("<string>") ) {
// 		url = QString("help:/kturtle/index.html");
// 	} else {
// 		url = QString("help:/kturtle/index.html");
// 	}
// 
// 	QString error;
// 	if ( !dcopClient()->isApplicationRegistered("khelpcenter") ) {
// 		if (startServiceByDesktopName("khelpcenter", url, &error, 0, 0, startup_id, true) ) {
// 			kdWarning() << "Could not launch help:\n" << error << endl;
// 			return;
// 		}
// 	} else {
// 		DCOPRef( "khelpcenter", "KHelpCenterIface" ).send( "openUrl", url, startup_id );
// 	}
}

void MainWindow::slotContextHelpUpdate() {
	uint row, col;
	dynamic_cast<KTextEditor::ViewCursorInterface*>(editor)->cursorPositionReal(&row, &col);
	QString line = dynamic_cast<KTextEditor::EditInterface*>(doc)->textLine(row);
	QString beforeCursor = line;
	beforeCursor.truncate(col);
	QString afterCursor = line.remove(0, col);
	QString cursorWord = beforeCursor.section(' ', -1) + afterCursor.section(' ', 0, 0);
	QRegExp string("\"[^\"]*\""); // kindly donated by blackie
	if ( cursorWord.isEmpty() ) {
		helpKeyword = i18n("<no keyword>");
	} else if ( cursorWord.find( QRegExp("[\\d.]+") ) == 0 ) {
		helpKeyword = i18n("<number>");
	} else if ( string.search(line) != -1 ) { // check if there are strings in the line
		int pos = 0;
		while ( ( pos = string.search(line, pos) ) != -1 ) { // and if the cursor is in the string
			if ( (int)col > pos && (int)col < pos + string.matchedLength() ) {
				helpKeyword = i18n("<string>");
			}
			pos += string.matchedLength();
		}
	} else {
		helpKeyword = cursorWord;
	}
	ContextHelp->setText( i18n("Help on: %1").arg(helpKeyword) );
}

// END


// Misc. functions


void MainWindow::slotColorPicker() {
	// in the constructor picker is initialised as 0
	// if picker is 0 when this funktion is called a colorpickerdialog is created and connected
	if(picker == 0) {
		picker = new ColorPicker(this);
		if(picker == 0) {
			return; // safety
		}
		connect( picker, SIGNAL( visible(bool) ), colorpicker, SLOT( setChecked(bool) ) );
		connect( picker, SIGNAL( ColorCode(QString) ), this, SLOT( slotInsertText(QString) ) );
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

void MainWindow::slotUpdateCanvas() {
	// fixes a non updateing bug
	// I tried doing this by connecting Canvas's resized to BaseWidget's update...
	// but i had no luck :(     this worked though
	TurtleView->hide();
	TurtleView->show();
}

#include "kturtle.moc"
