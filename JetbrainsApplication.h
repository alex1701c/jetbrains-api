#ifndef JETBRAINSAPPLICATION_H
#define JETBRAINSAPPLICATION_H

#include <QDebug>
#include <QFileSystemWatcher>
#include <QFile>
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
    void parseXMLFile(QString fileContent = QString(), QString *debugMessage = nullptr);

    /**
     * @param apps
     * @param debugMessage optional QString pointer which can be used to debug the parsing process
     * @return list of apps but apps without projects are removed
     */
    static QList<JetbrainsApplication *> filterApps(QList<JetbrainsApplication *> &apps, QString *debugMessage = nullptr);

    /**
     * @return hardcoded list of .desktop files that exist
     */
    static QStringList getAdditionalDesktopFileLocations();

    /**
     * Utility function that removes unwanted parts of the app name
     * @param name
     */
    static QString filterApplicationName(const QString &name);

    /**
     * Replace string variables by the parameters
     */
    QString formatOptionText(const QString &formatText, const QString &dir, const QString &path);

    const bool fileWatcher;
    QList<QString> recentlyUsed;
    QString desktopFilePath;
    QString executablePath;
    QString iconPath;
    QString name;
    QString configFolder;
    QString shortName;
    QString nameArray[2] = {"", ""};

public Q_SLOTS:

    void configChanged(const QString &file) {
        QFile f(file);
        if (f.exists() && f.open(QIODevice::ReadOnly)) {
            this->recentlyUsed.clear();
            QString content = f.readAll();
            this->parseXMLFile(content);
        }
    };
};


#endif //JETBRAINSAPPLICATION_H
