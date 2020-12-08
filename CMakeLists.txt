project(jetbrains_api)
cmake_minimum_required(VERSION 3.5)
set(jetbrains_api_SRCS JetbrainsApplication.cpp SettingsDirectory.cpp Project.cpp export.cpp)

add_library(jetbrains_api_static STATIC ${jetbrains_api_SRCS})

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})
find_package(KF5 REQUIRED COMPONENTS Config)

target_link_libraries(jetbrains_api_static KF5::ConfigCore Qt5::Xml)
