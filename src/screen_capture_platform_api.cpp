#include "screen_capture_internal.h"

/// @brief Enumerates the info of all open X11 windows.
/// @param includeIdentity 是否读取 title/class/instance（需要额外 X11 属性往返）。
/// @return A vector of WindowInfo with z-order based on _NET_CLIENT_LIST_STACKING order (bottom-to-top).
QVector<markshot::WindowInfo> enumerateX11WindowInfos(bool includeIdentity)
{
    QVector<markshot::WindowInfo> results;

#ifndef HAVE_XCB
    Q_UNUSED(includeIdentity);
#endif

#if defined(HAVE_XCB) && defined(Q_OS_LINUX)
    xcb_connection_t *connection = xcb_connect(nullptr, nullptr);
    if (!connection || xcb_connection_has_error(connection)) {
        if (connection) {
            xcb_disconnect(connection);
        }
        return results;
    }

    const xcb_setup_t *setup = xcb_get_setup(connection);
    if (!setup) {
        xcb_disconnect(connection);
        return results;
    }

    xcb_screen_iterator_t screenIter = xcb_setup_roots_iterator(setup);
    if (!screenIter.data) {
        xcb_disconnect(connection);
        return results;
    }

    xcb_window_t root = screenIter.data->root;
    const X11WindowAtoms atoms = readX11WindowAtoms(connection);

    auto appendInfo = [&](xcb_window_t window, int zOrder) {
        const std::optional<QRect> rect = x11WindowFrameGeometry(connection, root, window, atoms);
        if (!rect.has_value()) {
            return;
        }
        for (const markshot::WindowInfo &existing : std::as_const(results)) {
            if (existing.rect == *rect) {
                return;
            }
        }
        markshot::WindowInfo info;
        info.rect = *rect;
        info.zOrder = zOrder;
        info.id = QStringLiteral("0x%1").arg(static_cast<qulonglong>(window), 0, 16);
        if (includeIdentity) {
            info.title = x11WindowTitle(connection, window, atoms);
            x11WindowClass(connection, window, atoms, &info.instance, &info.className);
        }
        results.append(info);
    };

    QVector<xcb_window_t> managedWindows =
        readX11WindowListProperty(connection, root, atoms.netClientListStacking);
    if (managedWindows.isEmpty()) {
        managedWindows = readX11WindowListProperty(connection, root, atoms.netClientList);
    }
    if (!managedWindows.isEmpty()) {
        for (int i = 0; i < managedWindows.size(); ++i) {
            appendInfo(managedWindows.at(i), i);
        }
        xcb_disconnect(connection);
        return results;
    }

    // Fallback: walk the window tree when no _NET_CLIENT_LIST is present.
    QVector<xcb_window_t> stack;
    stack.append(root);
    int zOrder = 0;
    while (!stack.isEmpty()) {
        xcb_window_t parent = stack.takeLast();
        xcb_query_tree_cookie_t treeCookie = xcb_query_tree(connection, parent);
        xcb_query_tree_reply_t *treeReply = xcb_query_tree_reply(connection, treeCookie, nullptr);
        if (!treeReply) {
            continue;
        }

        int childCount = xcb_query_tree_children_length(treeReply);
        xcb_window_t *children = xcb_query_tree_children(treeReply);
        for (int i = 0; i < childCount; ++i) {
            stack.append(children[i]);
        }
        std::free(treeReply);

        if (parent != root) {
            appendInfo(parent, zOrder);
            ++zOrder;
        }
    }

    xcb_disconnect(connection);
#endif

    return results;
}

bool isGnomeWaylandSession()
{
#ifdef MARK_SHOT_WITH_DBUS
    if (!isWaylandSession()) {
        return false;
    }
    return desktopEnvironmentText().toLower().contains(QStringLiteral("gnome"));
#else
    return false;
#endif
}

