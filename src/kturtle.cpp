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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


// BEGIN includes and defines

#include <stdlib.h>

#include <tqbutton.h>
#include <tqregexp.h>
#include <tqpainter.h>
#include <tqtooltip.h>
#include <tqtimer.h>
#include <tqwhatsthis.h>

#include <kapplication.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kimageio.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmenubar.h>
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
#include <ktexteditor/encodinginterface.h>

#include "lexer.h"
#include "settings.h"
#include "translate.h"

#include "kturtle.h"

// StatusBar field IDs
#define IDS_STATUS      0
#define IDS_LINECOLUMN  2
#define IDS_INS         3
#define IDS_LANG        4

// END



// BEGIN constructor and destructor

MainWindow::MainWindow(KTextEditor::Document *document) : editor(0)
{
	// the initial values
	CurrentFile = KURL(); // fill with empty URL
	setCaption( i18n("Untitled") );
	picker = 0; // for the colorpickerdialog
	executing = false;
	b_fullscreen = false;

	// set the shell's ui resource file
	if (!document)
	{
		if ( !(document = KTextEditor::EditorChooser::createDocument(0,"KTextEditor::Document") ) )
		{
			KMessageBox::error( this, i18n("A KDE text-editor component could not be found;\n"
			                               "please check your KDE installation.") );
			kapp->exit(1);
		}
		// docList.append(doc);
	}
	doc = document;

	setupCanvas();
	setupEditor();

	setupStatusBar();
	setupActions();
	createShellGUI(true);
	setMinimumSize(200,200);

	// init with more usefull size, stolen from kwite (they stole it from konq)
	if ( !initialGeometrySet() && !kapp->config()->hasGroup("MainWindow Settings") ) resize(640, 480);

	KConfig *config = kapp->config();
	readConfig(config);
}

MainWindow::~MainWindow()
{
	delete editor->document();
}

// END



// BEGIN setup's (actions, editor, statusbar, canvas)

void MainWindow::setupActions()
{
	KActionCollection *ac = actionCollection(); // abbreviation
	
	// File actions
	KStdAction::openNew(this, TQT_SLOT(slotNewFile()), ac);
	openExAction = new KAction(i18n("Open Exa&mples..."), "bookmark_folder", CTRL+Key_E, this, TQT_SLOT(slotOpenExample()), ac, "open_examples");
	KStdAction::open(this, TQT_SLOT(slotOpenFile()), ac);
	m_recentFiles = KStdAction::openRecent(this, TQT_SLOT(slotOpenFile(const KURL&)), ac);
	KStdAction::save(this, TQT_SLOT(slotSaveFile()), ac);
	KStdAction::saveAs(this, TQT_SLOT(slotSaveAs()), ac);
	new KAction(i18n("Save &Canvas..."), 0, 0, this, TQT_SLOT(slotSaveCanvas()), ac, "save_canvas");
	speed = new KSelectAction(i18n("Execution Speed"), 0, ALT+Key_S, this, TQT_SLOT( slotChangeSpeed() ), ac, "speed");
	TQStringList speeds; speeds << i18n("Full Speed") << i18n("Slow") << i18n("Slower") << i18n("Slowest");
	speed->setItems(speeds);
	speed->setCurrentItem(0);
	run = new KAction(i18n("&Execute Commands"), "gear", ALT+Key_Return, this, TQT_SLOT( slotExecute() ), ac, "run");
	pause = new KToggleAction(i18n("Pause E&xecution"), "player_pause", Key_Pause, this, TQT_SLOT( slotPauseExecution() ), ac, "pause");
	pause->setChecked(false);
	pause->setEnabled(false);
	stop = new KAction(i18n("Stop E&xecution"), "stop", Key_Escape, this, TQT_SLOT( slotAbortExecution() ), ac, "stop");
	stop->setEnabled(false);
	KStdAction::print(this, TQT_SLOT(slotPrint()), ac);
	KStdAction::quit(this, TQT_SLOT(close()), ac);
	
	// Edit actions
	KStdAction::undo(this, TQT_SLOT(slotUndo()), ac);
	KStdAction::redo(this, TQT_SLOT(slotRedo()), ac);
	KStdAction::cut(this, TQT_SLOT(slotCut()), ac);
	KStdAction::copy(this, TQT_SLOT(slotCopy()), ac);
	KStdAction::paste(this, TQT_SLOT(slotPaste()), ac);
	KStdAction::selectAll(this, TQT_SLOT(slotSelectAll()), ac);
	KStdAction::deselect(this, TQT_SLOT(slotClearSelection()), ac);
	new KToggleAction(i18n("Toggle Insert"), Key_Insert, this, TQT_SLOT(slotToggleInsert()), ac, "set_insert");
	KStdAction::find(this, TQT_SLOT(slotFind()), ac);
	KStdAction::findNext(this, TQT_SLOT(slotFindNext()), ac);
	KStdAction::findPrev(this, TQT_SLOT(slotFindPrevious()), ac);
	KStdAction::replace(this, TQT_SLOT(slotReplace()), ac);
	
	// View actions
	new KToggleAction(i18n("Show &Line Numbers"), 0, Key_F11, this, TQT_SLOT(slotToggleLineNumbers()), ac, "line_numbers");
	m_fullscreen = KStdAction::fullScreen(this, TQT_SLOT(slotToggleFullscreen()), ac, this, "full_screen");
	m_fullscreen->setChecked(b_fullscreen);

	// Tools actions
	colorpicker = new KToggleAction(i18n("&Color Picker"), "colorize", ALT+Key_C, this, TQT_SLOT(slotColorPicker()), ac, "color_picker");
	new KAction(i18n("&Indent"), "indent", CTRL+Key_I, this, TQT_SLOT(slotIndent()), ac, "edit_indent");
	new KAction(i18n("&Unindent"), "unindent", CTRL+SHIFT+Key_I, this, TQT_SLOT(slotUnIndent()), ac, "edit_unindent");
	new KAction(i18n("Cl&ean Indentation"), 0, 0, this, TQT_SLOT(slotCleanIndent()), ac, "edit_cleanIndent");
	new KAction(i18n("Co&mment"), 0, CTRL+Key_D, this, TQT_SLOT(slotComment()), ac, "edit_comment");
	new KAction(i18n("Unc&omment"), 0, CTRL+SHIFT+Key_D, this, TQT_SLOT(slotUnComment()), ac, "edit_uncomment");
	
	// Settings actions
	KStdAction::preferences( this, TQT_SLOT(slotSettings()), ac );
	new KAction(i18n("&Configure Editor..."), "configure", 0, this, TQT_SLOT(slotEditor()), ac, "set_confdlg");
	
	// Help actions
	ContextHelp = new KAction(0, 0, Key_F2, this, TQT_SLOT(slotContextHelp()), ac, "context_help");
	slotContextHelpUpdate(); // this sets the label of this action
	
	// other
	setXMLFile("kturtleui.rc");
	setupGUI();
}

