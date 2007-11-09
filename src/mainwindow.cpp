/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

	This program is free software; you can redistribute it and/or
	modify it under the terms of version 2 of the GNU General Public
	License as published by the Free Software Foundation.

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

#include <kdebug.h>

#include <kactioncollection.h>
#include <kaction.h>
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

#include "errordialog.h"
#include "interpreter/errormsg.h"
#include "interpreter/translator.h"


#include "directiondialog.h"


static const int MARGIN_SIZE = 3;  // defaultly styled margins look shitty

MainWindow::MainWindow()
{
	setupDockWindows();  // the setup order matters
	setupActions();
	setupCanvas();
	setupInterpreter();
	setupEditor();
//	setupMenus();
	setupStatusBar();
//	setupToolBar();

	iterationTimer = new QTimer(this);
	connect(iterationTimer, SIGNAL(timeout()), this, SLOT(iterate()));

	statusBar()->showMessage(i18nc("@info:status the application is ready for commands", "Ready"));
	setCaption();  // also sets the window caption to 'untitled'
	setRunSpeed(0);
	abort();  // sets the run-states for the actions right*/

	setupGUI();
	// after all is set up:
	setAutoSaveSettings();
	readConfig();
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

void MainWindow::printDlg()
{
	QPrinter printer;
	QPrintDialog printDialog(&printer, this);
	if (printDialog.exec())
	{
		QPainter painter;
		painter.begin(&printer);

		editor->document()->drawContents(&painter);

		painter.end();
	}
}

void MainWindow::directionDialog()
{
	DirectionDialog ddialog(canvas->turtleAngle(), this);
	if(ddialog.exec()==QDialog::Accepted) {
		editor->view()->insertPlainText(ddialog.command());
	}
}

/*void MainWindow::about()
{
	KMessageBox::about(this,
		i18n("KTurtle is an educational programming environment that aims to make programming as easy as possible, especially for young children. KTurtle intends to help teaching kids the basics of math, geometry, and programming."), i18n("About KTurtle"));
// 	new KAboutApplication();
}*/


void MainWindow::contextHelp()
{
//TODO display a help dialog about syntax commands
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
		case 0: fullSpeedAct->setChecked(true);    break;
		case 1: slowSpeedAct->setChecked(true);    break;
		case 2: slowerSpeedAct->setChecked(true);  break;
		case 3: slowestSpeedAct->setChecked(true); break;
		case 4: stepSpeedAct->setChecked(true);    break;
	}
	//TODO runOptionBox->setCurrentIndex(speed);
	runSpeed = speed;
}

