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


#include <QtGui>

#include <kdebug.h>

#include <kaboutapplication.h>
#include <kaboutkde.h>
#include <kactioncollection.h>
#include <kaction.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kicon.h>
#include <kmessagebox.h>
#include <krecentfilesaction.h>
#include <kstatusbar.h>
#include <kstdaccel.h>
#include <kstdaction.h>

#include "errordialog.h"
#include "interpreter/errormsg.h"
#include "interpreter/translator.h"

#include "mainwindow.h"



static const int MARGIN_SIZE = 3;  // defaultly styled margins look shitty

MainWindow::MainWindow()
{
	toolBar = new LocalToolBar(i18n("Toolbar"), this);  // forward definition
	toolBar->setObjectName("toolbar");
	addToolBar(toolBar);

	setupDockWindows();  // the setup order matters
	setupActions();
	setupCanvas();
	setupInterpreter();
	setupEditor();
	setupMenus();
	setupStatusBar();
	setupToolBar();

	iterationTimer = new QTimer(this);
	connect(iterationTimer, SIGNAL(timeout()), this, SLOT(iterate()));

	statusBar()->showMessage(i18n("Ready"));
	setCaption();  // also sets the window caption to 'untitled'
	setRunSpeed(0);
	abort();  // sets the run-states for the actions right

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



void MainWindow::about()
{
	QMessageBox::about(this, i18n("About KTurtle"),
		i18n("KTurtle is an educational programming environment, that aims to make programming as easy and touchable as possible. KTurtle intends to help teaching kids the basics of math, geometry and... programming."));
// 	new KAboutApplication();
}

void MainWindow::aboutKDE()
{
// 	new KAboutKDE(this);
}


void MainWindow::documentWasModified()
{
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
	runOptionBox->setCurrentIndex(speed);
	runSpeed = speed;
}

void MainWindow::setupActions()
{
	KAction* a;
	KActionCollection* ac = actionCollection();

		//TODO WHATISTHIS to each action (same as statustip?)

  // File menu actions
  a = KStdAction::openNew(editor, SLOT(newFile()), ac, "file_new");
	a->setStatusTip(i18n("Create a new file"));

	a = KStdAction::open(editor, SLOT(openFile()), ac, "file_open");
	a->setStatusTip(i18n("Open an existing file"));

	recentFilesAction = KStdAction::openRecent(editor, SLOT(openFile(const KUrl&)), ac, "file_recent");
	recentFilesAction->setStatusTip(i18n("Open a recently used file"));

	a = KStdAction::save(editor, SLOT(saveFile()), ac, "file_save");
	a->setStatusTip(i18n("Save the current file to disk"));
	connect(editor, SIGNAL(modificationChanged(bool)), a, SLOT(setEnabled(bool)));

	a = KStdAction::saveAs(editor, SLOT(saveFileAs()), ac, "file_save_as");
	a->setStatusTip(i18n("Save the current file under a different name"));

	runAct = new KAction(KIcon("player_play"), i18n("&Run"), ac, "run");
	runAct->setShortcut(QKeySequence(Qt::Key_F5));
	runAct->setStatusTip(i18n("Execute the code in the Code Editor"));
	connect(runAct, SIGNAL(triggered()), this, SLOT(run()));

	pauseAct = new KAction(KIcon("player_pause"), i18n("&Pause"), ac, "pause");
	pauseAct->setCheckable(true);
	pauseAct->setShortcut(QKeySequence(Qt::Key_F6));
	pauseAct->setStatusTip(i18n("Pause execution"));
	connect(pauseAct, SIGNAL(triggered()), this, SLOT(pause()));

	abortAct = new KAction(KIcon("stop"), i18n("&Abort"), ac, "abort");
	abortAct->setShortcut(QKeySequence(Qt::Key_F7));
	abortAct->setStatusTip(i18n("Abort execution"));
	connect(abortAct, SIGNAL(triggered()), this, SLOT(abort()));

	a = KStdAction::print(this, SLOT(printDlg()), ac, "file_print");
	a->setStatusTip(i18n("Print the code"));

	a = KStdAction::quit(this, SLOT(close()), ac, "file_quit");
	a->setStatusTip(i18n("Quit KTurtle"));


	// Edit menu actions
	a = KStdAction::undo(editor->view(), SLOT(undo()), ac);
	a->setStatusTip(i18n("Undo a change in the Code Editor"));
	a->setEnabled(false);
	connect(editor->view(), SIGNAL(undoAvailable(bool)), a, SLOT(setEnabled(bool)));

	a = KStdAction::redo(editor->view(), SLOT(redo()), ac);
	a->setStatusTip(i18n("Redo a previously undone change in the Code Editor"));
	a->setEnabled(false);
	connect(editor->view(), SIGNAL(redoAvailable(bool)), a, SLOT(setEnabled(bool)));

	a = KStdAction::cut(editor->view(), SLOT(cut()), ac);
	a->setStatusTip(i18n("Cut the selected text to the clipboard"));
	a->setEnabled(false);
	connect(editor->view(), SIGNAL(copyAvailable(bool)), a, SLOT(setEnabled(bool)));

	a = KStdAction::copy(editor->view(), SLOT(copy()), ac);
	a->setStatusTip(i18n("Copy the selected text to the clipboard"));
	a->setEnabled(false);
	connect(editor->view(), SIGNAL(copyAvailable(bool)), a, SLOT(setEnabled(bool)));

	a = KStdAction::paste(editor->view(), SLOT(paste()), ac);
	a->setStatusTip(i18n("Paste the clipboard's content into the current selection"));

	a = KStdAction::selectAll(editor->view(), SLOT(selectAll()), ac);
	a->setStatusTip(i18n("Select all the code in the editor"));
	a->setEnabled(true);

	a = new KAction(i18n("Toggle &Insert"), ac, "set_insert");
	a->setStatusTip(i18n("Toggle between the 'insert' and 'overwrite' mode"));
	a->setShortcut(QKeySequence(Qt::Key_Insert));
	a->setCheckable(true);
	a->setChecked(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(toggleInsertMode(bool)));

	a = KStdAction::find(editor, SLOT(find()), ac);
	a->setStatusTip(i18n("Search through the code in the editor"));

	a = KStdAction::findNext(editor, SLOT(findNext()), ac);
	a->setStatusTip(i18n("Find the next occurrence"));

	a = KStdAction::findPrev(editor, SLOT(findPrev()), ac);
	a->setStatusTip(i18n("Find the previous occurrence"));

	a = KStdAction::replace(editor, SLOT(replace()), ac);
	a->setStatusTip(i18n("Search and replace"));


	// View menu actions
	a = new KAction(i18n("Show &Code Editor"), ac, "show_editor");
	a->setStatusTip(i18n("Show or hide the Code Editor"));
	a->setCheckable(true);
	a->setChecked(true);
	connect(a, SIGNAL(toggled(bool)), editorDock, SLOT(setVisible(bool)));
	connect(editorDock, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));

	a = new KAction(i18n("Show &Statusbar"), ac, "show_statusbar");
	a->setStatusTip(i18n("Show or hide the Statusbar"));
	a->setCheckable(true);
	a->setChecked(true);
	connect(a, SIGNAL(toggled(bool)), statusBar(), SLOT(setVisible(bool)));

	a = new KAction(i18n("Show &Toolbar"), ac, "show_toolbar");
	a->setStatusTip(i18n("Show or hide the Toolbar"));
	a->setCheckable(true);
	a->setChecked(true);
	connect(a, SIGNAL(toggled(bool)), toolBar, SLOT(setVisible(bool)));
	connect(toolBar, SIGNAL(visibilityChanged(bool)), a, SLOT(setChecked(bool)));


	// Tools menu actions
	a = new KAction(i18n("Show &Line Numbers"), ac, "line_numbers");
	a->setStatusTip(i18n("Turn the editors line numbers on/off"));
	a->setCheckable(true);
	a->setChecked(true);
	connect(a, SIGNAL(toggled(bool)), editor, SLOT(toggleLineNumbers(bool)));

// 	colorpicker = new KToggleAction(i18n("&Color Picker"), "colorize", ALT+Key_C, this, SLOT(slotColorPicker()), ac, "color_picker");
// 	new KAction(i18n("&Indent"), "indent", CTRL+Key_I, this, SLOT(slotIndent()), ac, "edit_indent");
// 	new KAction(i18n("&Unindent"), "unindent", CTRL+SHIFT+Key_I, this, SLOT(slotUnIndent()), ac, "edit_unindent");
// 	new KAction(i18n("Cl&ean Indentation"), 0, 0, this, SLOT(slotCleanIndent()), ac, "edit_cleanIndent");
// 	new KAction(i18n("Co&mment"), 0, CTRL+Key_D, this, SLOT(slotComment()), ac, "edit_comment");
// 	new KAction(i18n("Unc&omment"), 0, CTRL+SHIFT+Key_D, this, SLOT(slotUnComment()), ac, "edit_uncomment");


	// Settings menu actions
	a = KStdAction::keyBindings(this, SLOT(editKeys()), ac);
	a->setStatusTip(i18n("Configure the application's keyboard shortcut assignments."));


	// Help menu actions
	contextHelpAct = new KAction(KIcon("help"), "", ac, "context_help");
	contextHelpAct->setShortcut(QKeySequence(Qt::Key_F2));
	contextHelpAct->setStatusTip(i18n("Get help on the command under the cursor"));
	connect(contextHelpAct, SIGNAL(triggered()), this, SLOT(contextHelp()));
	setContextHelp();

	a = KStdAction::aboutApp(this, SLOT(about()), ac, "about_app");
	a->setStatusTip(i18n("Information about KTurtle"));

	a = KStdAction::aboutKDE(this, SLOT(aboutKDE()), ac, "about_kde");
	a->setStatusTip(i18n("Information about KDE"));

	a = KStdAction::help(this, SLOT(about()), ac, "help");
	a->setStatusTip(i18n("Help"));

	a = KStdAction::helpContents(this, SLOT(appHelpActivated()), ac, "help_contents");
	a->setStatusTip(i18n("Help"));

	a = KStdAction::whatsThis(this, SLOT(about()), ac, "whatsthis");
	a->setStatusTip(i18n("Point and click information about the interface of KTurtle"));


	// The run speed action group
	QActionGroup* runSpeedGroup = new QActionGroup(this);

	fullSpeedAct = new KAction(i18n("&Full Speed"), ac, "full_speed");
	fullSpeedAct->setCheckable(true);
	fullSpeedAct->setChecked(true);
	connect(fullSpeedAct, SIGNAL(triggered()), this, SLOT(setFullSpeed()));
	runSpeedGroup->addAction(fullSpeedAct);

	slowSpeedAct = new KAction(i18n("&Slow"), ac, "slow_speed");
	slowSpeedAct->setCheckable(true);
	connect(slowSpeedAct, SIGNAL(triggered()), this, SLOT(setSlowSpeed()));
	runSpeedGroup->addAction(slowSpeedAct);

	slowerSpeedAct = new KAction(i18n("S&lower"), ac, "slower_speed");
	slowerSpeedAct->setCheckable(true);
	connect(slowerSpeedAct, SIGNAL(triggered()), this, SLOT(setSlowerSpeed()));
	runSpeedGroup->addAction(slowerSpeedAct);

	slowestSpeedAct = new KAction(i18n("Sl&owest"), ac, "slowest_speed");
	slowestSpeedAct->setCheckable(true);
	connect(slowestSpeedAct, SIGNAL(triggered()), this, SLOT(setSlowestSpeed()));
	runSpeedGroup->addAction(slowestSpeedAct);

	stepSpeedAct = new KAction(i18n("S&tep-by-Step"), ac, "step_speed");
	stepSpeedAct->setCheckable(true);
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
}

