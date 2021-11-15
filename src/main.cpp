/*
    SPDX-FileCopyrightText: 2003-2006 Cies Breijs <cies AT kde DOT nl>

    SPDX-License-Identifier: GPL-2.0-or-later
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

    KAboutData aboutData(QStringLiteral("kturtle"), i18n("KTurtle"), QLatin1String(KTURTLE_VERSION_STRING));
	aboutData.setLicense(KAboutLicense::GPL);
    aboutData.setHomepage(i18n("https://edu.kde.org/kturtle"));
    aboutData.setShortDescription(i18n("KTurtle is an educational programming environment that aims to make learning how to program as easy as possible. To achieve this KTurtle makes all programming tools available from the user interface. The programming language used is TurtleScript which allows its commands to be translated."));
    aboutData.setCopyrightStatement(i18n("(c) 2003-2009 Cies Breijs"));

    aboutData.addAuthor(i18n("Cies Breijs"), i18n("Initiator and core developer"), QStringLiteral("cies@kde.nl"));
    aboutData.addAuthor(i18n("Niels Slot"), i18n("Core developer"), QStringLiteral("nielsslot@gmail.com"));
    aboutData.addAuthor(i18n("Mauricio Piacentini"), i18n("Core developer"), QStringLiteral("piacentini@kde.org"));

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
            std::cout << "Could not open file: " << qPrintable(parser.value(QStringLiteral("parse"))) << std::endl;
			std::cout << "Exiting..." << std::endl;
			return 1;
		}
		
		Translator::instance()->setLanguage();

        Tokenizer tokenizer;
        tokenizer.initialize(QString::fromUtf8(inputFile.readAll()));
		inputFile.close();

		const QStringList defaultLooks(Translator::instance()->allDefaultLooks());
		QString result;
		Token* t;
		while ((t = tokenizer.getToken())->type() != Token::EndOfInput) {
			if (defaultLooks.contains(t->look()))
				result.append(QStringLiteral("@(%1)").arg(t->look()));
			else
                result.append(t->look());
            if (t->type() == Token::EndOfLine) result.append(QLatin1Char('\n'));
		}

        const auto splitLst{result.split(QLatin1Char('\n'))};
        for (const QString &line : splitLst) std::cout << qPrintable(QStringLiteral("\"%1\"").arg(line)) << std::endl;
		std::cout << std::endl;

	} else {

		///////////////// run without a gui /////////////////
		std::cout << "KTurtle's interpreter in command line mode (version " << KTURTLE_VERSION_STRING << ")" << std::endl;
        std::cout << "(c) 2003-2009 Cies Breijs" << std::endl << std::endl;

		QString fileString = parser.value(QStringLiteral("test"));
		QFile inputFile(fileString);

        if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            std::cout << "Could not open input file: " << qPrintable(parser.value(QStringLiteral("test"))) << std::endl;
			std::cout << "Exiting..." << std::endl;
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
				std::cout << "Exiting...\n";
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
