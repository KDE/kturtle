/*
 * Copyright (C) 2003 Cies Breijs <cies # showroommama ! nl>
 */

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

#include "kturtle.h"

static const char description[] =
    I18N_NOOP("KTurtle - Educational programming environment using the Logo programming language");

static const char version[] = "0.1";

static const char copyright[] = "(C) 2003 The KTurtle Authors";

static const char website[] = "http://kturtle.sf.net";

static KCmdLineOptions options[] =
{
//    TODO implement this (first katepart should be used)
//    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("kturtle", I18N_NOOP("KTurtle"), version, description,
                     KAboutData::License_GPL, copyright, 0, website);
    about.addAuthor( "Cies Breijs",
           I18N_NOOP("Main developer and initiator"),
                     "cies # showroommama.nl" );
    about.addAuthor( "Walter Schreppers",
           I18N_NOOP("Author of \"wsbasic\" (wsbasic.sf.net) the base for the interpreter of KTurtle"),
                     "Walter.Schreppers # ua.ac.be" );
    about.addAuthor( "Anne-Marie Mahfouf",
           I18N_NOOP("Big contributor, supporter and fan"),
                      "annemarie.mahfouf # free.fr" );    
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;
    MainWindow *mainwindow = 0;

    if ( app.isRestored() ) {
        RESTORE(MainWindow);
    } else {
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
