/*
 * Copyright (C) 2003 Cies Breijs <cies # showroommama ! nl>

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

// BEGIN include and defines 

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
#include "colorpicker.h"
#include "executer.h"
#include "parser.h"

// END


class KRecentFilesAction;

/**
 *This file declares a class for  KTurtle main window. It provides geometry management, menubar, toolbar, statusbar, the editor and *the canvas. The editor is a KTextEditor part and the canvas is derived from its own class
 *
 * @short Main window class
 * @author Cies Breijs <cies # showroommama ! nl>
 * @version 0.1
 */
 
class MainWindow : public KParts::MainWindow
{   

 Q_OBJECT

  public:
  	/**
	Constructor
	@param KTextEditor::Document
	*/
    	MainWindow(KTextEditor::Document* = 0L);
    	virtual ~MainWindow();
  
  private:
	// init'ed in contructor
	Canvas             *TurtleView;
	QWidget            *BaseWidget;
	QGridLayout        *BaseLayout;
	QDockWindow        *EditorDock;
	QString             CurrentFile;
	QString             filename2saveAs;
		
	// setup's (actions, editor, statusbar, canvas)
	void setupActions();
	KAction            *run;
	KAction            *stop;
	KAction            *openExAction;
	KAction            *openFileAction;
	KAction            *newAction;
	KToggleAction      *m_fullscreen;
	KToggleAction      *colorpicker;
	KRecentFilesAction *m_recentFiles;
	KAction            *ContextHelp;
	bool                b_fullscreen:1;
	bool                b_editorShown:1;
	void setupEditor();
	KTextEditor::View           *view() const { return editor; }
	KTextEditor::EditInterface  *ei;    
	KTextEditor::Document       *doc;
	KTextEditor::View           *editor;
	void setupCanvas();
	void setupStatusBar();
	
	// configuration related
	void readConfig(KConfig *config);
	QWidget            *general;
	QWidget            *language;
	QGroupBox          *WidthHeightBox;
	QLabel             *WidthLabel;
	QLabel             *HeightLabel;
	KIntNumInput       *kcfg_CanvasWidth;
	KIntNumInput       *kcfg_CanvasHeight;
	KComboBox          *kcfg_LanguageComboBox;
	QLabel             *LanguageLabel;
	ColorPicker        *picker;
	QString             helpKeyword;
	
	// run related
	void startExecution();
	void finishExecution();
	Executer           *exe;
	bool                allreadyError;
	bool                executing;
	
	// fullscreen related
	void updateFullScreen();
    
  protected slots:
	void slotNewFile();
	void slotOpenFile();
	void loadFile(KURL url);
	void slotOpen(const KURL&);
	void slotOpenEx();
	void slotSaveFile();
	void slotSaveAs();
	void slotSave(KURL &);
	void slotSaveCanvas();
	void slotPrint();
	
	void slotExecute();
	void slotAbortExecution();
	void slotErrorDialog(QString msg, uint row = 0, uint col = 0, uint code = 0);
	void slotInputDialog(QString& value);
	void slotMessageDialog(QString text);
	
	void slotEditor();
	void slotShowEditor();
	void slotSetHighlightstyle(QString langCode);  
	///The Edit -> Undo action from the KTextEditor part
	void slotUndo();
	///The Edit -> Redo action from the KTextEditor part
	void slotRedo();
	///The Edit -> Cut action from the KTextEditor part
	void slotCut();
	///The Edit -> Copy action from the KTextEditor part
	void slotCopy();
	///The Edit -> Paste action from the KTextEditor part
	void slotPaste();
	///The Edit -> Select All action from the KTextEditor part
	void slotSelectAll();
	///The Edit -> Clear Selection action from the KTextEditor part
	void slotClearSelection();
	///The Edit-> Find action from the KTextEditor part
	void slotFind();
	///The Edit-> Find Next action from the KTextEditor part
	void slotFindNext();
	///The Edit-> Find Previous action from the KTextEditor part
	void slotFindPrevious();
	///The Edit -> Replace action from the KTextEditor part
	void slotReplace();
	
	void slotToggleInsert();
	
	void slotInsertText(QString);
	///The View-> Show Line Numbers action from the KTextEditor part
	void slotToggleLineNumbers();
	///The Tools-> Indent action from the KTextEditor part
	void slotIndent();
	///The Tools-> Unindent action from the KTextEditor part
	void slotUnIndent();
	///The Tools-> Clean Indentation action from the KTextEditor part
	void slotCleanIndent();
	///The Tools-> Comment action from the KTextEditor part
	void slotComment();
	///The Tools-> Uncomment action from the KTextEditor part
	void slotUnComment();
	///Create the Configure KTurtle dialog
	void slotSettings();
	///When a setting is changed in Configure KTurtle, it is updated here (change the language of the xml file)
	void slotUpdateSettings();
	
	void slotContextHelp();
	void slotContextHelpUpdate();
	
	void slotStatusBar(QString text, int place);
	void slotCursorStatusBar();
	
	void slotColorPicker();
	///Turn Full Screen mode on or off
	void slotToggleFullscreen();
	void slotUpdateCanvas();

  protected:
    	virtual bool event(QEvent* e);
		bool queryClose();
};


#endif // _KTURTLE_H_
