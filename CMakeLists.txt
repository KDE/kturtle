cmake_minimum_required(VERSION 2.8.12)
set(QT_MIN_VERSION "5.0.0")

project (kturtle)

find_package(ECM 5.15.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${ECM_MODULE_PATH}
    ${ECM_KDE_MODULE_DIR}
)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMAddAppIcon)
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Gui
    Svg
    Widgets
    PrintSupport
)

find_package(KF5 5.15 REQUIRED
    Crash
    KIO
    NewStuff
    DocTools
    TextWidgets
    #Gui
    #GuiAddons
    I18n
    #WidgetsAddons
    CoreAddons
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/interpreter
                    #${CMAKE_BINARY_DIR}
                    #${KDE4_INCLUDES}
                    #${QT4_INCLUDES}
)

#Allows QString concatenation to use a single memory allocation per source line.
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

add_subdirectory (doc)
add_subdirectory (src)
add_subdirectory (icons)

install(FILES org.kde.kturtle.appdata.xml DESTINATION ${KDE_INSTALL_DATAROOTDIR}/appdata/)
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
