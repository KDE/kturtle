/*
	Copyright (C) 2003-2009 Cies Breijs <cies AT kde DOT nl>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public
	License along with this program; if not, write to the Free
	Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.
*/

#include "mainwindow.h"

#include <QWhatsThis>
#include <QTimer>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QWidgetAction>

#include <QDebug>

#include <QAction>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <klocale.h>
#include <kicon.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <krecentfilesaction.h>
#include <kstatusbar.h>
#include <KStandardShortcut>
#include <kstandardaction.h>
#include <ktoolbar.h>
#include <ktoolbarlabelaction.h>
#include <ktoolbarpopupaction.h>
#include <ktoolinvocation.h>
#include <kdeprintdialog.h>
#include <kfiledialog.h>
#include <ksavefile.h>
#include <kio/netaccess.h>
#include <KTabWidget>
#include <knewstuff3/downloaddialog.h>
#include <kstandarddirs.h>

#include "interpreter/errormsg.h"
#include "interpreter/translator.h"


static const int MARGIN_SIZE = 3;  // defaultly styled margins look shitty
static const char* GHNS_TARGET = "kturtle/examples/*.turtle";

MainWindow::MainWindow()
{
	setupDockWindows();  // the setup order matters
	setupActions();
	setupCanvas();
	setupInterpreter();
	setupEditor();
	setupStatusBar();

	iterationTimer = new QTimer(this);
	connect(iterationTimer, SIGNAL(timeout()), this, SLOT(iterate()));

	connect(editor, SIGNAL(contentChanged()), inspector, SLOT(disable()));
	connect(editor, SIGNAL(contentChanged()), errorDialog, SLOT(disable()));

	connect(errorDialog, SIGNAL(currentlySelectedError(int, int, int, int)),
		editor, SLOT(markCurrentError(int, int, int, int)));

	colorPicker = 0;

	statusBar()->showMessage(i18nc("@info:status the application is ready for commands", "Ready"));
	updateContentName();  // also sets the window caption to 'untitled'
	setRunSpeed(1);  // full speed with highlighting
	abort();  // sets the run-states for the actions right

	setupGUI();

	// after all is set up:
	readConfig();
	updateLanguagesMenu();
	
	currentlyRunningConsole = false;
}

MainWindow::~MainWindow()
{
	delete editor;
	KGlobal::config()->sync();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (editor->maybeSave()) {
		event->accept();
		writeConfig();
		exit(0);
	} else {
		event->ignore();
	}
}

void MainWindow::filePrintDialog()
{
	QPrinter printer;
	QPrintDialog *printDialog = KdePrint::createPrintDialog(&printer, this);
	if (printDialog->exec()) {
		QPainter painter;
		painter.begin(&printer);
		editor->document()->drawContents(&painter);
		painter.end();
	}
	delete printDialog;
}

void MainWindow::canvasPrintDialog()
{
	QPrinter printer;
	QPrintDialog *printDialog = KdePrint::createPrintDialog(&printer, this);
	if (printDialog->exec()) {
		QPainter painter;
		painter.begin(&printer);
		canvas->scene()->render(&painter);
		painter.end();
	}
	delete printDialog;
}

void MainWindow::showDirectionDialog()
{
	directionDialog = new DirectionDialog(canvas->turtleAngle(), this);
	connect(directionDialog, SIGNAL(pasteText(const QString&)), editor, SLOT(insertPlainText(const QString&)));
}
void MainWindow::showColorPicker()
{
	if (colorPicker == 0) {
		colorPicker = new ColorPicker(this);
		connect(colorPicker, SIGNAL(pasteText(const QString&)), editor, SLOT(insertPlainText(const QString&)));
	}
	colorPicker->show();
}


void MainWindow::contextHelp()
{
  KToolInvocation::invokeHelp(contextHelpAnchor);
}

/*void MainWindow::whatsThis()
{
	QWhatsThis::enterWhatsThisMode();
}*/

void MainWindow::documentWasModified()
{
//TODO remove this function or make it do something
// 	setWindowModified(textEdit->document()->isModified());
}

void MainWindow::setRunSpeed(int speed)
{
	switch (speed) {
		case 0: dedicatedSpeedAct->setChecked(true); break;
		case 1: fullSpeedAct->setChecked(true);      break;
		case 2: slowSpeedAct->setChecked(true);      break;
		case 3: slowerSpeedAct->setChecked(true);    break;
		case 4: slowestSpeedAct->setChecked(true);   break;
		case 5: stepSpeedAct->setChecked(true);      break;
	}
	//TODO runOptionBox->setCurrentIndex(speed);
	runSpeed = speed;
}

