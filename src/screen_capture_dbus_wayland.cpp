#include "screen_capture_internal.h"

#include "capture_own_windows_policy.h"
#include "kde_capture_config.h"

/// @brief Captures the screen using the grim utility.
/// @param request The capture request details such as source geometry and output name.
/// @return The result of the screen capture operation.
CaptureResult captureWithGrim(const CaptureRequest &request)
{
    const QStringList baseArguments{QStringLiteral("-t"), QStringLiteral("ppm")};
    auto grimArguments = [&request, &baseArguments] {
        QStringList arguments = baseArguments;
        if (request.includeCursor) {
            arguments << QStringLiteral("-c");
        }
        return arguments;
    };

    if (!request.allOutputs && !request.preferredOutputName.isEmpty()) {
        const QRect outputGeometry = screenGeometryForRequest(request);
        QStringList arguments = grimArguments();
        arguments << QStringLiteral("-o") << request.preferredOutputName << QStringLiteral("-");
        CaptureResult outputCapture = runGrim(arguments, request.preferredOutputName, outputGeometry, request.includeCursor);
        if (!outputCapture.image.isNull()) {
            if (!outputGeometry.isEmpty()) {
                return cropGrimFrameToRequest(std::move(outputCapture), outputGeometry, request);
            }
            if (!request.sourceGeometry.isValid() || request.sourceGeometry.isEmpty()) {
                return outputCapture;
            }
        }

        const QString outputError = outputCapture.error;
        const QRect fullGeometry = fullGrimSourceGeometry(request);
        QStringList fullArguments = grimArguments();
        fullArguments << QStringLiteral("-");
        CaptureResult fullCapture = runGrim(fullArguments, {}, fullGeometry, request.includeCursor);
        if (!fullCapture.image.isNull()) {
            fullCapture.outputName = request.preferredOutputName;
            return cropGrimFrameToRequest(std::move(fullCapture), fullGeometry, request);
        }

        if (!outputError.isEmpty() && !fullCapture.error.isEmpty()) {
            return {{},
                    QStringLiteral("%1\nFull-desktop grim fallback: %2")
                        .arg(outputError, fullCapture.error),
                    request.preferredOutputName,
                    request.sourceGeometry};
        }
        if (!outputGeometry.isValid() || outputGeometry.isEmpty()) {
            const QString fallbackError = fullCapture.error.isEmpty()
                ? QStringLiteral("full-desktop grim fallback was not usable")
                : fullCapture.error;
            return {{},
                    QStringLiteral("grim output capture has no Qt output geometry for local crop\nFull-desktop grim fallback: %1")
                        .arg(fallbackError),
                    request.preferredOutputName,
                    request.sourceGeometry};
        }
        return outputCapture.image.isNull() ? fullCapture : outputCapture;
    }

    const QRect frameGeometry = fullGrimSourceGeometry(request);
    QStringList arguments = grimArguments();
    arguments << QStringLiteral("-");
    CaptureResult fullCapture = runGrim(arguments, {}, frameGeometry, request.includeCursor);
    return cropGrimFrameToRequest(std::move(fullCapture), frameGeometry, request);
}

#ifdef MARK_SHOT_WITH_DBUS

/// @brief 判断 KWin ScreenShot2 DBus 接口是否可用。
/// @return 接口存在时返回 true。
bool isKWinScreenShotAvailable()
{
    QDBusInterface kwin(QStringLiteral("org.kde.KWin.ScreenShot2"),
                        QStringLiteral("/org/kde/KWin/ScreenShot2"),
                        QStringLiteral("org.kde.KWin.ScreenShot2"),
                        QDBusConnection::sessionBus());
    return kwin.isValid();
}