void MainWindow::setupActions()
{
	QAction * a;
	KActionCollection* ac = actionCollection();

	//TODO WHATISTHIS to each action
	//Similar to a status tip, but not the same.
	//A status tip is displayed on hover, a whatisthis is displayed when
	//an item is clicked on it whatisthis mode

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

	a = actionCollection()->addAction(KStandardAction::Save,  "file_save", editor, SLOT(saveFile()));
	a->setStatusTip(i18n("Save the current file to disk"));
	a->setWhatsThis(i18n("Save File: Save the current file to disk"));
	connect(editor, SIGNAL(modificationChanged(bool)), a, SLOT(setEnabled(bool)));

	a = actionCollection()->addAction(KStandardAction::SaveAs,  "file_save_as", editor, SLOT(saveFileAs()));
	a->setStatusTip(i18n("Save the current file under a different name"));
	a->setWhatsThis(i18n("Save File As: Save the current file under a different name"));

	runAct  = new KAction(KIcon("media-playback-start"), i18n("&Run"), this);
	actionCollection()->addAction("run", runAct );
	runAct->setShortcut(QKeySequence(Qt::Key_F5));
	runAct->setStatusTip(i18n("Execute the program"));
	runAct->setWhatsThis(i18n("Run: Execute the program"));
	connect(runAct, SIGNAL(triggered()), this, SLOT(run()));

	pauseAct  = new KAction(KIcon("media-playback-pause"), i18n("&Pause"), this);
	actionCollection()->addAction("pause", pauseAct );
	pauseAct->setCheckable(true);
	pauseAct->setShortcut(QKeySequence(Qt::Key_F6));
	pauseAct->setStatusTip(i18n("Pause execution"));
	pauseAct->setWhatsThis(i18n("Pause: Pause execution"));
	connect(pauseAct, SIGNAL(triggered()), this, SLOT(pause()));

	abortAct  = new KAction(KIcon("process-stop"), i18n("&Abort"), this);
	actionCollection()->addAction("abort", abortAct );
	abortAct->setShortcut(QKeySequence(Qt::Key_F7));
	abortAct->setStatusTip(i18n("Stop executing program"));
	abortAct->setWhatsThis(i18n("Abort: Stop executing program"));
	connect(abortAct, SIGNAL(triggered()), this, SLOT(abort()));

	a = actionCollection()->addAction(KStandardAction::Print,  "file_print", this, SLOT(printDlg()));
	a->setStatusTip(i18n("Print the code"));
	a->setWhatsThis(i18n("Print: Print the code"));

	a = actionCollection()->addAction(KStandardAction::Quit,  "file_quit", this, SLOT(close()));
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

	a  = new KAction(i18n("Toggle &Insert"), this);
	actionCollection()->addAction("set_insert", a );
	a->setStatusTip(i18n("Toggle between the 'insert' and 'overwrite' mode"));
	a->setWhatsThis(i18n("Toggle Insert: Toggle between the 'insert' and 'overwrite' mode"));
	a->setShortcut(QKeySequence(Qt::Key_Insert));
	a->setCheckable(true);
	a->setChecked(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(toggleInsertMode(bool)));

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

	// View menu actions
	a  = new KAction(i18n("Show &Code Editor"), this);
	actionCollection()->addAction("show_editor", a );
	a->setStatusTip(i18n("Show or hide the Code Editor"));
	a->setWhatsThis(i18n("Show Code Editor: Show or hide the Code Editor"));
	a->setCheckable(true);
	a->setChecked(true);
	connect(a, SIGNAL(toggled(bool)), editorDock, SLOT(setVisible(bool)));
	connect(editorDock, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));

	a  = new KAction(i18n("Show &Inspector"), this);
	actionCollection()->addAction("show_inspector", a );
	a->setStatusTip(i18n("Show or hide the Inspector"));
	a->setWhatsThis(i18n("Show Inspector: Show or hide the Inspector"));
	a->setCheckable(true);
	a->setChecked(true);
	connect(a, SIGNAL(toggled(bool)), inspectorDock, SLOT(setVisible(bool)));
	connect(inspectorDock, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));

	// Tools menu actions
	a  = new KAction(i18n("&Direction chooser"), this);
	actionCollection()->addAction("direction", a);
	a->setStatusTip(i18n("Shows the direction chooser dialog"));
	a->setWhatsThis(i18n("Direction Chooser: Show the direction chooser dialog"));
	connect(a, SIGNAL(triggered()), this, SLOT(directionDialog()));

	a  = new KAction(i18n("Show &Line Numbers"), this);
	actionCollection()->addAction("line_numbers", a );
	a->setStatusTip(i18n("Turn the line numbers on/off in the editor"));
	a->setWhatsThis(i18n("Show Line Numbers: Turn the line numbers on/off in the editor"));
	a->setCheckable(true);
	a->setChecked(true);
	connect(a, SIGNAL(toggled(bool)), editor, SLOT(toggleLineNumbers(bool)));

//TODO: Bring back the colorpicker?
    // 	colorpicker  = new KToggleAction(i18n("&Color Picker"), "colorize"), this);
