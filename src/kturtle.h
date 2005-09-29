/*
    Copyright (C) 2003-04 Cies Breijs <cies # kde ! nl>

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


#ifndef _KTURTLE_H_
#define _KTURTLE_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <qgroupbox.h>

#include <kcombobox.h>
#include <klineedit.h>
#include <knuminput.h>

#include <kparts/mainwindow.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/view.h>

#include "canvas.h"
#include "dialogs.h"
#include "executer.h"
#include "parser.h"


class KRecentFilesAction;


class MainWindow : public KParts::MainWindow
{   
	Q_OBJECT

	public:
		MainWindow(KTextEditor::Document* = 0L);
		virtual ~MainWindow();


	signals:
		void changeSpeed(int speed);
		void unpauseExecution();


	protected slots:
		void slotNewFile();
		void slotOpenFile(const KURL& url = NULL);
		void slotOpenExample();
		void slotSaveFile();
		void slotSaveAs();
		void slotSaveCanvas();
		void slotPrint();
		
		void slotExecute();
		void slotPauseExecution();
		void slotAbortExecution();
		void slotChangeSpeed();
		void slotInputDialog(QString& value);
		void slotMessageDialog(QString text);
		
		void slotEditor();
		void slotSetHighlightstyle(QString langCode);  
		void slotUndo();
		void slotRedo();
		void slotCut();
		void slotCopy();
		void slotPaste();
		void slotSelectAll();
		void slotClearSelection();
		void slotFind();
		void slotFindNext();
		void slotFindPrevious();
		void slotReplace();
		void slotToggleInsert();
		void slotInsertText(QString);
		void slotToggleLineNumbers();
		void slotIndent();
		void slotUnIndent();
		void slotCleanIndent();
		void slotComment();
		void slotUnComment();
		void slotSetCursorPos(uint row, uint column);
		void slotSetSelection(uint StartLine, uint StartCol, uint EndLine, uint EndCol);
	
		void slotSettings();
		void slotUpdateSettings();
		void slotSettingsHelp();
		
		void slotContextHelp();
		void slotContextHelpUpdate();
		
		void slotStatusBar(QString text, int place);
		void slotCursorStatusBar();
		
		void slotToggleFullscreen();
		void slotFinishedFullScreenExecution();
		void slotRestartFullScreen();
		void slotBackToFullScreen();
		
		void slotColorPicker();
		void slotUpdateCanvas();


	protected:
		void setupActions();
		void setupEditor();
		void setupCanvas();
		void setupStatusBar();
		
		void loadFile(const KURL& url);
		void writeFile(const KURL& url);
		bool queryClose();
		
		void finishExecution();
		void readConfig(KConfig *config);
		
		void updateFullScreen();
		virtual bool event(QEvent* e);
		
		Canvas              *canvasView;
		QWidget             *baseWidget;
		QGridLayout         *baseLayout;
		QDockWindow         *editorDock;
		Executer            *exe;
		ErrorMessage        *errMsg;
		ColorPicker         *picker;
		RestartOrBack       *restartOrBackDialog;
		
		KAction             *run;
		KAction             *stop;
		KAction             *openExAction;
		KAction             *openFileAction;
		KAction             *newAction;
		KAction             *ContextHelp;
		KToggleAction       *pause;
		KToggleAction       *m_fullscreen;
		KToggleAction       *colorpicker;
		KSelectAction       *speed;
		KRecentFilesAction  *m_recentFiles;

		KTextEditor::View           *view() const { return editor; }
		KTextEditor::EditInterface  *editorInterface;    
		KTextEditor::Document       *doc;
		KTextEditor::View           *editor;
		
		bool                 executing;
		bool                 b_fullscreen;
		bool                 b_editorShown;
		
		QString              helpKeyword;
		Translate           *translate;
		KURL                 CurrentFile;
		
		// configuration related
		QWidget             *general;
		QWidget             *language;
		QGroupBox           *WidthHeightBox;
		QLabel              *WidthLabel;
		QLabel              *HeightLabel;
		KIntNumInput        *kcfg_CanvasWidth;
		KIntNumInput        *kcfg_CanvasHeight;
		KComboBox           *kcfg_LanguageComboBox;
		QLabel              *LanguageLabel;
};


#endif // _KTURTLE_H_
