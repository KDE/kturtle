include_directories(${CMAKE_SOURCE_DIR}/kturtle/interpreter)

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

qt5_add_dbus_adaptor(kturtle_SRCS interpreter/org.kde.kturtle.Interpreter.xml
                     interpreter/interpreter.h Interpreter)

set (kturtle_RCCS resources.qrc)
qt5_add_resources (kturtle_RCC_SRCS ${kturtle_RCCS})

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-kturtle.png")
ecm_add_app_icon(kturtle_SRCS ICONS ${ICONS_SRCS})

add_executable(kturtle ${kturtle_SRCS}
                       ${kturtle_RCC_SRCS}
)

target_link_libraries(kturtle
    KF5::KIOCore
    KF5::NewStuff
    KF5::I18n
    Qt5::Core
    Qt5::Gui
    Qt5::Xml
    Qt5::Svg
    Qt5::PrintSupport
    KF5::TextWidgets
    KF5::Crash
)

install (TARGETS  kturtle          ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install (PROGRAMS    org.kde.kturtle.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install (FILES    kturtleui.rc     DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kturtle)
install (FILES    kturtle.knsrc    DESTINATION ${KDE_INSTALL_CONFDIR}) 

