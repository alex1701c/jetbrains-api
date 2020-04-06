#include "JetbrainsApplication.h"
#include "SettingsDirectory.h"
#include "ConfigKeys.h"

namespace JetbrainsAPI {

    /**
     * Get all applications with manual configuration and file watchers
     * @param config Config group which has the custom mapping configuration
     * By using the config module from the KRunner plugin the config group gets created using:
     * @code KSharedConfig::openConfig(QDir::homePath() + "/.config/krunnerplugins/jetbrainsrunnerrc")->group("Config");
     */
    QList<JetbrainsApplication *> fetchApplications(const KConfigGroup &config, bool filterEmpty = true, bool fileWatchers = true);

    /**
     * Get the JetbrainsApplications without reading the recent projects
     * @param config Config group which has the custom mapping configuration
     */
    QList<JetbrainsApplication *> fetchRawApplications(const KConfigGroup &config);
}
