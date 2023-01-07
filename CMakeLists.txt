cmake_minimum_required(VERSION 3.16)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
find_package(KF5 REQUIRED COMPONENTS Config)

set(jetbrains_api_SRCS JetbrainsApplication.cpp SettingsDirectory.cpp export.cpp tests/XMLParserTest.cpp)
add_library(jetbrains_api_static STATIC ${jetbrains_api_SRCS})
target_link_libraries(jetbrains_api_static KF5::ConfigCore Qt5::Xml)

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
    FILE  ${PROJECT_NAME}.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

add_subdirectory(tests)

find_package(KF5 REQUIRED COMPONENTS Config Service)
target_link_libraries(jetbrains_api_static KF5::ConfigCore KF5::Service)
