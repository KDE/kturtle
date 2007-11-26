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

#include <QActionGroup>
#include <QTextStream>
#include <QToolBar>
#include <QDockWidget>
#include <QLabel>

#include <kaction.h>
#include <kxmlguiwindow.h>
#include <kmenubar.h>
#include <krecentfilesaction.h>
#include <kstatusbar.h>

#include "interpreter/interpreter.h"
#include "canvas.h"
#include "editor.h"
#include "inspector.h"


class KAction;
class KComboBox;
class QMenu;


//BEGIN small sub-classes to make life easy

// extends the QToolBar with an extra signal
class LocalToolBar : public QToolBar { Q_OBJECT
	public:
		LocalToolBar(const QString& title, QWidget* parent) { setWindowTitle(title); setParent(parent); }
		void setVisible(bool b) { QToolBar::setVisible(b); emit visibilityChanged(b); }
		void show() { QToolBar::show(); emit visibilityChanged(true); }
		void hide() { QToolBar::hide(); emit visibilityChanged(false); }
	signals:
		void visibilityChanged(bool);
};

// extends the QDockWidget with an extra signal
class LocalDockWidget : public QDockWidget { Q_OBJECT
	public:
		LocalDockWidget(const QString& title, QWidget* parent) { setWindowTitle(title); setParent(parent); }
		void setVisible(bool b) { QDockWidget::setVisible(b); emit visibilityChanged(b); }
		void show() { QDockWidget::show(); emit visibilityChanged(true); }
		void hide() { QDockWidget::hide(); emit visibilityChanged(false); }
	signals:
		void visibilityChanged(bool);
};

//END small sub-classes


class MainWindow : public KXmlGuiWindow
{
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();

	public slots:
		void open(const QString&);

	private slots:
		void openExample();
		void printDlg();

		void run();
		void pause();
		void abort();
		void iterate();

		void contextHelp();
		//void whatsThis();
		void documentWasModified();
		void setRunSpeed(int);

		void directionDialog();

		void setFullSpeed()    { setRunSpeed(0); }
		void setSlowSpeed()    { setRunSpeed(1); }
		void setSlowerSpeed()  { setRunSpeed(2); }
		void setSlowestSpeed() { setRunSpeed(3); }
		void setStepSpeed()    { setRunSpeed(4); }

		void slotInputDialog(QString& value);
		void slotMessageDialog(const QString& text);
		
	protected slots:
		void saveNewToolbarConfig();

	protected:
		void closeEvent(QCloseEvent *event);


	private:
		void setupActions();
		void setupCanvas();
		void setupDockWindows();
		void setupEditor();
		void setupInterpreter();
		//void setupMenus();
		//void setupToolBar();
		void setupStatusBar();

		void readConfig();
		void writeConfig();

		void setContextHelp(const QString& s = QString());
		QString codeToFullName(const QString& lang_code = QString());

		void updateExamplesMenu();
		void updateLanguagesMenu();
		bool setCurrentLanguage(const QString &);

	private slots:
		void setCaption(const KUrl& url = KUrl(), bool b = false);
		void setWindowModified(bool b) { setCaption(editor->currentUrl(), b); }
		void setLanguage(QAction *);
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
		QTimer      *iterationTimer;
		int          runSpeed;

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

		QAction *contextHelpAct;

		KAction *fullSpeedAct;
		KAction *slowSpeedAct;
		KAction *slowerSpeedAct;
		KAction *slowestSpeedAct;
		KAction *stepSpeedAct;

		QActionGroup *runSpeedGroup;
		KComboBox *runOptionBox;

		QLabel *statusBarMessageLabel;
		QLabel *statusBarLanguageLabel;
		QLabel *statusBarPositionLabel;
		QLabel *statusBarInsertModeLabel;
		QLabel *statusBarFileNameLabel;
};

#endif