void MainWindow::setupActions()
{
	QAction * a;
	KActionCollection* ac = actionCollection();

	// WHAT IS THIS?
	// Similar to a status tip, but not the same.
	// A status tip is displayed on hover, a whatisthis is displayed when
	// an item is clicked on it whatisthis mode  (cies doesn't like whatis mode for every little widget)
	// (he thinks whatis is to give the translators even more work)

	// File menu actions
	a = actionCollection()->addAction(KStandardAction::New,  "file_new", editor, SLOT(newFile()));
	a->setStatusTip(i18n("Create a new file"));
	a->setWhatsThis(i18n("New File: Create a new file"));

	a = actionCollection()->addAction(KStandardAction::Open,  "file_open", editor, SLOT(openFile()));
	a->setStatusTip(i18n("Open an existing file"));
	a->setWhatsThis(i18n("Open File: Open an existing file"));

	//TODO: Is this correct? -- It doesn't seem to be working
	recentFilesAction = (KRecentFilesAction*)actionCollection()->addAction(KStandardAction::OpenRecent,  "file_recent", editor, SLOT(openFile(const KUrl&)));
	recentFilesAction->setStatusTip(i18n("Open a recently used file"));
	recentFilesAction->setWhatsThis(i18n("Open Recent File: Open a recently used file"));
	
	a = new QAction(KIcon("get-hot-new-stuff"), i18n("Get more examples..."), this);
	actionCollection()->addAction("get_new_examples", a);
	a->setText(i18n("Get more examples..."));
	connect(a, SIGNAL(triggered()), this, SLOT(getNewExampleDialog()));

	a = actionCollection()->addAction(KStandardAction::Save,  "file_save", editor, SLOT(saveFile()));
	a->setStatusTip(i18n("Save the current file to disk"));
	a->setWhatsThis(i18n("Save File: Save the current file to disk"));
	connect(editor->document(), SIGNAL(modificationChanged(bool)), a, SLOT(setEnabled(bool)));

	a = actionCollection()->addAction(KStandardAction::SaveAs,  "file_save_as", editor, SLOT(saveFileAs()));
	a->setStatusTip(i18n("Save the current file under a different name"));
	a->setWhatsThis(i18n("Save File As: Save the current file under a different name"));

	exportToHtmlAct = actionCollection()->addAction("file_export_to_html");
	exportToHtmlAct->setText(i18n("Export to &HTML..."));
	exportToHtmlAct->setStatusTip(i18n("Export the contents of the editor as HTML"));
	exportToHtmlAct->setWhatsThis(i18n("Export to HTML: Export the contents of the editor as HTML"));
	connect(exportToHtmlAct, SIGNAL(triggered()), this, SLOT(exportToHtml()));

	a = actionCollection()->addAction(KStandardAction::Print, "file_print", this, SLOT(filePrintDialog()));
	a->setStatusTip(i18n("Print the code"));
	a->setWhatsThis(i18n("Print: Print the code"));

	a = actionCollection()->addAction(KStandardAction::Quit, "file_quit", this, SLOT(close()));
	a->setStatusTip(i18n("Quit KTurtle"));
	a->setWhatsThis(i18n("Quit: Quit KTurtle"));

	// Edit menu actions
	a = KStandardAction::undo(editor->view(), SLOT(undo()), ac);
	a->setStatusTip(i18n("Undo a change in the editor"));
	a->setWhatsThis(i18n("Undo: Undo a change in the editor"));
	a->setEnabled(false);
	connect(editor->view(), SIGNAL(undoAvailable(bool)), a, SLOT(setEnabled(bool)));

	a = KStandardAction::redo(editor->view(), SLOT(redo()), ac);
	a->setStatusTip(i18n("Redo a previously undone change in the editor"));
	a->setWhatsThis(i18n("Redo: Redo a previously undone change in the editor"));
	a->setEnabled(false);
	connect(editor->view(), SIGNAL(redoAvailable(bool)), a, SLOT(setEnabled(bool)));

	a = KStandardAction::cut(editor->view(), SLOT(cut()), ac);
	a->setStatusTip(i18n("Cut the selected text to the clipboard"));
	a->setWhatsThis(i18n("Cut: Cut the selected text to the clipboard"));
	a->setEnabled(false);
	connect(editor->view(), SIGNAL(copyAvailable(bool)), a, SLOT(setEnabled(bool)));

	a = KStandardAction::copy(editor->view(), SLOT(copy()), ac);
	a->setStatusTip(i18n("Copy the selected text to the clipboard"));
	a->setWhatsThis(i18n("Copy: Copy the selected text to the clipboard"));
	a->setEnabled(false);
	connect(editor->view(), SIGNAL(copyAvailable(bool)), a, SLOT(setEnabled(bool)));

	a = KStandardAction::paste(editor->view(), SLOT(paste()), ac);
	a->setStatusTip(i18n("Paste the clipboard's content into the current selection"));
	a->setWhatsThis(i18n("Paste: Paste the clipboard's content into the current selection"));

	a = KStandardAction::selectAll(editor->view(), SLOT(selectAll()), ac);
	a->setStatusTip(i18n("Select all the code in the editor"));
	a->setWhatsThis(i18n("Select All: Select all the code in the editor"));
	a->setEnabled(true);

	a  = new QAction(i18n("Overwrite Mode"), this);
	actionCollection()->addAction("overwrite", a );
	a->setStatusTip(i18n("Toggle between the 'insert' and 'overwrite' mode"));
	a->setWhatsThis(i18n("Overwrite Mode: Toggle between the 'insert' and 'overwrite' mode"));
	a->setShortcut(QKeySequence(Qt::Key_Insert));
	a->setCheckable(true);
	a->setChecked(false);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(toggleOverwriteMode(bool)));

	a = KStandardAction::find(editor, SLOT(find()), ac);
	a->setStatusTip(i18n("Search through the code in the editor"));
	a->setWhatsThis(i18n("Find: Search through the code in the editor"));

	a = KStandardAction::findNext(editor, SLOT(findNext()), ac);
	a->setStatusTip(i18n("Continue searching through the code in the editor"));
	a->setWhatsThis(i18n("Find Next: Continue searching through the code in the editor"));

	a = KStandardAction::findPrev(editor, SLOT(findPrev()), ac);
	a->setStatusTip(i18n("Continue searching backwards through the code in the editor"));
	a->setWhatsThis(i18n("Find Previous: Continue searching backwards through the code in the editor"));

	//TODO: Implement search/replace
	//a = KStandardAction::replace(editor, SLOT(replace()), ac);
	//a->setStatusTip(i18n("Search and replace"));
	//a->setWhatsThis(i18n("Replace: Replace text in the editor"));

	// Canvas menu action
	exportToPngAct = actionCollection()->addAction("canvas_export_to_png");
	exportToPngAct->setText(i18n("Export to &Image (PNG)..."));
	exportToPngAct->setStatusTip(i18n("Export the current canvas to a PNG raster image"));
	exportToPngAct->setWhatsThis(i18n("Export to PNG: Export the current canvas to a PNG raster image"));
	connect(exportToPngAct, SIGNAL(triggered()), this, SLOT(exportToPng()));

	exportToSvgAct = actionCollection()->addAction("canvas_export_to_svg");
	exportToSvgAct->setText(i18n("Export to &Drawing (SVG)..."));
	exportToSvgAct->setStatusTip(i18n("Export the current canvas to Scalable Vector Graphics"));
	exportToSvgAct->setWhatsThis(i18n("Export to SVG: Export the current canvas to Scalable Vector Graphics"));
	connect(exportToSvgAct, SIGNAL(triggered()), this, SLOT(exportToSvg()));

	printCanvasAct = new QAction(KIcon("document-print"), i18n("&Print Canvas..."), this);
	actionCollection()->addAction("canvas_print", printCanvasAct);
	printCanvasAct->setStatusTip(i18n("Print the canvas"));
	printCanvasAct->setWhatsThis(i18n("Print: Print the canvas"));
	connect(printCanvasAct, SIGNAL(triggered()), this, SLOT(canvasPrintDialog()));

	// Run menu actions
	runAct = new QAction(KIcon("media-playback-start"), i18n("&Run"), this);
	actionCollection()->addAction("run", runAct);
	runAct->setShortcut(QKeySequence(Qt::Key_F5));
	runAct->setStatusTip(i18n("Execute the program"));
	runAct->setWhatsThis(i18n("Run: Execute the program"));
	connect(runAct, SIGNAL(triggered()), this, SLOT(run()));

	pauseAct = new QAction(KIcon("media-playback-pause"), i18n("&Pause"), this);
	actionCollection()->addAction("pause", pauseAct);
	pauseAct->setCheckable(true);
	pauseAct->setShortcut(QKeySequence(Qt::Key_F6));
	pauseAct->setStatusTip(i18n("Pause execution"));
	pauseAct->setWhatsThis(i18n("Pause: Pause execution"));
	connect(pauseAct, SIGNAL(triggered()), this, SLOT(pause()));

	abortAct = new QAction(KIcon("process-stop"), i18n("&Abort"), this);
	actionCollection()->addAction("abort", abortAct);
	abortAct->setShortcut(QKeySequence(Qt::Key_F7));
	abortAct->setStatusTip(i18n("Stop executing program"));
	abortAct->setWhatsThis(i18n("Abort: Stop executing program"));
	connect(abortAct, SIGNAL(triggered()), this, SLOT(abort()));

