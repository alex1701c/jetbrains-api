#include "export.h"

namespace JetbrainsAPI {
QList<JetbrainsApplication *> fetchApplications(const KConfigGroup &config, bool filterEmpty, bool fileWatchers) {
    QList<JetbrainsApplication *> appList;
    QList<JetbrainsApplication *> automaticAppList;
    const auto mappingMap = config.group(Config::customMappingGroup).entryMap();
    const auto desktopPaths = JetbrainsApplication::getInstalledApplicationPaths(
        config.group(Config::customMappingGroup));

    // Split manually configured and automatically found apps
    for (const auto &p:desktopPaths.toStdMap()) {
        // Desktop file is manually specified
        if (mappingMap.contains(p.second)) {
            auto customMappedApp = new JetbrainsApplication(p.second, fileWatchers);
            QFile xmlConfigFile(mappingMap.value(p.second));
            if (xmlConfigFile.open(QFile::ReadOnly)) {
                customMappedApp->parseXMLFile(xmlConfigFile.readAll());
                // Add path for filewatcher
                customMappedApp->addPath(mappingMap.value(p.second));
                if (!filterEmpty || !customMappedApp->recentlyUsed.isEmpty()) {
                    appList.append(customMappedApp);
                }
            }
            xmlConfigFile.close();
        } else {
            automaticAppList.append(new JetbrainsApplication(p.second, fileWatchers));
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
    for (const auto &p:desktopPaths.toStdMap()) {
        appList.append(new JetbrainsApplication(p.second, fileWatchers));
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

    for (const auto &p:desktopPaths.toStdMap()) {
        appList.append(new JetbrainsApplication(p.second, false));
    }

    return appList;
}

QList<JetbrainsApplication *> fetchRawApplications(){
    return fetchRawApplications(KConfigGroup());
}

}
