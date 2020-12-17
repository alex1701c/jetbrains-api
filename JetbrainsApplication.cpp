#include "JetbrainsApplication.h"
#include "ConfigKeys.h"
#include <KSharedConfig>
#include <KConfigCore/KConfigGroup>
#include <QtGui/QtGui>
#include <QtXml/QDomDocument>
#include "macros.h"

JetbrainsApplication::JetbrainsApplication(const QString &desktopFilePath, bool fileWatcher) :
        QFileSystemWatcher(nullptr), fileWatcher(fileWatcher), desktopFilePath(desktopFilePath) {
    KConfigGroup config = KSharedConfig::openConfig(desktopFilePath)->group("Desktop Entry");
    iconPath = config.readEntry("Icon");
    executablePath = config.readEntry("Exec").remove("%u").remove("%f");
    name = config.readEntry("Name");
    shortName = QString(name)
            .remove(QLatin1String(" Edition"))
            .remove(QLatin1String(" Professional"))
            .remove(QLatin1String(" Ultimate"))
            .remove(QLatin1String(" + JBR11"))
            .remove(QLatin1String(" RC"))
            .remove(QLatin1String(" EAP"))
            .replace(QLatin1String("Community"), QLatin1String("CE"));

    // Allow the user to search for both names like Android Studio
    auto nameList = filterApplicationName(QString(name))
            .remove(QLatin1String(" Professional"))
            .remove(QLatin1String(" Community"))
            .remove(QLatin1String(" Ultimate"))
            .remove(QLatin1String(" Edu"))
            .split(' ');
    if (nameList.count() > 0) {
        nameArray[0] = nameList.at(0);
        if (nameList.count() == 2) {
            nameArray[1] = nameList.at(1);
        }
    }
    if (fileWatcher) {
        connect(this, &QFileSystemWatcher::fileChanged, this, &JetbrainsApplication::configChanged);
    }
}

void JetbrainsApplication::parseXMLFile(const QString &file, QString *debugMessage) {
    // Recent folders are in recentProjectDirectories.xml or in recentProjects.xml located
    // If the method is triggered by the file watcher the content is provided
    QString fileName = file;
    if (fileName.isEmpty()) {
        if (this->configFolder.isEmpty()) {
            return;
        }
        if (QFileInfo::exists(this->configFolder + "recentProjectDirectories.xml")) {
            fileName = this->configFolder + "recentProjectDirectories.xml";
        }
        if (QFileInfo::exists(this->configFolder + "recentProjects.xml")) {
            fileName = this->configFolder + "recentProjects.xml";
        }
        // JetBrains Rider has a differently called file
        if (QFileInfo::exists(this->configFolder + "recentSolutions.xml")) {
            fileName = this->configFolder + "recentSolutions.xml";
        }
        if (fileName.isEmpty()) {
            JBR_FILE_LOG_APPEND("No config file found for " + this->name + " " + this->desktopFilePath + "\n")
            return;
        }
        JBR_FILE_LOG_APPEND("Config file found for " + this->name + " " + fileName + "\n")
    }
    JBR_FILE_LOG_APPEND("Config file found for " + this->name + " " + fileName + "\n")
    if (fileWatcher) {
        this->addPath(fileName);
    }

    if (QFileInfo(fileName).size() == 0) {
        JBR_FILE_LOG_APPEND(name + " file content is empty \n")
        return;
    }

    parseOldStyleXMLFile(fileName);
    if (recentlyUsed.isEmpty()) {
        parseNewStyleXMLFile(fileName);
    }

    JBR_FILE_LOG_APPEND("===== Recently used project folder for " + this->name + "=====\n")
    if (!recentlyUsed.isEmpty()) {
        for (const auto &recent: qAsConst(recentlyUsed)) {
            Q_UNUSED(recent)
            JBR_FILE_LOG_APPEND("    " + recent.path + "\n")
        }
    } else {
        JBR_FILE_LOG_APPEND("    NO PROJECTS\n")
    }
    JBR_FILE_LOG_APPEND("\n")
}

void JetbrainsApplication::parseXMLFiles(QList<JetbrainsApplication *> &apps, QString *debugMessage) {
    for (auto &app: qAsConst(apps)) {
        app->parseXMLFile(QString(), debugMessage);
    }
}

QList<JetbrainsApplication *>
JetbrainsApplication::filterApps(QList<JetbrainsApplication *> &apps, QString *debugMessage) {
    QList<JetbrainsApplication *> notEmpty;
    JBR_FILE_LOG_APPEND("========== Filter Jetbrains Apps ==========\n")
    for (auto const &app: qAsConst(apps)) {
        if (!app->recentlyUsed.empty()) {
            notEmpty.append(app);
        } else {
            JBR_FILE_LOG_APPEND("Found not projects for: " + app->name + "\n")
            delete app;
        }
    }
    return notEmpty;
}