// 	new KAction(i18n("&Indent"), "format-indent-more", CTRL+Key_I, this, SLOT(slotIndent()), ac, "edit_indent");
// 	new KAction(i18n("&Unindent"), "format-indent-less", CTRL+SHIFT+Key_I, this, SLOT(slotUnIndent()), ac, "edit_unindent");
// 	new KAction(i18n("Cl&ean Indentation"), 0, 0, this, SLOT(slotCleanIndent()), ac, "edit_cleanIndent");
// 	new KAction(i18n("Co&mment"), 0, CTRL+Key_D, this, SLOT(slotComment()), ac, "edit_comment");
// 	new KAction(i18n("Unc&omment"), 0, CTRL+SHIFT+Key_D, this, SLOT(slotUnComment()), ac, "edit_uncomment");


	// Help menu actions
        //TODO: implement context help
	contextHelpAct = ac->addAction("context_help");
	contextHelpAct->setText("");
	contextHelpAct->setIcon(KIcon("help-contents"));
	contextHelpAct->setShortcut(QKeySequence(Qt::Key_F2));
	contextHelpAct->setStatusTip(i18n("Get help on the command under the cursor"));
	contextHelpAct->setWhatsThis(i18n("Context Help: Get help on the command under the cursor"));
	connect(contextHelpAct, SIGNAL(triggered()), this, SLOT(contextHelp()));
	setContextHelp();

	a = actionCollection()->addAction(KStandardAction::HelpContents,  "help_contents", this, SLOT(appHelpActivated()));
	a->setStatusTip(i18n("Help"));
	a->setWhatsThis(i18n("Help: Open manual for KTurtle"));

	/*a = actionCollection()->addAction(KStandardAction::WhatsThis,  "whatsthis", this, SLOT(whatsThis()));
	a->setStatusTip(i18n("Point and click information about the interface of KTurtle"));
	a->setWhatsThis(i18n("Whats This?: Point and click information about the interface of KTurtle"));*/

	// The run speed action group
	QActionGroup* runSpeedGroup = new QActionGroup(this);

	fullSpeedAct  = new KAction(i18nc("@option:radio", "&Full Speed"), this);
	actionCollection()->addAction("full_speed", fullSpeedAct );
	fullSpeedAct->setCheckable(true);
	fullSpeedAct->setChecked(true);
	fullSpeedAct->setStatusTip(i18n("Run the program at full speed"));
	fullSpeedAct->setWhatsThis(i18n("Full Speed: Run the program at full speed"));
	connect(fullSpeedAct, SIGNAL(triggered()), this, SLOT(setFullSpeed()));
	runSpeedGroup->addAction(fullSpeedAct);

	slowSpeedAct  = new KAction(i18nc("@option:radio choose the slow speed", "&Slow"), this);
	actionCollection()->addAction("slow_speed", slowSpeedAct );
	slowSpeedAct->setCheckable(true);
	slowSpeedAct->setStatusTip(i18n("Run the program at slow speed"));
	slowSpeedAct->setWhatsThis(i18n("Slow Speed: Run the program at slow speed"));
	connect(slowSpeedAct, SIGNAL(triggered()), this, SLOT(setSlowSpeed()));
	runSpeedGroup->addAction(slowSpeedAct);

	slowerSpeedAct  = new KAction(i18nc("@option:radio", "S&lower"), this);
	actionCollection()->addAction("slower_speed", slowerSpeedAct );
	slowerSpeedAct->setCheckable(true);
	slowerSpeedAct->setStatusTip(i18n("Run the program at slower speed"));
	slowerSpeedAct->setWhatsThis(i18n("Slower Speed: Run the program at slower speed"));
	connect(slowerSpeedAct, SIGNAL(triggered()), this, SLOT(setSlowerSpeed()));
	runSpeedGroup->addAction(slowerSpeedAct);

	slowestSpeedAct  = new KAction(i18nc("@option:radio", "Sl&owest"), this);
	actionCollection()->addAction("slowest_speed", slowestSpeedAct );
	slowestSpeedAct->setCheckable(true);
	slowestSpeedAct->setStatusTip(i18n("Run the program at slowest speed"));
	slowestSpeedAct->setWhatsThis(i18n("Slowest Speed: Run the program at slowest speed"));
	connect(slowestSpeedAct, SIGNAL(triggered()), this, SLOT(setSlowestSpeed()));
	runSpeedGroup->addAction(slowestSpeedAct);

	stepSpeedAct  = new KAction(i18nc("@option:radio", "S&tep-by-Step"), this);
	actionCollection()->addAction("step_speed", stepSpeedAct );
	stepSpeedAct->setCheckable(true);
	stepSpeedAct->setStatusTip(i18n("Run the program one step at a time"));
	stepSpeedAct->setWhatsThis(i18n("Step Speed: Run the program one step at a time"));
	connect(stepSpeedAct, SIGNAL(triggered()), this, SLOT(setStepSpeed()));
	runSpeedGroup->addAction(stepSpeedAct);
}


