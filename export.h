#include "JetbrainsApplication.h"
#include "SettingsDirectory.h"
#include "ConfigKeys.h"
#include "Project.h"

namespace JetbrainsAPI {

    /**
     * Get all applications with manual configuration and file watchers
     * @param config Config group which has the custom mapping configuration
     * By using the config module from the KRunner plugin the config group gets created using:
     * @code KSharedConfig::openConfig(QStringLiteral("krunnerplugins/jetbrainsrunnerrc"))->group("Config");
     */
    QList<JetbrainsApplication *> fetchApplications(const KConfigGroup &config, bool filterEmpty = true, bool fileWatchers = true);

    /**
     * Get the installed applications but don't use the manual mapping which can be configured in the
     * KRunner settings module https://github.com/alex1701c/JetBrainsRunner
     * @param filterEmpty remove apps that have no recent projects from the list
     * @param fileWatchers use file watchers to reparse the recent projects when they change
     */
    QList<JetbrainsApplication *> fetchApplications(bool filterEmpty = true, bool fileWatchers = true);

    /**
     * Get the JetbrainsApplications without reading the recent projects
     * @param config Config group which has the custom mapping configuration
     */
    QList<JetbrainsApplication *> fetchRawApplications(const KConfigGroup &config);

    /**
     * Overload for fetchRawApplications(const KConfigGroup &config)
     */
    QList<JetbrainsApplication *> fetchRawApplications();
}
