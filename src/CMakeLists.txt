ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX KTURTLE VERSION_HEADER kturtle_version.h)


qt_add_dbus_adaptor(kturtle_SRCS interpreter/org.kde.kturtle.Interpreter.xml
                     interpreter/interpreter.h Interpreter)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-kturtle.png")
ecm_add_app_icon(kturtle_SRCS ICONS ${ICONS_SRCS})

add_executable(kturtle
)
target_sources(kturtle PRIVATE
     ${kturtle_SRCS}
    canvas.cpp
    canvas.h
    colorpicker.cpp
    colorpicker.h
    console.cpp
    console.h
    directiondialog.cpp
    directiondialog.h
    editor.cpp
    editor.h
    errordialog.cpp
    errordialog.h
    highlighter.cpp
    highlighter.h
    inspector.cpp
    inspector.h
    interpreteradaptor.cpp
    interpreteradaptor.h
    interpreter/echoer.cpp
    interpreter/echoer.h
    interpreter/errormsg.cpp
    interpreter/errormsg.h
    interpreter/executer.cpp
    interpreter/executer.h
    interpreter/interpreter.cpp
    interpreter/interpreter.h
    interpreter/parser.cpp
    interpreter/parser.h
    interpreter/token.cpp
    interpreter/token.h
    interpreter/tokenizer.cpp
    interpreter/tokenizer.h
    interpreter/translator.cpp
    interpreter/translator.h
    interpreter/treenode.cpp
    interpreter/treenode.h
    interpreter/value.cpp
    interpreter/value.h
    main.cpp
    mainwindow.cpp
    mainwindow.h
    sprite.cpp
    sprite.h
    resources.qrc
)

target_link_libraries(kturtle
    KF${KF_MAJOR_VERSION}::KIOCore
    KF${KF_MAJOR_VERSION}::NewStuffWidgets
    KF${KF_MAJOR_VERSION}::I18n
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Xml
    Qt${QT_MAJOR_VERSION}::Svg
    Qt${QT_MAJOR_VERSION}::PrintSupport
    KF${KF_MAJOR_VERSION}::TextWidgets
    KF${KF_MAJOR_VERSION}::Crash
    KF${KF_MAJOR_VERSION}::XmlGui
)


if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(kturtle Qt6::Core5Compat Qt6::SvgWidgets)
endif()


install (TARGETS  kturtle          ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install (PROGRAMS    org.kde.kturtle.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install (FILES    kturtleui.rc     DESTINATION ${KDE_INSTALL_KXMLGUIDIR}/kturtle)
install (FILES    kturtle.knsrc    DESTINATION ${KDE_INSTALL_CONFDIR}) 

