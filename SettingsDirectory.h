#ifndef JETBRAINSRUNNER_SETTINGSDIRECTORY_H
#define JETBRAINSRUNNER_SETTINGSDIRECTORY_H

#include "JetbrainsApplication.h"

class SettingsDirectory
{
public:
    SettingsDirectory(QString directory, QString name);

    /**
     * returns absolute paths of all settings directories that can be found,
     * order of versions is ascending
     */
    static QStringList getAllSettingsDirectories(QString *debugMessage = nullptr);

    /**
     * Get the folders in the home directory that match the pattern o the Jetbrains IDE folder names
     * @param debugMessage optional QString pointer which can be used to debug the parsing process
     */
    static QList<SettingsDirectory> getSettingsDirectories(QString *debugMessage = nullptr);

    /**
     * Set the configFolder property in the app
     * @param dirs
     * @param app
     */
    static void findCorrespondingDirectory(const QList<SettingsDirectory> &dirs, JetbrainsApplication *app, QString *debugMessage);

    /**
     * Overload for findCorrespondingDirectory(const QList<SettingsDirectory> &dirs, JetbrainsApplication *app);
     */
    static void findCorrespondingDirectories(const QList<SettingsDirectory> &dirs, QList<JetbrainsApplication *> &apps, QString *debugMessage = nullptr);

    /**
     * Sometimes the name of the IDE differs from the name of the config directory, for example:
     * IntelliJ IDEA Community => IdeaIC
     * @return QMap of app name and config folder name
     */
    static QMap<QString, QString> getAliases();

    inline static QString getExistingConfigDir(const QString &dir);

    QString directory;
    QString name;
};

#endif // JETBRAINSRUNNER_SETTINGSDIRECTORY_H
