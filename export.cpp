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

    std::map<QString, QString> specialEditions;
     // This should extract "pycharm" from "jetbrains-pycharm-ce.desktop"
    const QRegularExpression baseNameExpr(QStringLiteral("jetbrains-([a-z]+)(-[a-z])?+"));
    for (const QString &path: desktopPaths) {
        const QRegularExpressionMatch regexMatch = baseNameExpr.match(path);
        if (regexMatch.hasMatch()) {
            Q_ASSERT(regexMatch.capturedTexts().length() >= 2);
            specialEditions.insert(std::pair<QString, QString>(QFileInfo(path).baseName(), regexMatch.capturedTexts().at(1)));
        }
    }

    // Split manually configured and automatically found apps
    for (const QString &path: desktopPaths) {
        // Desktop file is manually specified
        if (mappingMap.contains(path)) {
            auto customMappedApp = new JetbrainsApplication(path, fileWatchers);
            if (QFileInfo::exists(mappingMap.value(path))) {
                customMappedApp->parseXMLFile(mappingMap.value(path));
                // Add path for filewatcher
                customMappedApp->addPath(mappingMap.value(path));
                if (!filterEmpty || !customMappedApp->recentlyUsed.isEmpty()) {
                    appList.append(customMappedApp);
                }
            }
        } else {
            const QString baseName = QFileInfo(path).baseName();
            bool shouldNotTrimEdition = std::any_of(specialEditions.begin(), specialEditions.end(), [baseName](const std::pair<QString, QString> &value){
                return baseName.contains(value.second) && baseName != value.first;
            });
            automaticAppList.append(new JetbrainsApplication(path, fileWatchers, shouldNotTrimEdition));
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
