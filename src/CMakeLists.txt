include_directories (${CMAKE_SOURCE_DIR}/kturtle/interpreter)
include_directories (${KDE4_INCLUDES})


set (
  kturtle_SRCS
    canvas.cpp
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
#set(kturtle_icons
#    ${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/16x16/apps/kturtle.png
#    ${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/32x32/apps/kturtle.png
#    ${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/48x48/apps/kturtle.png
#    ${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/64x64/apps/kturtle.png
#    ${CMAKE_INSTALL_PREFIX}/share/icons/oxygen/128x128/apps/kturtle.png
#)
set(kturtle_icons
    ${CMAKE_CURRENT_SOURCE_DIR}/kturtle/icons/hi16-app-kturtle.png
    ${CMAKE_CURRENT_SOURCE_DIR}/kturtle/icons/hi32-app-kturtle.png
    ${CMAKE_CURRENT_SOURCE_DIR}/kturtle/icons/hi48-app-kturtle.png
    ${CMAKE_CURRENT_SOURCE_DIR}/kturtle/icons/hi64-app-kturtle.png
    ${CMAKE_CURRENT_SOURCE_DIR}/kturtle/icons/hi128-app-kturtle.png
)
kde4_add_win32_app_icon(kturtle kturtle_SRCS ${kturtle_icons})

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