// 	new QAction(i18n("&Indent"), "format-indent-more", CTRL+Key_I, this, SLOT(slotIndent()), ac, "edit_indent");
// 	new QAction(i18n("&Unindent"), "format-indent-less", CTRL+SHIFT+Key_I, this, SLOT(slotUnIndent()), ac, "edit_unindent");
// 	new QAction(i18n("Cl&ean Indentation"), 0, 0, this, SLOT(slotCleanIndent()), ac, "edit_cleanIndent");
// 	new QAction(i18n("Co&mment"), 0, CTRL+Key_D, this, SLOT(slotComment()), ac, "edit_comment");
// 	new QAction(i18n("Unc&omment"), 0, CTRL+SHIFT+Key_D, this, SLOT(slotUnComment()), ac, "edit_uncomment");

	// Tools menu actions
	a = new QAction(i18n("&Direction Chooser..."), this);
	actionCollection()->addAction("direction_chooser", a);
	a->setStatusTip(i18n("Shows the direction chooser dialog"));
	a->setWhatsThis(i18n("Direction Chooser: Show the direction chooser dialog"));
	connect(a, SIGNAL(triggered()), this, SLOT(showDirectionDialog()));

	a = new QAction(i18n("&Color Picker..."), this);
	actionCollection()->addAction("color_picker", a);
	a->setStatusTip(i18n("Shows the color picker dialog"));
	a->setWhatsThis(i18n("Color Picker: Show the color picker dialog"));
	connect(a, SIGNAL(triggered()), this, SLOT(showColorPicker()));

	// Settings menu action
	a = new QAction(i18n("Show &Editor"), this);
	actionCollection()->addAction("show_editor", a);
	a->setStatusTip(i18n("Show or hide the Code Editor"));
	a->setWhatsThis(i18n("Show Code Editor: Show or hide the Code Editor"));
	a->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
	a->setCheckable(true);
	a->setChecked(true);
	connect(a, SIGNAL(toggled(bool)), editorDock, SLOT(setVisible(bool)));
	connect(editorDock, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));

	a = new QAction(i18n("Show &Inspector"), this);
	actionCollection()->addAction("show_inspector", a);
	a->setStatusTip(i18n("Show or hide the Inspector"));
	a->setWhatsThis(i18n("Show Inspector: Show or hide the Inspector"));
	a->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
	a->setCheckable(true);
	a->setChecked(true);
	connect(a, SIGNAL(toggled(bool)), inspectorDock, SLOT(setVisible(bool)));
	connect(inspectorDock, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));

	a = new QAction(i18n("Show E&rrors"), this);
	actionCollection()->addAction("show_errors", a);
	a->setStatusTip(i18n("Show or hide the Errors tab"));
	a->setWhatsThis(i18n("Show Errors: Show or hide the Errors tab"));
	a->setCheckable(true);
	a->setChecked(false);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(showErrorDialog(bool)));
        
