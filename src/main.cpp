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


// general stuff
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>


// for gui execution
#include "mainwindow.h"


// for nogui execution
#include <stdio.h>
#include <stdlib.h>

#include <QString>
#include <QFile>
#include <QTextStream>

#include <QDebug>

#include "interpreter/interpreter.h"
#include "interpreter/echoer.h"


void messageOutput(QtMsgType type, const char *msg) {
	// message handler for nogui execution
	switch (type) {
		case QtDebugMsg:    fprintf(stderr, "debug: %s\n", msg);    break;
		case QtWarningMsg:  fprintf(stderr, "warning: %s\n", msg);  break;
		case QtCriticalMsg: fprintf(stderr, "critical: %s\n", msg); break;
		case QtFatalMsg:    fprintf(stderr, "fatal: %s\n", msg);    abort();
	}
}


static const char description[] =
	I18N_NOOP("KTurtle; an educational programming environment.");

static const char version[]   = "0.8 beta";
static const char copyright[] = "(C) 2006 Cies Breijs";
static const char website[]   = "http://edu.kde.org/kturtle";

static KCmdLineOptions options[] = {
	{"n", 0, 0},
	{"nogui", I18N_NOOP("Starts KTurtle without a GUI (i.e. for unit testing)"), 0},
	{"i", 0, 0},
	{"input <URL or file>", I18N_NOOP("File to open (in noGUI mode it is also run)"), 0},
	{"o", 0, 0},
	{"output <file>", I18N_NOOP("File to write the output to, by default the output is printed to the stdout (only works in nonGUI mode)"), 0 },
	{"l", 0, 0},
	{"lang <code>", I18N_NOOP("Language code of the dictionary to use, defaults to built in \"en_US\" (only works in nonGUI mode)"), 0},
	KCmdLineLastOption
};


int main(int argc, char* argv[])
{
	KAboutData about("kturtle", I18N_NOOP("KTurtle"), version, description, KAboutData::License_GPL, copyright, 0, website);
	about.addAuthor("Cies Breijs",
	      I18N_NOOP("Initiator and core developer"),
	                "cies # kde.nl");

	// include the only resources, namely: the default language (en_US) files
// 	Q_INIT_RESOURCE(resources);

	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions(options);
	KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

	if (args->isSet("gui")) {

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

	} else {

		///////////////// run without a gui /////////////////
		qDebug() << "A command line interface to the KTurtle interpreter";
		qDebug() << "Copyright (c) 2005-2006 by Cies Breijs\n";

		QFile inputFile (args->isSet("input")  ? args->getOption("input")  : "");
		QFile outputFile(args->isSet("output") ? args->getOption("output") : "");

		if (args->isSet("input")) {
			// open input file
			if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
				qDebug() << "Could not open input file:" << args->getOption("input");
				return 1;
			}
		} else {
			qDebug() << "No input file was specified, so nothing has to be done...\n";
			return 0;
		}

		// load dictionary
		if (args->isSet("lang")) {
			if (Translator::instance()->loadDictionary(args->getOption("lang"))) {
				qDebug() << "Successfully loaded dictionary:" << args->getOption("lang") << "\n";
			} else {
				qDebug() << "Could not open dictionary:" << args->getOption("lang") << "\n";
				return 1;
			}
		} else {
			qDebug() << "Using the built in (en_US) dictionary...\n";
			Translator::instance()->loadDictionary(BUILT_IN_DICTIONARY_RESOURCE);
		}

		// if output file is specified open it and set dump_signals to TRUE
		bool dump_signals = false;
		if (args->isSet("output")) {
			if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
				qDebug() << "Could not open output file:" << args->getOption("output") << "\n";
				return 1;
			}
			dump_signals = true;
		}

		args->clear();  // free some memory

		// install the msg handler
		qInstallMsgHandler(messageOutput);

		// init the interpreter
		Interpreter* interpreter = new Interpreter();
		QTextStream inputStream(&inputFile);
		interpreter->initialize(inputStream);

		// connect the Executer to the Echoer if dump_signals is TRUE
		QTextStream* outputStream;  // do not create too locally ;-)
		Echoer* echoer;
		if (dump_signals) {
			Executer* executer = interpreter->getExecuter();
			outputStream = new QTextStream(&outputFile);
			echoer = new Echoer(*outputStream);
			echoer->connectAllSlots(executer);
		}

		// the actual execution
		while (interpreter->state() != Interpreter::Finished) interpreter->interpret();

		// close the files
		inputFile.close();
		if (dump_signals) outputFile.close();
	}

	return 0;
}
