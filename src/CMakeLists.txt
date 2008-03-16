include_directories (${CMAKE_SOURCE_DIR}/kturtle/interpreter)
include_directories (${KDE4_INCLUDES})


set (
  kturtle_SRCS
    canvas.cpp
    console.cpp
    directiondialog.cpp
    highlighter.cpp
    editor.cpp
    errordialog.cpp
    main.cpp
    mainwindow.cpp
    sprite.cpp
    inspector.cpp
#     turtle.cpp
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


#uncomment this if oxygen icons for kturtle are available
#kde4_add_app_icon(kturtle_SRCS "${KDE4_ICON_DIR}/oxygen/*/apps/kturtle.png")
kde4_add_app_icon(kturtle_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/hi*-app-kturtle.png")

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
    ${XDG_APPS_INSTALL_DIR}
)

install( FILES kturtleui.rc  DESTINATION  ${DATA_INSTALL_DIR}/kturtle )

# install (
#   FILES
#     interpreter/KTurtleDictionary.en_US.xml
#   DESTINATION
#     ${DATA_INSTALL_DIR}/kturtle/dictionaries
# )

