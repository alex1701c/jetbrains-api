include(ECMAddTests)
find_package(Qt${QT_MAJOR_VERSION} REQUIRED CONFIG REQUIRED Test)
ecm_add_tests(
        XMLParserTest.cpp
        NameFormattingTest.cpp
        LINK_LIBRARIES Qt::Core Qt::Test jetbrains_api_static
)
