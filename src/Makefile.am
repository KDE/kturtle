# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

SUBDIRS = pics

# added from kjots
AM_LDFLAGS   = $(all_libraries) $(KDE_RPATH)

kturtle_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kturtle_LDADD = $(LIB_KDEUI) $(LIB_KIO) $(LIB_KDEPRINT) $(LIB_KPARTS) -lkatepartinterfaces

KDE_ICON = kturtle

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kturtle

# the application source, library search path, and link libraries
kturtle_SOURCES = main.cpp kturtle.cpp canvas.cpp dialogs.cpp value.cpp lexer.cpp parser.cpp executer.cpp treenode.cpp token.cpp translate.cpp settings.kcfgc

# let automoc handle all the meta source files (moc)
kturtle_METASOURCES = AUTO

# these are the headers for your project
noinst_HEADERS = kturtle.h canvas.h dialogs.cpp value.h lexer.h parser.h executer.h treenode.h token.h translate.h

kde_kcfg_DATA = kturtle.kcfg 

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/kturtle
shellrc_DATA = kturtleui.rc

# place KTurtle in the start menu under Miscellanous
xdg_apps_DATA = kturtle.desktop

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kturtle.pot
