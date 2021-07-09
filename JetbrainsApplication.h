#ifndef JETBRAINSAPPLICATION_H
#define JETBRAINSAPPLICATION_H

#include <QDebug>
#include "Project.h"
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QDomNodeList>
#include <KConfigCore/KConfigGroup>

class JetbrainsApplication : public QFileSystemWatcher {
Q_OBJECT
public:
    explicit JetbrainsApplication(const QString &desktopFilePath, bool fileWatcher = true);

    /**
     * Utility function that allows you to debug the apps using
     * @code qInfo() << app;
     */
    friend QDebug operator << (QDebug d, const JetbrainsApplication *app);

    /**
     * @param customMappingConfig
     * @param debugMessage optional QString pointer which can be used to debug the parsing process
     * @return all paths to the .desktop files of the installed Jetbrains IDEs
     */
    static QMap<QString, QString>
    getInstalledApplicationPaths(const KConfigGroup &customMappingConfig, QString *debugMessage = nullptr);

    /**
     * Utility function that calls the parseXMLFile function for each app
     * @param apps
     * @param debugMessage optional QString pointer which can be used to debug the parsing process
     */
    static void parseXMLFiles(QList<JetbrainsApplication *> &apps, QString *debugMessage = nullptr);

    /**
     *
     * @param fileContent method parses the fileContent QString, if it is empty the
     * recentProjectDirectories.xml or recentProjects.xml in the apps config dir get parsed.
     * @param debugMessage optional QString pointer which can be used to debug the parsing process
     */
    void parseXMLFile(const QString &fileName = QString(), QString *debugMessage = nullptr);

    /**
     * @param apps
     * @param debugMessage optional QString pointer which can be used to debug the parsing process
     * @return list of apps but apps without projects are removed
     */
    static QList<JetbrainsApplication *> filterApps(QList<JetbrainsApplication *> &apps, QString *debugMessage = nullptr);

    /**
     * Utility function that removes unwanted parts of the app name
     * @param name
     */
    static QString filterApplicationName(const QString &name);

    /**
     * Replace string variables by the parameters
     */
    QString formatOptionText(const QString &formatText, const Project &project);

    const bool fileWatcher;
    QList<Project> recentlyUsed;
    QString desktopFilePath;
    QString executablePath;
    QString iconPath;
    QString name;
    QString configFolder;
    QString shortName;
    QString nameArray[2] = {"", ""};
    bool checkIfProjectsExist = true; // for test purposes

public Q_SLOTS:

    void configChanged(const QString &file) {
        if (QFileInfo::exists(file)) {
            this->recentlyUsed.clear();
            this->parseXMLFile(file);
        }
    };
private:
    void parseOldStyleXMLFile(const QString &fileName);
    void parseNewStyleXMLFile(const QString &fileName);
    void addRecentlyUsed(const QString &path);
};


#endif //JETBRAINSAPPLICATION_H
