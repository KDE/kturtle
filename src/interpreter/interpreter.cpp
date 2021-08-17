/*
    SPDX-FileCopyrightText: 2003-2006 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "interpreter.h"
#include "interpreteradaptor.h"

#include <QtDebug>

#include <QFile>
#include <QDebug>



Interpreter::Interpreter(QObject* parent, bool testing)
	: QObject(parent), m_testing(testing)
{
	if (testing) {
		new InterpreterAdaptor(this);
		QDBusConnection dbus = QDBusConnection::sessionBus();
		dbus.registerObject(QStringLiteral("/Interpreter"), this);
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
			Q_EMIT parsing();
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
				Q_EMIT treeUpdated(tree);
// 				//qDebug() << "Node tree as returned by parser:";
// 				parser->printTree();
// 				//qDebug() << "";

				executer->initialize(tree, errorList);
				m_state = Executing;
// 				//qDebug() << "Initialized the executer, executing the node tree...";
				Q_EMIT executing();
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
				Q_EMIT finished();
				return;
			}
			break;


		case Finished:
			qCritical("Interpreter::interpret(): called while already finished");
			return;
	}
}
