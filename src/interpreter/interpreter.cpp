/*
	Copyright (C) 2003-2006 Cies Breijs <cies AT kde DOT nl>

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


#include <QtDebug>

#include <QFile>

#include <QStringList>

#include "errormsg.h"
#include "executer.h"
#include "parser.h"
#include "tokenizer.h"
#include "translator.h"

#include "interpreter.h"


Interpreter::Interpreter(QObject* parent, bool testing)
	: QObject(parent), m_testing(testing)
{
	errorList  = new ErrorList();
	tokenizer  = new Tokenizer();
	parser     = new Parser(testing);
	executer   = new Executer(testing);

	_state = Uninitialized;
}

void Interpreter::initialize(const QString& inString)
{
	errorList->clear();
	tokenizer->initialize(inString);
	_state = Initialized;
}

void Interpreter::interpret()
{
	switch (_state) {
		case Uninitialized:
			qCritical("Interpreter::interpret(): called without being initialized");
			return;


		case Initialized:
			parser->initialize(tokenizer, errorList);
			_state = Parsing;
			qDebug() << "Initialized the parser, parsing the code...";
			emit parsing();
			break;


		case Parsing:
			parser->parse();

			if (encounteredErrors()) {
				_state = Aborted;
				qDebug() << "Error encountered while parsing:";
				QStringList lines = errorList->asString().split('\n');
				foreach (const QString &line, lines) qDebug() << line;
				qDebug() << "Parsing was unsuccessful.";
				return;
			}

			if (parser->isFinished()) {
				qDebug() << "Finished parsing.\n";
				TreeNode* tree = parser->getRootNode();
				emit treeUpdated(tree);
				qDebug() << "Node tree as returned by parser:";
				parser->printTree();
				qDebug() << "";

// 				if (errorList->count() > 0) {
// 					qDebug() << "Parsing returned " << errorList->count() << " error(s):";
// 					QStringList lines = errorList->asString().split('\n');
// 					foreach (const QString &line, lines) qDebug(line.toLatin1());
// 					qDebug() << "Aborting because parsing was unsuccessful...";
// 					exit(0);
// 				}

				executer->initialize(tree, errorList);
				_state = Executing;
				qDebug() << "Initialized the executer, executing the node tree...";
				emit executing();
				return;
			}
			break;


		case Executing:
			executer->execute();

			if (executer->isFinished()) {
				qDebug() << "Finished executing.\n";
				if (encounteredErrors()) {
					qDebug() << "Execution returned " << errorList->count() << " error(s):";
					QStringList lines = errorList->asString().split('\n');
					foreach (const QString &line, lines) qDebug() << line;
				} else {
					qDebug() << "No errors encountered.";
				}
				_state = Finished;
				emit finished();
				return;
			}
			break;


		case Finished:
			qCritical("Interpreter::interpret(): called while already finished");
			return;
	}
}


#include "interpreter.moc"

