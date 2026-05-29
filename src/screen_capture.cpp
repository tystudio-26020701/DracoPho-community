#include "screen_capture.h"

#include <QGuiApplication>
#include <QPixmap>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRect>
#include <QScreen>
#include <QStringList>

#ifdef HAVE_XCB
#include <xcb/xcb.h>
#endif

namespace {

bool isWaylandSession()
{
    const QString sessionType = QProcessEnvironment::systemEnvironment().value(QStringLiteral("XDG_SESSION_TYPE")).toLower();
    return sessionType == QStringLiteral("wayland");
}

CaptureResult captureWithQScreen(const CaptureRequest &request)
{
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) {
        return {{}, QStringLiteral("no screen available for capture"), {}, {}};
    }

    QPixmap pixmap = screen->grabWindow(0);
    if (pixmap.isNull()) {
        return {{}, QStringLiteral("QScreen::grabWindow returned null pixmap"), {}, {}};
    }

    QImage image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

    if (request.sourceGeometry.isValid() && !request.sourceGeometry.isEmpty()) {
        const QRect geo = request.sourceGeometry.normalized().intersected(image.rect());
        if (!geo.isEmpty()) {
            image = image.copy(geo);
        }
    }

    return {image, {}, {}, request.sourceGeometry};
}

QString grimGeometry(QRect geometry)
{
    geometry = geometry.normalized();
    return QStringLiteral("%1,%2 %3x%4")
        .arg(geometry.x())
        .arg(geometry.y())
        .arg(geometry.width())
        .arg(geometry.height());
}

CaptureResult runGrim(const QStringList &arguments, const QString &outputName, QRect sourceGeometry)
{
    QProcess grim;
    grim.setProgram(QStringLiteral("grim"));
    grim.setArguments(arguments);
    grim.start(QIODevice::ReadOnly);

    if (!grim.waitForStarted(3000)) {
        return {{}, QStringLiteral("failed to start grim; install grim and run under a Wayland compositor that supports screencopy"), outputName, sourceGeometry};
    }

    if (!grim.waitForFinished(8000)) {
        grim.kill();
        grim.waitForFinished(1000);
        return {{}, QStringLiteral("grim timed out while capturing the screen"), outputName, sourceGeometry};
    }

    const QByteArray png = grim.readAllStandardOutput();
    const QByteArray stderrText = grim.readAllStandardError();

    if (grim.exitStatus() != QProcess::NormalExit || grim.exitCode() != 0) {
        QString error = QString::fromLocal8Bit(stderrText).trimmed();
        if (error.isEmpty()) {
            error = QStringLiteral("grim failed with exit code %1").arg(grim.exitCode());
        }
        return {{}, error, outputName, sourceGeometry};
    }

    QImage image;
    if (!image.loadFromData(png, "PPM") || image.isNull()) {
        return {{}, QStringLiteral("grim returned invalid PPM data"), outputName, sourceGeometry};
    }

    return {image.convertToFormat(QImage::Format_ARGB32_Premultiplied), {}, outputName, sourceGeometry};
}

} // namespace

CaptureResult captureScreenFrame(const CaptureRequest &request)
{
    if (!isWaylandSession()) {
        return captureWithQScreen(request);
    }

    const QStringList baseArguments{QStringLiteral("-t"), QStringLiteral("ppm"), QStringLiteral("-s"), QStringLiteral("1")};

    if (request.sourceGeometry.isValid() && !request.sourceGeometry.isEmpty()) {
        QStringList arguments = baseArguments;
        arguments << QStringLiteral("-g") << grimGeometry(request.sourceGeometry) << QStringLiteral("-");
        CaptureResult geometryCapture = runGrim(arguments, request.allOutputs ? QString() : request.preferredOutputName, request.sourceGeometry);
        if (!geometryCapture.image.isNull() || request.allOutputs || request.preferredOutputName.isEmpty()) {
            return geometryCapture;
        }
    }

    if (!request.allOutputs && !request.preferredOutputName.isEmpty()) {
        QStringList arguments = baseArguments;
        arguments << QStringLiteral("-o") << request.preferredOutputName << QStringLiteral("-");
        return runGrim(arguments, request.preferredOutputName, {});
    }

    QStringList arguments = baseArguments;
    arguments << QStringLiteral("-");
    return runGrim(arguments, {}, {});
}

QVector<QRect> enumerateX11WindowGeometries()
{
    QVector<QRect> results;

#ifdef HAVE_XCB
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

    struct WindowInfo {
        xcb_window_t window;
    };

    QVector<xcb_window_t> stack;
    stack.append(root);

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
            xcb_window_t child = children[i];

            xcb_get_window_attributes_cookie_t attrCookie = xcb_get_window_attributes(connection, child);
            xcb_get_window_attributes_reply_t *attrReply = xcb_get_window_attributes_reply(connection, attrCookie, nullptr);
            if (!attrReply) {
                continue;
            }

            const bool isViewable = (attrReply->map_state == XCB_MAP_STATE_VIEWABLE);
            free(attrReply);

            if (!isViewable) {
                continue;
            }

            xcb_get_geometry_cookie_t geoCookie = xcb_get_geometry(connection, child);
            xcb_get_geometry_reply_t *geoReply = xcb_get_geometry_reply(connection, geoCookie, nullptr);
            if (!geoReply) {
                continue;
            }

            xcb_translate_coordinates_cookie_t transCookie = xcb_translate_coordinates(connection, child, root, 0, 0);
            xcb_translate_coordinates_reply_t *transReply = xcb_translate_coordinates_reply(connection, transCookie, nullptr);

            if (transReply) {
                int x = transReply->dst_x;
                int y = transReply->dst_y;
                int w = geoReply->width;
                int h = geoReply->height;
                free(transReply);

                if (w > 1 && h > 1) {
                    results.append(QRect(x, y, w, h));
                }
            }

            free(geoReply);
            stack.append(child);
        }

        free(treeReply);
    }

    xcb_disconnect(connection);
#endif

    return results;
}
