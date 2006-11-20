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


#include <iostream>

#include <QString>
#include <QFile>

#include <kdebug.h>

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>

#include "mainwindow.h"  // for gui mode

#include "interpreter/interpreter.h"  // for non gui mode
#include "interpreter/echoer.h"
#include "interpreter/tokenizer.h"


static const char description[] =
	I18N_NOOP("KTurtle; an educational programming environment.");

static const char version[]   = "0.8 beta";
static const char copyright[] = "(c) 2003-2006 Cies Breijs";
static const char website[]   = "http://edu.kde.org/kturtle";

static KCmdLineOptions options[] = {
	{"i", 0, 0},
	{"input <URL or file>", I18N_NOOP("File to open (in the GUI mode)"), 0},
	{"t", 0, 0},
	{"test <file>", I18N_NOOP("Starts KTurtle in testing mode (without a GUI), directly runs the specified local file"), 0},
	{"l", 0, 0},
	{"lang <code>", I18N_NOOP("Specifies the localization language by a language code, defaults to \"en_US\" (only works in testing mode)"), 0},
	{"k", 0, 0},
	{"tokenize", I18N_NOOP("Only tokenizes the turtle code (only works in testing mode)"), 0},
	{"p", 0, 0},
	{"parse <file>", I18N_NOOP("Translates turtle code to embeddable C++ example strings (for developers only)"), 0},
	KCmdLineLastOption
};


int main(int argc, char* argv[])
{
	KAboutData aboutData("kturtle", I18N_NOOP("KTurtle"), version, description, KAboutData::License_GPL, copyright, 0, website);
	aboutData.addAuthor("Cies Breijs",
	          I18N_NOOP("Initiator and core developer"),
	                    "cies # kde.nl");

	KCmdLineArgs::init(argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions(options);
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

	if (!args->isSet("test") && !args->isSet("parse")) {

		///////////////// run in gui mode /////////////////
		KApplication app;
		if (app.isSessionRestored()) {
			RESTORE(MainWindow);
		} else {
			MainWindow* mainWindow = new MainWindow();
			mainWindow->show();
			if (args->isSet("input")) mainWindow->open(args->getOption("input"));
		}
		args->clear();  // free some memory
		return app.exec();  // the mainwindow has WDestructiveClose flag; it will destroy itself.

	} else if (args->isSet("parse")) {

		///////////////// run in example parsing mode /////////////////
		QFile inputFile(args->getOption("parse"));
		if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			std::cout << "Could not open file: " << qPrintable(QFile::decodeName(args->getOption("parse"))) << std::endl;
			std::cout << "Exitting..." << std::endl;
			return 1;
		}
		
		new KInstance(&aboutData);  // need a KInstance since we're using KLocale in the Translator class
		Translator::instance()->setLanguage();

		Tokenizer tokenizer;
		tokenizer.initialize(inputFile.readAll());
		inputFile.close();

		QStringList defaultLooks(Translator::instance()->allDefaultLooks());
		QString result;
		Token* t;
		while ((t = tokenizer.getToken())->type() != Token::EndOfInput) {
			if (defaultLooks.contains(t->look()))
				result.append(QString("@(%1)").arg(t->look()));
			else
				result.append(t->look());
			if (t->type() == Token::EndOfLine) result.append('\n');
		}

		foreach (QString line, result.split('\n')) std::cout << qPrintable(QString("\"%1\"").arg(line)) << std::endl;
		std::cout << std::endl;

	} else {

		///////////////// run without a gui /////////////////
		std::cout << "KTurtle's interpreter in command line mode (version " << version << ")" << std::endl;
		std::cout << copyright << std::endl << std::endl;

		QFile inputFile(args->getOption("test"));

		if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			std::cout << "Could not open input file: " << qPrintable(QFile::decodeName(args->getOption("test"))) << std::endl;
			std::cout << "Exitting..." << std::endl;
			return 1;
		}

		new KInstance(&aboutData);  // need a KInstance since we're using KLocale in the Translator class

		if (args->isSet("lang")) {
			if (Translator::instance()->setLanguage(args->getOption("lang"))) {
				std::cout << "Set localization to: " << args->getOption("lang").data() << std::endl;
			} else {
				std::cout << "Could not set localization to:" << args->getOption("lang").data() << std::endl;
				std::cout << "Exitting...\n";
				return 1;
			}
		} else {
			Translator::instance()->setLanguage();
			std::cout << "Using the default (en_US) localization." << std::endl;
		}

		if (args->isSet("tokenize")) {
			std::cout << "Tokenizing...\n" << std::endl;
			QString code = inputFile.readAll();
// 			for (int i = 0; i < code.length(); i++) kDebug() << code.at(i) << endl;
			Tokenizer tokenizer;
			tokenizer.initialize(code);
			Token* t;
			while ((t = tokenizer.getToken())->type() != Token::EndOfInput) {
				std::cout << "TOK> "
				          << qPrintable(QString("\"%1\"").arg(t->look()).leftJustified(15))
				          << qPrintable(QString("[%1]").arg(QString::number(t->type())).rightJustified(5))
				          << qPrintable(QString(" @ (%1,%2)").arg(t->startRow()).arg(t->startCol()))
				          << qPrintable(QString(" - (%1,%2)").arg(t->endRow()).arg(t->endCol()))
				          << std::endl;
			}
			return 0;
		}

		args->clear();  // free some memory

		// init the interpreter
		Interpreter* interpreter = new Interpreter(0, true);  // set testing to true
		interpreter->initialize(inputFile.readAll());
		inputFile.close();

		// install the echoer
		(new Echoer())->connectAllSlots(interpreter->getExecuter());

		// the actual execution (limited to a certain amount of iterations to break endless loops)
		static const int MAX_ITERATION_STEPS = 20000;
		int i;
		for (i = 0;
		     interpreter->state() != Interpreter::Finished &&
		     interpreter->state() != Interpreter::Aborted  &&
		     interpreter->getErrorList()->isEmpty() &&
		     i < MAX_ITERATION_STEPS;
		     i++)
			interpreter->interpret();

		if (i == MAX_ITERATION_STEPS)
			std::cout << "ERR> Iterated more than " << MAX_ITERATION_STEPS << " steps... Execution terminated." << std::endl;

	}

	return 0;
}
