#include "SettingsDirectory.h"
#include "JetbrainsApplication.h"
#include "macros.h"
#include <utility>

#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>

SettingsDirectory::SettingsDirectory(QString directory, QString name) : directory(std::move(
        directory)), name(std::move(name)) {}

QList<SettingsDirectory> SettingsDirectory::getSettingsDirectories(QString *debugMessage) {
    const QStringList entries = getAllSettingsDirectories();
    QList<SettingsDirectory> dirs;

    // Iterate reversed over entries
    const QRegularExpression configFolderName(R"(\.?([A-Z][a-zA-Z]+)(\d+\.\d+)$)");
    const int maxIndex = entries.size() - 1;
    for (int i = maxIndex; i <= maxIndex && i >= 0; i--) {
        auto const &e = entries.at(i);
        // Contains name and version number live testing => https://regex101.com/r/pMOkox/1
        const QString dirName = QDir(e).dirName();
        const auto regexMatch = configFolderName.match(dirName); // clazy:exclude=use-static-qregularexpression
        if (regexMatch.hasMatch() && regexMatch.lastCapturedIndex()) {
            dirs.append(SettingsDirectory(e, regexMatch.captured(1)));
        } else if (e.contains(QLatin1String(".var/app/com.jetbrains"))) {
            QDir flatpakSettingsDir(e + QLatin1String("/config/JetBrains"));
            const auto flatpakConfigEntries = flatpakSettingsDir.entryInfoList();
            for (const QFileInfo &info : flatpakConfigEntries) {
                auto match = configFolderName.match(info.fileName()); // clazy:exclude=use-static-qregularexpression
                if (match.hasMatch()) {
                    dirs.append(SettingsDirectory(info.absoluteFilePath(), match.captured(1)));
                }
            }
        }
    }
    JBR_FILE_LOG_APPEND("========== Find Available Config Folders ==========\n");
    for (const auto &d: qAsConst(dirs)) {
        JBR_FILE_LOG_APPEND(d.name + " " + d.directory + "\n");
    }
    return dirs;
}

void SettingsDirectory::findCorrespondingDirectory(const QList<SettingsDirectory> &dirs,
        JetbrainsApplication *app,
        QString *debugMessage) {
    app->name = JetbrainsApplication::filterApplicationName(app->name);
    // Exact match or space in name
    for (const auto &dir: qAsConst(dirs)) {
        if (dir.name == app->name) {
            app->configFolder = getExistingConfigDir(dir.directory);
            JBR_FILE_LOG_APPEND(app->name + ' ' + app->configFolder + '\n')
            return;
        }
        if (dir.name == QString(app->name).remove(' ')) {
            app->configFolder = getExistingConfigDir(dir.directory);
            JBR_FILE_LOG_APPEND(app->name + ' ' + app->configFolder + '\n')
            return;
        }
    }

    // Handle Ultimate/Community editions and experimental java runtime
    QMap<QString, QString> aliases = getAliases();
    if (!aliases.contains(app->name)) {
        JBR_FILE_LOG_APPEND(app->name + " is not contained in alias" + '\n')
        return;
    }
    for (const auto &dir: qAsConst(dirs)) {
        if (dir.name == aliases.find(app->name).value()) {
            app->configFolder = getExistingConfigDir(dir.directory);
            JBR_FILE_LOG_APPEND(app->name + ' ' + app->configFolder + ' '+  " from alias" + '\n')
            return;
        }
    }
}


void SettingsDirectory::findCorrespondingDirectories(const QList<SettingsDirectory> &dirs,
                                                     QList<JetbrainsApplication *> &apps,
                                                     QString *debugMessage) {
    for (auto &app: qAsConst(apps)) {
        findCorrespondingDirectory(dirs, app, debugMessage);
    }
}

QMap<QString, QString> SettingsDirectory::getAliases() {
    return {
            {"IntelliJ IDEA Community", "IdeaIC"},
            {"IntelliJ IDEA Ultimate",  "IntelliJIdea"},
            {"PyCharm Professional",    "PyCharm"},
            {"PyCharm Community",       "PyCharmCE"},
            {"Android Studio (Canary Branch)", "AndroidStudioPreview"},
    };
}

QString SettingsDirectory::getExistingConfigDir(const QString &dir) {
    // "/" at end is required for concatenation of filenames
    const QString path = dir + "/config/options/";
    QDir oldDir(path);
    if (oldDir.exists()) {
        return path;
    }
    return dir + "/options/";
}

QStringList SettingsDirectory::getAllSettingsDirectories(QString *debugMessage) {
    const QString home = QDir::homePath();
    const QRegularExpression configFolder(R"(^\.[A-Z][a-zA-Z]+(\d+\.\d+)$)");
    QStringList entries;
    const auto oldConfigLocations = QDir(home).entryList(QDir::Hidden | QDir::Dirs).filter(configFolder);
    for (const auto &e: oldConfigLocations) {
        entries.append(home + '/' + e);
    }

    using QSP = QStandardPaths;
    const QString jetbrainsConfigFolder = QSP::writableLocation(QSP::GenericConfigLocation) + "/JetBrains/";
    const QStringList newConfigLocations = QDir(jetbrainsConfigFolder).entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    for (const QString &entry : newConfigLocations) {
        entries.append(jetbrainsConfigFolder + entry);
    }
    // For Android Studio
    const QString googleConfigFolder = QSP::writableLocation(QSP::GenericConfigLocation) + "/Google/";
    const QStringList googleConfigLocations = QDir(googleConfigFolder).entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    for (const QString &entry : googleConfigLocations) {
        entries.append(googleConfigFolder + entry);
    }

    // For Flatpaks
    const QString flatpakConfigFolder = home + "/.var/app/";
    const QStringList flatpakLocations = QDir(flatpakConfigFolder).entryList({QStringLiteral("com.jetbrains*")}, QDir::NoDotAndDotDot | QDir::Dirs);
    for (const QString &entry : flatpakLocations) {
        entries.append(flatpakConfigFolder + entry);
    }
    JBR_FILE_LOG_APPEND(QString("All settings directories:") + '\n')
    JBR_FILE_LOG_APPEND(entries.join(';') + '\n')
    return entries;
}

