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


#include <iostream>


#include <QFile>

#include <QDebug>
#include <kapplication.h>
#include <k4aboutdata.h>
#include <kcmdlineargs.h>

#include "mainwindow.h"  // for gui mode

#include "interpreter/interpreter.h"  // for non gui mode
#include "interpreter/echoer.h"
#include "interpreter/tokenizer.h"


static const char description[] =
	I18N_NOOP("KTurtle is an educational programming environment that aims to make learning how to program as easy as possible. To achieve this KTurtle makes all programming tools available from the user interface. The programming language used is TurtleScript which allows its commands to be translated.");

static const char version[]   = "0.8.1 beta";
static const char copyright[] = "(c) 2003-2009 Cies Breijs";
static const char website[]   = "http://edu.kde.org/kturtle";


int main(int argc, char* argv[])
{
	K4AboutData aboutData("kturtle", 0, ki18n("KTurtle"), version, ki18n(description), K4AboutData::License_GPL, ki18n(copyright), KLocalizedString(), website);
	aboutData.addAuthor(ki18n("Cies Breijs"), ki18n("Initiator and core developer"), "cies@kde.nl");
	aboutData.addAuthor(ki18n("Niels Slot"), ki18n("Core developer"), "nielsslot@gmail.com");
	aboutData.addAuthor(ki18n("Mauricio Piacentini"), ki18n("Core developer"), "piacentini@kde.org");

	KCmdLineArgs::init(argc, argv, &aboutData);

	KCmdLineOptions options;
	options.add("i");
	options.add("input <URL or file>", ki18n("File or URL to open (in the GUI mode)"));
	options.add("d");
	options.add("dbus", ki18n("Starts KTurtle in D-Bus mode (without a GUI), good for automated unit test scripts"));
	options.add("t");
	options.add("test <file>", ki18n("Starts KTurtle in testing mode (without a GUI), directly runs the specified local file"));
	options.add("l");
	options.add("lang <code>", ki18n("Specifies the localization language by a language code, defaults to \"en_US\" (only works in testing mode)"));
// 	options.add("k");
// 	options.add("tokenize", ki18n("Only tokenizes the turtle code (only works in testing mode)"));
	options.add("p");
	options.add("parse <file>", ki18n("Translates turtle code to embeddable C++ example strings (for developers only)"));
	KCmdLineArgs::addCmdLineOptions(options);
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

	if (!args->isSet("test") && !args->isSet("parse") && !args->isSet("dbus")) {

		///////////////// run in GUI mode /////////////////
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

	} else if (args->isSet("dbus")) {

		///////////////// run in DBUS mode /////////////////
		KApplication app;
		Translator::instance()->setLanguage();
		new Interpreter(0, true);
		args->clear();
		return app.exec();

	} else if (args->isSet("parse")) {

		///////////////// run in example PARSING mode /////////////////
		QFile inputFile(args->getOption("parse"));
		if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			std::cout << "Could not open file: " << qPrintable(args->getOption("parse")) << std::endl;
			std::cout << "Exitting..." << std::endl;
			return 1;
		}
		
		Translator::instance()->setLanguage();

		Tokenizer tokenizer;
		tokenizer.initialize(inputFile.readAll());
		inputFile.close();

		const QStringList defaultLooks(Translator::instance()->allDefaultLooks());
		QString result;
		Token* t;
		while ((t = tokenizer.getToken())->type() != Token::EndOfInput) {
			if (defaultLooks.contains(t->look()))
				result.append(QString("@(%1)").arg(t->look()));
			else
				result.append(t->look());
			if (t->type() == Token::EndOfLine) result.append('\n');
		}

		foreach (const QString &line, result.split('\n')) std::cout << qPrintable(QString("\"%1\"").arg(line)) << std::endl;
		std::cout << std::endl;

	} else {

		///////////////// run without a gui /////////////////
		std::cout << "KTurtle's interpreter in command line mode (version " << version << ")" << std::endl;
		std::cout << copyright << std::endl << std::endl;

		QString fileString = args->getOption("test");
		QFile inputFile(fileString);

		if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			std::cout << "Could not open input file: " << qPrintable(args->getOption("test")) << std::endl;
			std::cout << "Exitting..." << std::endl;
			return 1;
		}

		QTextStream in(&inputFile);

		// check for our magic identifier
		QString s;
		s = in.readLine();
		if (s != KTURTLE_MAGIC_1_0) {
			std::cout << "The file you try to open is not a valid KTurtle script, or is incompatible with this version of KTurtle.\n";
			return 1;
		}

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

		QString localizedScript;
		localizedScript = Translator::instance()->localizeScript(in.readAll());

// /*		if (args->isSet("tokenize")) {
// 			std::cout << "Tokenizing...\n" << std::endl;
// 			QString code = inputFile.readAll();
// // 			for (int i = 0; i < code.length(); i++) //qDebug() << code.at(i);
// 			Tokenizer tokenizer;
// 			tokenizer.initialize(code);
// 			Token* t;
// 			while ((t = tokenizer.getToken())->type() != Token::EndOfInput) {
// 				std::cout << "TOK> "
// 				          << qPrintable(QString("\"%1\"").arg(t->look()).leftJustified(15))
// 				          << qPrintable(QString("[%1]").arg(QString::number(t->type())).rightJustified(5))
// 				          << qPrintable(QString(" @ (%1,%2)").arg(t->startRow()).arg(t->startCol()))
// 				          << qPrintable(QString(" - (%1,%2)").arg(t->endRow()).arg(t->endCol()))
// 				          << std::endl;
// 			}
// 			return 0;
// 		}*/

		args->clear();  // free some memory

		// init the interpreter
		Interpreter* interpreter = new Interpreter(0, true);  // set testing to true
		interpreter->initialize(localizedScript);

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
