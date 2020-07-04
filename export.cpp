#include "export.h"

namespace JetbrainsAPI {
QList<JetbrainsApplication *> fetchApplications(const KConfigGroup &config, bool filterEmpty, bool fileWatchers) {
    QList<JetbrainsApplication *> appList;
    QList<JetbrainsApplication *> automaticAppList;
    const auto mappingMap = config.group(Config::customMappingGroup).entryMap();
    const auto desktopPaths = JetbrainsApplication::getInstalledApplicationPaths(
        config.group(Config::customMappingGroup));

    // Split manually configured and automatically found apps
    auto desktopPathsIt = desktopPaths.constBegin();
    while (desktopPathsIt != desktopPaths.constEnd()) {
        const QString &path = desktopPathsIt.value();
        ++desktopPathsIt;
        // Desktop file is manually specified
        if (mappingMap.contains(path)) {
            auto customMappedApp = new JetbrainsApplication(path, fileWatchers);
            QFile xmlConfigFile(mappingMap.value(path));
            if (xmlConfigFile.open(QFile::ReadOnly)) {
                customMappedApp->parseXMLFile(xmlConfigFile.readAll());
                // Add path for filewatcher
                customMappedApp->addPath(mappingMap.value(path));
                if (!filterEmpty || !customMappedApp->recentlyUsed.isEmpty()) {
                    appList.append(customMappedApp);
                }
            }
        } else {
            automaticAppList.append(new JetbrainsApplication(path, fileWatchers));
        }
    }

    // Find automatically config directory, read config file and filter apps
    SettingsDirectory::findCorrespondingDirectories(SettingsDirectory::getSettingsDirectories(), automaticAppList);
    JetbrainsApplication::parseXMLFiles(automaticAppList);
    if (filterEmpty){
        automaticAppList = JetbrainsApplication::filterApps(automaticAppList);
    }
    appList.append(automaticAppList);
    return appList;
}

QList<JetbrainsApplication *> fetchApplications(bool filterEmpty, bool fileWatchers) {
    QList<JetbrainsApplication *> appList;
    const auto desktopPaths = JetbrainsApplication::getInstalledApplicationPaths(KConfigGroup());

    // Split manually configured and automatically found apps
    auto desktopPathsIt = desktopPaths.constBegin();
    while (desktopPathsIt != desktopPaths.constEnd()) {
        appList.append(new JetbrainsApplication(desktopPathsIt.value(), fileWatchers));
        ++desktopPathsIt;
    }

    SettingsDirectory::findCorrespondingDirectories(SettingsDirectory::getSettingsDirectories(), appList);
    JetbrainsApplication::parseXMLFiles(appList);
    if (filterEmpty){
        return JetbrainsApplication::filterApps(appList);
    }
    return appList;
}

QList<JetbrainsApplication *> fetchRawApplications(const KConfigGroup &config){
    QList<JetbrainsApplication *> appList;
    KConfigGroup grp;
    if (config.isValid()) {
         grp = config.group(Config::customMappingGroup);
    }
    const auto desktopPaths = JetbrainsApplication::getInstalledApplicationPaths(grp);

    auto desktopPathsIt = desktopPaths.constBegin();
    while (desktopPathsIt != desktopPaths.constEnd()) {
        appList.append(new JetbrainsApplication(desktopPathsIt.value(), false));
        ++desktopPathsIt;
    }

    return appList;
}

QList<JetbrainsApplication *> fetchRawApplications(){
    return fetchRawApplications(KConfigGroup());
}

}