/// @brief 从 KWin ScreenShot2 的管道文件描述符读回原始帧并包装为 QImage。
/// @param readFd 管道读端（调用方负责在返回后关闭）。
/// @param results KWin 返回的元数据（type/width/height/stride/format/scale）。
/// @param error 输出错误信息。
/// @return 读取成功时返回图像，失败返回空图像。
QImage readKWinScreenshotPipe(int readFd, const QVariantMap &results, QString *error)
{
    const int width = results.value(QStringLiteral("width")).toInt();
    const int height = results.value(QStringLiteral("height")).toInt();
    const int stride = results.value(QStringLiteral("stride")).toInt();
    const uint format = results.value(QStringLiteral("format")).toUInt();
    if (width <= 0 || height <= 0 || stride < width * 4) {
        if (error) {
            *error = QStringLiteral("KWin ScreenShot2 returned invalid buffer metadata (%1x%2 stride=%3)")
                         .arg(width).arg(height).arg(stride);
        }
        return {};
    }

    const qulonglong total = static_cast<qulonglong>(stride) * static_cast<qulonglong>(height);
    QByteArray buffer(static_cast<int>(total), Qt::Uninitialized);
    qulonglong received = 0;
    while (received < total) {
        struct pollfd pfd { readFd, POLLIN, 0 };
        const int polled = ::poll(&pfd, 1, 2000);
        if (polled <= 0) {
            break;  // timeout or poll error
        }
        const ssize_t bytes = ::read(readFd, buffer.data() + received, total - received);
        if (bytes <= 0) {
            break;  // EOF or read error
        }
        received += static_cast<qulonglong>(bytes);
    }

    if (received < total) {
        markshot::debugLog("kwin", "short-read got=%llu want=%llu %dx%d stride=%d",
                           received, total, width, height, stride);
        if (error) {
            *error = QStringLiteral("KWin ScreenShot2 delivered a truncated frame (%1/%2 bytes)")
                         .arg(received).arg(total);
        }
        return {};
    }

    const QImage::Format imageFormat =
        format != 0 ? static_cast<QImage::Format>(format) : QImage::Format_ARGB32_Premultiplied;
    const QImage view(reinterpret_cast<const uchar *>(buffer.constData()),
                      width, height, stride, imageFormat);
    if (view.isNull()) {
        if (error) {
            *error = QStringLiteral("KWin ScreenShot2 frame could not be wrapped as an image");
        }
        return {};
    }
    // 读取的 scale 写入 devicePixelRatio，保证 HiDPI 下窗口/区域尺寸与逻辑坐标一致。
    const double scale = results.value(QStringLiteral("scale")).toDouble();
    QImage copy = view.copy();
    copy.setDevicePixelRatio(scale > 0.0 && scale < 8.0 ? scale : 1.0);
    return copy.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}

/// @brief 使用 KWin ScreenShot2 接口截取指定 Wayland 区域。
/// @param request 捕获请求，包含源区域、输出名称和鼠标包含策略。
/// @return 捕获成功时返回图像，失败时返回错误信息供后续回退链路使用。
CaptureResult captureWithKWinScreenShot(const CaptureRequest &request)
{
    const QRect geometry = request.sourceGeometry.normalized();
    if (geometry.isEmpty()) {
        return {{}, QStringLiteral("KWin ScreenShot2 requires a non-empty geometry"), {}, request.sourceGeometry};
    }

    QDBusInterface kwin(QStringLiteral("org.kde.KWin.ScreenShot2"),
                        QStringLiteral("/org/kde/KWin/ScreenShot2"),
                        QStringLiteral("org.kde.KWin.ScreenShot2"),
                        QDBusConnection::sessionBus());
    if (!kwin.isValid()) {
        return {{}, QStringLiteral("org.kde.KWin.ScreenShot2 interface is not available"), {}, request.sourceGeometry};
    }

    int fds[2];
    if (::pipe2(fds, O_CLOEXEC) != 0) {
        return {{}, QStringLiteral("failed to create pipe for KWin ScreenShot2"), {}, request.sourceGeometry};
    }

    QVariantMap options;
    options.insert(QStringLiteral("include-cursor"), request.includeCursor);
    // native-resolution keeps device pixels on HiDPI instead of downscaling to
    // logical size, so the stitched result stays sharp.
    options.insert(QStringLiteral("native-resolution"), true);
    // 显式传 hide-caller-windows：KWin 会按调用进程 PID 匹配并隐藏本软件全部
    // 顶层窗口（悬浮球/设置窗口/托盘菜单等）后再合成截图，是"图形合成层级"
    // 的自身窗口排除，而不是截图前逐个 hide。KWin 对未知选项忽略，低版本
    // 不认识该键时行为与现状一致（靠其他后端/应用层隐藏兜底）。
    options.insert(QStringLiteral("hide-caller-windows"), request.hideOwnWindows);
    // KWin sends the D-Bus reply with the buffer metadata first, then writes the
    // pixels to the pipe, so this synchronous call does not deadlock even when
    // the image is larger than the pipe buffer.
    QDBusReply<QVariantMap> reply =
        kwin.call(QStringLiteral("CaptureArea"),
                  geometry.x(), geometry.y(),
                  static_cast<uint>(geometry.width()), static_cast<uint>(geometry.height()),
                  options,
                  QVariant::fromValue(QDBusUnixFileDescriptor(fds[1])));
    ::close(fds[1]);

    if (!reply.isValid()) {
        ::close(fds[0]);
        markshot::debugLog("kwin", "capture-area-error geom=%d,%d %dx%d name=%s msg=%s",
                           geometry.x(), geometry.y(), geometry.width(), geometry.height(),
                           reply.error().name().toUtf8().constData(),
                           reply.error().message().toUtf8().constData());
        return {{},
                QStringLiteral("KWin ScreenShot2 CaptureArea failed: %1: %2")
                    .arg(reply.error().name(), reply.error().message()),
                {},
                request.sourceGeometry};
    }

    const QVariantMap results = reply.value();
    QString pipeError;
    const QImage frame = readKWinScreenshotPipe(fds[0], results, &pipeError);
    ::close(fds[0]);

    if (frame.isNull()) {
        markshot::debugLog("kwin", "capture-area-error geom=%d,%d %dx%d err=%s",
                           geometry.x(), geometry.y(), geometry.width(), geometry.height(),
                           pipeError.toUtf8().constData());
        return {{}, pipeError, {}, request.sourceGeometry};
    }

    markshot::debugLog("kwin", "capture-area-ok geom=%d,%d %dx%d -> frame=%dx%d stride=%d format=%u",
                       geometry.x(), geometry.y(), geometry.width(), geometry.height(),
                       frame.width(), frame.height(), static_cast<int>(frame.bytesPerLine()),
                       results.value(QStringLiteral("format")).toUInt());
    // Detach from the soon-to-be-freed buffer and normalize the format.
    return {frame,
            {},
            request.allOutputs ? QString() : request.preferredOutputName,
            request.sourceGeometry,
            request.includeCursor};
}

