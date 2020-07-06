#include "JetbrainsApplication.h"
#include "ConfigKeys.h"
#include <KSharedConfig>
#include <KConfigCore/KConfigGroup>
#include <QtGui/QtGui>
#include "macros.h"

JetbrainsApplication::JetbrainsApplication(const QString &desktopFilePath, bool fileWatcher, QObject *parent) :
        QFileSystemWatcher(parent), fileWatcher(fileWatcher), desktopFilePath(desktopFilePath) {
    KConfigGroup config = KSharedConfig::openConfig(desktopFilePath)->group("Desktop Entry");
    iconPath = config.readEntry("Icon");
    executablePath = config.readEntry("Exec").remove(QRegularExpression(QStringLiteral("%\\w")));
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
    if (!nameList.isEmpty()) {
        nameArray[0] = nameList.at(0);
        if (nameList.count() == 2) {
            nameArray[1] = nameList.at(1);
        }
    }
    if (fileWatcher) {
        connect(this, &QFileSystemWatcher::fileChanged, this, &JetbrainsApplication::configChanged);
    }
}


void JetbrainsApplication::parseXMLFile(QString content, QString *debugMessage) {
    // Recent folders are in recentProjectDirectories.xml or in recentProjects.xml located
    // If the method is triggered by the file watcher the content is provided
    if (content.isEmpty()) {
        if (this->configFolder.isEmpty()) return;
        QFile f(this->configFolder + "recentProjectDirectories.xml");
        if (!f.exists()) {
            QFile f2(this->configFolder + "recentProjects.xml");
            if (!f2.open(QIODevice::ReadOnly)) {
                f2.close();
                JBR_FILE_LOG_APPEND("No config file found for " + this->name + " " + this->desktopFilePath + "\n")
                return;
            }
            if (fileWatcher) {
                this->addPath(f2.fileName());
            }
            JBR_FILE_LOG_APPEND("Config file found for " + this->name + " " + f2.fileName() + "\n")
            content = f2.readAll();
            f2.close();
        } else {
            if (f.open(QIODevice::ReadOnly)) {
                content = f.readAll();
                f.close();
                if (fileWatcher) {
                    this->addPath(f.fileName());
                }
                JBR_FILE_LOG_APPEND("Config file found for " + this->name + " " + f.fileName() + "\n")
            }
        }
    }

    if (content.isEmpty()) {
        JBR_FILE_LOG_APPEND(name + " file content is empty \n")
        return;
    }

    // Go to RecentDirectoryProjectsManager component
    QXmlStreamReader reader(content);
    reader.readNextStartElement();
    reader.readNextStartElement();

    // Go through elements until the recentPaths option element is selected
    for (int i = 0; i < 4; ++i) {
        reader.readNextStartElement();
        if (reader.name() != QLatin1String("option")
        || reader.attributes().value(QLatin1String("name")) != QLatin1String("recentPaths")) {
            reader.skipCurrentElement();
        } else {
            reader.name();
            break;
        }
    }

    // Extract paths from XML element
    while (reader.readNextStartElement()) {
        if (reader.name() == QLatin1String("option")) {
            QString recentPath = reader.attributes()
                    .value(QLatin1String("value"))
                    .toString()
                    .replace(QLatin1String("$USER_HOME$"), QDir::homePath());
            if (QDir(recentPath).exists()) {
                Project project;
                project.path = recentPath;
                QFile nameFile(recentPath + QStringLiteral("/.idea/.name"));
                if (nameFile.exists()) {
                    if (nameFile.open(QFile::ReadOnly)) {
                        project.name = nameFile.readAll();
                    }
                }
                if (project.name.isEmpty()) {
                    project.name = recentPath.split('/').last();
                }
                this->recentlyUsed.append(project);
            } else {
                JBR_FILE_LOG_APPEND(name + " the project path does not exist " + recentPath + '\n')
            }
            reader.readElementText();
        }
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

QList<JetbrainsApplication *> JetbrainsApplication::filterApps(QList<JetbrainsApplication *> &apps, QString *debugMessage) {
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
            QStringLiteral("/var/lib/snapd/desktop/applications/intellij-idea-community_intellij-idea-community.desktop"),
            QStringLiteral("/var/lib/snapd/desktop/applications/intellij-idea-educational_intellij-idea-educational.desktop"),
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
    const QDir localJetbrainsApplications(localPath, {QStringLiteral("jetbrains-*")});
    JBR_FILE_LOG_APPEND("========== Locally Installed Jetbrains Applications ==========\n")
    auto entries = localJetbrainsApplications.entryList();
    entries.removeOne(QStringLiteral("jetbrains-toolbox.desktop"));
    for (const auto &item: qAsConst(entries)) {
        if (!item.isEmpty()) {
            applicationPaths.insert(filterApplicationName(KSharedConfig::openConfig(localPath + item)->
                    group("Desktop Entry").readEntry("Name")), localPath + item);
            JBR_FILE_LOG_APPEND(localPath + item + "\n")
        }
    }
    // Globally installed
    const QString globalPath = QStringLiteral("/usr/share/applications/");
    const QDir globalJetbrainsApplications(globalPath, {QStringLiteral("jetbrains-*")});
    JBR_FILE_LOG_APPEND("========== Globally Installed Jetbrains Applications ==========\n")
    auto globalEntries = globalJetbrainsApplications.entryList();
    globalEntries.removeOne(QStringLiteral("jetbrains-toolbox.desktop"));
    for (const auto &item: qAsConst(globalEntries)) {
        if (!item.isEmpty()) {
            applicationPaths.insert(filterApplicationName(KSharedConfig::openConfig(globalPath + item)->
                    group("Desktop Entry").readEntry("Name")), globalPath + item);
            JBR_FILE_LOG_APPEND(globalPath + item + "\n")
        }
    }

    // AUR/Snap/Other  installed
    JBR_FILE_LOG_APPEND("========== AUR/Snap/Other Installed Jetbrains Applications ==========\n")
    const auto additionalEntries = getAdditionalDesktopFileLocations();
    for (const auto &item : additionalEntries) {
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
        JBR_FILE_LOG_APPEND(path.first + " ==> " +path.second + "\n")
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
        txt.replace(QLatin1String(FormatString::DIR), QString(project.path).replace(QDir::homePath(), QLatin1String("~")));
    }
    return txt;
}

QDebug operator<<(QDebug d, const JetbrainsApplication *app)
{
    d   << " name: " << app->name
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
