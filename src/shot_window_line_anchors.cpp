#include "shot_window_module.h"

namespace cfg = markshot::config;
namespace shortcuts = markshot::shortcut;
using namespace markshot::shot;

bool ShotWindow::annotationSupportsLineAnchors(const Annotation &annotation) const
{
    return annotationSupportsLineControl(annotation);
}

ShotWindow::SelectionDrag ShotWindow::lineAnchorDragAt(const Annotation &annotation, QPointF imagePoint) const
{
    if (!annotationSupportsLineAnchors(annotation) || annotation.points.size() < 2) {
        return SelectionDrag::None;
    }

    const qreal imageTolerance = 8.0 * m_frozenFrame.width() / std::max<qreal>(1.0, m_frozenImageRect.width());
    if (QLineF(imagePoint, annotation.points.first()).length() <= imageTolerance * 1.5) {
        return SelectionDrag::LineStart;
    }
    if (QLineF(imagePoint, annotation.points.at(1)).length() <= imageTolerance * 1.5) {
        return SelectionDrag::LineEnd;
    }
    if (QLineF(imagePoint, annotationLineControlPoint(annotation)).length() <= imageTolerance * 1.4) {
        return SelectionDrag::LineControl;
    }
    return SelectionDrag::None;
}

void ShotWindow::drawLineAnchorHandles(QPainter &painter,
                                       const Annotation &annotation,
                                       QPointF center,
                                       qreal angle,
                                       bool rotateHandles) const
{
    if (!annotationSupportsLineAnchors(annotation) || annotation.points.size() < 2) {
        return;
    }

    auto handlePoint = [this, center, angle, rotateHandles](QPointF imagePoint) {
        const QPointF widgetPoint = imageToWidget(imagePoint);
        return rotateHandles ? rotatedPoint(widgetPoint, center, angle) : widgetPoint;
    };

    const QPointF start = handlePoint(annotation.points.first());
    const QPointF end = handlePoint(annotation.points.at(1));
    const QPointF control = handlePoint(annotationLineControlPoint(annotation));

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(255, 255, 255), 2.0));
    painter.setBrush(QColor(251, 146, 60));
    painter.drawEllipse(QRectF(start.x() - 6.0, start.y() - 6.0, 12.0, 12.0));
    painter.drawEllipse(QRectF(end.x() - 6.0, end.y() - 6.0, 12.0, 12.0));

    painter.setPen(QPen(QColor(251, 146, 60), 2.0));
    painter.setBrush(QColor(255, 255, 255));
    painter.drawEllipse(QRectF(control.x() - 5.5, control.y() - 5.5, 11.0, 11.0));
    painter.restore();
}

bool ShotWindow::updateLineAnchorDrag(QPointF imagePoint)
{
    if (m_annotationDrag != SelectionDrag::LineStart
        && m_annotationDrag != SelectionDrag::LineEnd
        && m_annotationDrag != SelectionDrag::LineControl) {
        return false;
    }

    const QVector<int> selectedIds = selectedAnnotationIds();
    if (selectedIds.size() != 1 || !annotationSupportsLineAnchors(m_annotationBeforeDrag)
        || m_annotationBeforeDrag.points.size() < 2) {
        return true;
    }

    Annotation *annotation = annotationById(selectedIds.first());
    if (!annotation) {
        return true;
    }

    const QRectF beforeBounds = annotationUnrotatedBounds(m_annotationBeforeDrag);
    const QPointF localPoint = beforeBounds.isEmpty()
        ? clampImagePoint(imagePoint)
        : rotatedPoint(clampImagePoint(imagePoint),
                       beforeBounds.center(),
                       -m_annotationBeforeDrag.rotationDegrees);
    while (annotation->points.size() < 2) {
        annotation->points.append(localPoint);
    }

    if (m_annotationDrag == SelectionDrag::LineStart) {
        annotation->points[0] = clampImagePoint(localPoint);
    } else if (m_annotationDrag == SelectionDrag::LineEnd) {
        annotation->points[1] = clampImagePoint(localPoint);
    } else {
        while (annotation->points.size() < 3) {
            annotation->points.append(annotationLineControlPoint(m_annotationBeforeDrag));
        }
        annotation->points[2] = clampImagePoint(localPoint);
    }
    annotation->rotationDegrees = m_annotationBeforeDrag.rotationDegrees;
    return true;
}
