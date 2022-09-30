/*
    SPDX-FileCopyrightText: 2003-2006 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <QStringList>

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
        ~Interpreter() override;

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

	public Q_SLOTS:
		void        interpret();
		int         state() { return m_state; }
		void        initialize(const QString& inputString);  // resets
		bool        encounteredErrors() { return errorList->count() > 0; }
		QStringList getErrorStrings() { return errorList->asStringList(); }

	Q_SIGNALS:
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
