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
#include <QApplication>
#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <Kdelibs4ConfigMigrator>

#include "mainwindow.h"  // for gui mode

#include "interpreter/interpreter.h"  // for non gui mode
#include "interpreter/echoer.h"
#include "interpreter/tokenizer.h"

#include "kturtle_version.h"

static const char description[] =
	I18N_NOOP("KTurtle is an educational programming environment that aims to make learning how to program as easy as possible. To achieve this KTurtle makes all programming tools available from the user interface. The programming language used is TurtleScript which allows its commands to be translated.");

static const char copyright[] = "(c) 2003-2009 Cies Breijs";
static const char website[]   = "https://edu.kde.org/kturtle";


int main(int argc, char* argv[])
{
	KLocalizedString::setApplicationDomain("kturtle");

	QApplication app(argc, argv);
	app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

	KCrash::initialize();

	/* for migration*/
	QStringList configFiles;
	configFiles << QStringLiteral("kturtlerc");
	Kdelibs4ConfigMigrator migrator(QStringLiteral("kturtle"));
	migrator.setConfigFiles(configFiles);
	migrator.setUiFiles(QStringList() << QStringLiteral("kturtleui.rc"));
	migrator.migrate();

	KAboutData aboutData(QStringLiteral("kturtle"), ki18n("KTurtle").toString(), KTURTLE_VERSION_STRING);
	aboutData.setLicense(KAboutLicense::GPL);
	aboutData.setHomepage(ki18n(website).toString());
	aboutData.setShortDescription(ki18n(description).toString());
	aboutData.setCopyrightStatement(ki18n(copyright).toString());

	aboutData.addAuthor(ki18n("Cies Breijs").toString(), ki18n("Initiator and core developer").toString(), QStringLiteral("cies@kde.nl"));
	aboutData.addAuthor(ki18n("Niels Slot").toString(), ki18n("Core developer").toString(), QStringLiteral("nielsslot@gmail.com"));
	aboutData.addAuthor(ki18n("Mauricio Piacentini").toString(), ki18n("Core developer").toString(), QStringLiteral("piacentini@kde.org"));

	QCommandLineParser parser;

	KAboutData::setApplicationData(aboutData);
	aboutData.setupCommandLine(&parser);

	parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("i") << QStringLiteral("input"), i18n("File or URL to open (in the GUI mode)"), QStringLiteral("URL or file")));
	parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("d") << QStringLiteral("dbus"), i18n("Starts KTurtle in D-Bus mode (without a GUI), good for automated unit test scripts")));
	parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("t") << QStringLiteral("test"), i18n("Starts KTurtle in testing mode (without a GUI), directly runs the specified local file"), QStringLiteral("file")));
	parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("l") << QStringLiteral("lang"), i18n("Specifies the localization language by a language code, defaults to \"en_US\" (only works in testing mode)"), QStringLiteral("code")));
// 	parser.addOption(QCommandLineOption(QStringList() << QLatin1String("k") << QLatin1String("tokenize"), i18n("Only tokenizes the turtle code (only works in testing mode)")));
	parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("p") << QStringLiteral("parse"), i18n("Translates turtle code to embeddable C++ example strings (for developers only)"), QStringLiteral("file")));

	parser.process(app);
	aboutData.processCommandLine(&parser);

	if (!parser.isSet(QStringLiteral("test")) && !parser.isSet(QStringLiteral("parse")) && !parser.isSet(QStringLiteral("dbus"))) {

		///////////////// run in GUI mode /////////////////
		if (app.isSessionRestored()) {
                        kRestoreMainWindows<MainWindow>();
		} else {
			MainWindow* mainWindow = new MainWindow();
			mainWindow->show();
			if (parser.isSet(QStringLiteral("input"))) mainWindow->open(parser.value(QStringLiteral("input")));
		}
		  // free some memory
		return app.exec();  // the mainwindow has WDestructiveClose flag; it will destroy itself.

	} else if (parser.isSet(QStringLiteral("dbus"))) {

		///////////////// run in DBUS mode /////////////////
		Translator::instance()->setLanguage();
		new Interpreter(nullptr, true);
		
		return app.exec();

	} else if (parser.isSet(QStringLiteral("parse"))) {

		///////////////// run in example PARSING mode /////////////////
		QFile inputFile(parser.value(QStringLiteral("parse")));
		if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			std::cout << "Could not open file: " << qPrintable(parser.value("parse")) << std::endl;
			std::cout << "Exiting..." << std::endl;
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
				result.append(QStringLiteral("@(%1)").arg(t->look()));
			else
				result.append(t->look());
			if (t->type() == Token::EndOfLine) result.append('\n');
		}

		foreach (const QString &line, result.split('\n')) std::cout << qPrintable(QString("\"%1\"").arg(line)) << std::endl;
		std::cout << std::endl;

	} else {

		///////////////// run without a gui /////////////////
		std::cout << "KTurtle's interpreter in command line mode (version " << KTURTLE_VERSION_STRING << ")" << std::endl;
		std::cout << copyright << std::endl << std::endl;

		QString fileString = parser.value(QStringLiteral("test"));
		QFile inputFile(fileString);

		if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			std::cout << "Could not open input file: " << qPrintable(parser.value("test")) << std::endl;
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

		if (parser.isSet(QStringLiteral("lang"))) {
			if (Translator::instance()->setLanguage(parser.value(QStringLiteral("lang")))) {
				std::cout << "Set localization to: " << parser.value(QStringLiteral("lang")).data() << std::endl;
			} else {
				std::cout << "Could not set localization to:" << parser.value(QStringLiteral("lang")).data() << std::endl;
				std::cout << "Exitting...\n";
				return 1;
			}
		} else {
			Translator::instance()->setLanguage();
			std::cout << "Using the default (en_US) localization." << std::endl;
		}

		QString localizedScript;
		localizedScript = Translator::instance()->localizeScript(in.readAll());

// /*		if (parser.isSet("tokenize")) {
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

		  // free some memory

		// init the interpreter
		Interpreter* interpreter = new Interpreter(nullptr, true);  // set testing to true
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