QStringList JetbrainsApplication::getAdditionalDesktopFileLocations() {
    const QStringList additionalDesktopFileLocations = {
            // AUR applications
            QStringLiteral("/usr/share/applications/rubymine.desktop"),
            QStringLiteral("/usr/share/applications/pycharm-professional.desktop"),
            QStringLiteral("/usr/share/applications/pycharm-eap.desktop"),
            QStringLiteral("/usr/share/applications/charm.desktop"),
            QStringLiteral("/usr/share/applications/rider.desktop"),
            // Snap applications
            QStringLiteral("/var/lib/snapd/desktop/applications/clion_clion.desktop"),
            QStringLiteral("/var/lib/snapd/desktop/applications/datagrip_datagrip.desktop"),
            QStringLiteral("/var/lib/snapd/desktop/applications/goland_goland.desktop"),
            QStringLiteral("/var/lib/snapd/desktop/applications/pycharm-community_pycharm-community.desktop"),
            QStringLiteral("/var/lib/snapd/desktop/applications/pycharm-educational_pycharm-educational.desktop"),
            QStringLiteral("/var/lib/snapd/desktop/applications/pycharm-professional_pycharm-professional.desktop"),
            QStringLiteral("/var/lib/snapd/desktop/applications/rubymine_rubymine.desktop"),
            QStringLiteral("/var/lib/snapd/desktop/applications/webstorm_webstorm.desktop"),
            QStringLiteral(
                    "/var/lib/snapd/desktop/applications/intellij-idea-community_intellij-idea-community.desktop"),
            QStringLiteral(
                    "/var/lib/snapd/desktop/applications/intellij-idea-educational_intellij-idea-educational.desktop"),
            QStringLiteral("/var/lib/snapd/desktop/applications/intellij-idea-ultimate_intellij-idea-ultimate.desktop"),
            QStringLiteral("/var/lib/snapd/desktop/applications/phpstorm_phpstorm.desktop"),
            QStringLiteral("/var/lib/snapd/desktop/applications/rider_rider.desktop"),
    };
    QStringList validFiles;
    for (const auto &additionalFile: additionalDesktopFileLocations) {
        if (QFile::exists(additionalFile)) {
            validFiles.append(additionalFile);
        }
    }

    return validFiles;
}

QMap<QString, QString>
JetbrainsApplication::getInstalledApplicationPaths(const KConfigGroup &customMappingConfig, QString *debugMessage) {
    QMap<QString, QString> applicationPaths;

    // Manually, locally or with Toolbox installed
    const QString localPath = QDir::homePath() + "/.local/share/applications/";
    const QDir localJetbrainsApplications(localPath, {"jetbrains-*"});
    JBR_FILE_LOG_APPEND("========== Locally Installed Jetbrains Applications ==========\n")
    auto entries = localJetbrainsApplications.entryList();
    entries.removeOne("jetbrains-toolbox.desktop");
    for (const auto &item: qAsConst(entries)) {
        if (!item.isEmpty()) {
            applicationPaths.insert(filterApplicationName(KSharedConfig::openConfig(localPath + item)->
                    group("Desktop Entry").readEntry("Name")), localPath + item);
            JBR_FILE_LOG_APPEND(localPath + item + "\n")
        }
    }
    // Globally installed
    const QString globalPath = "/usr/share/applications/";
    const QDir globalJetbrainsApplications(globalPath, {"jetbrains-*"});
    JBR_FILE_LOG_APPEND("========== Globally Installed Jetbrains Applications ==========\n")
    auto globalEntries = globalJetbrainsApplications.entryList();
    globalEntries.removeOne("jetbrains-toolbox.desktop");
    for (const auto &item: qAsConst(globalEntries)) {
        if (!item.isEmpty()) {
            applicationPaths.insert(filterApplicationName(KSharedConfig::openConfig(globalPath + item)->
                    group("Desktop Entry").readEntry("Name")), globalPath + item);
            JBR_FILE_LOG_APPEND(globalPath + item + "\n")
        }
    }

    // AUR/Snap/Other  installed
    JBR_FILE_LOG_APPEND("========== AUR/Snap/Other Installed Jetbrains Applications ==========\n")
    for (const auto &item : getAdditionalDesktopFileLocations()) {
        if (!item.isEmpty()) {
            applicationPaths.insert(filterApplicationName(KSharedConfig::openConfig(item)->
                    group("Desktop Entry").readEntry("Name")), item);
            JBR_FILE_LOG_APPEND(item + "\n")
        }
    }

    // Add manually configured entries
    JBR_FILE_LOG_APPEND("========== Manually Configured Jetbrains Applications ==========\n")
    if (!customMappingConfig.isValid()) {
        return applicationPaths;
    }
    for (const auto &mappingEntry: customMappingConfig.entryMap().toStdMap()) {
        if (QFile::exists(mappingEntry.first) && QFile::exists(mappingEntry.second)) {
            applicationPaths.insert(filterApplicationName(KSharedConfig::openConfig(mappingEntry.first)->
                    group("Desktop Entry").readEntry("Name")), mappingEntry.first);
            JBR_FILE_LOG_APPEND(mappingEntry.first + "\n")
            JBR_FILE_LOG_APPEND(mappingEntry.second + "\n")
        }
    }
    JBR_FILE_LOG_APPEND("Application path map: \n")
    for (const auto &path : applicationPaths.toStdMap()) {
        Q_UNUSED(path)
        JBR_FILE_LOG_APPEND(path.first + " ==> " + path.second + "\n")
    }
    return applicationPaths;
}

