cmake_minimum_required(VERSION 3.16)
project(jetbrains-api)

set(KF_MIN_VERSION 5.82)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(KF${QT_MAJOR_VERSION} REQUIRED COMPONENTS Config Service I18n)
find_package(Qt${QT_MAJOR_VERSION} REQUIRED COMPONENTS Xml)

add_library(jetbrains_api_static STATIC JetbrainsApplication.cpp SettingsDirectory.cpp export.cpp)
target_link_libraries(jetbrains_api_static Qt::Xml KF${QT_MAJOR_VERSION}::ConfigCore KF${QT_MAJOR_VERSION}::Service)

include(ECMQtDeclareLoggingCategory)
ecm_qt_declare_logging_category(jetbrains_api_static
    HEADER jetbrains_api_debug.h
    IDENTIFIER JETBRAINS
    CATEGORY_NAME ${PROJECT_NAME}
    DESCRIPTION "Logging for ${PROJECT_NAME}"
    EXPORT JETBRAINS
)
ecm_qt_install_logging_categories(
    EXPORT JETBRAINS
    FILE ${PROJECT_NAME}.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

add_subdirectory(tests)
