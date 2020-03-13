namespace JetbrainsAPI {
    /**
     * Get all applications with manual configuration and file watchers
     * @param config
     */
    QList<JetbrainsApplication *> fetchApplications(const KConfigGroup &config) {
        QList<JetbrainsApplication *> appList;
        QList<JetbrainsApplication *> automaticAppList;
        const auto mappingMap = config.group(Config::customMappingGroup).entryMap();
        const auto desktopPaths = JetbrainsApplication::getInstalledApplicationPaths(
                config.group(Config::customMappingGroup));

        // Split manually configured and automatically found apps
        for (const auto &p:desktopPaths.toStdMap()) {
            // Desktop file is manually specified
            if (mappingMap.contains(p.second)) {
                auto customMappedApp = new JetbrainsApplication(p.second);
                QFile xmlConfigFile(mappingMap.value(p.second));
                if (xmlConfigFile.open(QFile::ReadOnly)) {
                    customMappedApp->parseXMLFile(xmlConfigFile.readAll());
                    // Add path for filewatcher
                    customMappedApp->addPath(mappingMap.value(p.second));
                    if (!customMappedApp->recentlyUsed.isEmpty()) {
                        appList.append(customMappedApp);
                    }
                }
                xmlConfigFile.close();
            } else {
                automaticAppList.append(new JetbrainsApplication(p.second));
            }
        }

        // Find automatically config directory, read config file and filter apps
        SettingsDirectory::findCorrespondingDirectories(SettingsDirectory::getSettingsDirectories(), automaticAppList);
        JetbrainsApplication::parseXMLFiles(automaticAppList);
        automaticAppList = JetbrainsApplication::filterApps(automaticAppList);
        appList.append(automaticAppList);
        return appList;
    }
}