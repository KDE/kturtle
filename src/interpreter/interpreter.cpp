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

#include "interpreter.h"
#include "interpreteradaptor.h"

#include <QtDebug>

#include <QFile>
#include <QDebug>

#include "errormsg.h"
#include "executer.h"
#include "parser.h"
#include "tokenizer.h"
#include "translator.h"


Interpreter::Interpreter(QObject* parent, bool testing)
	: QObject(parent), m_testing(testing)
{
	if (testing) {
		new InterpreterAdaptor(this);
		QDBusConnection dbus = QDBusConnection::sessionBus();
		dbus.registerObject("/Interpreter", this);
	}

	errorList  = new ErrorList();
	tokenizer  = new Tokenizer();
	parser     = new Parser(testing);
	executer   = new Executer(testing);

    m_state = Uninitialized;
}

Interpreter::~Interpreter()
{
    errorList->clear();
    delete errorList;
    delete tokenizer;
    delete parser;
    delete executer;
}

void Interpreter::initialize(const QString& inString)
{
	errorList->clear();
	tokenizer->initialize(inString);
	m_state = Initialized;
}

void Interpreter::interpret()
{
	switch (m_state) {
		case Uninitialized:
			qCritical("Interpreter::interpret(): called without being initialized");
			return;


		case Initialized:
			parser->initialize(tokenizer, errorList);
			m_state = Parsing;
// 			//qDebug() << "Initialized the parser, parsing the code...";
			emit parsing();
			break;


		case Parsing:
			parser->parse();

			if (encounteredErrors()) {
				m_state = Aborted;
// 				//qDebug() << "Error encountered while parsing:";
//				const QStringList lines = errorList->asStringList();
//				foreach (const QString &line, lines)
					//qDebug() << line;
// 				//qDebug() << "Parsing was unsuccessful.";
				return;
			}

			if (parser->isFinished()) {
// 				//qDebug() << "Finished parsing.\n";
				TreeNode* tree = parser->getRootNode();
				emit treeUpdated(tree);
// 				//qDebug() << "Node tree as returned by parser:";
// 				parser->printTree();
// 				//qDebug() << "";

				executer->initialize(tree, errorList);
				m_state = Executing;
// 				//qDebug() << "Initialized the executer, executing the node tree...";
				emit executing();
				return;
			}
			break;


		case Executing:
			executer->execute();

			if (executer->isFinished()) {
// 				//qDebug() << "Finished executing.\n";
				if (encounteredErrors()) {
// 					//qDebug() << "Execution returned " << errorList->count() << " error(s):";
// 					const QStringList lines = errorList->asStringList();
// 					foreach (const QString &line, lines)
// 						//qDebug() << line;
				} else {
// 					//qDebug() << "No errors encountered.";
				}
				m_state = Finished;
				emit finished();
				return;
			}
			break;


		case Finished:
			qCritical("Interpreter::interpret(): called while already finished");
			return;
	}
}
