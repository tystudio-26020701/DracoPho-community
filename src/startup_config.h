#pragma once

#include "debug_log.h"
#include "shot_window.h"
#include "ui/theme.h"

#include <QColor>
#include <QString>

#include <optional>

namespace markshot {

struct DefaultTools {
    ShotWindow::Tool normal = ShotWindow::Tool::Pen;
    ShotWindow::Tool fullscreen = ShotWindow::Tool::Pen;
    ShotWindow::Tool file = ShotWindow::Tool::Pen;
    QColor color = markshot::theme::kDefaultAnnotationColor;
};

struct DebugRuntimeConfig {
    bool enabled = markshot::debugEnabled();
    QString logPath = markshot::debugLogPath();
};

/// @brief 展开配置路径中的用户目录写法。
/// @param path 原始配置路径。
/// @return 展开后的路径。
QString expandedConfigPath(QString path);

/// @brief 从字符串解析默认标注颜色。
/// @param value 颜色字符串。
/// @return 解析出的颜色。
std::optional<QColor> colorFromString(QString value);

/// @brief 应用配置文件中的进程环境变量。
/// @return 无返回值。
void applyConfiguredEnvironment();

/// @brief 读取调试运行时配置。
/// @return 调试运行时配置。
DebugRuntimeConfig configuredDebugRuntimeConfig();

/// @brief 读取默认工具和默认颜色配置。
/// @param warning 输出配置警告。
/// @return 默认工具配置。
DefaultTools configuredDefaultTools(QString *warning);

/// @brief 禁用 Qt 默认门户服务以便应用自行注册门户会话。
/// @return 无返回值。
void disableQtPortalServicesForHostApp();

}  // namespace markshot
