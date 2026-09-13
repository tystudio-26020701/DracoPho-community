#pragma once

#include "window_detection.h"

#include <QVector>

namespace markshot {

// 枚举当前会话的可见窗口（Quartz CGWindowList，仅 macOS）。数组顺序即
// 前后台顺序（zOrder = 序号）；title/class 取 kCGWindowName 与
// kCGWindowOwnerName——无屏幕录制权限时 kCGWindowName 可能为空，属预期。
QVector<WindowInfo> enumerateDarwinWindowInfos(bool includeHidden);

} // namespace markshot
