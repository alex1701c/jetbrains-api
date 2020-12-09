include(ECMAddTests)
find_package(Qt5 REQUIRED CONFIG REQUIRED Test)
ecm_add_tests(
        XMLParserTest.cpp
        LINK_LIBRARIES Qt5::Core Qt5::Test jetbrains_api_static
)