void MainWindow::setupCanvas()
{
	// put the canvas in a layout as the cetral widget of the mainwindow
	QWidget* centralWidget = new QWidget(this);
	QHBoxLayout* hboxLayout = new QHBoxLayout(centralWidget);
	hboxLayout->setMargin(MARGIN_SIZE);
	canvas = new Canvas();
	canvas->setFocusPolicy(Qt::NoFocus);
	canvas->setRenderHint(QPainter::Antialiasing);
	hboxLayout->addWidget(canvas);
	setCentralWidget(centralWidget);
	canvas->setWhatsThis(i18n("Canvas: This is where the turtle moves and draws when the program is running"));
}

void MainWindow::setupDockWindows()
{
	editorDock = new LocalDockWidget(i18n("&Code Editor"), this);
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
	editor->setWhatsThis(i18n("Code Editor: Type your KTurtle programs here"));

	//Creating the debug window
	inspectorDock = new LocalDockWidget(i18n("&Inspector"), this);
	inspectorDock->setObjectName("inspector");
	QWidget* inspectorWrapWidget = new QWidget(inspectorDock);
	QHBoxLayout* inspectorDockLayout = new QHBoxLayout(inspectorWrapWidget);
	inspectorDockLayout->setMargin(MARGIN_SIZE);
	inspectorWrapWidget->setLayout(inspectorDockLayout);
	inspector = new Inspector(inspectorWrapWidget);
	inspectorDockLayout->addWidget(inspector);
	inspectorDock->setWidget(inspectorWrapWidget);
	addDockWidget(Qt::LeftDockWidgetArea, inspectorDock);
	inspector->setWhatsThis(i18n("Inspector: See information about variables and functions when the program runs"));
}

void MainWindow::setupEditor()
{
// 	editor->setTranslator(Translator::instance());
	connect(editor, SIGNAL(modificationChanged(bool)), this, SLOT(setWindowModified(bool)));
	connect(editor, SIGNAL(currentUrlChanged(const KUrl&)), this, SLOT(setCaption(const KUrl&)));
	connect(editor, SIGNAL(cursorPositionChanged(int, int, const QString&)),
		this, SLOT(updateOnCursorPositionChange(int, int, const QString&)));
}

void MainWindow::setupInterpreter()
{
	interpreter = new Interpreter(this, false);
	connect(interpreter, SIGNAL(finished()), this, SLOT(abort()));
	Executer* executer = interpreter->getExecuter();
	connect(executer, SIGNAL(currentlyExecuting(int, int, int, int)),
		editor, SLOT(markCurrentWord(int, int, int, int)));

	// the code to connect the executer with the canvas is auto generated:
#include "interpreter/gui_connect.inc"
	connect(executer, SIGNAL(variableTableUpdated(const QString&, const Value&)),
		inspector, SLOT(updateVariable(const QString&, const Value&)));
	connect(executer, SIGNAL(functionTableUpdated(const QString&, const QStringList&)),
		inspector, SLOT(updateFunction(const QString&, const QStringList&)));
	connect(interpreter, SIGNAL(treeUpdated(TreeNode*)),
		inspector, SLOT(updateTree(TreeNode*)));
}