// 	a = new QAction(i18n("Show &Console"), this);
// 	actionCollection()->addAction("show_console", a);
// 	a->setStatusTip(i18n("Show or hide the interative Console tab"));
// 	a->setWhatsThis(i18n("Show Console: Show or hide the interactive Console tab"));
// 	a->setCheckable(true);
// 	a->setChecked(false);
// 	connect(a, SIGNAL(toggled(bool)), consoleDock, SLOT(setVisible(bool)));
// 	connect(consoleDock, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));

	a = new QAction(i18n("Show &Line Numbers"), this);
	actionCollection()->addAction("line_numbers", a);
	a->setStatusTip(i18n("Turn the line numbers on/off in the editor"));
	a->setWhatsThis(i18n("Show Line Numbers: Turn the line numbers on/off in the editor"));
	a->setShortcut(QKeySequence(Qt::Key_F11));
	a->setCheckable(true);
	a->setChecked(true);
	connect(a, SIGNAL(toggled(bool)), editor, SLOT(toggleLineNumbers(bool)));

	// Help menu actions
	contextHelpAct = ac->addAction("context_help");
	contextHelpAct->setText("");
	contextHelpAct->setIcon(KIcon("help-about"));
	contextHelpAct->setShortcut(QKeySequence(Qt::Key_F2));
	contextHelpAct->setStatusTip(i18n("Get help on the command under the cursor"));
	contextHelpAct->setWhatsThis(i18n("Context Help: Get help on the command under the cursor"));
	connect(contextHelpAct, SIGNAL(triggered()), this, SLOT(contextHelp()));
	updateContextHelpAction();

	a = actionCollection()->addAction(KStandardAction::HelpContents, "help_contents", this, SLOT(appHelpActivated()));
	a->setStatusTip(i18n("Help"));
	a->setWhatsThis(i18n("Help: Open manual for KTurtle"));

	// Menuless actions
	console = new Console(this);
	console->setText(i18n("Console"));
	console->setShortcut(QKeySequence(Qt::Key_F4));
	actionCollection()->addAction("console", console);
	connect(console, SIGNAL(execute(const QString&)), this, SLOT(execute(const QString&)));

	executeConsoleAct = actionCollection()->addAction("execute_console");
	executeConsoleAct->setIcon(KIcon("go-jump-locationbar"));
	executeConsoleAct->setText(i18n("Execute"));
	connect(executeConsoleAct, SIGNAL(triggered()), console, SLOT(executeActionTriggered()));
	executeConsoleAct->setWhatsThis(i18n("Execute: Executes the current line in the console"));

	// The run speed action group
	QActionGroup* runSpeedGroup = new QActionGroup(this);

	// The run action collection, this is used in the toolbar to create a dropdown menu on the run button
	KToolBarPopupAction* runSpeedAction = new KToolBarPopupAction(KIcon("media-playback-start"), i18n("&Run"), this);
	connect(runSpeedAction, SIGNAL(triggered()), this, SLOT(run()));
	QMenu* runSpeedActionMenu = runSpeedAction->menu();
	actionCollection()->addAction("run_speed", runSpeedAction);
	runSpeedActionMenu->setStatusTip(i18n("Execute the program, or use the drop down menu to select the run speed"));
	runSpeedActionMenu->setWhatsThis(i18n("Run: Execute the program, or use the drop down menu to select the run speed"));
	connect(runSpeedActionMenu, SIGNAL(triggered (QAction*)), this, SLOT(run()));

	dedicatedSpeedAct = new QAction(i18nc("@option:radio", "Full Speed (&no highlighting and inspector)"), this);
	actionCollection()->addAction("dedicated_speed", dedicatedSpeedAct);
	dedicatedSpeedAct->setCheckable(true);
	dedicatedSpeedAct->setStatusTip(i18n("Run the program at full speed, with highlighting and inspector disabled"));
	dedicatedSpeedAct->setWhatsThis(i18n("Full Speed: Run the program at full speed, with highlighting and inspector disabled"));
	connect(dedicatedSpeedAct, SIGNAL(triggered()), this, SLOT(setDedicatedSpeed()));
	runSpeedGroup->addAction(dedicatedSpeedAct);
	runSpeedActionMenu->addAction(dedicatedSpeedAct);

	fullSpeedAct = new QAction(i18nc("@option:radio", "&Full Speed"), this);
	actionCollection()->addAction("full_speed", fullSpeedAct);
	fullSpeedAct->setCheckable(true);
	fullSpeedAct->setChecked(true);
	fullSpeedAct->setStatusTip(i18n("Run the program at full speed"));
	fullSpeedAct->setWhatsThis(i18n("Full Speed: Run the program at full speed"));
	connect(fullSpeedAct, SIGNAL(triggered()), this, SLOT(setFullSpeed()));
	runSpeedGroup->addAction(fullSpeedAct);
	runSpeedActionMenu->addAction(fullSpeedAct);

	slowSpeedAct = new QAction(i18nc("@option:radio choose the slow speed", "&Slow"), this);
	actionCollection()->addAction("slow_speed", slowSpeedAct);
	slowSpeedAct->setCheckable(true);
	slowSpeedAct->setStatusTip(i18n("Run the program at a slow speed"));
	slowSpeedAct->setWhatsThis(i18n("Slow Speed: Run the program at a slow speed"));
	connect(slowSpeedAct, SIGNAL(triggered()), this, SLOT(setSlowSpeed()));
	runSpeedGroup->addAction(slowSpeedAct);
	runSpeedActionMenu->addAction(slowSpeedAct);

	slowerSpeedAct = new QAction(i18nc("@option:radio", "S&lower"), this);
	actionCollection()->addAction("slower_speed", slowerSpeedAct);
	slowerSpeedAct->setCheckable(true);
	slowerSpeedAct->setStatusTip(i18n("Run the program at a slower speed"));
	slowerSpeedAct->setWhatsThis(i18n("Slower Speed: Run the program at a slower speed"));
	connect(slowerSpeedAct, SIGNAL(triggered()), this, SLOT(setSlowerSpeed()));
	runSpeedGroup->addAction(slowerSpeedAct);
	runSpeedActionMenu->addAction(slowerSpeedAct);

	slowestSpeedAct = new QAction(i18nc("@option:radio", "Sl&owest"), this);
	actionCollection()->addAction("slowest_speed", slowestSpeedAct);
	slowestSpeedAct->setCheckable(true);
	slowestSpeedAct->setStatusTip(i18n("Run the program at the slowest speed"));
	slowestSpeedAct->setWhatsThis(i18n("Slowest Speed: Run the program at the slowest speed"));
	connect(slowestSpeedAct, SIGNAL(triggered()), this, SLOT(setSlowestSpeed()));
	runSpeedGroup->addAction(slowestSpeedAct);
	runSpeedActionMenu->addAction(slowestSpeedAct);

	stepSpeedAct = new QAction(i18nc("@option:radio", "S&tep-by-Step"), this);
	actionCollection()->addAction("step_speed", stepSpeedAct);
	stepSpeedAct->setCheckable(true);
	stepSpeedAct->setStatusTip(i18n("Run the program one step at a time"));
	stepSpeedAct->setWhatsThis(i18n("Step Speed: Run the program one step at a time"));
	connect(stepSpeedAct, SIGNAL(triggered()), this, SLOT(setStepSpeed()));
	runSpeedGroup->addAction(stepSpeedAct);
	runSpeedActionMenu->addAction(stepSpeedAct);
}


