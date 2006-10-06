/*
	Copyright (C) 2003-2006 Cies Breijs <cies # kde ! nl>

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


#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

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

	public:
		/**
		 * Default Constructor
		 */
		Interpreter(QObject* parent = 0, bool testing = false);

		/**
		 * Default Destructor
		 */
		virtual ~Interpreter() {}

		enum State {
			Uninitialized,
			Initialized,
			Parsing,
			Executing,
			Finished,
			Aborted
		};

		void        interpret();

		int         state() { return _state; }
		void        abort() { _state = Aborted; }

		Executer*   getExecuter() { return executer; }
// 		Translator* getTranslator();

		void        initialize(const QString& inputString);  // resets
		bool        encounteredErrors() { return errorList->count() > 0; }
		ErrorList*  getErrorList() { return errorList; }


	private:
		int            _state;

		Translator    *translator;
		Tokenizer     *tokenizer;
		Parser        *parser;
		Executer      *executer;

		ErrorList     *errorList;

		bool           m_testing;


	signals:
		void parsing();
		void executing();
		void finished();
};

#endif  // _INTERPRETER_H_
