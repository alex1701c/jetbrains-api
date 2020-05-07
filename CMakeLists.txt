set(jetbrains_api_SRCS JetbrainsApplication.cpp SettingsDirectory.cpp Project.cpp export.cpp)

add_library(jetbrains_api_static STATIC ${jetbrains_api_SRCS})

find_package(KF5 REQUIRED COMPONENTS Config)
target_link_libraries(jetbrains_api_static KF5::ConfigCore)