void MainWindow::setupDockWindows()
{
	editorDock = new LocalDockWidget(i18n("Code Editor"), this);
	editorDock->setObjectName("editor");
	QWidget* wrapWidget = new QWidget(editorDock);
	QHBoxLayout* hboxLayout = new QHBoxLayout(wrapWidget);
 	hboxLayout->setMargin(MARGIN_SIZE);
 	wrapWidget->setLayout(hboxLayout);
// 	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	editor = new Editor(wrapWidget);  // create this here to prevent crashes
	hboxLayout->addWidget(editor);
	editorDock->setWidget(wrapWidget);
// 	editorDock->setFeatures(QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable);
	addDockWidget(Qt::LeftDockWidgetArea, editorDock);
	editor->show();
	editor->setFocus();
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
	interpreter = new Interpreter(this);
	connect(interpreter, SIGNAL(finished()), this, SLOT(abort()));
	Executer* executer = interpreter->getExecuter();
	connect(executer, SIGNAL(currentlyExecuting(int, int, int, int)),
		editor, SLOT(markCurrentWord(int, int, int, int)));

	// the code to connect the executer with the canvas is auto generated:
#include "interpreter/gui_connect.inc"
}

void MainWindow::setupMenus()
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
	      << "set_insert" << "-"
// 	      << "edit_find" << "edit_find_next" << "edit_find_prev" << "edit_replace" << "-"
	      << i18n("&View")
	      << "full_screen" << "-"
	      << "show_editor" << "show_statusbar" << "show_toolbar" << "-"
	      << i18n("&Tools")
	      << "line_numbers" << "-"
	      << "edit_indent" << "edit_unindent" << "-"
	      << "edit_comment" << "edit_uncomment" << "-"
	      << i18n("&Settings")
	      << "#language" << "-"
	      << "set_font_size" << "set_font_type" << "-"
	      << i18n("&Help")
	      << "help_contents" << "whatsthis" << "-" << "context_help" << "-" << "about_app" << "about_kde" << "-"
	      ;

	QMenu* currentMenu = 0;
	foreach (QString item, items) {
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
				ValueAction* a;
				// sort the dictionaries using an algorithm found the the qt docs:
				QMap<QString, QString> map;
				foreach (QString lang_code, KGlobal::locale()->languageList())
					map.insert(codeToFullName(lang_code), lang_code);
				// populate the menu:
				foreach (QString lang_code, map.values()) {
					a = new ValueAction(codeToFullName(lang_code), lang_code);
					a->setStatusTip(i18n("Switch to the %1 dictionary", codeToFullName(lang_code)));
					a->setCheckable(true);
					languageMenu->addAction(a);
					languageGroup->addAction(a);
					languageActions.insert(lang_code, a);
					connect(a, SIGNAL(triggered(const QString&, bool)), this, SLOT(setLanguage(const QString&)));
				}
			} else if (item == "#examples") {
				examplesMenu = currentMenu->addMenu(i18n("&Open Example"));
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
	      << "run" << "#speed" << "pause" << "abort";

	toolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	foreach (QString item, items) {
		if (item == "-") {
			toolBar->addSeparator();
		} else if (item == "#speed") {
			runOptionBox = new QComboBox(toolBar);
			runOptionBox->addItem(i18n("Full Speed"));
			runOptionBox->addItem(i18n("Slow"));
			runOptionBox->addItem(i18n("Slower"));
			runOptionBox->addItem(i18n("Slowest"));
			runOptionBox->addItem(i18n("Step by Step"));
			connect(runOptionBox, SIGNAL(activated(int)), this, SLOT(setRunSpeed(int)));
			toolBar->addWidget(runOptionBox);
		} else {
			toolBar->addAction(actionCollection()->action(item));
		}
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
	foreach (QAction* action, examplesMenu->actions()) examplesMenu->removeAction(action);

	ValueAction* a;
	foreach (QString exampleName, Translator::instance()->exampleNames()) {
		a = new ValueAction(exampleName, exampleName);
		examplesMenu->addAction(a);
		connect (a, SIGNAL(triggered(const QString&, bool)), this, SLOT(openExample(const QString&)));
	}

	if (examplesMenu->actions().isEmpty()) {
		QAction* a = new QAction(i18n("(empty)"), this);
		a->setEnabled(false);
		examplesMenu->addAction(a);
	}
}

void MainWindow::open(const QString& pathOrUrl)
{
	editor->openFile(KUrl(pathOrUrl));
}

void MainWindow::openExample(const QString& exampleName)
{
	editor->openExample(Translator::instance()->example(exampleName), exampleName);
}

void MainWindow::toggleInsertMode(bool b)
{
	statusBarInsertModeLabel->setText(b ? i18n(" INS ") : i18n(" OVR "));
	editor->setInsertMode(b);
}

void MainWindow::setContextHelp(const QString& s)
{
	contextHelpString = s.isEmpty() ? i18n("<no keyword>") : s;
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
			case Token::VariableCategory:          setContextHelp(i18n("<variable>"));    return;
			case Token::NumberCategory:            setContextHelp(i18n("<number>"));      return;
			case Token::CommentCategory:           setContextHelp(i18n("<comment>"));     return;
			case Token::StringCategory:            setContextHelp(i18n("<string>"));      return;
			case Token::ScopeCategory:             setContextHelp(i18n("<scope>"));       return;
			case Token::AssignmentCategory:        setContextHelp(i18n("<assignment>"));  return;
			case Token::ParenthesisCategory:       setContextHelp(i18n("<parenthesis>")); return;
		
			case Token::MathOperatorCategory:      desc = i18n("mathmatical operator");
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
	setContextHelp(i18n("<no keyword>"));
}


bool MainWindow::setLanguage(const QString& lang_code)
{
	bool result = false;
	kDebug(0) << "MainWindow::setLanguage: " << lang_code << endl;
	if (Translator::instance()->setLanguage(lang_code)) {
		currentLanguageCode = lang_code;
		statusBarLanguageLabel->setText(' ' + codeToFullName(lang_code) + ' ');
		updateExamplesMenu();
		result = true;
	} else {
		KMessageBox::error(this, i18n("Could not change the language to %1.", codeToFullName(lang_code)));
	}
	languageActions[currentLanguageCode]->setChecked(true);
	return result;
}

QString MainWindow::codeToFullName(const QString& lang_code)
{
	return QString(lang_code.isNull() ?
			i18n("%1 [built in]", codeToFullName("en_US")) :
			i18n("%1 (%2)", KGlobal::locale()->twoAlphaToLanguageName(lang_code.left(2)), lang_code)
		);
}


void MainWindow::run()
{
	if (interpreter->state() == Interpreter::Uninitialized ||
	    interpreter->state() == Interpreter::Finished ||
	    interpreter->state() == Interpreter::Aborted) {
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
	KMainWindow::setCaption(filename, modified);
	statusBarFileNameLabel->setText(QString(" %1%2 ").arg(filename).arg(modified ? "*" : ""));
}

void MainWindow::addToRecentFiles(const KUrl &url)
{
	recentFilesAction->addUrl(url);
}


void MainWindow::readConfig()
{
  KConfig *config = KGlobal::config();
	config->setGroup("General Options");

//   m_paShowStatusBar->setChecked(config->readEntry("ShowStatusBar", QVariant(false)).toBool());
//   m_paShowPath->setChecked(config->readEntry("ShowPath", QVariant(false)).toBool());
	recentFilesAction->loadEntries(config, "Recent Files");

	QString lang_code(config->readEntry("currentLanguageCode", QVariant(QString())).toString());
	if (lang_code.isEmpty()) lang_code = "en_US";  // null-string are saved as empty-strings
	setLanguage(lang_code);
	
// 	if(m_paShowStatusBar->isChecked())
// 		statusBar()->show();
// 	else
// 		statusBar()->hide();
}

void MainWindow::writeConfig()
{
  KConfig *config = KGlobal::config();
	config->setGroup("General Options");
	
// 	config->writeEntry("ShowStatusBar",m_paShowStatusBar->isChecked());
// 	config->writeEntry("ShowPath",m_paShowPath->isChecked());
	recentFilesAction->saveEntries(config, "Recent Files");
	config->writeEntry("currentLanguageCode", currentLanguageCode);

	config->sync();
}



#include "mainwindow.moc"