void MainWindow::setupEditor()
{
	editorDock = new TQDockWindow(this);
	editorDock->setNewLine(true);
	editorDock->setFixedExtentWidth(250);
	editorDock->setFixedExtentHeight(150);
	editorDock->setResizeEnabled(true);
	editorDock->setFrameShape(TQFrame::ToolBarPanel);
	TQWhatsThis::add( editorDock, i18n( "This is the code editor, here you type the Logo commands to instruct the turtle. You can also open an existing Logo program with File->Open Examples... or File->Open." ) );
	moveDockWindow(editorDock, Qt::DockLeft);
	editor = doc->createView (editorDock, 0L);
	// editorInterface is the editor interface which allows us to access the text in the part
	editorInterface = dynamic_cast<KTextEditor::EditInterface*>(doc);
	dynamic_cast<KTextEditor::EncodingInterface*>(doc)->setEncoding("UTF-8");
	editorDock->setWidget(editor);

	// default the highlightstyle to "logo" using the needed i18n
	kdDebug(0)<<"The HighlightStyle for the Code Editor: "<<Settings::logoLanguage()<<endl;
	slotSetHighlightstyle( Settings::logoLanguage() );

	// allow the cursor position to be indicated in the statusbar
	connect( editor, TQT_SIGNAL(cursorPositionChanged()), this, TQT_SLOT(slotCursorStatusBar()) );
	// and update the context help menu item
	connect( editor, TQT_SIGNAL(cursorPositionChanged()), this, TQT_SLOT(slotContextHelpUpdate()) );
	
	translate = new Translate();
}

void MainWindow::setupStatusBar()
{
	statusBar()->insertItem("", IDS_STATUS, 1, false);
	statusBar()->setItemAlignment(IDS_STATUS, AlignLeft);
	statusBar()->insertItem("", IDS_LANG, 0, true);
	statusBar()->insertItem("", IDS_LINECOLUMN, 0, true);
	statusBar()->insertItem("", IDS_INS, 0, true);
	
	// fill the statusbar
	slotStatusBar(i18n("Welcome to KTurtle..."),  IDS_STATUS); // the message part
	slotStatusBar(i18n("Line: %1 Column: %2").arg(1).arg(1), IDS_LINECOLUMN);
	slotStatusBar(i18n("INS"), IDS_INS);
}

void MainWindow::setupCanvas()
{
	baseWidget = new TQWidget(this);
	setCentralWidget(baseWidget);
	baseLayout = new TQGridLayout(baseWidget, 0, 0);
	canvasView = new Canvas(baseWidget);
	baseLayout->addWidget(canvasView, 0, 0, AlignCenter);
	baseLayout->setRowStretch(0, 1); // this apperntly fixes a pre-usefull scrollbars bug
	baseLayout->setColStretch(0, 1);
	TQWhatsThis::add( canvasView, i18n("This is the canvas, here the turtle draws a picture.") );
	canvasView->show();
	connect( canvasView, TQT_SIGNAL( CanvasResized() ), this, TQT_SLOT( slotUpdateCanvas() ) );
}

// END



// BEGIN staturbar related functions

void MainWindow::slotStatusBar(TQString text, int id)
{
	text = " " + text + " "; // help the layout
	statusBar()->changeItem(text, id);
}

void MainWindow::slotCursorStatusBar()
{
	uint cursorLine;
	uint cursorCol;
	dynamic_cast<KTextEditor::ViewCursorInterface*>(editor)->cursorPositionReal(&cursorLine, &cursorCol);
	TQString linenumber = i18n(" Line: %1 Column: %2 ").arg(cursorLine + 1).arg(cursorCol + 1);
	statusBar()->changeItem(linenumber, IDS_LINECOLUMN);
}

// END



// BEGIN file menu related fuctions (new, open, save()as, image, print, quit)

void MainWindow::slotNewFile()
{
	if ( !editor->document()->isModified() && CurrentFile.isEmpty() ) return; // do nothing when nothing can be done
	if ( editor->document()->isModified() )
	{
		int result = KMessageBox::warningContinueCancel( this,
		i18n("The program you are currently working on is not saved. "
		     "By continuing you may lose the changes you have made."),
		i18n("Unsaved File"), i18n("&Discard Changes") );
		if (result != KMessageBox::Continue) return;
	}
	editorInterface->clear(); // clear the editor
	canvasView->slotClear(); // clear the view
	editor->document()->setModified(false);
	CurrentFile = KURL();
	setCaption( i18n("Untitled") );
	slotStatusBar(i18n("New file... Happy coding!"), IDS_STATUS);
}



void MainWindow::slotOpenFile(const KURL &urlRef)
{
	KURL url = urlRef;
	if ( url.isEmpty() )
	{
		url = KFileDialog::getOpenURL( TQString(":logo_dir"), TQString("*.logo|") + i18n("Logo Files"), this, i18n("Open Logo File") );
	}
	loadFile(url);
}

