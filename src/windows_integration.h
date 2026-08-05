#pragma once

#include "window_detection.h"

#include <QImage>
#include <QRect>
#include <QString>
#include <QVector>

class QWidget;
class QScreen;

namespace markshot::windows {

QVector<QRect> enumerateWindowGeometries();
QVector<markshot::WindowInfo> enumerateWindowInfos();
/// @brief 使用 PrintWindow(PW_RENDERFULLCONTENT) 抓取 Windows 窗口自身内容，
/// 遮挡/最小化窗口也能拿到真实像素（对标 ShareX/Greenshot 的 occluded 路径）；
/// 硬件加速或 DirectComposition 窗口需该标志否则为黑帧，失败时回退无标志重试。
/// @param hwnd 目标窗口句柄。
/// @param error 输出错误信息。
/// @return 抓取成功时返回窗口图像（含装饰），失败返回空图像。
QImage captureWindowsWindowContent(qulonglong hwnd, QString *error = nullptr);
void setExcludedFromCapture(QWidget *widget, bool excluded = true);
/// @brief 将窗口从系统任务栏/坞中排除（Windows: WS_EX_TOOLWINDOW；
/// X11: _NET_WM_STATE_SKIP_TASKBAR）。截图/滚动覆盖层不应出现在任务栏中。
/// @param widget 要处理的窗口。
/// @param excluded 是否从任务栏排除。
void setExcludedFromTaskbar(QWidget *widget, bool excluded = true);
void showFullScreenOnScreen(QWidget *widget, QScreen *screen);
/// @brief 当前 Qt 平台是否为 xcb（原生 X11 或 XWayland）。
/// @return 是 xcb 平台时返回 true。
bool isX11QtPlatform();
/// @brief 当前是否运行在真正的 X11/Xorg 会话（非 XWayland）。
/// @return 原生 X11 会话时返回 true。
bool isNativeX11Session();
/// @brief 将窗口切换为 Windows 原生置顶或取消置顶。
/// @param widget 要处理的窗口。
/// @param alwaysOnTop 是否启用原生置顶。
void setWindowTopMost(QWidget *widget, bool alwaysOnTop);
/// @brief 使用 Windows 原生置顶层级提升窗口。
/// @param widget 要提升的窗口。
void raiseTopMostWindow(QWidget *widget);
/// @brief GNOME Shell 下将匹配标题的窗口置顶/取消置顶。
///
/// GNOME (mutter) 不遵守 Qt::WindowStaysOnTopHint（Wayland 无标准置顶协议），
/// 悬浮球与贴纸窗口需借助随软件安装的 MarkShotScrollHelper 扩展的
/// SetWindowsAbove 实现置顶。非 GNOME 会话或扩展不可用时为空操作。
/// @param title 匹配的窗口标题（扩展按标题精确匹配）。
/// @param alwaysOnTop 置顶或取消置顶。
void setGnomeWindowAbove(const QString &title, bool alwaysOnTop);

} // namespace markshot::windows
