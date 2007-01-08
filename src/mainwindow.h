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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QActionGroup>
#include <QTextStream>
#include <QToolBar>

#include <kaction.h>
#include <kmainwindow.h>
#include <kmenubar.h>
#include <krecentfilesaction.h>
#include <kstatusbar.h>

#include "interpreter/interpreter.h"
#include "canvas.h"
#include "editor.h"
#include "inspector.h"
#include "valueaction.h"


class KAction;
class QListWidget;
class QMenu;
class QTextEdit;


//BEGIN small sub-classes to make life easy

// extends the QToolBar with an extra signal
class LocalToolBar : public QToolBar { Q_OBJECT
	public:
		LocalToolBar(const QString& title, QWidget* parent = 0) { setWindowTitle(title); setParent(parent); }
		void setVisible(bool b) { QToolBar::setVisible(b); emit visibilityChanged(b); }
		void show() { QToolBar::show(); emit visibilityChanged(true); }
		void hide() { QToolBar::hide(); emit visibilityChanged(false); }
	signals:
		void visibilityChanged(bool);
};

// extends the QDockWidget with an extra signal
class LocalDockWidget : public QDockWidget { Q_OBJECT
	public:
		LocalDockWidget(const QString& title, QWidget* parent = 0) { setWindowTitle(title); setParent(parent); }
		void setVisible(bool b) { QDockWidget::setVisible(b); emit visibilityChanged(b); }
		void show() { QDockWidget::show(); emit visibilityChanged(true); }
		void hide() { QDockWidget::hide(); emit visibilityChanged(false); }
	signals:
		void visibilityChanged(bool);
};

//END small sub-classes


class MainWindow : public KMainWindow
{
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();

	public slots:
		void open(const QString&);

	private slots:
		void openExample(const QString&);

		void run();
		void pause();
		void abort();
		void iterate();

		void about();
		void aboutKDE();
		void documentWasModified();
		void setRunSpeed(int);

		void setFullSpeed()    { setRunSpeed(0); }
		void setSlowSpeed()    { setRunSpeed(1); }
		void setSlowerSpeed()  { setRunSpeed(2); }
		void setSlowestSpeed() { setRunSpeed(3); }
		void setStepSpeed()    { setRunSpeed(4); }


	protected:
		void closeEvent(QCloseEvent *event);


	private:
		void setupActions();
		void setupCanvas();
		void setupDockWindows();
		void setupEditor();
		void setupInterpreter();
		void setupMenus();
		void setupToolBar();
		void setupStatusBar();

		void readConfig();
		void writeConfig();

		void setContextHelp(const QString& s = QString());
		QString codeToFullName(const QString& lang_code = QString());

		void updateExamplesMenu();


	private slots:
		void setCaption(const KUrl& url = KUrl(), bool b = false);
		void setWindowModified(bool b) { setCaption(editor->currentUrl(), b); }
		bool setLanguage(const QString& lang_code = QString());
		void addToRecentFiles(const KUrl&);
		void toggleInsertMode(bool b);
		void updateOnCursorPositionChange(int row, int col, const QString& line);


	private:
		Canvas      *canvas;
		Editor      *editor;
		Interpreter *interpreter;
		Inspector   *inspector;

		LocalDockWidget*  editorDock;
		LocalDockWidget*  inspectorDock;
		LocalToolBar*     toolBar;
		QTimer      *iterationTimer;
		int          runSpeed;

		QHash<QString, ValueAction*> languageActions;  // a <lang_code, dictionary_filename> - map
		QString currentLanguageCode;

		QString contextHelpString;
	
		QMenu *examplesMenu;
		QMenu *runSpeedMenu;

		KRecentFilesAction *recentFilesAction;

		KAction *newAct;
		KAction *openAct;
		KAction *saveAct;
		KAction *saveAsAct;
		KAction *runAct;
		KAction *pauseAct;
		KAction *abortAct;
		KAction *quitAct;

// 		KAction *undoAct;
// 		KAction *redoAct;
// 		KAction *cutAct;
// 		KAction *copyAct;
// 		KAction *pasteAct;
// 		KAction *selectAllAct;
// 		KAction *deselectAllAct;
// 		KAction *toggleInsertAct;
// 		KAction *findAct;
// 		KAction *findNextAct;
// 		KAction *findPreviousAct;
// 		KAction *replaceAct;

// 		KToggleAction* showStatusbarAct;
//
// 		KAction *aboutAct;
// 		KAction *aboutKdeAct;

		QAction *contextHelpAct;

		KAction *fullSpeedAct;
		KAction *slowSpeedAct;
		KAction *slowerSpeedAct;
		KAction *slowestSpeedAct;
		KAction *stepSpeedAct;

		QActionGroup *runSpeedGroup;
		QComboBox *runOptionBox;

		QLabel *statusBarMessageLabel;
		QLabel *statusBarLanguageLabel;
		QLabel *statusBarPositionLabel;
		QLabel *statusBarInsertModeLabel;
		QLabel *statusBarFileNameLabel;
};

#endif
