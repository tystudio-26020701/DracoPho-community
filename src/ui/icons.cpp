#include "ui/icons.h"

#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPolygonF>
#include <QStringLiteral>

namespace markshot::ui {

namespace {

constexpr int kIconSize = 32;

// All glyphs share a single ink color so the toolbar reads as one family.
// Active-state inversion is handled by the button stylesheet (teal background +
// dark text), so a glyph drawn in slate-200 stays legible across normal,
// hover, and active states.
const QColor kInk(229, 231, 235);            // slate-200
const QColor kInkSoft(229, 231, 235, 130);   // slate-200 @ 50%
const QColor kInkFaint(229, 231, 235, 80);   // slate-200 @ 30%
const QColor kSaveInk(31, 19, 0);            // dark amber for primary button

QPen makePen(QColor color, qreal width = 1.75)
{
    return QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
}

}  // namespace

QString actionName(ShotWindow::Action action)
{
    switch (action) {
    case ShotWindow::Action::ToolMove:
        return QStringLiteral("Move");
    case ShotWindow::Action::ToolSelect:
        return QStringLiteral("Select");
    case ShotWindow::Action::ToolPen:
        return QStringLiteral("Pen");
    case ShotWindow::Action::ToolLine:
        return QStringLiteral("Line");
    case ShotWindow::Action::ToolHighlighter:
        return QStringLiteral("Highlighter");
    case ShotWindow::Action::ToolRectangle:
        return QStringLiteral("Rect");
    case ShotWindow::Action::ToolEllipse:
        return QStringLiteral("Ellipse");
    case ShotWindow::Action::ToolArrow:
        return QStringLiteral("Arrow");
    case ShotWindow::Action::ToolText:
        return QStringLiteral("Text");
    case ShotWindow::Action::ToolNumber:
        return QStringLiteral("Number");
    case ShotWindow::Action::ToolMosaic:
        return QStringLiteral("Mosaic");
    case ShotWindow::Action::Clear:
        return QStringLiteral("Clear");
    case ShotWindow::Action::Undo:
        return QStringLiteral("Undo");
    case ShotWindow::Action::Redo:
        return QStringLiteral("Redo");
    case ShotWindow::Action::OpenWith:
        return QStringLiteral("Open With");
    case ShotWindow::Action::Copy:
        return QStringLiteral("Copy");
    case ShotWindow::Action::Save:
        return QStringLiteral("Save");
    case ShotWindow::Action::Cancel:
        return QStringLiteral("Cancel");
    }
    return {};
}

QIcon makeToolIcon(ShotWindow::Action action)
{
    QPixmap pixmap(kIconSize, kIconSize);
    pixmap.fill(Qt::transparent);

    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setBrush(Qt::NoBrush);
    p.setPen(makePen(kInk));

    switch (action) {
    case ShotWindow::Action::ToolMove: {
        // Four-direction arrow centered on the icon. A single open path keeps
        // round joins at the arrow heads continuous.
        p.drawLine(QPointF(16, 7), QPointF(16, 25));
        p.drawLine(QPointF(7, 16), QPointF(25, 16));
        QPainterPath head;
        head.moveTo(12, 11); head.lineTo(16, 7);  head.lineTo(20, 11);
        head.moveTo(12, 21); head.lineTo(16, 25); head.lineTo(20, 21);
        head.moveTo(11, 12); head.lineTo(7, 16);  head.lineTo(11, 20);
        head.moveTo(21, 12); head.lineTo(25, 16); head.lineTo(21, 20);
        p.drawPath(head);
        break;
    }
    case ShotWindow::Action::ToolSelect: {
        // Classic mouse cursor. Filled body keeps it readable at 32px.
        QPainterPath cursor;
        cursor.moveTo(9.5, 7);
        cursor.lineTo(9.5, 22.5);
        cursor.lineTo(13.5, 18.5);
        cursor.lineTo(15.8, 24.5);
        cursor.lineTo(18.0, 23.5);
        cursor.lineTo(15.7, 17.6);
        cursor.lineTo(21.5, 17.0);
        cursor.closeSubpath();
        p.setPen(makePen(kInk, 1.5));
        p.setBrush(QColor(229, 231, 235, 30));
        p.drawPath(cursor);
        p.setBrush(Qt::NoBrush);
        break;
    }
    case ShotWindow::Action::ToolPen: {
        // Diagonal pen body with a small chisel tip and rear cap.
        p.setPen(makePen(kInk, 2.4));
        p.drawLine(QPointF(11, 21), QPointF(22, 10));
        p.setPen(makePen(kInk, 1.5));
        // tip wedge
        QPainterPath tip;
        tip.moveTo(7.5, 24.5);
        tip.lineTo(11, 21);
        tip.lineTo(13.5, 23.5);
        tip.closeSubpath();
        p.setBrush(kInk);
        p.drawPath(tip);
        p.setBrush(Qt::NoBrush);
        // rear cap
        p.drawLine(QPointF(20, 7.5), QPointF(24.5, 12));
        break;
    }
    case ShotWindow::Action::ToolLine:
        // Single clean diagonal stroke.
        p.setPen(makePen(kInk, 2.0));
        p.drawLine(QPointF(8, 24), QPointF(24, 8));
        break;
    case ShotWindow::Action::ToolHighlighter: {
        // Translucent horizontal trail beneath a tilted marker body.
        p.setPen(QPen(kInkFaint, 6.0, Qt::SolidLine, Qt::RoundCap));
        p.drawLine(QPointF(8, 25), QPointF(22, 25));
        p.setPen(makePen(kInk, 2.4));
        p.drawLine(QPointF(11, 21), QPointF(20, 9));
        p.setPen(makePen(kInk, 1.5));
        p.drawLine(QPointF(8, 23.5), QPointF(11, 21));   // tip
        p.drawLine(QPointF(20, 9), QPointF(23.5, 12));   // cap
        break;
    }
    case ShotWindow::Action::ToolRectangle:
        p.setPen(makePen(kInk, 1.9));
        p.drawRoundedRect(QRectF(7.5, 10.5, 17, 13), 2.6, 2.6);
        break;
    case ShotWindow::Action::ToolEllipse:
        p.setPen(makePen(kInk, 1.9));
        p.drawEllipse(QRectF(7, 10, 18, 13));
        break;
    case ShotWindow::Action::ToolArrow: {
        // Slim diagonal arrow with a narrow open head.
        p.setPen(makePen(kInk, 2.0));
        p.drawLine(QPointF(7, 25), QPointF(25, 7));
        QPainterPath head;
        head.moveTo(18.2, 7.2);
        head.lineTo(25, 7);
        head.lineTo(24.8, 13.8);
        p.drawPath(head);
        break;
    }
    case ShotWindow::Action::ToolText: {
        // Serif-free T with a slightly heavier vertical for weight.
        p.setPen(makePen(kInk, 2.0));
        p.drawLine(QPointF(8, 9), QPointF(24, 9));
        p.drawLine(QPointF(16, 9), QPointF(16, 24));
        p.setPen(makePen(kInkSoft, 1.5));
        p.drawLine(QPointF(13, 24), QPointF(19, 24));
        break;
    }
    case ShotWindow::Action::ToolNumber: {
        // Outlined disc with a numeral 1 — replaces the filled orange chip.
        p.setPen(makePen(kInk, 1.8));
        p.drawEllipse(QRectF(6.5, 6.5, 19, 19));
        QFont font(QStringLiteral("Sans Serif"), 12, QFont::Bold);
        p.setFont(font);
        p.setPen(kInk);
        p.drawText(QRectF(6.5, 6.5, 19, 19), Qt::AlignCenter, QStringLiteral("1"));
        break;
    }
    case ShotWindow::Action::ToolMosaic: {
        // 3x3 checker grid, alternating opacity to imply pixelation.
        p.setPen(Qt::NoPen);
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                const int alpha = ((row + col) % 2) ? 230 : 100;
                p.setBrush(QColor(229, 231, 235, alpha));
                p.drawRoundedRect(QRectF(8 + col * 6, 8 + row * 6, 4.5, 4.5), 1.0, 1.0);
            }
        }
        break;
    }
    case ShotWindow::Action::Clear: {
        // Trash can for clearing all annotations.
        p.setPen(makePen(kInk, 1.8));
        p.drawLine(QPointF(10, 10), QPointF(22, 10));
        p.drawLine(QPointF(13, 7.5), QPointF(19, 7.5));
        p.drawLine(QPointF(15, 7.5), QPointF(15, 6.5));
        p.drawLine(QPointF(17, 7.5), QPointF(17, 6.5));
        p.drawRoundedRect(QRectF(11.5, 11.5, 9.0, 13.0), 1.8, 1.8);
        p.setPen(makePen(kInkSoft, 1.25));
        p.drawLine(QPointF(14.0, 14.0), QPointF(14.7, 22.0));
        p.drawLine(QPointF(18.0, 14.0), QPointF(17.3, 22.0));
        break;
    }
    case ShotWindow::Action::Undo: {
        // Counter-clockwise arc terminating in a left-pointing arrowhead.
        p.setPen(makePen(kInk, 2.0));
        QPainterPath arc;
        arc.moveTo(9, 12.5);
        arc.cubicTo(12.5, 7.0, 22.0, 7.5, 24.0, 14.0);
        arc.cubicTo(26.0, 20.5, 18.5, 24.0, 14.5, 22.5);
        p.drawPath(arc);
        // arrowhead
        QPainterPath head;
        head.moveTo(6.5, 9.5);
        head.lineTo(9, 12.5);
        head.lineTo(13, 11);
        p.setBrush(kInk);
        p.setPen(Qt::NoPen);
        p.drawPath(head);
        p.setBrush(Qt::NoBrush);
        break;
    }
    case ShotWindow::Action::Redo: {
        p.setPen(makePen(kInk, 2.0));
        QPainterPath arc;
        arc.moveTo(23, 12.5);
        arc.cubicTo(19.5, 7.0, 10.0, 7.5, 8.0, 14.0);
        arc.cubicTo(6.0, 20.5, 13.5, 24.0, 17.5, 22.5);
        p.drawPath(arc);
        QPainterPath head;
        head.moveTo(25.5, 9.5);
        head.lineTo(23, 12.5);
        head.lineTo(19, 11);
        p.setBrush(kInk);
        p.setPen(Qt::NoPen);
        p.drawPath(head);
        p.setBrush(Qt::NoBrush);
        break;
    }
    case ShotWindow::Action::OpenWith: {
        // Box with an arrow leaving the upper-right corner.
        p.setPen(makePen(kInk, 1.8));
        p.drawRoundedRect(QRectF(7, 11, 13, 13), 2.5, 2.5);
        p.drawLine(QPointF(14, 18), QPointF(24.5, 7.5));
        // arrow head (open angle)
        p.drawLine(QPointF(18, 7.5), QPointF(24.5, 7.5));
        p.drawLine(QPointF(24.5, 7.5), QPointF(24.5, 14));
        break;
    }
    case ShotWindow::Action::Copy: {
        // Two stacked rounded rectangles. The back card is dimmed so the
        // overlap reads as depth.
        p.setPen(makePen(kInkSoft, 1.5));
        p.drawRoundedRect(QRectF(11.5, 7.5, 13, 14), 2.5, 2.5);
        p.setPen(makePen(kInk, 1.8));
        p.drawRoundedRect(QRectF(7.5, 11.5, 13, 14), 2.5, 2.5);
        break;
    }
    case ShotWindow::Action::Save: {
        // Floppy disk silhouette in dark amber to contrast the orange button.
        p.setPen(makePen(kSaveInk, 2.0));
        p.drawRoundedRect(QRectF(7, 7, 18, 18), 2.4, 2.4);
        // sliding cover (top)
        p.drawLine(QPointF(11, 7), QPointF(11, 11.5));
        p.drawLine(QPointF(20, 7), QPointF(20, 11.5));
        p.drawLine(QPointF(11, 11.5), QPointF(20, 11.5));
        // label area (bottom)
        p.setPen(makePen(kSaveInk, 1.5));
        p.drawRoundedRect(QRectF(10, 15, 12, 9), 1.0, 1.0);
        p.drawLine(QPointF(12, 18.5), QPointF(20, 18.5));
        p.drawLine(QPointF(12, 21), QPointF(20, 21));
        break;
    }
    case ShotWindow::Action::Cancel: {
        // Slate cross — danger hue is delivered by the button background on
        // hover, not by the glyph itself.
        p.setPen(makePen(kInk, 2.1));
        p.drawLine(QPointF(10, 10), QPointF(22, 22));
        p.drawLine(QPointF(22, 10), QPointF(10, 22));
        break;
    }
    }

    p.end();
    return QIcon(pixmap);
}

}  // namespace markshot::ui
