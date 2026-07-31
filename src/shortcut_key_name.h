#pragma once

#include <QString>

namespace markshot::shortcut {

/// @brief 将 Qt 键值转换为 xkbcommon/X11 风格的按键名。
///
/// 返回值同时符合 XDG Shortcuts 规范的标识符（xkbcommon-keysyms.h 去掉
/// XKB_KEY_ 前缀后的名字）与 X11 keysym 名称（如 "a"、"Return"、
/// "BackSpace"、"space"、"F1"）。portal 后端与 X11 XGrabKey 后端共用同一张
/// 转换表，避免两处各自维护导致快捷键在两套后端下行为不一致。
/// @param key Qt 键值。
/// @return 规范按键名；不支持的键返回空字符串。
QString qtKeyToShortcutKeyName(int key);

}  // namespace markshot::shortcut
