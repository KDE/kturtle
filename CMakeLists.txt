cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "24")
set(RELEASE_SERVICE_VERSION_MINOR "07")
set(RELEASE_SERVICE_VERSION_MICRO "70")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kturtle VERSION ${RELEASE_SERVICE_VERSION})

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "6.0.0")


find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${ECM_MODULE_PATH}
    
)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMSetupVersion)
include(ECMDeprecationSettings)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Gui
    Svg
    Widgets
    PrintSupport
)
find_package(Qt6SvgWidgets)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED
    Crash
    KIO
    NewStuff
    TextWidgets
    I18n
    CoreAddons
    XmlGui
)
find_package(KF6DocTools ${KF_MIN_VERSION})
set_package_properties(KF6DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)

ecm_set_disabled_deprecation_versions(
    QT 6.7
    KF 6.4.0
)

#Allows QString concatenation to use a single memory allocation per source line.
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_subdirectory (src)
add_subdirectory (icons)

ki18n_install(po)
if (KF6DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(doc)
endif()

install(FILES org.kde.kturtle.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
