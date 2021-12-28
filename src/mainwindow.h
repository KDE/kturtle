/*
    SPDX-FileCopyrightText: 2003-2009 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <QDockWidget>

#include <KXmlGuiWindow>

#include "interpreter/interpreter.h"
#include "canvas.h"
#include "colorpicker.h"
#include "directiondialog.h"
#include "console.h"
#include "editor.h"
#include "errordialog.h"
#include "inspector.h"

class QStackedWidget;

class KRecentFilesAction;


// extends the QDockWidget with an extra signal
class LocalDockWidget : public QDockWidget {
    Q_OBJECT
	public:
		LocalDockWidget(const QString& title, QWidget* parent) : QDockWidget(parent) { setWindowTitle(title); }
		void setVisible(bool b) override { QDockWidget::setVisible(b); Q_EMIT visibilityChanged(b); }
		void show() { QDockWidget::show(); Q_EMIT visibilityChanged(true); }
		void hide() { QDockWidget::hide(); Q_EMIT visibilityChanged(false); }
	Q_SIGNALS:
		void visibilityChanged(bool);
};



class MainWindow : public KXmlGuiWindow
{
	Q_OBJECT

	public:
		MainWindow();
        ~MainWindow() override;
		void open(const QString& pathOrUrl) { editor->openFile(QUrl(pathOrUrl)); }  // for main.cpp

// 	public Q_SLOTS:

	private Q_SLOTS:
		void addToRecentFilesList(const QUrl&);
		void showErrorDialog(bool show = false);
		void openExample();
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
		void addToRecentFiles(const QUrl&);
		void toggleOverwriteMode(bool b);
		void updateOnCursorPositionChange();

	protected Q_SLOTS:
		void saveNewToolbarConfig() override;

	protected:
		void closeEvent(QCloseEvent *event) override;

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
		QTabWidget      *canvasTabWidget;
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
		QComboBox *runOptionBox;

		QLabel *statusBarMessageLabel;
		QLabel *statusBarLanguageLabel;
		QLabel *statusBarPositionLabel;
		QLabel *statusBarOverwriteModeLabel;
		QLabel *statusBarFileNameLabel;
};

#endif  // _MAINWINDOW_H_
