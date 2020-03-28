#include "JetbrainsApplication.h"
#include "SettingsDirectory.h"
#include "ConfigKeys.h"

namespace JetbrainsAPI {

    /**
     * Get all applications with manual configuration and file watchers
     * @param config Config group which has the custom mapping configuration
     */
    QList<JetbrainsApplication *> fetchApplications(const KConfigGroup &config, bool filterEmpty = true, bool fileWatchers = true);
}