void MainWindow::setupCanvas()
{
	// put the canvas in a layout as the cetral widget of the mainwindow
	QWidget* centralWidget = new QWidget(this);
	QHBoxLayout* centralLayout = new QHBoxLayout(centralWidget);
	centralLayout->setMargin(0);  // MARGIN_SIZE);

	canvasTabWidget = new KTabWidget(this);

	canvasTab = new QWidget();
	QHBoxLayout* canvasLayout = new QHBoxLayout(canvasTab);
	canvas = new Canvas(this);
	canvas->setFocusPolicy(Qt::NoFocus);
	canvas->setRenderHint(QPainter::Antialiasing);
	canvas->setWhatsThis(i18n("Canvas: This is where the turtle moves and draws when the program is running"));
	canvasLayout->addWidget(canvas);
        canvasLayout->setMargin(0);
	canvasTabWidget->insertTab(0, canvasTab, i18n("&Canvas"));

	QWidget* errorTab = new QWidget();
	QHBoxLayout* errorLayout = new QHBoxLayout(errorTab);
	errorDialog = new ErrorDialog(this);
	connect(errorDialog, SIGNAL(user1Clicked()), this, SLOT(showErrorDialog()));
	errorLayout->addWidget(errorDialog);
	canvasTabWidget->insertTab(1, errorTab, i18n("E&rrors"));

	// a widget stach with 2 layers: 1st with only a canvas, 2nd with the canvas/error tabs 
	stackedWidget = new QStackedWidget;
	stackedWidget->insertWidget(0, canvasTab);
	stackedWidget->insertWidget(1, canvasTabWidget);

	centralLayout->addWidget(stackedWidget);
	setCentralWidget(centralWidget);
}

void MainWindow::showErrorDialog(bool show)
{
	if (show) {
		// show the canvas and errors in a tab widget, focussing on the errors
		stackedWidget->setCurrentIndex(1);
		canvasTabWidget->insertTab(0, canvasTab, i18n("&Canvas"));
		canvasTabWidget->setCurrentIndex(1);
		actionCollection()->action("show_errors")->setChecked(true);
	} else {
		// show the canvas only
		stackedWidget->insertWidget(0, canvasTab);
		stackedWidget->setCurrentIndex(0);
		actionCollection()->action("show_errors")->setChecked(false);
	}
}


void MainWindow::setupDockWindows()
{
	editorDock = new LocalDockWidget(i18n("&Editor"), this);
	editorDock->setObjectName("editor");
	QWidget* editorWrapWidget = new QWidget(editorDock);
	QHBoxLayout* editorDockLayout = new QHBoxLayout(editorWrapWidget);
 	editorDockLayout->setMargin(MARGIN_SIZE);
 	editorWrapWidget->setLayout(editorDockLayout);
// 	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	editor = new Editor(editorWrapWidget);  // create this here to prevent crashes
	editorDockLayout->addWidget(editor);
	editorDock->setWidget(editorWrapWidget);
// 	editorDock->setFeatures(QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable);
	addDockWidget(Qt::LeftDockWidgetArea, editorDock);
	editor->show();
	editor->setFocus();
	editor->setWhatsThis(i18n("Editor: Write your KTurtle commands here"));

	// Creating the debug window
	inspectorDock = new LocalDockWidget(i18n("&Inspector"), this);
	inspectorDock->setObjectName("inspector");
	QWidget* inspectorWrapWidget = new QWidget(inspectorDock);
	QHBoxLayout* inspectorDockLayout = new QHBoxLayout(inspectorWrapWidget);
	inspectorDockLayout->setMargin(MARGIN_SIZE);
	inspectorWrapWidget->setLayout(inspectorDockLayout);
	inspector = new Inspector(inspectorWrapWidget);
	inspectorDockLayout->addWidget(inspector);
	inspectorDock->setWidget(inspectorWrapWidget);
	addDockWidget(Qt::RightDockWidgetArea, inspectorDock);
	inspector->setWhatsThis(i18n("Inspector: See information about variables and functions when the program runs"));
}

void MainWindow::setupEditor()
{
// 	editor->setTranslator(Translator::instance());
	connect(editor, SIGNAL(modificationChanged()), this, SLOT(updateModificationState()));
	connect(editor, SIGNAL(contentNameChanged(const QString&)), this, SLOT(updateContentName(const QString&)));
	connect(editor, SIGNAL(fileOpened(const KUrl&)), this, SLOT(addToRecentFilesList(const KUrl&)));
	connect(editor, SIGNAL(fileSaved(const KUrl&)), this, SLOT(addToRecentFilesList(const KUrl&)));
	connect(editor, SIGNAL(cursorPositionChanged()), this, SLOT(updateOnCursorPositionChange()));
}

void MainWindow::setupInterpreter()
{
	interpreter = new Interpreter(this, false);
	connect(interpreter, SIGNAL(finished()), this, SLOT(abort()));
	Executer* executer = interpreter->getExecuter();

	// the code to connect the executer with the canvas is auto generated:
#include "interpreter/gui_connect.inc"
	connect(interpreter, SIGNAL(treeUpdated(TreeNode*)),
		inspector, SLOT(updateTree(TreeNode*)));

	toggleGuiFeedback(true);
}

void MainWindow::toggleGuiFeedback(bool b)
{
	Executer* executer = interpreter->getExecuter();
	if (b) {
		connect(executer, SIGNAL(currentlyExecuting(TreeNode*)), editor, SLOT(markCurrentWord(TreeNode*)));
		connect(executer, SIGNAL(currentlyExecuting(TreeNode*)), inspector, SLOT(markTreeNode(TreeNode*)));
			
		connect(executer, SIGNAL(variableTableUpdated(const QString&, const Value&)),
			inspector, SLOT(updateVariable(const QString&, const Value&)));
		connect(executer, SIGNAL(functionTableUpdated(const QString&, const QStringList&)),
			inspector, SLOT(updateFunction(const QString&, const QStringList&)));
	} else {
		disconnect(executer, SIGNAL(currentlyExecuting(TreeNode*)), editor, SLOT(markCurrentWord(TreeNode*)));
		disconnect(executer, SIGNAL(currentlyExecuting(TreeNode*)), inspector, SLOT(markTreeNode(TreeNode*)));
		
		disconnect(executer, SIGNAL(variableTableUpdated(const QString&, const Value&)),
			inspector, SLOT(updateVariable(const QString&, const Value&)));
		disconnect(executer, SIGNAL(functionTableUpdated(const QString&, const QStringList&)),
			inspector, SLOT(updateFunction(const QString&, const QStringList&)));
		editor->removeMarkings();
	}
}

