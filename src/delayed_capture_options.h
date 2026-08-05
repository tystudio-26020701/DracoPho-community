#pragma once

#include <QStringList>

#include <array>

namespace markshot {

/// @brief 延时截图预设秒数（托盘/悬浮球"延时截图"子菜单选项）。
constexpr std::array<int, 4> kDelayedCapturePresets = {1, 3, 5, 10};

/// @brief 返回延时预设对应的本地化菜单文案。
/// @param seconds 秒数。
/// @return 如 "1 Second(s)"。
QString delayedCapturePresetLabel(int seconds);

/// @brief 返回全部延时预设的本地化文案（与 kDelayedCapturePresets 顺序一致）。
/// @return 文案列表。
QStringList delayedCapturePresetLabels();

}  // namespace markshot
