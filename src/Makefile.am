# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

SUBDIRS = data

# added from kjots
AM_LDFLAGS   = $(all_libraries) $(KDE_RPATH)

kturtle_LDFLAGS = $(KDE_RPATH) $(all_libraries)
kturtle_LDADD = $(LIB_KDEUI) $(LIB_KIO)

KDE_ICON = kturtle

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
bin_PROGRAMS = kturtle

# the application source, library search path, and link libraries
kturtle_SOURCES = main.cpp kturtle.cpp canvas.cpp number.cpp lexer.cpp parser.cpp executer.cpp treenode.cpp settings.cpp colorpicker.cpp

# let automoc handle all the meta source files (moc)
kturtle_METASOURCES = AUTO

# these are the headers for your project
noinst_HEADERS = kturtle.h canvas.h number.h lexer.h parser.h executer.h treenode.h settings.h colorpicker.h

# this is where the desktop file will go
shelldesktopdir   = $(kde_appsdir)/Utilities
shelldesktop_DATA = kturtle.desktop

# this is where the shell's XML-GUI resource file goes
shellrcdir   = $(kde_datadir)/kturtle
shellrc_DATA = kturtleui.rc

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kturtle.pot