/*void MainWindow::setupMenus()
{
	QList<QString> items;
	items << i18n("&File")
	      << "file_new" << "file_open" << "file_recent" << "#examples" << "-"
	      << "file_save" << "file_save_as" << "-"
	      << "#speed"
	      << "run" << "pause" << "abort" << "-"
	      << "file_print" << "-"
	      << "file_quit" << "-"
	      << i18n("&Edit")
	      << "edit_undo" << "edit_redo" << "-"
	      << "edit_cut" << "edit_copy" << "edit_paste" << "-"
	      << "edit_find" << "edit_find_next" << "edit_find_prev" << "edit_replace" << "-"
	      << "set_insert" << "-"
	      << i18n("&View")
	      //<< "full_screen" << "-"
	      << "show_editor" << "line_numbers" << "show_inspector" << "show_statusbar" << "show_toolbar" << "-"
	      << i18n("&Tools")
	      << "direction" << "-"
	      //<< "edit_indent" << "edit_unindent" << "-"
	      //<< "edit_comment" << "edit_uncomment" << "-"
	      << i18n("&Settings")
	      << "#language" << "-"
	      //<< "set_font_size" << "set_font_type" << "-"
	      << i18n("&Help")
	      << "help_contents" << "whatsthis" << "-" << "context_help" << "-" << "about_app" 
            //FIX it << "about_kde" << "-"
	      ;

	QMenu* currentMenu = 0;
	foreach (const QString &item, items) {
		if (item.contains('&')) {
			currentMenu = menuBar()->addMenu(item);
		} else {
			if (item == "-") {
				currentMenu->addSeparator();
			} else if (item == "#speed") {
				QMenu* runSpeedMenu = currentMenu->addMenu(i18n("R&un Speed"));
				runSpeedMenu->addAction(fullSpeedAct);
				runSpeedMenu->addAction(slowSpeedAct);
				runSpeedMenu->addAction(slowerSpeedAct);
				runSpeedMenu->addAction(slowestSpeedAct);
				runSpeedMenu->addAction(stepSpeedAct);
			} else if (item == "#language") {
				QMenu* languageMenu = currentMenu->addMenu(i18n("&Language"));
				QActionGroup* languageGroup = new QActionGroup(this);
				connect(languageGroup, SIGNAL(triggered(QAction *)), this, SLOT(setLanguage(QAction *)));
				QAction* a;
				// sort the dictionaries using an algorithm found the the qt docs:
				QMap<QString, QString> map;
				foreach (const QString &lang_code, KGlobal::locale()->languageList())
					map.insert(codeToFullName(lang_code), lang_code);
				// populate the menu:
				foreach (const QString &lang_code, map.values()) {
					a = new QAction(codeToFullName(lang_code), actionCollection());
					a->setData(lang_code);
					a->setStatusTip(i18n("Switch to the %1 dictionary", codeToFullName(lang_code)));
					a->setCheckable(true);
					languageMenu->addAction(a);
					languageGroup->addAction(a);
				}
			} else if (item == "#examples") {
				examplesMenu = currentMenu->addMenu(i18n("&Open Example"));
				connect(examplesMenu, SIGNAL(triggered(QAction *)), this, SLOT(openExample(QAction *)));
			} else {
				currentMenu->addAction(actionCollection()->action(item));
			}
		}
	}
}


void MainWindow::setupToolBar()
{
	QList<QString> items;
	items << "file_new" << "file_open" << "file_save" << "-"
	      << "edit_undo" << "edit_redo" << "-"
	      << "edit_cut" << "edit_copy" << "edit_paste" << "-"
              << "edit_find" << "-"
	      << "run" << "#speed" << "pause" << "abort";

	toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	foreach (const QString &item, items) {
		if (item == "-") {
			toolBar->addSeparator();
		} else if (item == "#speed") {
			runOptionBox = new KComboBox(toolBar);
			runOptionBox->addItem(i18nc("@option:radio", "Full Speed"));
			runOptionBox->addItem(i18nc("@option:radio choose slow speed", "Slow"));
			runOptionBox->addItem(i18nc("@option:radio", "Slower"));
			runOptionBox->addItem(i18nc("@option:radio", "Slowest"));
			runOptionBox->addItem(i18nc("@option:radio", "Step by Step"));
			runOptionBox->setStatusTip(i18n("Choose program run speed"));
			runOptionBox->setWhatsThis(i18n("Run Speed: Choose program run speed"));
			connect(runOptionBox, SIGNAL(activated(int)), this, SLOT(setRunSpeed(int)));
			toolBar->addWidget(runOptionBox);
		} else {
			toolBar->addAction(actionCollection()->action(item));
		}
	}
}*/

