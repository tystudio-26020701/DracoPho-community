#include "screen_capture.h"

#include "screen_capture_internal.h"
#include "windows_integration.h"

#ifdef MARK_SHOT_WITH_DBUS
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#endif
#include <QDir>
#include <QFile>
#include <QUuid>

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

#ifdef MARK_SHOT_WITH_DBUS
/// @brief 通过 GNOME Shell 扩展的 CaptureWindow 接口截取指定窗口的无遮挡画面。
///
/// GNOME (mutter) 没有窗口缓冲接口（对比 KWin ScreenShot2.CaptureWindow），
/// 也没有 portal 窗口捕获协议。随软件安装的扩展在合成器内按 pid 定位窗口，
/// 临时隐藏其他窗口的 actor（仅合成器可见性，不改变任何窗口的状态/层叠/
/// 焦点）后区域截图，从而得到"不被其他窗口遮挡"的目标窗口当前画面。
/// 最小化窗口/不在当前工作区的窗口不支持（GNOME 无内容接口），如实返回失败。
/// @param window 目标窗口（必须带 pid 与矩形）。
/// @param error 输出错误信息。
/// @return 成功时返回窗口画面，失败返回空图像。
QImage captureGnomeWindowContent(const markshot::WindowInfo &window, QString *error)
{
    if (window.pid <= 0 || window.rect.isEmpty()) {
        if (error) {
            *error = QStringLiteral("GNOME window capture requires pid and geometry");
        }
        return {};
    }

    QDBusInterface helper(QStringLiteral("org.gnome.Shell"),
                          QStringLiteral("/org/gnome/Shell/Extensions/MarkShotScrollHelper"),
                          QStringLiteral("org.gnome.Shell.Extensions.MarkShotScrollHelper"),
                          QDBusConnection::sessionBus());
    if (!helper.isValid()) {
        if (error) {
            *error = QStringLiteral("GNOME shell helper is not available");
        }
        return {};
    }

    const QString tempDir = QFile::exists(QStringLiteral("/dev/shm"))
        ? QStringLiteral("/dev/shm")
        : QDir::tempPath();
    const QString tempPath = QStringLiteral("%1/dracoPho-window-%2.png")
        .arg(tempDir, QUuid::createUuid().toString(QUuid::Id128));

    const QDBusMessage reply = helper.call(QStringLiteral("CaptureWindow"),
                                           static_cast<int>(window.pid),
                                           window.rect.x(),
                                           window.rect.y(),
                                           window.rect.width(),
                                           window.rect.height(),
                                           tempPath);
    const QList<QVariant> args = reply.arguments();
    const bool ok = reply.type() == QDBusMessage::ReplyMessage
        && args.size() >= 1 && args.at(0).toBool();
    if (!ok) {
        QFile::remove(tempPath);
        if (error) {
            *error = reply.type() == QDBusMessage::ErrorMessage
                ? reply.errorMessage()
                : QStringLiteral("GNOME window capture failed (window may be minimized or on another workspace)");
        }
        return {};
    }

    const QString usedPath = args.size() >= 2 && !args.at(1).toString().isEmpty()
        ? args.at(1).toString()
        : tempPath;
    const QImage image(usedPath);
    QFile::remove(usedPath);
    if (image.isNull()) {
        if (error) {
            *error = QStringLiteral("failed to load GNOME window capture from %1").arg(usedPath);
        }
        return {};
    }
    return image;
}
#endif  // MARK_SHOT_WITH_DBUS

}  // namespace

QImage captureWindowObjectContent(const markshot::WindowInfo &window, bool includeCursor, QString *error)
{
    QImage captured;
#if defined(Q_OS_WIN)
    Q_UNUSED(includeCursor);
    // Windows：PrintWindow(PW_RENDERFULLCONTENT) 读窗口自身缓冲，遮挡/最小化
    // 窗口也能拿到真实内容；失败回退由调用方做屏幕区域裁剪。
    // 非十六进制句柄视为"无对象路径可用"，与 Linux 一致地返回空图且不报错，
    // 由调用方走冻结帧裁剪回退。
    qulonglong hwnd = 0;
    if (parseWindowHexId(window.id, &hwnd)) {
        captured = markshot::windows::captureWindowsWindowContent(hwnd, error);
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
    // GNOME Wayland 原生窗口：无窗口缓冲接口（window.id 为空），经随软件安装的
    // 扩展 CaptureWindow 按 pid 定位窗口、临时隐藏其他窗口 actor 后区域截图，
    // 得到无遮挡的窗口当前画面。最小化窗口不支持，如实返回空图由调用方处理。
    if (captured.isNull() && window.pid > 0 && isWaylandSession() && isGnomeWaylandSession()) {
        captured = captureGnomeWindowContent(window, error);
    }
#else
    Q_UNUSED(includeCursor);
#endif
#endif
    return captured;
}
