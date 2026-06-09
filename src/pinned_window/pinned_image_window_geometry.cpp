#include "pinned_window/pinned_image_window.h"

#include "pinned_window_top.h"

#include <QMouseEvent>
#include <QVariant>

#include <algorithm>

namespace {

constexpr int kPinnedMinimumExtent = 24;
constexpr qreal kPinnedResizeMargin = 8.0;

}  // namespace

namespace markshot::shot {

QSize PinnedImageWindow::displayBaseSizeForPixmap() const
{
    const QSizeF logicalSize = m_pixmap.deviceIndependentSize();
    return QSize(std::max(1, qRound(logicalSize.width())),
                 std::max(1, qRound(logicalSize.height())));
}

void PinnedImageWindow::resizeByScale(qreal scale, QPoint globalAnchor, QPointF localAnchor)
{
    scale = std::clamp(scale, 0.1, 6.0);
    QSize targetSize(qMax(kPinnedMinimumExtent, qRound(m_displayBaseSize.width() * scale)),
                     qMax(kPinnedMinimumExtent, qRound(m_displayBaseSize.height() * scale)));
    targetSize.scale(targetSize, Qt::KeepAspectRatio);
    const qreal xRatio = width() > 0 ? localAnchor.x() / width() : 0.5;
    const qreal yRatio = height() > 0 ? localAnchor.y() / height() : 0.5;
    const QPoint topLeft(globalAnchor.x() - qRound(targetSize.width() * xRatio),
                         globalAnchor.y() - qRound(targetSize.height() * yRatio));
    m_scale = static_cast<qreal>(targetSize.width()) / std::max(1, m_displayBaseSize.width());
    setMinimumSize(QSize(kPinnedMinimumExtent, kPinnedMinimumExtent));
    setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    setFixedSize(targetSize);
    setPinnedGeometry(QRect(topLeft, targetSize), !pinnedWindowHasLayerShellTop(this));
}

QPoint PinnedImageWindow::pinnedTopLeft() const
{
    return m_logicalGeometry.isValid() ? m_logicalGeometry.topLeft() : frameGeometry().topLeft();
}

QPoint PinnedImageWindow::logicalGlobalPointForLocalAnchor(QPointF localAnchor, QPoint fallbackGlobal)
{
    if (!m_config.alwaysOnTop || !pinnedWindowHasLayerShellTop(this)) {
        return fallbackGlobal;
    }
    return pinnedTopLeft() + localAnchor.toPoint();
}

void PinnedImageWindow::setPinnedGeometry(QRect geometry, bool moveWidget)
{
    if (!geometry.isValid() || geometry.isEmpty()) {
        return;
    }
    m_logicalGeometry = geometry;
    setProperty("markShotPinnedGeometry", m_logicalGeometry);
    if (moveWidget) {
        setGeometry(m_logicalGeometry);
    }
    if (m_config.alwaysOnTop) {
        syncPinnedWindowTopGeometry(this, m_logicalGeometry);
    }
}

PinnedResizeDirection PinnedImageWindow::resizeDirectionAt(QPointF widgetPoint) const
{
    const qreal margin = std::max(kPinnedResizeMargin, m_config.borderWidth + 5.0);
    return pinnedResizeDirectionAt(QRectF(rect()), widgetPoint, margin);
}

bool PinnedImageWindow::startResizeDrag(QMouseEvent *event)
{
    const PinnedResizeDirection direction = resizeDirectionAt(event->position());
    if (!isPinnedResizeDirection(direction)) {
        return false;
    }

    clearTextSelection();
    const QRect startGeometry(pinnedTopLeft(), size());
    m_resizeDrag = beginPinnedResizeDrag(direction, startGeometry, event->globalPosition().toPoint());
    setCursor(cursorForPinnedResizeDirection(direction));
    return true;
}

bool PinnedImageWindow::continueResizeDrag(QMouseEvent *event)
{
    if (!isPinnedResizeDirection(m_resizeDrag.direction)) {
        return false;
    }
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        finishResizeDrag(event->position());
        return true;
    }

    const QRect geometry = pinnedResizeGeometry(m_resizeDrag,
                                                event->globalPosition().toPoint(),
                                                QSize(kPinnedMinimumExtent, kPinnedMinimumExtent));
    applyResizeGeometry(geometry);
    setCursor(cursorForPinnedResizeDirection(m_resizeDrag.direction));
    return true;
}

void PinnedImageWindow::finishResizeDrag(QPointF widgetPoint)
{
    m_resizeDrag = {};
    updateCursorForPosition(widgetPoint);
}

void PinnedImageWindow::applyResizeGeometry(QRect geometry)
{
    if (!geometry.isValid() || geometry.isEmpty()) {
        return;
    }

    // 1. 先解除固定尺寸限制，再按拖拽几何设置固定大小
    setMinimumSize(QSize(kPinnedMinimumExtent, kPinnedMinimumExtent));
    setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    setFixedSize(geometry.size());

    // 2. 更新缩放比例，保持滚轮缩放和菜单缩放从当前尺寸继续计算
    m_scale = static_cast<qreal>(geometry.width()) / std::max(1, m_displayBaseSize.width());

    // 3. 根据平台协议同步窗口位置，layer-shell 场景只更新逻辑几何
    setPinnedGeometry(geometry, !pinnedWindowHasLayerShellTop(this));
}

}  // namespace markshot::shot
