#include "display_capture/display_capture_snapshot.h"

#include "capture_geometry.h"
#include "screen_capture.h"
#include "ui/i18n.h"

#include <QGuiApplication>
#include <QScreen>

namespace markshot::display_capture {
namespace {

/**
 * 计算全部显示器组成的虚拟桌面几何。
 * @return 虚拟桌面几何。
 */
QRect virtualScreensGeometry()
{
    QRect geometry;
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        if (!screen) {
            continue;
        }
        geometry = geometry.isNull() ? screen->geometry() : geometry.united(screen->geometry());
    }
    return geometry;
}

/**
 * 生成显示器卡片使用的轻量缩略图。
 * @param image 显示器完整截图。
 * @return 缩略图。
 */
QImage targetThumbnail(const QImage &image)
{
    if (image.isNull()) {
        return {};
    }
    return image.scaled(QSize(360, 220), Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

/**
 * 生成目标副标题。
 * @param geometry 目标全局逻辑几何。
 * @param image 目标图像。
 * @return 副标题文本。
 */
QString targetSubtitle(QRect geometry, const QImage &image)
{
    const QSize size = image.isNull() ? geometry.size() : image.size();
    return QStringLiteral("%1 x %2").arg(size.width()).arg(size.height());
}

/**
 * 构建完整虚拟桌面目标。
 * @param image 虚拟桌面截图。
 * @param geometry 图像对应的全局逻辑几何。
 * @return 截取目标。
 */
Target allDisplaysTarget(const QImage &image, QRect geometry)
{
    Target target;
    target.allOutputs = true;
    target.outputName = QStringLiteral("all-displays");
    target.title = MS_TR("All displays");
    target.subtitle = targetSubtitle(geometry, image);
    target.geometry = geometry;
    target.image = image;
    target.thumbnail = targetThumbnail(image);
    return target;
}

/**
 * 构建单个显示器目标。
 * @param screen 显示器对象。
 * @param image 显示器截图。
 * @param geometry 图像对应的全局逻辑几何。
 * @return 截取目标。
 */
Target screenTarget(QScreen *screen, const QImage &image, QRect geometry)
{
    Target target;
    target.screenName = screen ? screen->name() : QString();
    target.outputName = target.screenName.isEmpty() ? QStringLiteral("display") : target.screenName;
    target.title = target.screenName.isEmpty() ? MS_TR("Display") : target.screenName;
    target.subtitle = targetSubtitle(geometry, image);
    target.geometry = geometry;
    target.image = image;
    target.thumbnail = targetThumbnail(image);
    return target;
}

}  // namespace

QVector<Target> captureDisplayTargets(bool includeCursor, QString *error)
{
    QVector<Target> targets;
    const QList<QScreen *> screens = QGuiApplication::screens();
    const QRect virtualGeometry = virtualScreensGeometry();
    if (virtualGeometry.isEmpty()) {
        if (error) {
            *error = QStringLiteral("no virtual screen geometry available for display capture");
        }
        return targets;
    }

    CaptureRequest request;
    request.sourceGeometry = virtualGeometry;
    request.allOutputs = true;
    request.includeCursor = includeCursor;
    const CaptureResult capture = captureScreenFrame(request);
    if (capture.image.isNull()) {
        if (error) {
            *error = capture.error;
        }
        return targets;
    }

    const QRect frameGeometry = capture.sourceGeometry.isValid() && !capture.sourceGeometry.isEmpty()
        ? capture.sourceGeometry
        : virtualGeometry;
    if (screens.size() > 1) {
        targets.append(allDisplaysTarget(capture.image, frameGeometry));
    }

    for (QScreen *screen : screens) {
        if (!screen || screen->geometry().isEmpty()) {
            continue;
        }

        const QRect screenGeometry = screen->geometry().intersected(frameGeometry);
        if (screenGeometry.isEmpty()) {
            continue;
        }

        const QImage image = markshot::capture::cropFrameToRequest(capture.image, frameGeometry, screenGeometry);
        if (image.isNull()) {
            if (error) {
                *error = QStringLiteral("failed to crop shared display capture for screen %1").arg(screen->name());
            }
            targets.clear();
            return targets;
        }
        targets.append(screenTarget(screen, image, screenGeometry));
    }

    return targets;
}

}  // namespace markshot::display_capture
