#include "export.h"
#include <KConfigGroup>
#include <QRegularExpression>
#include <algorithm>


namespace JetbrainsAPI {
QList<JetbrainsApplication *> fetchApplications(const KConfigGroup &config, bool filterEmpty, bool fileWatchers) {
    QList<JetbrainsApplication *> appList;
    QList<JetbrainsApplication *> automaticAppList;
    const QMap<QString, QString> mappingMap = config.isValid() ? config.group(Config::customMappingGroup).entryMap() : QMap<QString, QString>();
    const auto desktopPaths = JetbrainsApplication::getInstalledApplicationPaths(config.isValid() ? config.group(Config::customMappingGroup): config);

    const auto desktopPathsMap = desktopPaths.toStdMap();
    std::map<QString, QString> specialEditions;
     // This should extract "pycharm" from "jetbrains-pycharm-ce.desktop"
    const QRegularExpression baseNameExpr(QStringLiteral("jetbrains-([a-z]+)(-[a-z])?+"));
    for (const auto &p: desktopPathsMap) {
        const QRegularExpressionMatch regexMatch = baseNameExpr.match(p.second);
        if (regexMatch.hasMatch()) {
            Q_ASSERT(regexMatch.capturedTexts().length() >= 2);
            specialEditions.insert(std::pair(QFileInfo(p.second).baseName(), regexMatch.capturedTexts().at(1)));
        }
    }

    // Split manually configured and automatically found apps
    for (const auto &p: desktopPathsMap) {
        // Desktop file is manually specified
        if (mappingMap.contains(p.second)) {
            auto customMappedApp = new JetbrainsApplication(p.second, fileWatchers);
            if (QFileInfo::exists(mappingMap.value(p.second))) {
                customMappedApp->parseXMLFile(mappingMap.value(p.second));
                // Add path for filewatcher
                customMappedApp->addPath(mappingMap.value(p.second));
                if (!filterEmpty || !customMappedApp->recentlyUsed.isEmpty()) {
                    appList.append(customMappedApp);
                }
            }
        } else {
            const QString baseName = QFileInfo(p.second).baseName();
            bool shouldNotTrimEdition = std::any_of(specialEditions.begin(), specialEditions.end(), [baseName](const std::pair<QString, QString> &value){
                return baseName.contains(value.second) && baseName != value.first;
            });
            automaticAppList.append(new JetbrainsApplication(p.second, fileWatchers, shouldNotTrimEdition));
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