void MainWindow::slotOpenExample()
{
	KURL url;
	url.setPath( locate("data", "kturtle/examples/" + Settings::logoLanguage() + "/") );
	url = KFileDialog::getOpenURL( url.path(), TQString("*.logo|") + i18n("Logo Examples Files"), this, i18n("Open Logo Example File") );
	loadFile(url);
}

void MainWindow::loadFile(const KURL &url)
{
	if ( !url.isEmpty() )
	{
		TQFile file( url.path() );
		if ( file.open(IO_ReadOnly) )
		{
			if ( editor->document()->isModified() )
			{
				int result = KMessageBox::warningContinueCancel( this,
						i18n("The program you are currently working on is not saved. "
						     "By continuing you may lose the changes you have made."),
						i18n("Unsaved File"), i18n("&Discard Changes") );
				if (result != KMessageBox::Continue)
				{
					slotStatusBar(i18n("Opening aborted, nothing opened."),  IDS_STATUS);
					return;
				}
			}
			TQTextStream stream(&file);
			stream.setEncoding(TQTextStream::UnicodeUTF8);
			editorInterface->setText( stream.read() );
			file.close();
			m_recentFiles->addURL(url);
			setCaption( url.fileName() );
			slotStatusBar(i18n("Opened file: %1").arg( url.fileName() ), IDS_STATUS);
			editor->document()->setModified(false);
			CurrentFile = url;
			return;
		}
		else
		{
			KMessageBox::error( this,
				i18n("KTurtle was unable to open: \n%1.").arg( url.prettyURL() ),
				i18n("Open Error") );
			slotStatusBar(i18n("Opening aborted because of error."),  IDS_STATUS);
			return;
		}
	}
	slotStatusBar(i18n("Opening aborted."),  IDS_STATUS); // fallback
}

void MainWindow::slotSaveFile()
{
	writeFile(CurrentFile);
}

void MainWindow::slotSaveAs()
{
	KURL url;
	while (true)
	{
		url = KFileDialog::getSaveURL( TQString(":logo_dir"), TQString("*.logo|") + i18n("Logo Files"), this, i18n("Save As") );
		if ( url.isEmpty() ) // cancelled the save dialog
		{
			slotStatusBar(i18n("Saving aborted."),  IDS_STATUS);
			return;
		}
		if ( TQFile( url.path() ).exists() )
		{
			int result = KMessageBox::warningContinueCancel ( this,
					i18n("A program named \"%1\" already exists in this folder. "
							"Do you want to overwrite it?").arg( url.fileName() ),
					i18n("Overwrite?"), i18n("&Overwrite") );
			if (result != KMessageBox::Continue) return;
		}
		break;
	}
	writeFile(url);
}

void MainWindow::writeFile(const KURL &url)
{
	if ( url.isEmpty() ) slotSaveAs();
	else
	{
		editor->document()->saveAs(url); // use the KateParts method for saving
		loadFile(url); // reload the file as utf8 otherwise display weird chars
		setCaption( url.fileName() );
		slotStatusBar(i18n("Saved to: %1").arg( url.fileName() ),  IDS_STATUS);
		m_recentFiles->addURL(url);
		editor->document()->setModified(false);
		CurrentFile = url;
	}
}



void MainWindow::slotSaveCanvas()
{
	KURL url;
	while (true)
	{
		url = KFileDialog::getSaveURL( TQString(":logo_dir"), TQString("*.png|") +
		i18n("Pictures"), this, i18n("Save Canvas as Picture") );
		if ( url.isEmpty() ) return; // when cancelled the KFiledialog?
		if ( TQFile( url.path() ).exists() )
		{
			int result = KMessageBox::warningContinueCancel( this,
				i18n("A picture named \"%1\" already in this folder. "
				     "Do you want to overwrite it?").arg( url.fileName() ),
				i18n("Overwrite?"), i18n("&Overwrite") );
			if (result != KMessageBox::Continue) return;
		}
		break;
	}
	
	TQString type( KImageIO::type( url.path() ) );
	if ( type.isNull() ) type = "PNG";
	bool ok = false;
	TQPixmap* pixmap = canvasView->canvas2Pixmap();
	if ( url.isLocalFile() )
	{
		KSaveFile saveFile( url.path() );
		if ( saveFile.status() == 0 )
		{
			if ( pixmap->save( saveFile.file(), type.latin1() ) ) ok = saveFile.close();
		}
	}
	if (!ok)
	{
		kdWarning() << "KTurtle was unable to save the canvas drawing" << endl;
		KMessageBox::error(this,
			i18n("KTurtle was unable to save the image to: \n%1.").arg( url.prettyURL() ),
			i18n("Unable to Save Image") );
		slotStatusBar(i18n("Could not save image."),  IDS_STATUS);
		return;
	}
	slotStatusBar(i18n("Saved canvas to: %1").arg( url.fileName() ),  IDS_STATUS);
}



void MainWindow::slotPrint()
{
	int result = KMessageBox::questionYesNoCancel( this,
	i18n("Do you want to print the Logo code or the canvas?"),
	i18n("What to Print?"), i18n("Print &Logo Code"), i18n("Print &Canvas") );
	if (result == KMessageBox::Yes)
	{
		dynamic_cast<KTextEditor::PrintInterface*>(doc)->printDialog();
		return;
	}
	if (result == KMessageBox::No)
	{
		KPrinter printer;
		if ( printer.setup(this) )
		{
			TQPainter painter(&printer);
			TQPixmap *CanvasPic = canvasView->canvas2Pixmap();
			painter.drawPixmap(0, 0, *CanvasPic);
		}
		return;
	}
	slotStatusBar(i18n("Printing aborted."),  IDS_STATUS);
}



