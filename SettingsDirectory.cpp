#include <utility>
#include <QtCore>
#include "SettingsDirectory.h"
#include "JetbrainsApplication.h"


SettingsDirectory::SettingsDirectory(QString directory, QString name) : directory(std::move(
        directory)), name(std::move(name)) {}

QList<SettingsDirectory> SettingsDirectory::getSettingsDirectories(QString *debugMessage) {
    const QString home = QDir::homePath();
    QList<SettingsDirectory> dirs;
    const QStringList entries = QDir(home).entryList(QDir::Hidden | QDir::Dirs);

    // Iterate reversed over entries
    const int maxIndex = entries.size() - 1;
    QRegularExpression configFolder(R"(^\.[A-Z][a-zA-Z]+(\d+\.\d+)$)");
    QRegularExpression configFolderName(R"(^\.([A-Z][a-zA-Z]+)(\d+\.\d+)$)");
    for (int i = maxIndex; i <= maxIndex && i >= 0; i--) {
        auto const &e = entries.at(i);
        // Contains name and version number live testing => https://regex101.com/r/pMOkox/1
        if (e.contains(configFolder)) {
            const auto regexMatch = configFolderName.match(e);
            dirs.append(SettingsDirectory(home + "/" + e, regexMatch.captured(1)));
        }
    }
    if (debugMessage) {
        debugMessage->append("========== Find Available Config Folders ==========\n");
        for (const auto &d: qAsConst(dirs)) {
            debugMessage->append(d.name + " " + d.directory + "\n");
        }
    }
    return dirs;
}

void SettingsDirectory::findCorrespondingDirectory(const QList<SettingsDirectory> &dirs, JetbrainsApplication *app) {
    app->name = JetbrainsApplication::filterApplicationName(app->name);
    // Exact match or space in name
    for (const auto &dir: qAsConst(dirs)) {
        if (dir.name == app->name) {
            app->configFolder = dir.directory + "/config/options/";
            return;
        }
        if (dir.name == QString(app->name).remove(" ")) {
            app->configFolder = dir.directory + "/config/options/";
            return;
        }
    }

    // Handle Ultimate/Community editions and experimental java runtime
    QMap<QString, QString> aliases = getAliases();
    if (aliases.count(app->name) == 0) return;
    for (const auto &dir: qAsConst(dirs)) {
        if (dir.name == aliases.find(app->name).value()) {
            app->configFolder = dir.directory + "/config/options/";
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