void MainWindow::setupStatusBar()
{
	statusBarLanguageLabel = new QLabel(statusBar());
	statusBar()->addPermanentWidget(statusBarLanguageLabel, 0);
	statusBarLanguageLabel->setAlignment(Qt::AlignRight);

	statusBarPositionLabel = new QLabel(statusBar());
	statusBar()->addPermanentWidget(statusBarPositionLabel, 0);
	statusBarPositionLabel->setAlignment(Qt::AlignRight);

	statusBarInsertModeLabel = new QLabel(statusBar());
	statusBar()->addPermanentWidget(statusBarInsertModeLabel, 0);
	statusBarInsertModeLabel->setAlignment(Qt::AlignRight);

	statusBarFileNameLabel = new QLabel(statusBar());
	statusBar()->addPermanentWidget(statusBarFileNameLabel, 0);
	statusBarFileNameLabel->setAlignment(Qt::AlignRight);

	toggleInsertMode(true);
	updateOnCursorPositionChange(1, 1, "");
}


void MainWindow::updateExamplesMenu()
{
	KAction * newExample;
	QString actionName;
	QList<QAction *> exampleList;
	QActionGroup * exampleGroup = new QActionGroup (this);

	foreach (const QString &exampleName, Translator::instance()->exampleNames()) {
		newExample = new KAction (exampleName, this);
		newExample->setData(exampleName);
		exampleGroup->addAction (newExample);

		connect (newExample, SIGNAL(triggered()), this, SLOT(openExample()));
		exampleList.append (newExample);
	}
	unplugActionList ("examples_actionlist");
	plugActionList   ("examples_actionlist", exampleList);
}

void MainWindow::open(const QString& pathOrUrl)
{
	editor->openFile(KUrl(pathOrUrl));
}

void MainWindow::openExample()
{
	QAction *action = qobject_cast<QAction*>(sender());
	QString exampleName = action->data().toString();
	editor->openExample(Translator::instance()->example(exampleName), exampleName);
}

void MainWindow::toggleInsertMode(bool b)
{
	statusBarInsertModeLabel->setText(b ? i18n(" INS ") : i18n(" OVR "));
	editor->setInsertMode(b);
}

void MainWindow::setContextHelp(const QString& s)
{
	contextHelpString = s.isEmpty() ? i18n("<placeholder>no keyword</placeholder>") : s;
	contextHelpAct->setText(i18n("Help on: %1", contextHelpString));
}

void MainWindow::updateOnCursorPositionChange(int row, int col, const QString& line)
{
	statusBarPositionLabel->setText(i18n(" Line: %1 Column: %2 ", row, col));

	Token* cursorToken = editor->currentToken(line, col);
	QString desc = QString();
	if (cursorToken != 0) {
		QString look = cursorToken->look();
		int cat = cursorToken->category();
		delete cursorToken;
		cursorToken = 0;
		switch (cat) {
			case Token::VariableCategory:          setContextHelp(i18n("<placeholder>variable</placeholder>"));    return;
			case Token::NumberCategory:            setContextHelp(i18n("<placeholder>number</placeholder>"));      return;
			case Token::CommentCategory:           setContextHelp(i18n("<placeholder>comment</placeholder>"));     return;
			case Token::StringCategory:            setContextHelp(i18n("<placeholder>string</placeholder>"));      return;
			case Token::ScopeCategory:             setContextHelp(i18n("<placeholder>scope</placeholder>"));       return;
			case Token::AssignmentCategory:        setContextHelp(i18n("<placeholder>assignment</placeholder>"));  return;
			case Token::ParenthesisCategory:       setContextHelp(i18n("<placeholder>parenthesis</placeholder>")); return;

			case Token::MathOperatorCategory:      desc = i18n("mathematical operator");
			case Token::ExpressionCategory:        desc = i18n("expression");
			case Token::BooleanOperatorCategory:   desc = i18n("boolean operator");
			case Token::TrueFalseCategory:         desc = i18n("boolean");
			case Token::CommandCategory:           desc = i18n("command");
			case Token::ControllerCommandCategory: desc = i18n("command");
			case Token::LearnCommandCategory:      desc = i18n("command");
				setContextHelp(QString("\"%1\" (%2)").arg(look).arg(desc));
				return;

			case Token::MetaCategory:
			case Token::WhiteSpaceCategory:
			case Token::DecimalSeparatorCategory:
			case Token::FunctionCallCategory:
			case Token::ArgumentSeparatorCategory:
				// do nothing with these... yet.
				break;
		}
	}
	setContextHelp(i18n("<placeholder>no keyword</placeholder>"));
}


