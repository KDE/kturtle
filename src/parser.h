/*
     Copyright (C) 2003 by Walter Schreppers 
     Copyright (C) 2004 by Cies Breijs   
     
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

 
#ifndef _PARSER_H_
#define _PARSER_H_

#include <qobject.h>

#include "lexer.h"
#include "treenode.h"


class Parser : public QObject {
Q_OBJECT

	public:
	Parser(QTextIStream& in);
	virtual ~Parser();

	void parse();
	TreeNode* getTree() const { return tree; }

	signals:
	void ErrorMsg(QString s, uint row, uint col, uint code);

	private:
	bool isAddOp(token);
	bool isMulOp(token);
	
	void getToken();
	void matchToken(int);
	void Error(const QString& s, uint code = 1000, uint r = NA, uint c = NA);

	TreeNode* Program();
	//TreeNode* Function();
	TreeNode* IdList();
	TreeNode* ParamList();
	TreeNode* Block();
	TreeNode* Statement();
	
	TreeNode* ExternalRun();

	TreeNode* getId();
	TreeNode* signedFactor();
	TreeNode* Factor();
	TreeNode* Term();
	TreeNode* Expression();
	
	TreeNode* Assignment  ( const QString& );
	TreeNode* FunctionCall( const QString&, uint r = NA, uint c = NA);
	TreeNode* Other();
	
	TreeNode* While();
	TreeNode* For();
	TreeNode* ForEach();
	TreeNode* If();
	TreeNode* getString();
	TreeNode* Return();
	TreeNode* Break();
	
	TreeNode* Clear();
	TreeNode* Go();
	TreeNode* GoX();
	TreeNode* GoY();
	TreeNode* Forward();
	TreeNode* Backward();
	TreeNode* Direction();
	TreeNode* TurnLeft();
	TreeNode* TurnRight();
	TreeNode* Center();
	TreeNode* SetPenWidth();
	TreeNode* PenUp();
	TreeNode* PenDown();
	TreeNode* SetFgColor();
	TreeNode* SetBgColor();
	TreeNode* ResizeCanvas();
	TreeNode* SpriteShow();
	TreeNode* SpriteHide();
	TreeNode* SpritePress();
	TreeNode* SpriteChange();

	TreeNode* Message();
	TreeNode* InputWindow();
	TreeNode* Print();
	TreeNode* FontType();
	TreeNode* FontSize();
	TreeNode* Repeat();
	TreeNode* Random();
	TreeNode* Wait();
	TreeNode* WrapOn();
	TreeNode* WrapOff();
	TreeNode* Reset();
	
	TreeNode* LineBreak();
	
	TreeNode* Learn();

	//private locals
	Lexer       *lexer;
	token        currentToken;
	TreeNode    *tree;
	TreeNode    *currentNode;
	uint         row;
	uint         col;
	QStringList  learnedFunctionList;
};

#endif // _PARSER_H_
