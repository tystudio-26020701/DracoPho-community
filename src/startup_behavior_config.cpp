#include "startup_behavior_config.h"

#include "app_config_store.h"
#include "debug_log.h"
#include "window_detection.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QStringList>

namespace markshot {

QString startupModeName(StartupMode mode)
{
    switch (mode) {
    case StartupMode::DirectCapture:
        return QStringLiteral("capture");
    case StartupMode::Tray:
        return QStringLiteral("tray");
    case StartupMode::FloatingBall:
        return QStringLiteral("floating");
    case StartupMode::SettingsWindow:
        return QStringLiteral("settings");
    }
    return {};
}

std::optional<StartupMode> startupModeFromName(const QString &name)
{
    const QString trimmed = name.trimmed().toLower();
    if (trimmed == QStringLiteral("capture")
        || trimmed == QStringLiteral("screenshot")
        || trimmed == QStringLiteral("shot")
        || trimmed == QStringLiteral("direct")) {
        return StartupMode::DirectCapture;
    }
    if (trimmed == QStringLiteral("tray")
        || trimmed == QStringLiteral("trayIcon")
        || trimmed == QStringLiteral("systemTray")) {
        return StartupMode::Tray;
    }
    if (trimmed == QStringLiteral("floating")
        || trimmed == QStringLiteral("floatingBall")
        || trimmed == QStringLiteral("ball")
        || trimmed == QStringLiteral("dock")) {
        return StartupMode::FloatingBall;
    }
    if (trimmed == QStringLiteral("settings")
        || trimmed == QStringLiteral("settingsWindow")
        || trimmed == QStringLiteral("config")) {
        return StartupMode::SettingsWindow;
    }
    return std::nullopt;
}

StartupBehaviorConfig defaultStartupBehavior()
{
    StartupBehaviorConfig config;
    config.tray = true;
    config.floatingBall = true;
    config.configured = true;
    return config;
}

StartupBehaviorConfig startupBehaviorFromRoot(const QJsonObject &root)
{
    StartupBehaviorConfig config;

    const QJsonObject startup = root.value(QStringLiteral("startup")).toObject();
    const QJsonValue modesValue = startup.value(QStringLiteral("modes"));
    if (modesValue.isUndefined()) {
        return config;
    }

    config.configured = true;
    if (modesValue.isArray()) {
        const QJsonArray modes = modesValue.toArray();
        for (const QJsonValue &value : modes) {
            const std::optional<StartupMode> mode = startupModeFromName(value.toString());
            if (!mode.has_value()) {
                markshot::debugLog("config",
                                   "unsupported startup mode: %s",
                                   value.toString().toUtf8().constData());
                continue;
            }
            switch (*mode) {
            case StartupMode::DirectCapture:
                config.directCapture = true;
                break;
            case StartupMode::Tray:
                config.tray = true;
                break;
            case StartupMode::FloatingBall:
                config.floatingBall = true;
                break;
            case StartupMode::SettingsWindow:
                config.settingsWindow = true;
                break;
            }
        }
    } else {
        // 兼容旧写法：startup.modes 直接为单个字符串。
        const std::optional<StartupMode> single = startupModeFromName(modesValue.toString());
        if (single.has_value()) {
            switch (*single) {
            case StartupMode::DirectCapture:
                config.directCapture = true;
                break;
            case StartupMode::Tray:
                config.tray = true;
                break;
            case StartupMode::FloatingBall:
                config.floatingBall = true;
                break;
            case StartupMode::SettingsWindow:
                config.settingsWindow = true;
                break;
            }
        }
    }

    // 空组合（如手写 "modes": [] 或全部未知模式）归一化为托盘，
    // 与 main 的启动解析兜底保持一致，保证应用始终有入口。
    if (!config.directCapture && !config.tray && !config.floatingBall && !config.settingsWindow) {
        config.tray = true;
    }
    return config;
}

StartupBehaviorConfig configuredStartupBehavior()
{
    QFile file(appConfigPath());
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return {};
    }

    return startupBehaviorFromRoot(document.object());
}

QJsonArray startupModeArray(const StartupBehaviorConfig &config)
{
    QJsonArray modes;
    if (config.directCapture) {
        modes.append(startupModeName(StartupMode::DirectCapture));
    }
    if (config.tray) {
        modes.append(startupModeName(StartupMode::Tray));
    }
    if (config.floatingBall) {
        modes.append(startupModeName(StartupMode::FloatingBall));
    }
    if (config.settingsWindow) {
        modes.append(startupModeName(StartupMode::SettingsWindow));
    }
    if (modes.isEmpty()) {
        // 不允许空组合：至少保留托盘，避免应用启动后无任何入口。
        modes.append(startupModeName(StartupMode::Tray));
    }
    return modes;
}

bool writeStartupBehaviorConfig(const StartupBehaviorConfig &config, QString *error)
{
    // 与设置页保存一致：同时同步旧键 windows.tray.enabled / autoStart，
    // 供旧逻辑与外部工具读取，避免两套写入分叉。
    if (!writeAppConfigValue(QStringList{QStringLiteral("startup"), QStringLiteral("modes")},
                             startupModeArray(config),
                             error)) {
        return false;
    }
    if (!writeAppConfigValue(QStringList{QStringLiteral("windows"), QStringLiteral("tray"), QStringLiteral("enabled")},
                             config.tray,
                             error)) {
        return false;
    }
    return writeAppConfigValue(QStringList{QStringLiteral("windows"), QStringLiteral("tray"), QStringLiteral("autoStart")},
                               config.tray,
                               error);
}

}  // namespace markshot
