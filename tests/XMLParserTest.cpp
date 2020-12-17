#include <QObject>
#include <QtTest/QtTest>
#include "../JetbrainsApplication.h"

class XMLParserTest : public QObject {
Q_OBJECT
public:
    XMLParserTest(QObject *parent = nullptr) : QObject(parent) {}
    ~XMLParserTest() {};

private Q_SLOTS:
    void testRiderXMLParsing(){
        JetbrainsApplication application(QFINDTESTDATA("data/jetbrains-clion.desktop"));
        application.checkIfProjectsExist = false;
        QCOMPARE(application.name, QStringLiteral("CLion"));
        application.configFolder = QFINDTESTDATA("data/jetbrains-rider/");
        application.parseXMLFile();
        const QList<Project> projects = application.recentlyUsed;
        QVERIFY(!projects.isEmpty());
        QStringList expected = {
          "$USER_HOME$/Projects/Blazor/ComponentLibrary/ComponentLibrary.sln",
          "$USER_HOME$/UnityProjects/Pentecost/Pentecost.sln",
          "$USER_HOME$/UnityProjects/tower-defense/tower-defense.sln"
        };
        QCOMPARE(projects.size(), expected.size());
        for(int i = 0; i < projects.size(); ++i) {
            QString expectedPath = expected.at(i);
            expectedPath.replace("$USER_HOME$", QDir::homePath());
            QCOMPARE(projects.at(i).path, expectedPath);
        }
    };
    void testCLionXMLParsing_data(){
        QTest::addColumn<QString>("configFolder");
        QTest::newRow("old format") << QFINDTESTDATA("data/jetbrains-clion/");
        QTest::newRow("new format") << QFINDTESTDATA("data/jetbrains-clion2020.3/");
    }
    void testCLionXMLParsing(){
        QFETCH(QString, configFolder);
        JetbrainsApplication application(QFINDTESTDATA("data/jetbrains-clion.desktop"));
        application.checkIfProjectsExist = false;
        QCOMPARE(application.name, QStringLiteral("CLion"));
        application.configFolder = configFolder;
        application.parseXMLFile();
        const QList<Project> projects = application.recentlyUsed;
        QVERIFY(!projects.isEmpty());
        QStringList expected = {
            "$USER_HOME$/projects/JetBrainsRunner/src/jetbrains-api",
            "$USER_HOME$/kde/src/knewstuff",
            "$USER_HOME$/kde/src/parley",
            "$USER_HOME$/kde/src/plasma-workspace",
            "$USER_HOME$/kde/src/krunner",
            "$USER_HOME$/kde/src/dolphin",
            "$USER_HOME$/kde/src/plasma-desktop",
            "$USER_HOME$/kde/src/kio",
            "$USER_HOME$/kde/src/kdevelop/app_templates/kdevelop",
            "$USER_HOME$/kde/src/discover",
            "$USER_HOME$/kde/src/kdeplasma-addons",
        };
        QCOMPARE(projects.size(), expected.size());
        for(int i = 0; i < projects.size(); ++i) {
            QString expectedPath = expected.at(i);
            expectedPath.replace("$USER_HOME$", QDir::homePath());
            QCOMPARE(projects.at(i).path, expectedPath);
        }
    };
};
QTEST_MAIN(XMLParserTest)

#include "XMLParserTest.moc"
