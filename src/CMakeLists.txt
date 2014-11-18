include_directories(${CMAKE_SOURCE_DIR}/kturtle/interpreter)
#include_directories(${KDE4_INCLUDES})

set(kturtle_SRCS
    canvas.cpp
    colorpicker.cpp
    console.cpp
    directiondialog.cpp
    highlighter.cpp
    inspector.cpp
    editor.cpp
    errordialog.cpp
    main.cpp
    mainwindow.cpp
    sprite.cpp
    interpreter/echoer.cpp
    interpreter/errormsg.cpp
    interpreter/executer.cpp
    interpreter/interpreter.cpp
    interpreter/parser.cpp
    interpreter/token.cpp
    interpreter/tokenizer.cpp
    interpreter/translator.cpp
    interpreter/treenode.cpp
    interpreter/value.cpp
)

#FIXME: Reenable.  Why does this not work?
#qt5_add_dbus_adaptor(kturtle_SRCS interpreter/org.kde.kturtle.Interpreter.xml
#                     interpreter/interpreter.h Interpreter)
#QT4_ADD_DBUS_ADAPTOR(kturtle_SRCS interpreter/org.kde.kturtle.Interpreter.xml interpreter/interpreter.h Interpreter)

set (kturtle_RCCS resources.qrc)
# FIXME: Reenable
#qt5_add_resources (kturtle_RCC_SRCS ${kturtle_RCCS})
#qt4_add_resources (kturtle_RCC_SRCS ${kturtle_RCCS})

# uncomment this if oxygen icons for kturtle are available
#kde4_add_app_icon (kturtle_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/kturtle.png")
# FIXME: Reeneable
#kde5_add_app_icon (kturtle_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-kturtle.png")
#kde4_add_app_icon (kturtle_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-kturtle.png")

add_executable(kturtle ${kturtle_SRCS}
                       ${kturtle_RCC_SRCS}
)

target_link_libraries(kturtle
    KF5::KIOCore
    KF5::NewStuff
    Qt5::Core
    Qt5::Gui
    Qt5::Xml
    #${QT_QTCORE_LIBRARY}
    #${QT_QTGUI_LIBRARY}
    #${QT_QTXML_LIBRARY}
    #${KDE4_KIO_LIBS}
    #${KDE4_KNEWSTUFF3_LIBS}
)

install (TARGETS  kturtle          ${INSTALL_TARGETS_DEFAULT_ARGS})
install (PROGRAMS    kturtle.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install (FILES    kturtleui.rc     DESTINATION ${DATA_INSTALL_DIR}/kturtle)
install (FILES    kturtle.knsrc    DESTINATION ${CONFIG_INSTALL_DIR}) 

