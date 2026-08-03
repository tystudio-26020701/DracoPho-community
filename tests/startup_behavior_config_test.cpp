#include "startup_behavior_config.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QtTest/QtTest>

class StartupBehaviorConfigTest : public QObject {
    Q_OBJECT

private slots:
    void defaultBehaviorEnablesTrayAndFloatingBall()
    {
        const markshot::StartupBehaviorConfig config = markshot::defaultStartupBehavior();
        QVERIFY(config.configured);
        QVERIFY(config.tray);
        QVERIFY(config.floatingBall);
        QVERIFY(!config.directCapture);
        QVERIFY(!config.settingsWindow);
    }

    void missingModesIsNotConfigured()
    {
        const markshot::StartupBehaviorConfig config = markshot::startupBehaviorFromRoot(QJsonObject());
        QVERIFY(!config.configured);
        QVERIFY(!config.tray);
        QVERIFY(!config.directCapture);
    }

    void emptyModesFallsBackToTray()
    {
        QJsonObject startup;
        startup.insert(QStringLiteral("modes"), QJsonArray());
        QJsonObject root;
        root.insert(QStringLiteral("startup"), startup);

        const markshot::StartupBehaviorConfig config = markshot::startupBehaviorFromRoot(root);
        QVERIFY(config.configured);
        // 空组合归一化为托盘，保证应用始终有入口（与 main 的启动解析兜底一致）。
        QVERIFY(config.tray);
        QVERIFY(!config.directCapture && !config.floatingBall && !config.settingsWindow);
    }

    void parsesCombinedModes()
    {
        QJsonArray modes;
        modes.append(QStringLiteral("capture"));
        modes.append(QStringLiteral("tray"));
        modes.append(QStringLiteral("floating"));
        modes.append(QStringLiteral("settings"));
        QJsonObject startup;
        startup.insert(QStringLiteral("modes"), modes);
        QJsonObject root;
        root.insert(QStringLiteral("startup"), startup);

        const markshot::StartupBehaviorConfig config = markshot::startupBehaviorFromRoot(root);
        QVERIFY(config.configured);
        QVERIFY(config.directCapture);
        QVERIFY(config.tray);
        QVERIFY(config.floatingBall);
        QVERIFY(config.settingsWindow);
    }

    void ignoresUnknownModes()
    {
        QJsonArray modes;
        modes.append(QStringLiteral("tray"));
        modes.append(QStringLiteral("unknown-mode"));
        modes.append(QStringLiteral("floating"));
        QJsonObject startup;
        startup.insert(QStringLiteral("modes"), modes);
        QJsonObject root;
        root.insert(QStringLiteral("startup"), startup);

        const markshot::StartupBehaviorConfig config = markshot::startupBehaviorFromRoot(root);
        QVERIFY(config.configured);
        QVERIFY(config.tray);
        QVERIFY(config.floatingBall);
        QVERIFY(!config.directCapture);
        QVERIFY(!config.settingsWindow);
    }

    void acceptsSingleStringMode()
    {
        QJsonObject startup;
        startup.insert(QStringLiteral("modes"), QStringLiteral("capture"));
        QJsonObject root;
        root.insert(QStringLiteral("startup"), startup);

        const markshot::StartupBehaviorConfig config = markshot::startupBehaviorFromRoot(root);
        QVERIFY(config.configured);
        QVERIFY(config.directCapture);
        QVERIFY(!config.tray);
    }

    void modeNamesRoundTrip()
    {
        QCOMPARE(markshot::startupModeName(markshot::StartupMode::DirectCapture), QStringLiteral("capture"));
        QCOMPARE(markshot::startupModeName(markshot::StartupMode::Tray), QStringLiteral("tray"));
        QCOMPARE(markshot::startupModeName(markshot::StartupMode::FloatingBall), QStringLiteral("floating"));
        QCOMPARE(markshot::startupModeName(markshot::StartupMode::SettingsWindow), QStringLiteral("settings"));
        QVERIFY(markshot::startupModeFromName(QStringLiteral("TRAY")).has_value());
        QVERIFY(!markshot::startupModeFromName(QStringLiteral("bogus")).has_value());
    }
};

QTEST_GUILESS_MAIN(StartupBehaviorConfigTest)

#include "startup_behavior_config_test.moc"
