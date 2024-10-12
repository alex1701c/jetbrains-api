
#include <QObject>
#include <KSycoca>
#include <QDir>
#include <KSharedConfig>
#include <kservice_version.h>
#include <QTest>
#include "../JetbrainsApplication.h"
#include "../export.h"

using QSP = QStandardPaths;
class NameFormattingTest : public QObject {
    Q_OBJECT
private:
    QString dir;
    QString fileTargetUltimate ;
    QString fileTargetCommunity;
private Q_SLOTS:
    void initTestCase() {
        QSP::setTestModeEnabled(true);
#if KSERVICE_VERSION_MAJOR == 6
        KSycoca::self()->setupTestMenu();
#endif

        dir = QSP::writableLocation(QSP::ApplicationsLocation);
        QDir(dir).removeRecursively(); // Clean up leftovers from other test
        QDir(dir).mkpath(QStringLiteral("."));

        fileTargetUltimate =  dir+ QLatin1String("/jetbrains-idea.desktop");
        fileTargetCommunity =  dir+ QLatin1String("/jetbrains-idea-ce.desktop");
        QFile::remove(fileTargetUltimate);
        QFile::remove(fileTargetCommunity);
        qputenv("XDG_DATA_DIR", qPrintable(QSP::writableLocation(QSP::GenericDataLocation)));
    }

    void ensureCacheValid(){
        system("sleep 2"); // Hack for KSycoca not picking up newly copied files
        KSycoca::self()->ensureCacheValid();
    }

    void testNameFormattingWithOnlyOneEdition() {
        QFile::remove(fileTargetCommunity);
        QVERIFY(QFile::copy(QFINDTESTDATA("data/jetbrains-idea-ce.desktop"), fileTargetCommunity));
        ensureCacheValid();

        const auto apps = JetbrainsAPI::fetchApplications(false, false);
        QCOMPARE(apps.count(), 1);
        QCOMPARE(apps.first()->name, "IntelliJ IDEA Community");
        QCOMPARE(apps.first()->shortName, "IntelliJ");

        QVERIFY(QFile::remove(fileTargetCommunity));
    }

    void testNameFormattingWithTwoEditions() {
        QVERIFY(QFile::copy(QFINDTESTDATA("data/jetbrains-idea-ce.desktop"), fileTargetCommunity));
        QVERIFY(QFile::copy(QFINDTESTDATA("data/jetbrains-idea.desktop"), fileTargetUltimate));
        ensureCacheValid();

        const auto apps = JetbrainsAPI::fetchApplications(KSharedConfig::openStateConfig()->group("Test"), false);
        QCOMPARE(apps.count(), 2);
        QCOMPARE(apps.first()->name, "IntelliJ IDEA Community");
        QCOMPARE(apps.first()->shortName, "IntelliJ CE");
        QCOMPARE(apps.at(1)->name, "IntelliJ IDEA Ultimate");
        QCOMPARE(apps.at(1)->shortName, "IntelliJ");

        QVERIFY(QFile::remove(fileTargetUltimate));
        QVERIFY(QFile::remove(fileTargetCommunity));
    }
};


QTEST_MAIN(NameFormattingTest)

#include "NameFormattingTest.moc"
