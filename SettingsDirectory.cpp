#include <utility>
#include <QtCore>
#include "SettingsDirectory.h"
#include "JetbrainsApplication.h"
#include "macros.h"


SettingsDirectory::SettingsDirectory(QString directory, QString name) : directory(std::move(
        directory)), name(std::move(name)) {}

QList<SettingsDirectory> SettingsDirectory::getSettingsDirectories(QString *debugMessage) {
    const QString home = QDir::homePath();
    QStringList entries = getAllSettingsDirectories();
    QList<SettingsDirectory> dirs;

    // Iterate reversed over entries
    QRegularExpression configFolderName(R"(\.?([A-Z][a-zA-Z]+)(\d+\.\d+)$)");
    const int maxIndex = entries.size() - 1;
    for (int i = maxIndex; i <= maxIndex && i >= 0; i--) {
        auto const &e = entries.at(i);
        // Contains name and version number live testing => https://regex101.com/r/pMOkox/1
        const auto regexMatch = configFolderName.match(QDir(e).dirName());
        if (regexMatch.hasMatch() && regexMatch.lastCapturedIndex()) {
            dirs.append(SettingsDirectory(e, regexMatch.captured(1)));
        }

    }
    JBR_FILE_LOG_APPEND("========== Find Available Config Folders ==========\n");
#ifndef NO_JBR_FILE_LOG
    for (const auto &d: qAsConst(dirs)) {
        JBR_FILE_LOG_APPEND(d.name + " " + d.directory + "\n");
    }
#endif
    return dirs;
}

void SettingsDirectory::findCorrespondingDirectory(const QList<SettingsDirectory> &dirs, JetbrainsApplication *app) {
    app->name = JetbrainsApplication::filterApplicationName(app->name);
    // Exact match or space in name
    for (const auto &dir: qAsConst(dirs)) {
        if (dir.name == app->name) {
            app->configFolder = getExistingConfigDir(dir.directory);
            return;
        }
        if (dir.name == QString(app->name).remove(' ')) {
            app->configFolder = getExistingConfigDir(dir.directory);
            return;
        }
    }

    // Handle Ultimate/Community editions and experimental java runtime
    QMap<QString, QString> aliases = getAliases();
    if (!aliases.contains(app->name)) {
        return;
    }
    for (const auto &dir: qAsConst(dirs)) {
        if (dir.name == aliases.find(app->name).value()) {
            app->configFolder = getExistingConfigDir(dir.directory);
            return;
        }
    }
}


void SettingsDirectory::findCorrespondingDirectories(const QList<SettingsDirectory> &dirs,
                                                     QList<JetbrainsApplication *> &apps) {
    for (auto &app: qAsConst(apps)) {
        findCorrespondingDirectory(dirs, app);
    }
}

QMap<QString, QString> SettingsDirectory::getAliases() {
    return {
            {"IntelliJ IDEA Community", "IdeaIC"},
            {"IntelliJ IDEA Ultimate",  "IntelliJIdea"},
            {"PyCharm Professional",    "PyCharm"},
            {"PyCharm Community",       "PyCharmCE"}
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

QStringList SettingsDirectory::getAllSettingsDirectories() {
    const QString home = QDir::homePath();
    const QRegularExpression configFolder(R"(^\.[A-Z][a-zA-Z]+(\d+\.\d+)$)");
    QStringList entries;
    const auto oldConfigLocations = QDir(home).entryList(QDir::Hidden | QDir::Dirs).filter(configFolder);
    for (const auto &e: oldConfigLocations) {
        entries.append(home + '/' + e);
    }

    const auto newConfigLocations = QDir(home + "/.config/JetBrains").entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    for (const auto &entry: newConfigLocations) {
        entries.append(home + "/.config/JetBrains/" + entry);
    }
    return entries;
}

