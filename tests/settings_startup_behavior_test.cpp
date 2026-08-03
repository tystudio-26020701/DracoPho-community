#include "app_config_store.h"
#include "settings/settings_config.h"
#include "startup_behavior_config.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest/QtTest>

namespace {

/// @brief 在独立临时目录下隔离 XDG_CONFIG_HOME，避免触碰真实用户配置。
class IsolatedConfigScope {
public:
    bool init()
    {
        if (!m_dir.isValid()) {
            return false;
        }
        const QString configDir = m_dir.path() + QStringLiteral("/mark-shot");
        if (!QDir().mkpath(configDir)) {
            return false;
        }
        QFile file(configDir + QStringLiteral("/config.json"));
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }
        file.write("{}");
        file.close();
        return qputenv("XDG_CONFIG_HOME", m_dir.path().toUtf8());
    }

private:
    QTemporaryDir m_dir;
};

}  // namespace

class SettingsStartupBehaviorTest : public QObject {
    Q_OBJECT

private slots:
    void readMapsStartupModesToFields()
    {
        IsolatedConfigScope scope;
        QVERIFY(scope.init());

        QJsonArray modes;
        modes.append(QStringLiteral("capture"));
        modes.append(QStringLiteral("tray"));
        modes.append(QStringLiteral("floating"));
        modes.append(QStringLiteral("settings"));
        QString error;
        QVERIFY(markshot::writeAppConfigValue(
            {QStringLiteral("startup"), QStringLiteral("modes")}, modes, &error));

        const markshot::settings::SettingsConfig config = markshot::settings::readSettingsConfig(&error);
        QVERIFY(config.general.startupDirectCapture);
        QVERIFY(config.general.startupTray);
        QVERIFY(config.general.startupFloatingBall);
        QVERIFY(config.general.startupSettings);
    }

    void legacyConfigDerivesTrayFallback()
    {
        IsolatedConfigScope scope;
        QVERIFY(scope.init());

        const markshot::settings::SettingsConfig config = markshot::settings::readSettingsConfig(nullptr);
        // 无 startup.modes 的旧配置：与 main 的启动解析一致，回退为托盘，不直接截图。
        QVERIFY(config.general.startupTray);
        QVERIFY(!config.general.startupDirectCapture);
        QVERIFY(!config.general.startupFloatingBall);
        QVERIFY(!config.general.startupSettings);
    }

    void writeStartupBehaviorRoundTrips()
    {
        IsolatedConfigScope scope;
        QVERIFY(scope.init());

        markshot::StartupBehaviorConfig behavior;
        behavior.directCapture = true;
        behavior.tray = true;
        behavior.floatingBall = true;
        behavior.configured = true;

        QString error;
        QVERIFY(markshot::writeStartupBehaviorConfig(behavior, &error));

        const markshot::StartupBehaviorConfig read = markshot::configuredStartupBehavior();
        QVERIFY(read.configured);
        QVERIFY(read.directCapture);
        QVERIFY(read.tray);
        QVERIFY(read.floatingBall);
        QVERIFY(!read.settingsWindow);
    }

    void emptyStartupModesFallsBackToTray()
    {
        IsolatedConfigScope scope;
        QVERIFY(scope.init());

        markshot::StartupBehaviorConfig behavior;
        behavior.configured = true;

        QString error;
        QVERIFY(markshot::writeStartupBehaviorConfig(behavior, &error));

        // 空组合回退为托盘，保证应用始终有入口。
        const markshot::StartupBehaviorConfig read = markshot::configuredStartupBehavior();
        QVERIFY(read.configured);
        QVERIFY(read.tray);
        QVERIFY(!read.directCapture);
        QVERIFY(!read.floatingBall);
        QVERIFY(!read.settingsWindow);
    }
};

QTEST_APPLESS_MAIN(SettingsStartupBehaviorTest)

#include "settings_startup_behavior_test.moc"
