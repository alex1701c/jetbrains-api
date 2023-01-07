#include "JetbrainsApplication.h"
#include "ConfigKeys.h"
#include <KSharedConfig>
#include <KApplicationTrader>
#include <KConfigCore/KConfigGroup>
#include <QtGui/QtGui>
#include <QMetaType>
#include <QtXml/QDomDocument>
#include "macros.h"

#include "kservice_version.h"

JetbrainsApplication::JetbrainsApplication(const QString &desktopFilePath, bool fileWatcher, bool shouldNotTrimEdition) :
        QFileSystemWatcher(nullptr), fileWatcher(fileWatcher), desktopFilePath(desktopFilePath) {
    KConfigGroup config = KSharedConfig::openConfig(desktopFilePath)->group("Desktop Entry");
    iconPath = config.readEntry("Icon");
    executablePath = config.readEntry("Exec")
                         .remove("%u")
                         .remove("%f") // Remove placeholders
                         .trimmed(); // Trim leftover whitespaces
    name = filterApplicationName(config.readEntry("Name"));
    shortName = QString(name)
            .remove(QLatin1String(" Edition"))
            .remove(QLatin1String(" Professional"))
            .remove(QLatin1String(" Ultimate"))
            .remove(QLatin1String(" + JBR11"))
            .remove(QLatin1String(" RC"))
            .remove(QLatin1String(" EAP"))
            .replace(QLatin1String("IntelliJ IDEA"), QLatin1String("IntelliJ"))
            .replace(QLatin1String(" Community"), shouldNotTrimEdition ? QStringLiteral(" CE") : QString());

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
            JBR_FILE_LOG_APPEND("No config file found for " + this->name + " " + this->desktopFilePath)
            return;
        }
        JBR_FILE_LOG_APPEND("Config file found for " + this->name + " " + fileName)
    }
    if (fileWatcher) {
        this->addPath(fileName);
    }

    if (QFileInfo(fileName).size() == 0) {
        JBR_FILE_LOG_APPEND("File " + fileName + "for app " + name + " has empty content")
        return;
    }

    parseOldStyleXMLFile(fileName);
    if (recentlyUsed.isEmpty()) {
        parseNewStyleXMLFile(fileName);
    }

    JBR_FILE_LOG_APPEND("===== Recently used project folder for " + this->name + "=====")
    if (!recentlyUsed.isEmpty()) {
        for (const auto &recent: qAsConst(recentlyUsed)) {
            Q_UNUSED(recent)
            JBR_FILE_LOG_APPEND("    " + recent.path)
        }
    } else {
        JBR_FILE_LOG_APPEND("    NO PROJECTS")
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
    JBR_FILE_LOG_APPEND("========== Filter Jetbrains Apps ==========")
    for (auto const &app: qAsConst(apps)) {
        if (!app->recentlyUsed.empty()) {
            notEmpty.append(app);
        } else {
            JBR_FILE_LOG_APPEND("Found not projects for: " + app->name)
            delete app;
        }
    }
    return notEmpty;
}

QMap<QString, QString>
JetbrainsApplication::getInstalledApplicationPaths(const KConfigGroup &customMappingConfig, QString *debugMessage) {
    QMap<QString, QString> applicationPaths;
    const KService::List apps = KApplicationTrader::query([debugMessage](const KService::Ptr &service) {
        // Only take test files into account
        if (QStandardPaths::isTestModeEnabled()) {
            if (!service->entryPath().contains(".qttest")) {
                return false;
            }
        }

        if (service->entryPath().contains("jetbrains-toolbox")) {
            return false;
        }
        if (service->entryPath().contains("jetbrains-") || service->entryPath().contains("com.jetbrains.")) {
            JBR_FILE_LOG_APPEND("Found " + service->entryPath() + " based on entry path")
            return true;
        }
#if KSERVICE_VERSION >= QT_VERSION_CHECK(5, 102, 0)
        const auto stringVariant = QMetaType::QString;
#else
        const auto stringVariant = QVariant::String;
#endif
        if (service->property("StartupWMClass", stringVariant).toString().startsWith("jetbrains-")) {
            JBR_FILE_LOG_APPEND("Found " + service->entryPath() + " based on StartupWMClass")
            return true;
        }
        return false;
    });

    for (const auto &service : apps) {
        applicationPaths.insert(filterApplicationName(service->name()), service->entryPath());
    }

    // Add manually configured entries
    if (!customMappingConfig.isValid()) {
        return applicationPaths;
    }
    if (!customMappingConfig.keyList().isEmpty()) {
        JBR_FILE_LOG_APPEND("========== Manually Configured Jetbrains Applications ==========")
        for (const auto &mappingEntry : customMappingConfig.entryMap().toStdMap()) {
            if (QFile::exists(mappingEntry.first) && QFile::exists(mappingEntry.second)) {
                applicationPaths.insert(filterApplicationName(KService(mappingEntry.first).name()), mappingEntry.first);
                JBR_FILE_LOG_APPEND(mappingEntry.first)
                JBR_FILE_LOG_APPEND(mappingEntry.second)
            }
        }
    }
    JBR_FILE_LOG_APPEND("Application path map:")
    for (const auto &path : applicationPaths.toStdMap()) {
        Q_UNUSED(path)
        JBR_FILE_LOG_APPEND(path.first + " ==> " + path.second)
    }
    return applicationPaths;
}

QString JetbrainsApplication::filterApplicationName(const QString &name) {
    const static QRegularExpression versionPostfixRegex(QStringLiteral(" \\d{4}.\\d(.\\d)?"));
    return QString(name)
            .remove(QLatin1String(" Release"))
            .remove(QLatin1String(" Edition"))
            .remove(QLatin1String(" + JBR11"))
            .remove(QLatin1String(" RC"))
            .remove(QLatin1String(" EAP"))
            .remove(versionPostfixRegex);
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
