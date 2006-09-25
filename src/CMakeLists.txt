include (${CMAKE_ROOT}/Modules/FindQt4.cmake)
include (${CMAKE_ROOT}/Modules/FindKDE4.cmake)

include_directories (${CMAKE_SOURCE_DIR}/kturtle/interpreter)
include_directories (${KDE4_INCLUDES})


set (
  kturtle_SRCS
    canvas.cpp
    highlighter.cpp
    editor.cpp
    errordialog.cpp
    main.cpp
    mainwindow.cpp
    sprite.cpp
    turtle.cpp
    valueaction.cpp
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


set (kturtle_RCCS resources.qrc)
qt4_add_resources (kturtle_RCC_SRCS ${kturtle_RCCS})


kde4_automoc (${kturtle_SRCS})


kde4_add_executable (
  kturtle
    ${kturtle_SRCS}
    ${kturtle_RCC_SRCS}
)


target_link_libraries (
  kturtle
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
)


install (
  TARGETS
    kturtle
  DESTINATION
    ${BIN_INSTALL_DIR}
)

install (
  FILES
    kturtle.desktop
  DESTINATION
    ${XDG_APPS_DIR}
)

install (
  FILES
    interpreter/KTurtleDictionary.en_US.xml
  DESTINATION
    ${DATA_INSTALL_DIR}/kturtle/dictionaries
)