bool hasGnomeScrollHelper()
{
#ifdef MARK_SHOT_WITH_DBUS
    QDBusInterface helper(QStringLiteral("org.gnome.Shell"),
                          QStringLiteral("/org/gnome/Shell/Extensions/MarkShotScrollHelper"),
                          QStringLiteral("org.gnome.Shell.Extensions.MarkShotScrollHelper"),
                          QDBusConnection::sessionBus());
    if (!helper.isValid()) {
        return false;
    }

    QDBusMessage reply = helper.call(QStringLiteral("Version"));
    return reply.type() != QDBusMessage::ErrorMessage && !reply.arguments().isEmpty();
#else
    return false;
#endif
}

/// @brief 读取 GNOME Shell 滚动截图扩展的主版本号。
/// @return 扩展不可用时返回 0，否则返回语义版本中的主版本号。
int gnomeScrollHelperMajorVersion()
{
#ifdef MARK_SHOT_WITH_DBUS
    QDBusInterface helper(QStringLiteral("org.gnome.Shell"),
                          QStringLiteral("/org/gnome/Shell/Extensions/MarkShotScrollHelper"),
                          QStringLiteral("org.gnome.Shell.Extensions.MarkShotScrollHelper"),
                          QDBusConnection::sessionBus());
    if (!helper.isValid()) {
        return 0;
    }

    QDBusMessage reply = helper.call(QStringLiteral("Version"));
    if (reply.type() == QDBusMessage::ErrorMessage || reply.arguments().isEmpty()) {
        return 0;
    }

    const QString version = reply.arguments().first().toString();
    bool ok = false;
    const int major = version.section(QLatin1Char('.'), 0, 0).toInt(&ok);
    return ok ? major : 0;
#else
    return 0;
#endif
}

bool hasGnomeScrollPreviewHelper()
{
#ifdef MARK_SHOT_WITH_DBUS
    return gnomeScrollHelperMajorVersion() >= 3;
#else
    return false;
#endif
}

bool hasGnomeScrollOverlayHelper()
{
#ifdef MARK_SHOT_WITH_DBUS
    return gnomeScrollHelperMajorVersion() >= 5;
#else
    return false;
#endif
}

/// @brief Captures a screen area using the GNOME Shell extension scroll helper.
/// @param request The capture request specifying the target geometry.
/// @return The result of the capture operation.
CaptureResult captureWithGnomeScrollHelper(const CaptureRequest &request)
{
#ifdef MARK_SHOT_WITH_DBUS
    const QString tempDir = QFile::exists(QStringLiteral("/dev/shm"))
        ? QStringLiteral("/dev/shm")
        : QDir::tempPath();
    const QString tempPath = QStringLiteral("%1/mark-shot-scroll-frame-%2.png")
        .arg(tempDir, QUuid::createUuid().toString(QUuid::Id128));

    QDBusMessage message = QDBusMessage::createMethodCall(
        QStringLiteral("org.gnome.Shell"),
        QStringLiteral("/org/gnome/Shell/Extensions/MarkShotScrollHelper"),
        QStringLiteral("org.gnome.Shell.Extensions.MarkShotScrollHelper"),
        QStringLiteral("ScreenshotArea")
    );
    message << request.sourceGeometry.x()
            << request.sourceGeometry.y()
            << request.sourceGeometry.width()
            << request.sourceGeometry.height()
            << tempPath;

    QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        QFile::remove(tempPath);
        return {{}, reply.errorMessage(), {}, request.sourceGeometry};
    }

    QList<QVariant> args = reply.arguments();
    if (args.size() < 2 || !args.at(0).toBool()) {
        QFile::remove(tempPath);
        return {{}, QStringLiteral("Failed to capture area via GNOME Shell extension"), {}, request.sourceGeometry};
    }

    QString actualPath = args.at(1).toString();
    QImage img(actualPath);
    if (img.isNull()) {
        return {{}, QStringLiteral("Failed to load captured frame from %1").arg(actualPath), {}, request.sourceGeometry};
    }

    QFile::remove(actualPath);
    return {img, {}, {}, request.sourceGeometry};
#else
    return {{}, QStringLiteral("GNOME scroll helper support was not enabled at build time"), {}, request.sourceGeometry};
#endif
}