void MainWindow::setupStatusBar()
{
	statusBarLanguageLabel = new QLabel(statusBar());
	statusBar()->addPermanentWidget(statusBarLanguageLabel, 0);
	statusBarLanguageLabel->setAlignment(Qt::AlignRight);

	statusBarPositionLabel = new QLabel(statusBar());
	statusBar()->addPermanentWidget(statusBarPositionLabel, 0);
	statusBarPositionLabel->setAlignment(Qt::AlignRight);

	statusBarOverwriteModeLabel = new QLabel(statusBar());
	statusBar()->addPermanentWidget(statusBarOverwriteModeLabel, 0);
	statusBarOverwriteModeLabel->setAlignment(Qt::AlignRight);

	statusBarFileNameLabel = new QLabel(statusBar());
	statusBar()->addPermanentWidget(statusBarFileNameLabel, 0);
	statusBarFileNameLabel->setAlignment(Qt::AlignRight);

	toggleOverwriteMode(false);
	updateOnCursorPositionChange();
}

void MainWindow::saveNewToolbarConfig()
{
	// this destroys our actions lists ...
	KXmlGuiWindow::saveNewToolbarConfig();
	// ... so plug them again
	updateLanguagesMenu();
	updateExamplesMenu();
}

void MainWindow::updateLanguagesMenu()
{
	QList<QAction *> languageList;
	QActionGroup* languageGroup = new QActionGroup(this);
	connect(languageGroup, SIGNAL(triggered(QAction *)), this, SLOT(setLanguage(QAction *)));
	QAction* a;
	// sort the dictionaries using an algorithm found in the qt docs:
	QMap<QString, QString> map;
	foreach (const QString &lang_code, KGlobal::locale()->languageList())
		map.insert(codeToFullName(lang_code), lang_code);
	// populate the menu:
	foreach (const QString &lang_code, map) {
		a = new QAction(codeToFullName(lang_code), actionCollection());
		a->setData(lang_code);
		a->setStatusTip(i18n("Switch to the %1 dictionary", codeToFullName(lang_code)));
		a->setCheckable(true);
		if (lang_code == currentLanguageCode) {
			a->setChecked(true);
		}
		//languageMenu->addAction(a);
		languageGroup->addAction(a);
		languageList.append(a);
	}
	unplugActionList ("languages_actionlist");
	plugActionList   ("languages_actionlist", languageList);
}

void MainWindow::updateExamplesMenu()
{
	QAction * newExample;
	QString actionName;
	QList<QAction*> exampleList;
	QActionGroup* exampleGroup = new QActionGroup (this);

	foreach (const QString &exampleName, Translator::instance()->exampleNames()) {
		newExample = new QAction (exampleName, this);
		newExample->setData(exampleName);
		exampleGroup->addAction (newExample);

		connect (newExample, SIGNAL(triggered()), this, SLOT(openExample()));
		exampleList.append (newExample);
	}

	QStringList allExamples = KGlobal::dirs()->findAllResources("data", GHNS_TARGET, KStandardDirs::NoDuplicates);

	if(allExamples.size()>0) {
		newExample = new QAction(this);
		newExample->setSeparator(true);
		exampleGroup->addAction(newExample);
		exampleList.append(newExample);
	}

	foreach(const QString& exampleFilename, allExamples) {
		QFileInfo fileInfo(exampleFilename);
		newExample = new QAction (fileInfo.baseName(), this);
		newExample->setData(exampleFilename);
		exampleGroup->addAction (newExample);
		exampleList.append (newExample);
			
		connect(newExample, SIGNAL(triggered()), this, SLOT(openDownloadedExample()));
	}

	unplugActionList ("examples_actionlist");
	plugActionList   ("examples_actionlist", exampleList);
}

void MainWindow::addToRecentFilesList(const KUrl& url)
{
	recentFilesAction->addUrl(url);
}

void MainWindow::openExample()
{
	QAction* action = qobject_cast<QAction*>(sender());
	QString exampleName = action->data().toString();
	editor->openExample(Translator::instance()->example(exampleName), exampleName);
}

void MainWindow::openDownloadedExample()
{
	QAction* action = qobject_cast<QAction*>(sender());
	QString exampleFilename = action->data().toString();
	editor->openFile(KUrl(exampleFilename));
	QFileInfo fileInfo(exampleFilename);
	editor->setCurrentUrl(fileInfo.baseName());
}

void MainWindow::toggleOverwriteMode(bool b)
{
	statusBarOverwriteModeLabel->setText(b ? i18n(" OVR ") : i18n(" INS "));
	editor->setOverwriteMode(b);
}

void MainWindow::updateContextHelpAction(const QString& s, const QString& anchor)
{
	//qDebug() << QString("%1 (help anchor: %2)").arg(s).arg(anchor);
	contextHelpAnchor = anchor;
	contextHelpString = s.isEmpty() ? i18n("<nothing under cursor>") : s;
	contextHelpAct->setText(i18n("Help on: %1", contextHelpString));
}

void MainWindow::updateOnCursorPositionChange()
{
	statusBarPositionLabel->setText(i18n(" Line: %1 Column: %2 ", editor->row(), editor->col()));

	Token* cursorToken = editor->currentToken();
	QString desc;
	if (cursorToken != 0) {
		QString look = cursorToken->look();
		int cat = cursorToken->category();
		delete cursorToken;
		cursorToken = 0;
		KLocalizedString layout = ki18n("\"%1\" <%2>");
		switch (cat) {
			// not showing the look (only the name):
			case Token::VariableCategory:     updateContextHelpAction(i18n("<variable>"), "variable"); return;
			case Token::NumberCategory:       updateContextHelpAction(i18n("<number>"), "number");     return;
			case Token::CommentCategory:      updateContextHelpAction(i18n("<comment>"), "comment");   return;
			case Token::StringCategory:       updateContextHelpAction(i18n("<string>"), "string");     return;
			// only showing the look:
			case Token::LearnCommandCategory: updateContextHelpAction(look, "learn");                  return;
			case Token::TrueFalseCategory:    updateContextHelpAction(look, "boolean");                return;
			// showing the look and the name:
			case Token::ScopeCategory:
				updateContextHelpAction(layout.subs(look).subs(i18n("scope")).toString(), "scope"); return;
			case Token::AssignmentCategory:
				updateContextHelpAction(layout.subs(look).subs(i18n("assignment")).toString(), "assignment"); return;
			case Token::ParenthesisCategory:
				updateContextHelpAction(layout.subs(look).subs(i18n("parenthesis")).toString(), "parenthesis"); return;
			case Token::MathOperatorCategory:
				updateContextHelpAction(layout.subs(look).subs(i18n("mathematical operator")).toString(), "math-operator"); return;
			case Token::ExpressionCategory:
				updateContextHelpAction(layout.subs(look).subs(i18n("expression")).toString(), "expression"); return;
			case Token::BooleanOperatorCategory:
				updateContextHelpAction(layout.subs(look).subs(i18n("boolean operator")).toString(), "boolean-operator"); return;
			case Token::FunctionCallCategory:
				updateContextHelpAction(layout.subs(look).subs(i18n("learned command")).toString(), "learned-command"); return;
			case Token::ArgumentSeparatorCategory:
				updateContextHelpAction(layout.subs(look).subs(i18n("argument separator")).toString(), "argument-separator"); return;
			// showing the look and the name, and linking to the help through their default look (en_US):
			case Token::CommandCategory:
				updateContextHelpAction(layout.subs(look).subs(i18n("command")).toString(), Translator::instance()->defaultLook(look));
				return;
			case Token::ControllerCommandCategory:
				updateContextHelpAction(layout.subs(look).subs(i18n("controller command")).toString(), Translator::instance()->defaultLook(look));
				return;
		}
	}
	updateContextHelpAction();  // display the 'nothing under cursor thing'
}


