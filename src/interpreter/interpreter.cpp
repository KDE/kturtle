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


#include <QtDebug>

#include <QFile>
#include <QString>
#include <QStringList>

#include "errormsg.h"
#include "executer.h"
#include "parser.h"
#include "tokenizer.h"
#include "translator.h"

#include "interpreter.h"


Interpreter::Interpreter(QObject* parent)
	: QObject(parent)
{
	errorList  = new ErrorList();
	tokenizer  = new Tokenizer();
	parser     = new Parser();
	executer   = new Executer();

	_state = Uninitialized;
}

void Interpreter::initialize(QTextStream& inStream)
{
	errorList->clear();
	tokenizer->initialize(inStream);
	_state = Initialized;
}

void Interpreter::interpret()
{
	switch (_state) {
		case Uninitialized:
			qCritical("Interpreter::interpret(): called without being initialized");
			return;


		case Initialized:
			qDebug() << "Initializing Parser";
			parser->initialize(tokenizer, errorList);
			_state = Parsing;
			emit parsing();
			break;


		case Parsing:
			parser->parse();

			if (encounteredErrors()) {
				_state = Aborted;
				qDebug() << "Error encountered while parsing:";
				QStringList lines = errorList->asString().split('\n');
				foreach (QString line, lines) qDebug() << line;
				qDebug() << "Aborting because parsing was unsuccessful...";
				return;
			}

			if (parser->isFinished()) {
				qDebug() << "Finished parsing\n\n";
				TreeNode* tree = parser->getTree();
				QString treeString = tree->asString();
				qDebug() << "Node tree as returned by parser:";
				QStringList lines = treeString.split('\n');
				foreach (QString line, lines) qDebug() << line;
				qDebug() << "\n\n";

// 				if (errorList->count() > 0) {
// 					qDebug() << "Parsing returned " << errorList->count() << " error(s):";
// 					QStringList lines = errorList->asString().split('\n');
// 					foreach (QString line, lines) qDebug(line.toLatin1());
// 					qDebug() << "Aborting because parsing was unsuccessful...";
// 					exit(0);
// 				}

				qDebug() << "Executing the node tree:";
				executer->initialize(tree, errorList);
				_state = Executing;
				emit executing();
				return;
			}
			break;


		case Executing:
			executer->execute();

			if (executer->isFinished()) {
				qDebug() << "Finished executing\n\n";
				if (encounteredErrors()) {
					qDebug() << "Parsing and execution returned " << errorList->count() << " error(s):";
					QStringList lines = errorList->asString().split('\n');
					foreach (QString line, lines) qDebug() << line;
				} else {
					qDebug() << "Parsed and executed code without encountering any errors...";
				}
				_state = Finished;
				emit finished();
				return;
			}
			break;


		case Finished:
			qCritical("Interpreter::interpret(): called whe already finished");
			return;
	}
}


#include "interpreter.moc"

