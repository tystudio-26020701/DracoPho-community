#include "screen_capture.h"

#include "screen_capture_internal.h"
#include "windows_integration.h"

namespace {

/// @brief 把窗口 id 解析为无符号整数（兼容 "0x..." 前缀）。
/// @param id 窗口标识。
/// @param value 输出解析结果。
/// @return 解析成功且非零时返回 true。
bool parseWindowHexId(const QString &id, qulonglong *value)
{
    if (!value || id.isEmpty()) {
        return false;
    }
    const QString text = id.startsWith(QLatin1String("0x")) ? id.mid(2) : id;
    bool ok = false;
    const qulonglong parsed = text.toULongLong(&ok, 16);
    if (!ok || parsed == 0) {
        return false;
    }
    *value = parsed;
    return true;
}

}  // namespace

QImage captureWindowObjectContent(const markshot::WindowInfo &window, bool includeCursor, QString *error)
{
    QImage captured;
#if defined(Q_OS_WIN)
    Q_UNUSED(includeCursor);
    // Windows：PrintWindow(PW_RENDERFULLCONTENT) 读窗口自身缓冲，遮挡/最小化
    // 窗口也能拿到真实内容；失败回退由调用方做屏幕区域裁剪。
    qulonglong hwnd = 0;
    if (parseWindowHexId(window.id, &hwnd)) {
        captured = markshot::windows::captureWindowsWindowContent(hwnd, error);
    } else if (error && error->isEmpty()) {
        *error = QStringLiteral("missing Windows window handle");
    }
#else
    // X11 对象抓取：原生 X11 会话与 Wayland 下的 XWayland（DISPLAY 指向
    // XWayland 服务）均可从合成命名 pixmap 读取窗口自身内容，遮挡/最小化
    // 的 XWayland 窗口同样有效。
    const bool x11CaptureAvailable = qEnvironmentVariableIsSet("DISPLAY");
    if (x11CaptureAvailable && window.id.startsWith(QLatin1String("0x"))) {
        qulonglong xid = 0;
        if (parseWindowHexId(window.id, &xid)) {
            captured = captureX11WindowContent(xid, error);
        }
    }
#ifdef MARK_SHOT_WITH_DBUS
    // KWin Wayland 原生窗口：window.id 为检测脚本上报的 KWin internalId
    // （uuid，无 0x 前缀）。由 KWin 直接渲染窗口合成缓冲，对标 Spectacle。
    if (captured.isNull() && !window.id.isEmpty() && !window.id.startsWith(QLatin1String("0x"))
        && isWaylandSession() && isKdePlasma()) {
        captured = captureKWinWindowContent(window.id, includeCursor, error);
    }
#else
    Q_UNUSED(includeCursor);
#endif
#endif
    return captured;
}
