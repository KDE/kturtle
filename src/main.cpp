/*
    Copyright (C) 2003 Cies Breijs <cies # kde ! nl>
 
    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

 
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include "kturtle.h"


static const char description[] =
	I18N_NOOP("Educational Programming Environment using the Logo programming language");

static const char version[] = "0.6";

static const char copyright[] = I18N_NOOP("(C) 2003 The KTurtle Authors");

static const char website[] = "http://edu.kde.org/kturtle";

static KCmdLineOptions options[] =
{
///   @todo  so some about the command line arguments... Something like:
// 	{ "e <argument>", I18N_NOOP( "sets the folder with the examples to <argument>" ), 0 }
// 	{ "t <argument>", I18N_NOOP( "sets the xml file containing the translatiosns of the Logo commands" ), 0 }
// 	{ "h <argument>", I18N_NOOP( "sets the xml file containing the highlightstyle" ), 0 }
// 	{ "+[URL]", I18N_NOOP( "A Logo file to open" ), 0 },
	KCmdLineLastOption
};


int main(int argc, char **argv)
{
	KAboutData about("kturtle", I18N_NOOP("KTurtle"), version, description, KAboutData::License_GPL, copyright, 0, website);
	about.addAuthor("Cies Breijs",
	      I18N_NOOP("Main developer and initiator"),
	                "cies # kde.nl");
	about.addAuthor("Anne-Marie Mahfouf",
	      I18N_NOOP("Big contributor, supporter and fan"),
	                "annemarie.mahfouf # free.fr");    
	about.addAuthor("Walter Schreppers",
	      I18N_NOOP("Author of \"wsbasic\" (wsbasic.sf.net) the base for the interpreter of KTurtle"),
	                "Walter.Schreppers # ua.ac.be");
	about.addCredit("Matthias Meßmer",
	      I18N_NOOP("German Data Files"),
	                "bmlmessmer # web.de"); 
	about.addCredit("Burkhard Lück",
	      I18N_NOOP("German Data Files"),
			"lueck # hube-lueck.de"); 
	about.addCredit("Stefan Asserhäll",
	      I18N_NOOP("Swedish Data Files"),
	                "stefan.asserhall # telia.com"); 
	about.addCredit("Jure Repinc",
	      I18N_NOOP("Slovenian Data Files"),
	                "jlp # holodeck1.com");
	about.addCredit("Chusslove Illich",
	      I18N_NOOP("Serbian (Latin and Cyrillic) Data Files"),
	                "caslav.ilic # gmx.net");
	about.addCredit("Pino Toscano",
	      I18N_NOOP("Italian Data Files"),
	                "toscano.pino # tiscali.it");
	about.addCredit("Andy Potter",
	      I18N_NOOP("British English Data Files"),
	                "A.J.Potter # rhul.ac.uk");	                
	about.addCredit("Rafael Beccar",
	      I18N_NOOP("Spanish Data Files"),
	                "rafael.beccar # kdemail.net");
	about.addCredit("Riverson Rios",
			I18N_NOOP("Brazilian Portuguese Data Files"),
			"riverson # ccv.ufc.br");
        about.addCredit("Karl Ove Hufthammer",
			 I18N_NOOP("Norwegian Nynorsk and Bokmål Data files"), "karl # huftis.org");
	about.addCredit("Albert Astals Cid",
	      I18N_NOOP("Parser Cyrillic support"),
	                "astals11 # terra.es");
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions( options );
	KApplication app;
	MainWindow *mainwindow = 0;

	if ( app.isRestored() )
	{
		RESTORE(MainWindow);
	}
	else
	{
		// no session... just start up normally
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

		/// @todo do something with the command line args here

		mainwindow = new MainWindow();
		app.setMainWidget(mainwindow);
		mainwindow->show();

		args->clear();
	}

	// mainwindow has WDestructiveClose flag by default, so it will delete itself.
	return app.exec();
}
