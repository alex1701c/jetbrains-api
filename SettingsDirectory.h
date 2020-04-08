#ifndef JETBRAINSRUNNER_SETTINGSDIRECTORY_H
#define JETBRAINSRUNNER_SETTINGSDIRECTORY_H


#include "JetbrainsApplication.h"

class SettingsDirectory {
public:

    SettingsDirectory(QString directory, QString name);

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
    static void findCorrespondingDirectory(const QList<SettingsDirectory> &dirs, JetbrainsApplication *app);

    /**
     * Overload for findCorrespondingDirectory(const QList<SettingsDirectory> &dirs, JetbrainsApplication *app);
     */
    static void findCorrespondingDirectories(const QList<SettingsDirectory> &dirs, QList<JetbrainsApplication *> &apps);

    /**
     * Sometimes the name of the IDE differs from the name of the config directory, for example:
     * IntelliJ IDEA Community => IdeaIC
     * @return QMap of app name and config folder name
     */
    static QMap<QString, QString> getAliases();


    QString directory;
    QString name;
};


#endif //JETBRAINSRUNNER_SETTINGSDIRECTORY_H
