project(jetbrains_api)
cmake_minimum_required(VERSION 3.5)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
find_package(KF5 REQUIRED COMPONENTS Config)

set(jetbrains_api_SRCS JetbrainsApplication.cpp SettingsDirectory.cpp Project.cpp export.cpp tests/XMLParserTest.cpp)
add_library(jetbrains_api_static STATIC ${jetbrains_api_SRCS})
target_link_libraries(jetbrains_api_static KF5::ConfigCore Qt5::Xml)

add_subdirectory(tests)

find_package(KF5 REQUIRED COMPONENTS Config Service)
target_link_libraries(jetbrains_api_static KF5::ConfigCore KF5::Service)
