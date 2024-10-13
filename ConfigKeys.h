#ifndef JETBRAINSRUNNER_CONFIGKEYS_H
#define JETBRAINSRUNNER_CONFIGKEYS_H

#include <QObject>

struct Config {
    constexpr static const auto launchByAppName = "LaunchByAppName";
    constexpr static const auto launchByProjectName = "LaunchByProjectName";
    constexpr static const auto notifyUpdates = "NotifyUpdates";
    constexpr static const auto formatString = "FormatString";
    constexpr static const auto formatStringDefault = "%APPNAME launch %PROJECT";
    constexpr static const auto displayInCategories = "DisplayInCategories";
    constexpr static const auto customMappingGroup = "CustomMapping";
    constexpr static const auto checkedUpdateDate = "checkedUpdateDate";
    constexpr static const auto filterSearchResults = "filterSearchResults";
};

// Text display inside KRunner
struct FormatString {
    constexpr static const auto PROJECT = "%PROJECT";
    constexpr static const auto APPNAME = "%APPNAME";
    constexpr static const auto APP = "%APP";
    constexpr static const auto DIR = "%DIR";
};

// Used for filtering the project in KRunner
enum SearchResultChoice {
    ProjectNameContains,
    PathContains,
    ProjectNameStartsWith,
};
Q_ENUMS(SearchResultChoice)

#endif // JETBRAINSRUNNER_CONFIGKEYS_H