/// @brief 使用 KWin ScreenShot2.CaptureWindow 抓取 Wayland 原生窗口自身内容。
/// 与区域抓屏不同，该接口由 KWin 直接渲染目标窗口的合成缓冲，遮挡/最小化
/// 窗口也能拿到真实内容（对标 Spectacle 的 Wayland 窗口捕获）。
/// @param windowHandle KWin 窗口的 internalId（uuid，窗口检测脚本提供）。
/// @param includeCursor 是否包含鼠标。
/// @param error 输出错误信息。
/// @return 抓取成功时返回窗口图像，失败返回空图像。
QImage captureKWinWindowContent(const QString &windowHandle, bool includeCursor, QString *error)
{
    if (windowHandle.isEmpty()) {
        if (error) {
            *error = QStringLiteral("KWin window handle is empty");
        }
        return {};
    }

    QDBusInterface kwin(QStringLiteral("org.kde.KWin.ScreenShot2"),
                        QStringLiteral("/org/kde/KWin/ScreenShot2"),
                        QStringLiteral("org.kde.KWin.ScreenShot2"),
                        QDBusConnection::sessionBus());
    if (!kwin.isValid()) {
        if (error) {
            *error = QStringLiteral("org.kde.KWin.ScreenShot2 interface is not available");
        }
        return {};
    }

    int fds[2];
    if (::pipe2(fds, O_CLOEXEC) != 0) {
        if (error) {
            *error = QStringLiteral("failed to create pipe for KWin window capture");
        }
        return {};
    }

    QVariantMap options;
    options.insert(QStringLiteral("include-cursor"), includeCursor);
    // 包含窗口装饰：与交互式高亮的窗口边界（含标题栏）一致。
    options.insert(QStringLiteral("include-decoration"), true);
    options.insert(QStringLiteral("include-shadow"), true);
    options.insert(QStringLiteral("native-resolution"), true);

    QDBusReply<QVariantMap> reply =
        kwin.call(QStringLiteral("CaptureWindow"),
                  windowHandle,
                  options,
                  QVariant::fromValue(QDBusUnixFileDescriptor(fds[1])));
    ::close(fds[1]);

    if (!reply.isValid()) {
        ::close(fds[0]);
        markshot::debugLog("kwin", "capture-window-error handle=%s name=%s msg=%s",
                           windowHandle.toUtf8().constData(),
                           reply.error().name().toUtf8().constData(),
                           reply.error().message().toUtf8().constData());
        if (error) {
            *error = QStringLiteral("KWin ScreenShot2 CaptureWindow failed: %1: %2")
                         .arg(reply.error().name(), reply.error().message());
        }
        return {};
    }

    const QVariantMap results = reply.value();
    QString pipeError;
    const QImage image = readKWinScreenshotPipe(fds[0], results, &pipeError);
    ::close(fds[0]);
    if (image.isNull() && error) {
        *error = pipeError;
    }
    markshot::debugLog("kwin", "capture-window-ok handle=%s frame=%dx%d dpr=%.3f",
                       windowHandle.toUtf8().constData(),
                       image.width(), image.height(),
                       image.devicePixelRatio());
    return image;
}

