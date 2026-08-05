#pragma once

#include <QJsonObject>

namespace markshot {

/// @brief 返回延时截图的内置默认秒数（0 = 立即截图）。
/// @return 默认秒数。
int defaultCaptureDelaySeconds();

/// @brief 从应用配置根对象解析延时截图秒数。
/// @param root 应用配置根对象。
/// @return 配置的秒数，缺失或非法时返回默认值。
int captureDelaySecondsFromConfigRoot(const QJsonObject &root);

/// @brief 从当前应用配置文件读取延时截图秒数。
/// @return 配置的秒数。
int configuredCaptureDelaySeconds();

}  // namespace markshot
