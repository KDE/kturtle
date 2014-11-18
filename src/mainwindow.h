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

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QActionGroup>
#include <QStackedWidget>
#include <QTextStream>
#include <QToolBar>
#include <QDockWidget>
#include <QLabel>

#include <QAction>
#include <kxmlguiwindow.h>
#include <kmenubar.h>
#include <krecentfilesaction.h>
#include <kstatusbar.h>

#include "interpreter/interpreter.h"
#include "canvas.h"
#include "colorpicker.h"
#include "directiondialog.h"
#include "console.h"
#include "editor.h"
#include "errordialog.h"
#include "inspector.h"


class QAction;
class KComboBox;
class KTabWidget;
class QMenu;


// extends the QDockWidget with an extra signal
class LocalDockWidget : public QDockWidget { Q_OBJECT
	public:
		LocalDockWidget(const QString& title, QWidget* parent) : QDockWidget(parent) { setWindowTitle(title); }
		void setVisible(bool b) { QDockWidget::setVisible(b); emit visibilityChanged(b); }
		void show() { QDockWidget::show(); emit visibilityChanged(true); }
		void hide() { QDockWidget::hide(); emit visibilityChanged(false); }
	signals:
		void visibilityChanged(bool);
};



class MainWindow : public KXmlGuiWindow
{
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();
		void open(const QString& pathOrUrl) { editor->openFile(KUrl(pathOrUrl)); }  // for main.cpp

// 	public slots:

	private slots:
		void addToRecentFilesList(const KUrl&);
		void showErrorDialog(bool show = false);
		void openExample();
		void getNewExampleDialog();
		void openDownloadedExample();
		void filePrintDialog();
		void canvasPrintDialog();
		void exportToPng();
		void exportToSvg();
		void exportToHtml();

		void contextHelp();
		//void whatsThis();
		void documentWasModified();

		void showDirectionDialog();
		void showColorPicker();

		void setRunSpeed(int);
		void run();
		void pause();
		void abort();
		void iterate();
		QString execute(const QString&);  // for single command execution as by the console
		void setDedicatedSpeed() { setRunSpeed(0); }
		void setFullSpeed()      { setRunSpeed(1); }
		void setSlowSpeed()      { setRunSpeed(2); }
		void setSlowerSpeed()    { setRunSpeed(3); }
		void setSlowestSpeed()   { setRunSpeed(4); }
		void setStepSpeed()      { setRunSpeed(5); }

		void slotInputDialog(QString& value);
		void slotMessageDialog(const QString& text);

		bool setCurrentLanguage(const QString&);
		void setLanguage(QAction*);
		void updateContentName(const QString& str = QString());
		void updateModificationState() { setCaption(editor->currentUrl().fileName()); }
		void addToRecentFiles(const KUrl&);
		void toggleOverwriteMode(bool b);
		void updateOnCursorPositionChange();

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
		void setupStatusBar();

		void readConfig();
		void writeConfig();

		void updateContextHelpAction(const QString& s = QString(), const QString& anchor = QString());
		QString codeToFullName(const QString&);

		void updateExamplesMenu();
		void updateLanguagesMenu();
		void toggleGuiFeedback(bool b);

		Canvas          *canvas;
		Console         *console;
		Editor          *editor;
		Interpreter     *interpreter;
		Inspector       *inspector;
		ErrorDialog     *errorDialog;
		DirectionDialog *directionDialog;
		ColorPicker     *colorPicker;
		KTabWidget      *canvasTabWidget;
		QWidget         *canvasTab;
		QStackedWidget  *stackedWidget;
		LocalDockWidget *editorDock;
		LocalDockWidget *inspectorDock;
		QTimer          *iterationTimer;
		int              runSpeed;
		bool             currentlyRunningConsole;

		QString currentLanguageCode;

		QString contextHelpString;
        QString contextHelpAnchor;
	
		QMenu *examplesMenu;

		KRecentFilesAction *recentFilesAction;

		QAction *newAct;
		QAction *openAct;
		QAction *saveAct;
		QAction *saveAsAct;
		QAction *exportToPngAct;
		QAction *exportToSvgAct;
		QAction *exportToHtmlAct;
		QAction *printCanvasAct;
		QAction *runAct;
		QAction *pauseAct;
		QAction *abortAct;
		QAction *executeConsoleAct;
		QAction *quitAct;
		QAction *contextHelpAct;
		QAction *dedicatedSpeedAct;
		QAction *fullSpeedAct;
		QAction *slowSpeedAct;
		QAction *slowerSpeedAct;
		QAction *slowestSpeedAct;
		QAction *stepSpeedAct;

		QActionGroup *runSpeedGroup;
		KComboBox *runOptionBox;

		QLabel *statusBarMessageLabel;
		QLabel *statusBarLanguageLabel;
		QLabel *statusBarPositionLabel;
		QLabel *statusBarOverwriteModeLabel;
		QLabel *statusBarFileNameLabel;
};

#endif  // _MAINWINDOW_H_
