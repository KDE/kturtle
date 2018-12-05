/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

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


#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <QStringList>
#include <QTextStream>

#include "errormsg.h"
#include "executer.h"
#include "parser.h"
#include "tokenizer.h"
#include "translator.h"
#include "treenode.h"


/**
 * @short Step-wise interpreter for KTurtle code.
 *
 * This class handles the interpreting of KTurtle code.
 * It does so in a few steps:
 * 1. @TODO 
 *
 * @author Cies Breijs
 */
class Interpreter : public QObject
{
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.kturtle.Interpreter")

	public:
		/**
		 * Default Constructor
		 */
		Interpreter(QObject* parent, bool testing);

		/**
		 * Default Destructor
		 */
        virtual ~Interpreter();

		enum State {
			Uninitialized, // unusable
			Initialized,   // ready to interpret something
			Parsing,       // parsing the code string to a tree
			Executing,     // executing the tree
			Finished,      // successfully finished executing
			Aborted        // unsuccessfully finished
		};

		void        abort() { m_state = Aborted; }

		Executer*   getExecuter() { return executer; }
		ErrorList*  getErrorList() { return errorList; }

	public slots:
		void        interpret();
		int         state() { return m_state; }
		void        initialize(const QString& inputString);  // resets
		bool        encounteredErrors() { return errorList->count() > 0; }
		QStringList getErrorStrings() { return errorList->asStringList(); }

	signals:
		void parsing();
		void executing();
		void finished();
		
		void treeUpdated(TreeNode* rootNode);

	private:
		int            m_state;

		Translator    *translator;
		Tokenizer     *tokenizer;
		Parser        *parser;
		Executer      *executer;

		ErrorList     *errorList;

		bool           m_testing;
};

#endif  // _INTERPRETER_H_