QString JetbrainsApplication::filterApplicationName(const QString &name) {
    return QString(name)
            .remove(QLatin1String(" Release"))
            .remove(QLatin1String(" Edition"))
            .remove(QLatin1String(" + JBR11"))
            .remove(QLatin1String(" RC"))
            .remove(QLatin1String(" EAP"));
}

QString JetbrainsApplication::formatOptionText(const QString &formatText, const Project &project) {
    QString txt = QString(formatText)
            .replace(QLatin1String(FormatString::PROJECT), project.name)
            .replace(QLatin1String(FormatString::APPNAME), this->name)
            .replace(QLatin1String(FormatString::APP), this->shortName);
    if (txt.contains(QLatin1String(FormatString::DIR))) {
        txt.replace(QLatin1String(FormatString::DIR),
                    QString(project.path).replace(QDir::homePath(), QLatin1String("~")));
    }
    return txt;
}

QDebug operator<<(QDebug d, const JetbrainsApplication *app) {
    d << " name: " << app->name
      << " desktopFilePath: " << app->desktopFilePath
      << " executablePath: " << app->executablePath
      << " configFolder: " << app->configFolder
      << " iconPath: " << app->iconPath
      << " shortName: " << app->shortName
      << " recentlyUsed: ";
    for (const auto &project : qAsConst(app->recentlyUsed)) {
        d << project.name << project.path;
    }
    return d;
}

void JetbrainsApplication::parseOldStyleXMLFile(const QString &fileName)
{
    QDomDocument d;
    QFile xmlFile(fileName);
    d.setContent(&xmlFile);
    const QDomNodeList list = d.elementsByTagName(QStringLiteral("option"));
    for (int i = 0; i < list.count(); ++i) {
        const QDomNode optionNode = list.at(i);
        if (!optionNode.isElement()) {
            continue;
        }
        const QDomElement option = optionNode.toElement();
        if (option.attribute("name") == QLatin1String("recentPaths")) {
            const QDomNodeList recentPathsList = option.elementsByTagName("list");
            if (recentPathsList.isEmpty()) {
                continue;
            }
            const QDomNodeList recentPathNodes = recentPathsList.at(0).childNodes();
            for (int j = 0; j < recentPathNodes.count(); ++j) {
                addRecentlyUsed(recentPathNodes.at(j).toElement().attribute(QStringLiteral("value")));
            }
        }
    }
}

void JetbrainsApplication::parseNewStyleXMLFile(const QString &fileName)
{
    // Initialize variables and find element containing the data
    QDomDocument d;
    QFile xmlFile(fileName);
    d.setContent(&xmlFile);
    QDomNodeList recentPathsMap;
    QList<QPair<QString, double>> pathTimestampMap;
    const auto component = d.elementsByTagName(QStringLiteral("option"));
    for (int i = 0; i < component.count(); ++i) {
        if (component.at(i).isElement() && component.at(i).toElement().attribute("name") == "additionalInfo") {
            recentPathsMap = component.at(i).firstChild().childNodes();
        }
    }

    // Parse the data
    for (int i = 0; i < recentPathsMap.count(); ++i) {
        const QDomNode _entry = recentPathsMap.at(i);
        if (!_entry.isElement()) {
            continue;
        }
        const QDomElement entry = _entry.toElement();
        const QString path = entry.attribute("key");
        const QDomElement valueEntry = entry.firstChildElement("value");
        const QDomElement metaInfo = valueEntry.firstChildElement("RecentProjectMetaInfo");
        double projectOpenTimestamp = metaInfo.lastChildElement("option").attribute("value").toDouble();
        pathTimestampMap.append(qMakePair(path, projectOpenTimestamp));
    }
    std::sort(pathTimestampMap.begin(), pathTimestampMap.end(),
          [](QPair<QString, double> &first, QPair<QString, double> &second) { return first.second > second.second; });
    for (const auto &pair : qAsConst(pathTimestampMap)) {
        addRecentlyUsed(pair.first);
    }
}

void JetbrainsApplication::addRecentlyUsed(const QString &path)
{
    QString recentPath = QString(path).replace(QLatin1String("$USER_HOME$"), QDir::homePath());
    if (!checkIfProjectsExist || QFileInfo::exists(recentPath)) {
        Project project;
        project.path = recentPath;
        QString projectRootPath = recentPath;
        if (QFileInfo(projectRootPath).isFile()) {
            projectRootPath = QFileInfo(projectRootPath).dir().absolutePath();
        }
        QFile nameFile(projectRootPath + QStringLiteral("/.idea/.name"));
        if (nameFile.exists()) {
            if (nameFile.open(QFile::ReadOnly)) {
                project.name = nameFile.readAll();
            }
        }
        if (project.name.isEmpty()) {
            project.name = projectRootPath.split('/').last();
        }
        this->recentlyUsed.append(project);
    }
}