bool MainWindow::queryClose()
{
	if ( editor->document()->isModified() )
	{
		slotStatusBar(i18n("Quitting KTurtle..."),  IDS_STATUS);
		// make sure the dialog looks good with new -- unnamed -- files.
		int result = KMessageBox::warningYesNoCancel( this,
			i18n("The program you are currently working on is not saved. "
			     "By quitting KTurtle you may lose the changes you have made."),
			i18n("Unsaved File"), i18n("&Save"), i18n("Discard Changes && &Quit") );
		if (result == KMessageBox::Cancel)
		{
			slotStatusBar(i18n("Quitting aborted."),  IDS_STATUS);
			return false;
		}
		else if (result == KMessageBox::Yes)
		{
			slotSaveFile();
			if ( CurrentFile.isEmpty() )
			{
				// when saveAs get cancelled or X-ed it should not quit
				slotStatusBar(i18n("Quitting aborted."),  IDS_STATUS);
				return false;
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

void MainWindow::slotExecute()
{
	if (b_fullscreen) // check if execution should be execution in 'fullscreen' mode
	{
		editorDock->hide();
		statusBar()->hide();
		menuBar()->hide();
		toolBar()->hide();
	}

	run->setEnabled(false); // set action indications
	pause->setEnabled(true);
	stop->setEnabled(true);

	// start paring
	slotStatusBar(i18n("Parsing commands..."),  IDS_STATUS);
	kdDebug(0)<<"############## PARSING STARTED ##############"<<endl;
	kapp->processEvents();
	errMsg = new ErrorMessage(this); // create an empty errorDialog
	TQString txt = editorInterface->text() + "\x0a\x0a"; // parser expects input to have 2 delimiting newlines
	TQTextIStream in(&txt); // create the stream
	Parser parser(in); // pass the reference to the stream to the parse object
	connect(&parser, TQT_SIGNAL( ErrorMsg(Token&, const TQString&, uint) ),
	         errMsg, TQT_SLOT( slotAddError(Token&, const TQString&, uint) ) );
	connect( errMsg, TQT_SIGNAL(setSelection(uint, uint, uint, uint) ),
	           this, TQT_SLOT(slotSetSelection(uint, uint, uint, uint) ) );
	parser.parse(); // and GO!
	TreeNode* root = parser.getTree(); // when finished parsing get the nodeTree
	kdDebug(0)<<"############## PARSING FINISHED ##############"<<endl;
	
	kdDebug(0)<<"TreeNode::showTree():"<<endl;
	root->showTree(root); // show nodeTree, this is a DEBUG OPTION (but nice)

	// start execution
	slotStatusBar(i18n("Executing commands..."),  IDS_STATUS);
	kdDebug(0)<<"############## EXECUTION STARTED ##############"<<endl;
	exe = new Executer(root); // make Executer object, 'exe', and pass it the nodeTree
	
	connect(this, TQT_SIGNAL( changeSpeed(int) ), exe, TQT_SLOT(slotChangeSpeed(int) ) );
	connect(this, TQT_SIGNAL( unpauseExecution() ), exe, TQT_SLOT( slotStopPausing() ) );
	connect( exe, TQT_SIGNAL( setSelection(uint, uint, uint, uint) ),
	        this, TQT_SLOT  ( slotSetSelection(uint, uint, uint, uint) ) );
	connect( exe, TQT_SIGNAL( ErrorMsg(Token&, const TQString&, uint) ),
	      errMsg, TQT_SLOT  ( slotAddError(Token&, const TQString&, uint) ) );
	connect( exe, TQT_SIGNAL( InputDialog(TQString&) ), this, TQT_SLOT( slotInputDialog(TQString&) ) );
	connect( exe, TQT_SIGNAL( MessageDialog(TQString) ), this, TQT_SLOT( slotMessageDialog(TQString) ) );

	// Connect the signals form Executer to the slots from Canvas:
	connect( exe, TQT_SIGNAL( Clear() ), canvasView, TQT_SLOT( slotClear() ) );
	connect( exe, TQT_SIGNAL( Go(double, double) ), canvasView, TQT_SLOT( slotGo(double, double) ) );
	connect( exe, TQT_SIGNAL( GoX(double) ), canvasView, TQT_SLOT( slotGoX(double) ) );
	connect( exe, TQT_SIGNAL( GoY(double) ), canvasView, TQT_SLOT( slotGoY(double) ) );
	connect( exe, TQT_SIGNAL( Forward(double) ), canvasView, TQT_SLOT( slotForward(double) ) );
	connect( exe, TQT_SIGNAL( Backward(double) ), canvasView, TQT_SLOT( slotBackward(double) ) );
	connect( exe, TQT_SIGNAL( Direction(double) ), canvasView, TQT_SLOT( slotDirection(double) ) );
	connect( exe, TQT_SIGNAL( TurnLeft(double) ), canvasView, TQT_SLOT( slotTurnLeft(double) ) );
	connect( exe, TQT_SIGNAL( TurnRight(double) ), canvasView, TQT_SLOT( slotTurnRight(double) ) );
	connect( exe, TQT_SIGNAL( Center() ), canvasView, TQT_SLOT( slotCenter() ) );
	connect( exe, TQT_SIGNAL( SetPenWidth(int) ), canvasView, TQT_SLOT( slotSetPenWidth(int) ) );
	connect( exe, TQT_SIGNAL( PenUp() ), canvasView, TQT_SLOT( slotPenUp() ) );
	connect( exe, TQT_SIGNAL( PenDown() ), canvasView, TQT_SLOT( slotPenDown() ) );
	connect( exe, TQT_SIGNAL( SetFgColor(int, int, int) ), canvasView, TQT_SLOT( slotSetFgColor(int, int, int) ) );
	connect( exe, TQT_SIGNAL( SetBgColor(int, int, int) ), canvasView, TQT_SLOT( slotSetBgColor(int, int, int) ) );
	connect( exe, TQT_SIGNAL( ResizeCanvas(int, int) ), canvasView, TQT_SLOT( slotResizeCanvas(int, int) ) );
	connect( exe, TQT_SIGNAL( SpriteShow() ), canvasView, TQT_SLOT( slotSpriteShow() ) );
	connect( exe, TQT_SIGNAL( SpriteHide() ), canvasView, TQT_SLOT( slotSpriteHide() ) );
	connect( exe, TQT_SIGNAL( SpritePress() ), canvasView, TQT_SLOT( slotSpritePress() ) );
	connect( exe, TQT_SIGNAL( SpriteChange(int) ), canvasView, TQT_SLOT( slotSpriteChange(int) ) );
	connect( exe, TQT_SIGNAL( Print(TQString) ), canvasView, TQT_SLOT( slotPrint(TQString) ) );
	connect( exe, TQT_SIGNAL( FontType(TQString, TQString) ), canvasView, TQT_SLOT( slotFontType(TQString, TQString) ) );
	connect( exe, TQT_SIGNAL( FontSize(int) ), canvasView, TQT_SLOT( slotFontSize(int) ) );
	connect( exe, TQT_SIGNAL( WrapOn() ), canvasView, TQT_SLOT( slotWrapOn() ) );
	connect( exe, TQT_SIGNAL( WrapOff() ), canvasView, TQT_SLOT( slotWrapOff() ) );
	connect( exe, TQT_SIGNAL( Reset() ), canvasView, TQT_SLOT( slotReset() ) );

	// START EXECUTION on the selected speed, and use the feedbacked boolean value
	slotChangeSpeed();
	if ( exe->run() ) slotStatusBar(i18n("Done."),  IDS_STATUS);
	else slotStatusBar(i18n("Execution aborted."),  IDS_STATUS);
	finishExecution();
	
	delete exe; // clean-up

	if ( errMsg->containsErrors() ) errMsg->display(); // if errors show them
}

void MainWindow::slotPauseExecution()
{
	if ( pause->isChecked() )
	{
		exe->pause();
		slotStatusBar(i18n("Execution paused."),  IDS_STATUS);
	}
	else
	{
		emit unpauseExecution();
		slotStatusBar(i18n("Executing commands..."),  IDS_STATUS);
	}
}

void MainWindow::slotAbortExecution()
{
	// To abort the executor, this can for instance be handy when the 
	// executer got stuck in a  user made endless loops...
	// to keep the program responding to interrupts like this, all the
	// loops have possible breaks. This activates them breaks:
	exe->abort();
}

void MainWindow::finishExecution()
{
	kdDebug(0)<<"############## EXECUTION FINISHED ##############"<<endl;
	executing = false;
	pause->setEnabled(false);
	pause->setChecked(false);
	run->setEnabled(true);
	stop->setEnabled(false);
	
	// if execution finished on FullSpeed the selection does not match the last executed command -> clear-it
	if (speed->currentItem() == 0) slotClearSelection();

	// if coming from fullscreen-mode show the editor, menu- and statusbar
	if (b_fullscreen) TQTimer::singleShot( 1000, this, TQT_SLOT( slotFinishedFullScreenExecution() ) );
}

void MainWindow::slotChangeSpeed()
{
	emit changeSpeed( speed->currentItem() );
}


// slots for logo functions that need to use the MainWindow class:

void MainWindow::slotInputDialog(TQString& value)
{
	value = KInputDialog::getText(i18n("Input"), value);
}

void MainWindow::slotMessageDialog(TQString text)
{
	KMessageBox::information( this, text, i18n("Message") );
}

// END



// BEGIN editor connections (undo, redo, cut, copy, paste, cursor, selections, find, replace, linenumbers etc.)

void MainWindow::slotEditor()
{
	KAction *a = editor->actionCollection()->action("set_confdlg");
	a->activate();
}

void MainWindow::slotSetHighlightstyle(TQString langCode)
{
	KTextEditor::HighlightingInterface *hli = dynamic_cast<KTextEditor::HighlightingInterface*>(doc);
	for (uint i = 0; i < hli->hlModeCount(); i++)
	{
		if (hli->hlModeName(i) == langCode)
		{
			hli->setHlMode(i);
			return;
		}
	}
	// and the fallback:
	for (uint i = 0; i < hli->hlModeCount(); i++)
	{
		if(hli->hlModeName(i) == "en_US") hli->setHlMode(i);
	}
}


void MainWindow::slotUndo()
{
	dynamic_cast<KTextEditor::UndoInterface*>(doc)->undo();
}

void MainWindow::slotRedo()
{
	dynamic_cast<KTextEditor::UndoInterface*>(doc)->redo();
}


void MainWindow::slotCut()
{
	dynamic_cast<KTextEditor::ClipboardInterface*>(editor)->cut();
}

void MainWindow::slotCopy()
{
	dynamic_cast<KTextEditor::ClipboardInterface*>(editor)->copy();
}

void MainWindow::slotPaste()
{
	dynamic_cast<KTextEditor::ClipboardInterface*>(editor)->paste();
}


void MainWindow::slotSelectAll()
{
	dynamic_cast<KTextEditor::SelectionInterface*>(doc)->selectAll();
}

void MainWindow::slotClearSelection()
{
	dynamic_cast<KTextEditor::SelectionInterface*>(doc)->clearSelection();
}

void MainWindow::slotFind()
{
	KAction *a = editor->actionCollection()->action("edit_find");
	a->activate();
}

void MainWindow::slotFindNext()
{
	KAction *a = editor->actionCollection()->action("edit_find_next");
	a->activate();
}

void MainWindow::slotFindPrevious()
{
	KAction *a = editor->actionCollection()->action("edit_find_prev");
	a->activate();
}

void MainWindow::slotReplace()
{
	KAction* a = editor->actionCollection()->action("edit_replace");
	a->activate();
}

void MainWindow::slotToggleInsert()
{
	KToggleAction *a = dynamic_cast<KToggleAction*>(editor->actionCollection()->action("set_insert"));
	a->activate();
	if (a) statusBar()->changeItem(a->isChecked() ? i18n(" OVR ") : i18n(" INS "), IDS_INS);
}

void MainWindow::slotIndent()
{
	KAction *a = editor->actionCollection()->action("tools_indent");
	a->activate();
}

void MainWindow::slotUnIndent()
{
	KAction *a = editor->actionCollection()->action("tools_unindent");
	a->activate();
}

void MainWindow::slotCleanIndent()
{
	KAction *a = editor->actionCollection()->action("tools_cleanIndent");
	a->activate();
}

void MainWindow::slotComment()
{
	KAction *a = editor->actionCollection()->action("tools_comment");
	a->activate();
}

void MainWindow::slotUnComment()
{
	KAction *a = editor->actionCollection()->action("tools_uncomment");
	a->activate();
}

void MainWindow::slotToggleLineNumbers()
{
	KToggleAction *a = dynamic_cast<KToggleAction*>( editor->actionCollection()->action("view_line_numbers") );
	a->activate();
}

void MainWindow::slotInsertText(TQString str)
{
	uint StartLine, StartCol, EndLine, EndCol;
	dynamic_cast<KTextEditor::ViewCursorInterface*>(editor)->cursorPositionReal(&StartLine, &StartCol);
	dynamic_cast<KTextEditor::EditInterface*>(doc)->insertText(StartLine, StartCol, str);
	dynamic_cast<KTextEditor::ViewCursorInterface*>(editor)->cursorPositionReal(&EndLine, &EndCol);
	dynamic_cast<KTextEditor::SelectionInterface*>(doc)->setSelection(StartLine, StartCol, EndLine, EndCol);
}

void MainWindow::slotSetCursorPos(uint row, uint col)
{
	dynamic_cast<KTextEditor::ViewCursorInterface*>(editor)->setCursorPositionReal(row - 1, col);
	// kdDebug(0)<<"Cursor set to: ("<<row-1<<", "<<col<<")"<<endl;
}

void MainWindow::slotSetSelection(uint StartLine, uint StartCol, uint EndLine, uint EndCol)
{
	dynamic_cast<KTextEditor::SelectionInterface*>(doc)->setSelection(StartLine - 1, StartCol - 1, EndLine - 1, EndCol - 1);
	// kdDebug(0)<<"Selection set to: ("<<StartLine<<", "<<StartCol<<", "<<EndLine<<", "<<EndCol<<")"<<endl;
}
// END



// BEGIN fullscreen functions

void MainWindow::slotToggleFullscreen()
{
	if (!b_fullscreen) showFullScreen(); // both calls will generate event triggering updateFullScreen()
	else if ( isFullScreen() ) showNormal();
}

bool MainWindow::event(TQEvent* e)
{
	// executes updateFullScreen() after a ShowFullScreen or ShowNormal event got triggered
	if (e->type() == TQEvent::ShowFullScreen || e->type() == TQEvent::ShowNormal) updateFullScreen();
	return KMainWindow::event(e);
}

void MainWindow::updateFullScreen()
{
	if (isFullScreen() == b_fullscreen) return;
	b_fullscreen = isFullScreen();
	if (m_fullscreen) m_fullscreen->setChecked(b_fullscreen);
}

void MainWindow::slotFinishedFullScreenExecution()
{
	restartOrBackDialog = new RestartOrBack(this); // we have to make some to delete some
	if ( errMsg->containsErrors() ) slotBackToFullScreen(); // straight back to edit if there where errors 
	else
	{
		connect( restartOrBackDialog, TQT_SIGNAL( user1Clicked() ), this, TQT_SLOT( slotRestartFullScreen() ) );
		connect( restartOrBackDialog, TQT_SIGNAL( user2Clicked() ), this, TQT_SLOT( slotBackToFullScreen() ) );
		connect( restartOrBackDialog, TQT_SIGNAL( finished() ), this, TQT_SLOT( slotBackToFullScreen() ) );
		restartOrBackDialog->show();
		restartOrBackDialog->move(50, 50);
	}
}

void MainWindow::slotBackToFullScreen()
{
	delete restartOrBackDialog;
	editorDock->show();
	statusBar()->show();
	menuBar()->show();
	toolBar()->show();
}

void MainWindow::slotRestartFullScreen()
{
	delete restartOrBackDialog;
	slotExecute();
}

// END



// BEGIN configuration related functions

void MainWindow::slotSettings()
{
	// Check if there is already a dialog, if so bring it to the foreground.
	if ( KConfigDialog::showDialog("settings") ) return;

	// Create a new dialog with the same name as the above checking code.
	KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self() );
	// connect the help
	connect( dialog, TQT_SIGNAL( helpClicked() ), this, TQT_SLOT( slotSettingsHelp() ) );
	
	// making the filling for the 'General' settings dept.
	general = new TQWidget();
	TQGridLayout *generalLayout = new TQGridLayout( general, 1, 1, 11, 6, "generalLayout");
	WidthHeightBox = new TQGroupBox( i18n("Initial Canvas Size"), general );
	WidthHeightBox->setColumnLayout(0, Qt::Vertical );
	WidthHeightBox->layout()->setSpacing( 6 );
	WidthHeightBox->layout()->setMargin( 11 );
	TQVBoxLayout *WidthHeightBoxLayout = new TQVBoxLayout( WidthHeightBox->layout() );
	WidthHeightBoxLayout->setAlignment( Qt::AlignTop );
	TQHBoxLayout *layout3 = new TQHBoxLayout( 0, 0, 6, "layout3");
	TQVBoxLayout *layout2 = new TQVBoxLayout( 0, 0, 6, "layout2");
	
	TQVBoxLayout *layout1 = new TQVBoxLayout( 0, 0, 6, "layout1");

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

	WidthLabel = new TQLabel( kcfg_CanvasWidth, i18n("Canvas &width:"), WidthHeightBox );
	layout2->addWidget( WidthLabel );
	HeightLabel = new TQLabel( kcfg_CanvasHeight, i18n("Ca&nvas height:"), WidthHeightBox );
	layout2->addWidget( HeightLabel );
	layout3->addLayout( layout2 );

	layout3->addLayout( layout1 );
	WidthHeightBoxLayout->addLayout( layout3 );
	TQLabel* WidthHeightLabel = new TQLabel(i18n("You need to restart before these settings have effect"), WidthHeightBox);
	WidthHeightBoxLayout->addWidget( WidthHeightLabel );
	generalLayout->addWidget( WidthHeightBox, 0, 0 );
	general->resize( TQSize(234, 109).expandedTo(minimumSizeHint()) );

	dialog->addPage( general, i18n("General"), "package_settings", i18n("General Settings") );

	// making the filling for the 'Language' settings dept.
	TQWidget *language = new TQWidget();
	TQGridLayout *languageLayout = new TQGridLayout( language, 1, 1, 11, 6, "Form1Layout");
	TQGroupBox *groupBox1 = new TQGroupBox( language, "groupBox1" );
	groupBox1->setColumnLayout(0, Qt::Vertical );
	groupBox1->layout()->setSpacing( 6 );
	groupBox1->layout()->setMargin( 11 );
	TQGridLayout *groupBox1Layout = new TQGridLayout( groupBox1->layout() );
	groupBox1Layout->setAlignment( Qt::AlignTop );

	TQVBoxLayout *layout4 = new TQVBoxLayout( 0, 0, 6, "layout4");

	kcfg_LanguageComboBox = new KComboBox(groupBox1, "kcfg_LanguageComboBox");
	kcfg_LanguageComboBox->setEditable(false);
	TQStringList LogoLanguageList = Settings::logoLanguageList();
	// Add the full language names to the items
	for ( TQStringList::Iterator it = LogoLanguageList.begin(); it != LogoLanguageList.end(); ++it ) {
		*it = KGlobal::locale()->twoAlphaToLanguageName( (*it).left(2) ) + " (" + *it + ")";
	}
	kcfg_LanguageComboBox->insertStringList(LogoLanguageList);
	
	LanguageLabel = new TQLabel(kcfg_LanguageComboBox, i18n("&Select the language for the Logo commands:"), groupBox1);
	layout4->addWidget( LanguageLabel );
	layout4->addWidget( kcfg_LanguageComboBox );
	LanguageLabel->setBuddy( kcfg_LanguageComboBox );

	groupBox1Layout->addLayout( layout4, 0, 0 );
	languageLayout->addWidget( groupBox1, 0, 0 );
	language->resize( TQSize(373, 80).expandedTo(minimumSizeHint()) );

	dialog->addPage( language, i18n("Language"), "locale", i18n("Language Settings") );

	// When the user clicks OK or Apply we want to update our settings.
	connect( dialog, TQT_SIGNAL( settingsChanged() ), this, TQT_SLOT( slotUpdateSettings() ) );

	// Display the dialog is there where errors.
	dialog->setInitialSize( TQSize(550, 300) );
	dialog->show();
}

void MainWindow::slotUpdateSettings()
{
	// get the selected language as a language code
	TQString selectedLogoLanguage = kcfg_LanguageComboBox->currentText().section( "(", -1, -1 ).remove(")");
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
	slotStatusBar(i18n("Command language: %1").arg( entry.readEntry("Name") ), IDS_LANG);
	
	delete translate; // to update the currently used language
	translate = new Translate();
}

void MainWindow::readConfig(KConfig *config)
{
	config->setGroup("General Options");
	m_recentFiles->loadEntries(config, "Recent Files");
	KConfig entry(locate("locale", "all_languages"));
	entry.setGroup(Settings::logoLanguage().left(2));
	slotStatusBar(i18n("Command language: %1").arg( entry.readEntry("Name") ), IDS_LANG);
}

void MainWindow::slotSettingsHelp()
{
	kapp->invokeHelp("settings-configure", "", "");
}

// END



// BEGIN help related functions

void MainWindow::slotContextHelp()
{
// somehow the 'anchor' parameter of invokeHelp is not working correcctly (yet)

// this is/was appearently a bug in KHTML that Waldo Bastian kindly and quikly fixed.

// Ooh... and we also want a DCOPmethod to close the sidebar since it over-informs...
// Ooh3... we want fancy help (using KHTML) @ errormessage dialog
// Ooh4... And we might also have to keep track of the KHelpCenter instance we open so
// we will not end up with loads of them
// 

// IDEA!!:  put all the keyword in a i18n("...") this will make translating them a lot easier!!!
// MAYBE THIS IS ALSO POSSIBLE FOR THE INTERPRETER!!!!
// this should be discussed with translators (and please think of the highlight-themes too (since
// right now thay can probably be translated with a simple perl script 

	kdDebug(0)<<"help requested on this text: "<<helpKeyword<<endl;
	
	TQString helpWord;
	if ( helpKeyword == i18n("<no keyword>") )
	{
		KMessageBox::information( this, i18n("There is currently no text under the cursor to get help on."), i18n("Nothing Under Cursor") );
		return;
	}
	else if ( helpKeyword == i18n("<number>") )     helpWord = "number";
	else if ( helpKeyword == i18n("<string>") )     helpWord = "string";
	else if ( helpKeyword == i18n("<assignment>") ) helpWord = "assignment";
	else if ( helpKeyword == i18n("<question>") )   helpWord = "questions";
	else if ( helpKeyword == i18n("<name>") )       helpWord = "name";
	else if ( helpKeyword == i18n("<comment>") )    helpWord = "comment";
	else
	{
		// make lowercase
		helpWord = helpKeyword.lower();
		// if the key is an alias translate that alias to a key
		if      ( !translate->alias2key(helpKeyword).isEmpty() ) helpWord = translate->alias2key(helpKeyword);
		else if ( !translate->name2key (helpKeyword).isEmpty() ) helpWord = translate->name2key (helpKeyword);
		
		// at this point helpKeyword should contain a valid
		// section/anchor-id that can be used found in the doc
		// if not... who cares :)... well let's put an debugMsg for that occasion:
		else kdDebug(0)<<"Error in MainWindow::slotContextHelp: could not translate \""<<helpKeyword<<"\""<<endl;
	}
	
	kdDebug(0)<<"trying to open a help page using this keyword: "<<helpWord<<endl;
	
	kapp->invokeHelp(helpWord, "", "");
	
	TQString help2statusBar;
	if ( helpKeyword.startsWith("<") ) help2statusBar = helpKeyword;
	else                               help2statusBar = i18n("\"%1\"").arg(helpKeyword);
	slotStatusBar(i18n("Displaying help on %1").arg(help2statusBar), IDS_STATUS);
}

void MainWindow::slotContextHelpUpdate()
{
	uint row, col;
	dynamic_cast<KTextEditor::ViewCursorInterface*>(editor)->cursorPositionReal(&row, &col);
	TQString line = dynamic_cast<KTextEditor::EditInterface*>(doc)->textLine(row);
	
	// two shortcuts so we dont do all the CPU intensive regexp stuff when it not needed
	if ( line.stripWhiteSpace().startsWith("#") )
	{
		helpKeyword = i18n("<comment>");
		ContextHelp->setText( i18n("Help on: %1").arg(helpKeyword) );
		return;
	}
	if ( line.stripWhiteSpace().isEmpty() || line.mid(col-1,2).stripWhiteSpace().isEmpty() )
	{
		helpKeyword = i18n("<no keyword>");
		ContextHelp->setText( i18n("Help on: %1").arg(helpKeyword) );
		return;
	}
	
	int start, length, pos;
	
	pos = 0;
	if ( line.contains('"') )
	{
		TQRegExp delimitedStrings("(\"[^\"\\\\\\r\\n]*(\\\\.[^\"\\\\\\r\\n]*)*\")");
		while ( delimitedStrings.search(line, pos) != -1 )
		{
			// kdDebug(0)<<"stringsearch: >>"<<pos<<"<<"<<endl;
			start  = delimitedStrings.search(line, pos);
			length = delimitedStrings.matchedLength(); // the length of the last matched string, or -1 if there was no match
			if ( col >= (uint)start && col < (uint)(start+length) )
			{
				helpKeyword = i18n("<string>");
				ContextHelp->setText( i18n("Help on: %1").arg(helpKeyword) );
				return;
			}
			pos += (length <= 0 ? 1 : length);
		}
	}
	
	
	// except for "strings" this regexp effectively separates logo code in 'words' (in a very broad meaning)
	TQRegExp splitter("(([^ ,+\\-*/()=<>[!]|(?!==|<=|>=|!=))*)|(([ ,+\\-*/()=<>[!]|==|<=|>=|!=))");
	
	pos = 0;
	while (splitter.search(line, pos) != -1)
	{
		start  = splitter.search(line, pos);
		length = splitter.matchedLength();
		if ( col <  (uint)start ) break;
		if ( col >= (uint)start && col < (uint)(start+length) )
		{
			TQString cursorWord = line.mid( (uint)start, (uint)length );
			kdDebug(0)<<"cursorWord: '"<<cursorWord<<"'"<<endl;
			
			kdDebug(0)<<"(translate->name2key(  cursorWord.lower() )): '"<<translate->name2key(  cursorWord.lower() )<<"'"<<endl;
			kdDebug(0)<<"(translate->alias2key( cursorWord.lower() )): '"<<translate->alias2key( cursorWord.lower() )<<"'"<<endl;

			if      ( cursorWord.stripWhiteSpace().isEmpty() ) helpKeyword = i18n("<no keyword>");

			else if ( !translate->name2key(  cursorWord.lower() ).isEmpty() ) helpKeyword = cursorWord;
			else if ( !translate->alias2key( cursorWord.lower() ).isEmpty() ) helpKeyword = cursorWord;

			else if ( cursorWord.find( TQRegExp("[\\d.]+") ) == 0 ) helpKeyword = i18n("<number>");

			else if ( cursorWord.find( TQRegExp("[+\\-*/\\(\\)]") ) == 0 ) helpKeyword = i18n("<math>");

			else if ( cursorWord == TQString("=") ) helpKeyword = i18n("<assignment>");

			else if ( cursorWord.find( TQRegExp("==|<|>|<=|>=|!=") ) == 0 ) helpKeyword = i18n("<question>");

			// if we come here we either have an ID of some sort or an error
			// all we can do is try to catch some errors (TODO) and then...
			
			else helpKeyword = i18n("<name>");
			
			ContextHelp->setText( i18n("Help on: %1").arg(helpKeyword) );
			return;
		}
		pos += (length <= 0 ? 1 : length); // the pos had to be increased with at least one
	}
	
	// we allready cached some in the beginning of this method; yet its still needed as fall-through
	helpKeyword = i18n("<no keyword>");
	ContextHelp->setText( i18n("Help on: %1").arg(helpKeyword) );
}

// END



// BEGIN misc. functions

void MainWindow::slotColorPicker()
{
	// in the constructor picker is initialised as 0
	// if picker is 0 when this funktion is called a colorpickerdialog is created and connected
	if (picker == 0)
	{
		picker = new ColorPicker(this);
		if(picker == 0) return; // safety
		connect( picker, TQT_SIGNAL( visible(bool) ), colorpicker, TQT_SLOT( setChecked(bool) ) );
		connect( picker, TQT_SIGNAL( ColorCode(TQString) ), this, TQT_SLOT( slotInsertText(TQString) ) );
	}
	// if picker is not 0, there is a colorpickerdialog which only needs to be shown OR hidden
	if ( picker->isHidden() )
	{
		picker->show();
		colorpicker->setChecked(true);
	}
	else
	{
		picker->hide();
		colorpicker->setChecked(false);
	}
}

void MainWindow::slotUpdateCanvas()
{
	// fixes a non updateing bug
	// I tried doing this by connecting Canvas's resized to baseWidget's update...
	// but i had no luck :(    ...    this worked though :)
	canvasView->hide();
	canvasView->show();
}

// END


#include "kturtle.moc"