void MainWindow::setLanguage(QAction *action)
{
	if (setCurrentLanguage(action->data().toString()))
		action->setChecked(true);
}

bool MainWindow::setCurrentLanguage(const QString &lang_code)  // 2 or 5 digit code (en, en_US, nl, pt_BR)
{
	bool result = false;
	//qDebug() << "MainWindow::setCurrentLanguage: " << lang_code;
	if (Translator::instance()->setLanguage(lang_code)) {
		currentLanguageCode = lang_code;
		statusBarLanguageLabel->setText(' ' + codeToFullName(lang_code) + ' ');
		updateExamplesMenu();
		editor->rehighlight();
		result = true;
	} else {
		KMessageBox::error(this, i18n("Could not change the language to %1.", codeToFullName(lang_code)));
	}
	return result;
}

QString MainWindow::codeToFullName(const QString& lang_code)
{
	// TODO test this function with more than one language loaded into KDE
	return QString(lang_code == "en_US" ?
			i18n("English [built in]") :
			i18n("%1 (%2)", KGlobal::locale()->languageCodeToName(lang_code.left(2)), lang_code)
		);
}


void MainWindow::run()
{
	if (interpreter->state() == Interpreter::Uninitialized ||
	    interpreter->state() == Interpreter::Finished ||
	    interpreter->state() == Interpreter::Aborted) {
		// reset inspector and interpreter
		editor->removeMarkings();
		inspector->clear();
		errorDialog->clear();
		showErrorDialog(false);
		interpreter->initialize(editor->content());
	}
	editor->disable();
	console->disable();
	executeConsoleAct->setEnabled(false);
	toggleGuiFeedback(runSpeed != 0);

	// start parsing (always in full speed)
	iterationTimer->setSingleShot(false);
	iterationTimer->start(0);
}

QString MainWindow::execute(const QString &operation)
{
	disconnect(interpreter, SIGNAL(finished()), this, SLOT(abort()));
	disconnect(interpreter, SIGNAL(treeUpdated(TreeNode*)), inspector, SLOT(updateTree(TreeNode*)));
	Executer* executer = interpreter->getExecuter();
	disconnect(executer, SIGNAL(currentlyExecuting(TreeNode*)), editor, SLOT(markCurrentWord(TreeNode*)));
	disconnect(executer, SIGNAL(currentlyExecuting(TreeNode*)), inspector, SLOT(markTreeNode(TreeNode*)));
	disconnect(executer, SIGNAL(variableTableUpdated(const QString&, const Value&)),
		inspector, SLOT(updateVariable(const QString&, const Value&)));
	disconnect(executer, SIGNAL(functionTableUpdated(const QString&, const QStringList&)),
		inspector, SLOT(updateFunction(const QString&, const QStringList&)));

	if (interpreter->state() == Interpreter::Uninitialized ||
	    interpreter->state() == Interpreter::Finished ||
	    interpreter->state() == Interpreter::Aborted) {
		interpreter->initialize(operation);
	}

	runAct->setEnabled(false);
	pauseAct->setEnabled(false);
	abortAct->setEnabled(false);

	currentlyRunningConsole = true;

	while (!(interpreter->state() == Interpreter::Finished ||
	         interpreter->state() == Interpreter::Aborted)) {
		interpreter->interpret();
	}
	
	currentlyRunningConsole = false;

	runAct->setEnabled(true);
	pauseAct->setEnabled(false);
	abortAct->setEnabled(false);

	QString errorMessage;

	if (interpreter->encounteredErrors()) {
		ErrorList* errorList = interpreter->getErrorList();
		//qDebug() << errorList->first().text();
		errorMessage = errorList->first().text();
	}

	connect(interpreter, SIGNAL(finished()), this, SLOT(abort()));
	connect(interpreter, SIGNAL(treeUpdated(TreeNode*)), inspector, SLOT(updateTree(TreeNode*)));
	connect(executer, SIGNAL(currentlyExecuting(TreeNode*)), editor, SLOT(markCurrentWord(TreeNode*)));
	connect(executer, SIGNAL(currentlyExecuting(TreeNode*)), inspector, SLOT(markTreeNode(TreeNode*)));
	connect(executer, SIGNAL(variableTableUpdated(const QString&, const Value&)),
		inspector, SLOT(updateVariable(const QString&, const Value&)));
	connect(executer, SIGNAL(functionTableUpdated(const QString&, const QStringList&)),
		inspector, SLOT(updateFunction(const QString&, const QStringList&)));

	return errorMessage;
}

void MainWindow::iterate()
{
	if (interpreter->state() == Interpreter::Finished || interpreter->state() == Interpreter::Aborted) {
		abort();
		return;
	}

	runAct->setEnabled(false);
	pauseAct->setChecked(false);
	pauseAct->setEnabled(true);
	abortAct->setEnabled(true);

	if (interpreter->state() == Interpreter::Executing) {
		iterationTimer->stop();
		iterationTimer->setSingleShot(true);
		switch (runSpeed) {
			case 0: iterationTimer->start(0);    break;
			case 1: iterationTimer->start(0);    break;
			case 2: iterationTimer->start(500);  break;
			case 3: iterationTimer->start(1000); break;
			case 4: iterationTimer->start(3000); break;
			case 5:
				iterationTimer->stop();
				interpreter->interpret();
				pauseAct->setChecked(true);
				pause();
				return;
		}
	}
	interpreter->interpret();
}

