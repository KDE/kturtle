cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

# KDE Application Version, managed by release script
set(KDE_APPLICATIONS_VERSION_MAJOR "19")
set(KDE_APPLICATIONS_VERSION_MINOR "07")
set(KDE_APPLICATIONS_VERSION_MICRO "90")
set(KDE_APPLICATIONS_VERSION "${KDE_APPLICATIONS_VERSION_MAJOR}.${KDE_APPLICATIONS_VERSION_MINOR}.${KDE_APPLICATIONS_VERSION_MICRO}")

project(kturtle VERSION ${KDE_APPLICATIONS_VERSION})

cmake_minimum_required(VERSION 2.8.12)
set(QT_MIN_VERSION "5.0.0")

find_package(ECM 5.15.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${ECM_MODULE_PATH}
    ${ECM_KDE_MODULE_DIR}
)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMSetupVersion)

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
    I18n
    CoreAddons
)

#Allows QString concatenation to use a single memory allocation per source line.
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

add_subdirectory (doc)
add_subdirectory (src)
add_subdirectory (icons)

install(FILES org.kde.kturtle.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