CaptureResult captureWaylandFrame(const CaptureRequest &request)
{
    const bool grimPreferred = prefersGrim();
    const bool kdeSession = isKdePlasma();
    const bool kwinConfigured = markshot::configuredKdeKWinScreenshotEnabled();
    const bool kwinAvailable = kwinConfigured ? isKWinScreenShotAvailable() : false;
    markshot::debugLog("capture",
                       "wayland-frame geom=%d,%d %dx%d output=%s all_outputs=%d "
                       "prefer_screencast=%d allow_interactive=%d allow_screenshot_fallback=%d "
                       "prefers_grim=%d kde=%d kwin=%d kwin_configured=%d desktop_file=%s desktop=%s",
                       request.sourceGeometry.x(), request.sourceGeometry.y(),
                       request.sourceGeometry.width(), request.sourceGeometry.height(),
                       request.preferredOutputName.toUtf8().constData(),
                       request.allOutputs ? 1 : 0, request.preferScreencast ? 1 : 0,
                       request.allowInteractivePortal ? 1 : 0,
                       request.allowPortalScreenshotFallback ? 1 : 0, grimPreferred ? 1 : 0,
                       kdeSession ? 1 : 0, kwinAvailable ? 1 : 0, kwinConfigured ? 1 : 0,
                       QGuiApplication::desktopFileName().toUtf8().constData(),
                       desktopEnvironmentText().toUtf8().constData());

    if (isGnomeWaylandSession() && hasGnomeScrollHelper() && request.sourceGeometry.isValid()
        && !request.sourceGeometry.isEmpty() && !request.allOutputs) {
        markshot::debugLog("capture", "route=gnome-scroll-helper");
        CaptureResult gnomeCapture = captureWithGnomeScrollHelper(request);
        if (!gnomeCapture.image.isNull() && !markshot::isSuspiciousSolidFrame(gnomeCapture.image)) {
            markshot::debugLog("capture", "gnome-scroll-helper-ok frame=%dx%d",
                               gnomeCapture.image.width(), gnomeCapture.image.height());
            return gnomeCapture;
        }
        if (!gnomeCapture.image.isNull()) {
            markshot::debugLog("capture", "gnome-scroll-helper-solid-frame (falling back)");
        }
        markshot::debugLog("capture", "gnome-scroll-helper-failed (falling back) error=%s",
                           gnomeCapture.error.toUtf8().constData());
    }

    // KDE 使用 ScreenShot2.CaptureArea 截取精确区域，失败后继续进入现有回退链路
    const bool kwinAllowedForRequest =
        markshot::kwinScreenShotSupportsOwnWindowPolicy(request.hideOwnWindows);
    if (kwinConfigured && kwinAllowedForRequest
        && (kdeSession || kwinAvailable) && request.sourceGeometry.isValid()
        && !request.sourceGeometry.isEmpty()) {
        markshot::debugLog("capture", "route=kwin-screenshot kde=%d kwin=%d all_outputs=%d",
                           kdeSession ? 1 : 0, kwinAvailable ? 1 : 0,
                           request.allOutputs ? 1 : 0);
        CaptureResult kwinCapture = captureWithKWinScreenShot(request);
        if (!kwinCapture.image.isNull() && !markshot::isSuspiciousSolidFrame(kwinCapture.image)) {
            markshot::debugLog("capture", "kwin-screenshot-ok frame=%dx%d",
                               kwinCapture.image.width(), kwinCapture.image.height());
            return kwinCapture;
        }
        if (!kwinCapture.image.isNull()) {
            markshot::debugLog("capture", "kwin-screenshot-solid-frame (falling back)");
        }
        markshot::debugLog("capture", "kwin-screenshot-failed (falling back) error=%s",
                           kwinCapture.error.toUtf8().constData());
    } else if (kdeSession || kwinAvailable) {
        markshot::debugLog("capture",
                           "【Wayland捕获】【KWin路由】skipped configured=%d hide_own_windows=%d",
                           kwinConfigured ? 1 : 0,
                           request.hideOwnWindows ? 1 : 0);
    }

    if (request.preferScreencast) {
        markshot::debugLog("capture", "route=screencast (preferScreencast)");
        CaptureResult screencastCapture = captureWithPortalScreencast(request);
        if (!screencastCapture.image.isNull()) {
            markshot::debugLog("capture", "screencast-ok frame=%dx%d",
                               screencastCapture.image.width(), screencastCapture.image.height());
            return screencastCapture;
        }
        markshot::debugLog("capture", "screencast-failed error=%s",
                           screencastCapture.error.toUtf8().constData());
        stopPortalScreencast();

        CaptureResult portalCapture;
        if (request.allowPortalScreenshotFallback) {
            markshot::debugLog("capture", "fallback=portal-screenshot");
            portalCapture = captureWithPortalScreenshot(request);
            if (!portalCapture.image.isNull()) {
                markshot::debugLog("capture", "portal-screenshot-ok frame=%dx%d",
                                   portalCapture.image.width(), portalCapture.image.height());
                return portalCapture;
            }
            markshot::debugLog("capture", "portal-screenshot-failed error=%s",
                               portalCapture.error.toUtf8().constData());
        } else {
            markshot::debugLog("capture", "portal-screenshot-fallback disabled");
        }

        markshot::debugLog("capture", "fallback=grim");
        CaptureResult grimCapture = captureWithGrim(request);
        if (!grimCapture.image.isNull()) {
            markshot::debugLog("capture", "grim-ok frame=%dx%d",
                               grimCapture.image.width(), grimCapture.image.height());
            return grimCapture;
        }
        markshot::debugLog("capture", "grim-failed error=%s all-routes-exhausted",
                           grimCapture.error.toUtf8().constData());
        return {{},
                QStringLiteral("%1\nPortal screenshot fallback: %2\nGrim fallback: %3")
                    .arg(screencastCapture.error,
                         request.allowPortalScreenshotFallback
                             ? portalCapture.error
                             : QStringLiteral("disabled for live scrolling capture"),
                         grimCapture.error),
                {},
                request.sourceGeometry};
    }

    if (prefersGrim()) {
        CaptureResult grimCapture = captureWithGrim(request);
        if (!grimCapture.image.isNull() && !markshot::isSuspiciousSolidFrame(grimCapture.image)) {
            return grimCapture;
        }
        if (!grimCapture.image.isNull()) {
            markshot::debugLog("capture", "grim-solid-frame (falling back)");
        }

        if (!request.allowPortalScreenshotFallback) {
            markshot::debugLog("capture", "【Wayland捕获】【Portal截图回退】已禁用");
            return {{},
                    QStringLiteral("%1\nPortal fallback: disabled for live capture").arg(grimCapture.error),
                    {},
                    request.sourceGeometry};
        }

        CaptureResult portalCapture = captureWithPortalScreenshot(request);
        if (!portalCapture.image.isNull()) {
            return portalCapture;
        }

        return {{}, QStringLiteral("%1\nPortal fallback: %2").arg(grimCapture.error, portalCapture.error), {}, request.sourceGeometry};
    }

    CaptureResult portalCapture;
    if (request.allowPortalScreenshotFallback) {
        portalCapture = captureWithPortalScreenshot(request);
        if (!portalCapture.image.isNull() && !markshot::isSuspiciousSolidFrame(portalCapture.image)) {
            return portalCapture;
        }
        if (!portalCapture.image.isNull()) {
            markshot::debugLog("capture", "portal-screenshot-solid-frame (falling back)");
        }
    } else {
        markshot::debugLog("capture", "【Wayland捕获】【Portal截图回退】已禁用");
    }

    CaptureResult grimCapture = captureWithGrim(request);
    if (!grimCapture.image.isNull()) {
        return grimCapture;
    }

    return {{},
            QStringLiteral("%1\nGrim fallback: %2")
                .arg(request.allowPortalScreenshotFallback
                         ? portalCapture.error
                         : QStringLiteral("Portal fallback disabled for live capture"),
                     grimCapture.error),
            {},
            request.sourceGeometry};
}

#endif  // MARK_SHOT_WITH_DBUS