void MainWindow::setLanguage(QAction *action)
{
	if (setCurrentLanguage(action->data().toString()))
		action->setChecked(true);
}

bool MainWindow::setCurrentLanguage(const QString &lang_code)
{
	bool result = false;
	kDebug(0) << "MainWindow::setCurrentLanguage: " << lang_code;
	if (Translator::instance()->setLanguage(lang_code)) {
		currentLanguageCode = lang_code;
		statusBarLanguageLabel->setText(' ' + codeToFullName(lang_code) + ' ');
		updateExamplesMenu();
		result = true;
	} else {
		KMessageBox::error(this, i18n("Could not change the language to %1.", codeToFullName(lang_code)));
	}
	return result;
}

QString MainWindow::codeToFullName(const QString& lang_code)
{
	return QString(lang_code.isNull() ?
			i18n("%1 [built in]", codeToFullName("en_US")) :
			i18n("%1 (%2)", KGlobal::locale()->languageCodeToName(lang_code.left(2)), lang_code)
		);
}


void MainWindow::run()
{
	if (interpreter->state() == Interpreter::Uninitialized ||
	    interpreter->state() == Interpreter::Finished ||
	    interpreter->state() == Interpreter::Aborted) {
		//Reset inspector and interpreter
		inspector->clear();
		interpreter->initialize(editor->content());
		editor->removeMarkings();
	}
	// start parsing (always in full speed)
	iterationTimer->setSingleShot(false);
	iterationTimer->start(0);
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
			case 1: iterationTimer->start(500);  break;
			case 2: iterationTimer->start(1000); break;
			case 3: iterationTimer->start(3000); break;
			case 4:
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

	runAct->setEnabled(true);
	pauseAct->setChecked(false);
	pauseAct->setEnabled(false);
	abortAct->setEnabled(false);

	if (interpreter->encounteredErrors()) {
		ErrorDialog* errorDialog = new ErrorDialog(interpreter->getErrorList(), this);
		connect(errorDialog, SIGNAL(currentlySelectedError(int, int, int, int)),
			editor, SLOT(markCurrentError(int, int, int, int)));
	}
}


void MainWindow::setCaption(const KUrl &url, bool modified)
{
	QString filename = url.isEmpty() ? i18n("untitled") : url.fileName();
	KXmlGuiWindow::setCaption(filename, modified);
	statusBarFileNameLabel->setText(QString(" %1%2 ").arg(filename).arg(modified ? "*" : ""));
}

void MainWindow::addToRecentFiles(const KUrl &url)
{
	recentFilesAction->addUrl(url);
}


void MainWindow::readConfig()
{
	KConfigGroup config(KGlobal::config(), "General Options");

//   m_paShowStatusBar->setChecked(config->readEntry("ShowStatusBar", QVariant(false)).toBool());
//   m_paShowPath->setChecked(config->readEntry("ShowPath", QVariant(false)).toBool());
    recentFilesAction->loadEntries(KGlobal::config()->group( "Recent Files") );

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

// slots for logo functions that need to use the MainWindow class:

void MainWindow::slotInputDialog(QString& value)
{
	iterationTimer->stop();
	value = KInputDialog::getText(i18n("Input"), value);
	run();
}

void MainWindow::slotMessageDialog(const QString& text)
{
	iterationTimer->stop();
	KMessageBox::information( this, text, i18n("Message") );
	run();
}

// END

#include "mainwindow.moc"
