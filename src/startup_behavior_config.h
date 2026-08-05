#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include <optional>

namespace markshot {

/// @brief 启动行为模式。
///
/// 用户点击 DracoPho 图标启动应用后执行的动作。允许多选组合：
/// - DirectCapture：直接进入截图模式；
/// - Tray：显示系统托盘图标并保持后台运行；
/// - FloatingBall：显示可拖动的悬浮球快捷入口；
/// - SettingsWindow：打开设置窗口。
enum class StartupMode {
    DirectCapture,
    Tray,
    FloatingBall,
    SettingsWindow,
};

/// @brief 启动行为配置（多选组合）。
struct StartupBehaviorConfig {
    bool directCapture = false;
    bool tray = false;
    bool floatingBall = false;
    bool settingsWindow = false;
    /// 配置文件中是否存在 startup.modes 键。为 false 时表示旧版配置，
    /// 需要回退到历史行为（windows.tray.enabled 决定托盘 / 截图）。
    bool configured = false;
};

/// @brief 返回模式对应的配置文件名称。
/// @param mode 启动模式。
/// @return 配置名称。
QString startupModeName(StartupMode mode);

/// @brief 从配置名称解析启动模式。
/// @param name 配置名称。
/// @return 匹配的启动模式；无法识别时返回空。
std::optional<StartupMode> startupModeFromName(const QString &name);

/// @brief 返回新安装的默认启动行为（类似 PixPin：托盘 + 悬浮球）。
/// @return 默认启动行为。
StartupBehaviorConfig defaultStartupBehavior();

/// @brief 从配置根对象解析启动行为。
/// @param root 应用配置根对象。
/// @return 启动行为；未配置 startup.modes 时 configured 为 false。
StartupBehaviorConfig startupBehaviorFromRoot(const QJsonObject &root);

/// @brief 将启动行为序列化为 startup.modes 数组（空组合回退为托盘）。
/// @param config 启动行为。
/// @return modes 数组。
QJsonArray startupModeArray(const StartupBehaviorConfig &config);

/// @brief 读取应用配置中的启动行为。
/// @return 启动行为；未配置 startup.modes 时 configured 为 false。
StartupBehaviorConfig configuredStartupBehavior();

/// @brief 将启动行为写入应用配置（startup.modes 数组）。
/// @param config 启动行为。
/// @param error 写入失败时输出错误信息。
/// @return 写入成功返回 true。
bool writeStartupBehaviorConfig(const StartupBehaviorConfig &config, QString *error = nullptr);

}  // namespace markshot