void MainWindow::pause()
{
	if (pauseAct->isChecked()) {
		runAct->setEnabled(true);
		iterationTimer->stop();
		return;
	}
	iterate();
}

void MainWindow::abort()
{
	iterationTimer->stop();
	interpreter->abort();

	editor->removeMarkings();
	inspector->clearAllMarks();

	runAct->setEnabled(true);
	pauseAct->setChecked(false);
	pauseAct->setEnabled(false);
	abortAct->setEnabled(false);

	editor->enable();
	console->enable();
	executeConsoleAct->setEnabled(true);

	if (interpreter->encounteredErrors()) {
		errorDialog->setErrorList(interpreter->getErrorList());
		showErrorDialog(true);
	}
}


void MainWindow::updateContentName(const QString& str)
{
	QString caption = str.isEmpty() ? i18n("untitled") : str;
	bool modified = editor->isModified();
	KXmlGuiWindow::setCaption(caption, modified);
	statusBarFileNameLabel->setText(QString(" %1%2 ").arg(caption).arg(modified ? "*" : ""));
}

void MainWindow::addToRecentFiles(const KUrl &url)
{
	recentFilesAction->addUrl(url);
}


void MainWindow::readConfig()
{
	KConfigGroup config(KGlobal::config(), "General Options");
// 	m_paShowStatusBar->setChecked(config->readEntry("ShowStatusBar", QVariant(false)).toBool());
// 	m_paShowPath->setChecked(config->readEntry("ShowPath", QVariant(false)).toBool());
	recentFilesAction->loadEntries(KGlobal::config()->group("Recent Files"));
	QString lang_code(config.readEntry("currentLanguageCode", QVariant(QString())).toString());
	if (lang_code.isEmpty()) lang_code = "en_US";  // null-string are saved as empty-strings
	setCurrentLanguage(lang_code);
// 	if(m_paShowStatusBar->isChecked())
// 		statusBar()->show();
// 	else
// 		statusBar()->hide();
}

void MainWindow::writeConfig()
{
	KConfigGroup config(KGlobal::config(), "General Options");
// 	config.writeEntry("ShowStatusBar",m_paShowStatusBar->isChecked());
// 	config.writeEntry("ShowPath",m_paShowPath->isChecked());
	recentFilesAction->saveEntries(KGlobal::config()->group( "Recent Files"));
	config.writeEntry("currentLanguageCode", currentLanguageCode);
	config.sync();
}

void MainWindow::exportToPng()
{
	// copied from edit code for file selection
	KUrl url = KFileDialog::getSaveUrl(QString(), QString("*.png|%1\n*|%2").arg(i18n("PNG Images")).arg(i18n("All files")), this, i18n("Save as Picture"));
    if (url.isEmpty())
        return;
	if (KIO::NetAccess::exists(url, KIO::NetAccess::SourceSide, this) &&
	    KMessageBox::warningContinueCancel(this,
			i18n("Are you sure you want to overwrite %1?", url.fileName()),
			i18n("Overwrite Existing File"),KGuiItem(i18n("&Overwrite")),
            KStandardGuiItem::cancel(), i18n("&Overwrite")) != KMessageBox::Continue) {
		return;
    }
    // get our image from the canvas and save to png
    QImage pict = canvas->getPicture();
    if (url.isLocalFile()) {
        pict.save(url.toLocalFile(), "PNG");
    } else {
        pict.save(url.path(), "PNG");
    }
}

void MainWindow::exportToSvg()
{
    // copied from edit code for file selection
    // canvas->saveAsSvg() does not handle KUrl, so only local files are accepted
	QString path = KFileDialog::getSaveFileName(QString(), QString("*.svg|%1\n*|%2").arg(i18n("Scalable Vector Graphics")).arg(i18n("All files")), this, i18n("Save as SVG"));
	if (path.isEmpty())
		return;
	if (KIO::NetAccess::exists(path, KIO::NetAccess::SourceSide, this) &&
	    KMessageBox::warningContinueCancel(this,
			i18n("Are you sure you want to overwrite %1?", path),
			i18n("Overwrite Existing File"),KGuiItem(i18n("&Overwrite")),
			KStandardGuiItem::cancel(), i18n("&Overwrite")) != KMessageBox::Continue)
		return;
	canvas->saveAsSvg(windowTitle(), path);
}

void MainWindow::exportToHtml()
{
    // copied from edit code for file selection
    // we do not handle KUrl, so only local files are accepted
	QString path = KFileDialog::getSaveFileName(QString(), QString("*.html|%1\n*|%2").arg(i18n("HTML documents")).arg(i18n("All files")), this, i18n("Save code as HTML"));
	if (path.isEmpty())
		return;
	if (KIO::NetAccess::exists(path, KIO::NetAccess::SourceSide, this) &&
	    KMessageBox::warningContinueCancel(this,
			i18n("Are you sure you want to overwrite %1?", path),
			i18n("Overwrite Existing File"),KGuiItem(i18n("&Overwrite")),
			KStandardGuiItem::cancel(), i18n("&Overwrite")) != KMessageBox::Continue)
		return;
	KSaveFile file(path);
	if (!file.open())
		return;
	QTextStream out(&file);
	out << editor->toHtml(windowTitle(), currentLanguageCode);
	out.flush();
	file.finalize();
}


// slots for logo functions that need to use the MainWindow class:

void MainWindow::slotInputDialog(QString& value)
{
	iterationTimer->stop();
	value = KInputDialog::getText(i18n("Input"), value);
	
	if(!currentlyRunningConsole)
		run();
}

void MainWindow::slotMessageDialog(const QString& text)
{
	iterationTimer->stop();
	KMessageBox::information(this, text, i18n("Message"));
	if(!currentlyRunningConsole)
		run();
}

void MainWindow::getNewExampleDialog()
{
	KNS3::DownloadDialog dialog(this);
	dialog.exec();
	updateExamplesMenu();
}
