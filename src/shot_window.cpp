#include "shot_window.h"

#include <LayerShellQt/Window>

#include <QAbstractItemView>
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QLineF>
#include <QListWidget>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QScreen>
#include <QStandardPaths>
#include <QStyle>
#include <QTextEdit>
#include <QTextOption>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QWindow>
#include <QWheelEvent>

#include <algorithm>
#include <cmath>
#include <utility>

namespace {

constexpr qreal kMinSelectionSize = 8.0;
constexpr qreal kToolbarMargin = 12.0;
constexpr qreal kMinStrokeWidth = 1.0;
constexpr qreal kMaxStrokeWidth = 24.0;
constexpr qreal kMinMosaicBlockSize = 4.0;
constexpr qreal kMaxMosaicBlockSize = 48.0;

QVector<QColor> paletteColors()
{
    return {
        QColor(255, 77, 77),
        QColor(255, 159, 67),
        QColor(255, 221, 87),
        QColor(46, 213, 115),
        QColor(0, 210, 255),
        QColor(84, 160, 255),
        QColor(165, 94, 234),
        QColor(255, 107, 180),
        QColor(255, 255, 255),
        QColor(17, 24, 39),
    };
}

QRectF normalizedRect(QPointF a, QPointF b)
{
    return QRectF(a, b).normalized();
}

QString panelStyleSheet()
{
    return QStringLiteral(
        "QWidget#shotToolbar, QWidget#actionToolbar {"
        " background: rgba(8, 13, 19, 238);"
        " border: 1px solid rgba(148, 163, 184, 95);"
        " border-radius: 18px;"
        "}"
        "QPushButton {"
        " color: rgba(241,245,249,245);"
        " background: rgba(255,255,255,20);"
        " border: 1px solid rgba(255,255,255,28);"
        " border-radius: 13px;"
        " padding: 0;"
        " min-width: 44px;"
        " min-height: 44px;"
        " max-width: 44px;"
        " max-height: 44px;"
        "}"
        "QPushButton:hover {"
        " background: rgba(20,184,166,64);"
        " border-color: rgba(94,234,212,150);"
        "}"
        "QPushButton:pressed {"
        " background: rgba(20,184,166,115);"
        " border-color: rgba(153,246,228,210);"
        "}"
        "QPushButton[active=\"true\"] {"
        " color: #042F2E;"
        " background: rgb(94,234,212);"
        " border-color: rgb(153,246,228);"
        "}"
        "QPushButton[role=\"primary\"] {"
        " color: #431407;"
        " background: rgb(251,146,60);"
        " border-color: rgb(253,186,116);"
        "}"
        "QPushButton[role=\"primary\"]:hover { background: rgb(249,115,22); }"
        "QPushButton[role=\"danger\"]:hover {"
        " background: rgba(239,68,68,110);"
        " border-color: rgba(252,165,165,190);"
        "}");
}

QString openWithPanelStyleSheet()
{
    return QStringLiteral(
        "QWidget#openWithPanel {"
        " background: rgba(8, 13, 19, 244);"
        " border: 1px solid rgba(148, 163, 184, 105);"
        " border-radius: 18px;"
        "}"
        "QLabel {"
        " color: rgba(241,245,249,238);"
        " font-size: 13px;"
        " font-weight: 800;"
        " letter-spacing: 0.4px;"
        " padding: 2px 4px 6px 4px;"
        "}"
        "QListWidget {"
        " color: white;"
        " background: transparent;"
        " border: 0;"
        " outline: 0;"
        "}"
        "QListWidget::item {"
        " background: rgba(255,255,255,24);"
        " border: 1px solid rgba(255,255,255,24);"
        " border-radius: 11px;"
        " padding: 10px 12px;"
        " margin: 3px 0;"
        "}"
        "QListWidget::item:hover {"
        " background: rgba(20,184,166,70);"
        " border-color: rgba(94,234,212,145);"
        "}"
        "QListWidget::item:selected {"
        " color: #042F2E;"
        " background: rgb(94,234,212);"
        " border-color: rgb(153,246,228);"
        "}"
        "QPushButton {"
        " color: white;"
        " text-align: left;"
        " background: rgba(255,255,255,24);"
        " border: 1px solid rgba(255,255,255,24);"
        " border-radius: 11px;"
        " padding: 10px 12px;"
        " min-height: 22px;"
        "}"
        "QPushButton:hover {"
        " background: rgba(20,184,166,70);"
        " border-color: rgba(94,234,212,145);"
        "}");
}

QString textEditorStyleSheet(const QColor &color, int pointSize)
{
    return QStringLiteral(
        "QTextEdit#textEditor {"
        " color: %1;"
        " background: transparent;"
        " border: 1px solid rgb(94,234,212);"
        " border-radius: 0;"
        " padding: 4px 6px;"
        " font-size: %2px;"
        " font-weight: 800;"
        " selection-background-color: rgb(94,234,212);"
        " selection-color: #042F2E;"
        "}"
        "QTextEdit#textEditor QAbstractScrollArea { background: transparent; }"
        "QTextEdit#textEditor QWidget { background: transparent; }")
        .arg(color.name())
        .arg(pointSize);
}

QString colorPaletteStyleSheet()
{
    return QStringLiteral(
        "QWidget#colorPalette { background: transparent; }"
        "QPushButton {"
        " border: 2px solid rgba(8,13,19,210);"
        " border-radius: 15px;"
        " min-width: 30px;"
        " min-height: 30px;"
        " max-width: 30px;"
        " max-height: 30px;"
        "}"
        "QPushButton:hover {"
        " border: 3px solid rgb(94,234,212);"
        "}");
}

QString actionName(ShotWindow::Action action)
{
    switch (action) {
    case ShotWindow::Action::ToolMove:
        return QStringLiteral("Move");
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
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    const QColor ink(226, 232, 240);
    const QColor muted(148, 163, 184);
    const QColor teal(94, 234, 212);
    const QColor orange(251, 146, 60);
    const QColor red(248, 113, 113);
    const QPen stroke(ink, 2.15, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    const QPen softStroke(QColor(226, 232, 240, 175), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(stroke);
    painter.setBrush(Qt::NoBrush);

    switch (action) {
    case ShotWindow::Action::ToolMove:
        painter.setPen(QPen(teal, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(16, 5), QPointF(16, 27));
        painter.drawLine(QPointF(5, 16), QPointF(27, 16));
        painter.drawLine(QPointF(16, 5), QPointF(12, 9));
        painter.drawLine(QPointF(16, 5), QPointF(20, 9));
        painter.drawLine(QPointF(16, 27), QPointF(12, 23));
        painter.drawLine(QPointF(16, 27), QPointF(20, 23));
        painter.drawLine(QPointF(5, 16), QPointF(9, 12));
        painter.drawLine(QPointF(5, 16), QPointF(9, 20));
        painter.drawLine(QPointF(27, 16), QPointF(23, 12));
        painter.drawLine(QPointF(27, 16), QPointF(23, 20));
        break;
    case ShotWindow::Action::ToolPen: {
        QPolygonF nib;
        nib << QPointF(9, 23) << QPointF(12.5, 14.2) << QPointF(20.8, 5.8) << QPointF(26.2, 11.2)
            << QPointF(17.8, 19.5) << QPointF(9, 23);
        painter.setBrush(QColor(94, 234, 212, 38));
        painter.drawPolygon(nib);
        painter.setPen(QPen(teal, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(13, 14), QPointF(18, 19));
        painter.setPen(stroke);
        painter.drawLine(QPointF(7, 25), QPointF(12, 23));
        break;
    }
    case ShotWindow::Action::ToolLine:
        painter.setPen(QPen(teal, 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(8, 24), QPointF(24, 8));
        painter.setBrush(teal);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(8, 24), 2.1, 2.1);
        painter.drawEllipse(QPointF(24, 8), 2.1, 2.1);
        break;
    case ShotWindow::Action::ToolHighlighter:
        painter.setPen(QPen(QColor(251, 191, 36, 172), 7.0, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(8, 21), QPointF(24, 13));
        painter.setPen(stroke);
        painter.drawRoundedRect(QRectF(9, 8, 12, 8), 2.5, 2.5);
        painter.drawLine(QPointF(19, 14), QPointF(24, 19));
        break;
    case ShotWindow::Action::ToolRectangle:
        painter.setPen(QPen(ink, 2.05, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawRoundedRect(QRectF(7, 9, 18, 14), 3.5, 3.5);
        painter.setPen(QPen(teal, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(12, 9), QPointF(20, 9));
        break;
    case ShotWindow::Action::ToolEllipse:
        painter.drawEllipse(QRectF(6.5, 8.5, 19, 15));
        painter.setPen(QPen(teal, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawArc(QRectF(6.5, 8.5, 19, 15), 40 * 16, 90 * 16);
        break;
    case ShotWindow::Action::ToolArrow:
        painter.setPen(QPen(teal, 2.35, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(8, 24), QPointF(24, 8));
        painter.drawLine(QPointF(24, 8), QPointF(23, 16));
        painter.drawLine(QPointF(24, 8), QPointF(16, 9));
        break;
    case ShotWindow::Action::ToolText:
        painter.setPen(softStroke);
        painter.drawLine(QPointF(8, 8), QPointF(24, 8));
        painter.setPen(stroke);
        painter.drawLine(QPointF(16, 8), QPointF(16, 24));
        painter.drawLine(QPointF(12, 24), QPointF(20, 24));
        break;
    case ShotWindow::Action::ToolNumber:
        painter.setPen(Qt::NoPen);
        painter.setBrush(orange);
        painter.drawEllipse(QRectF(6, 6, 20, 20));
        painter.setPen(QColor(67, 20, 7));
        painter.setFont(QFont(QStringLiteral("Sans Serif"), 12, QFont::Black));
        painter.drawText(QRectF(6, 6, 20, 20), Qt::AlignCenter, QStringLiteral("1"));
        break;
    case ShotWindow::Action::ToolMosaic:
        painter.setPen(Qt::NoPen);
        for (int y = 7; y < 25; y += 6) {
            for (int x = 7; x < 25; x += 6) {
                const int alpha = ((x + y) / 6) % 2 ? 230 : 110;
                painter.setBrush(QColor(226, 232, 240, alpha));
                painter.drawRoundedRect(QRectF(x, y, 4.5, 4.5), 1.0, 1.0);
            }
        }
        break;
    case ShotWindow::Action::Undo:
        painter.setPen(QPen(ink, 2.25, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath([] {
            QPainterPath path;
            path.moveTo(10.0, 13.0);
            path.cubicTo(13.0, 8.3, 20.6, 8.0, 23.6, 12.5);
            path.cubicTo(27.0, 17.5, 23.4, 24.0, 16.2, 24.0);
            return path;
        }());
        painter.setBrush(ink);
        painter.setPen(Qt::NoPen);
        {
            QPolygonF head;
            head << QPointF(8.2, 13.2) << QPointF(15.0, 8.6) << QPointF(14.1, 16.8);
            painter.drawPolygon(head);
        }
        break;
    case ShotWindow::Action::Redo:
        painter.setPen(QPen(ink, 2.25, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawPath([] {
            QPainterPath path;
            path.moveTo(22.0, 13.0);
            path.cubicTo(19.0, 8.3, 11.4, 8.0, 8.4, 12.5);
            path.cubicTo(5.0, 17.5, 8.6, 24.0, 15.8, 24.0);
            return path;
        }());
        painter.setBrush(ink);
        painter.setPen(Qt::NoPen);
        {
            QPolygonF head;
            head << QPointF(23.8, 13.2) << QPointF(17.0, 8.6) << QPointF(17.9, 16.8);
            painter.drawPolygon(head);
        }
        break;
    case ShotWindow::Action::OpenWith:
        painter.drawRoundedRect(QRectF(7, 10, 14, 14), 3.0, 3.0);
        painter.setPen(QPen(teal, 2.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(17, 7), QPointF(25, 7));
        painter.drawLine(QPointF(25, 7), QPointF(25, 15));
        painter.drawLine(QPointF(24, 8), QPointF(15, 17));
        break;
    case ShotWindow::Action::Copy:
        painter.setPen(softStroke);
        painter.drawRoundedRect(QRectF(11, 7, 12, 15), 3.0, 3.0);
        painter.setPen(stroke);
        painter.drawRoundedRect(QRectF(7, 11, 12, 15), 3.0, 3.0);
        break;
    case ShotWindow::Action::Save:
        painter.setPen(QPen(QColor(67, 20, 7), 2.15, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawRoundedRect(QRectF(8, 7, 16, 18), 3.2, 3.2);
        painter.drawLine(QPointF(12, 8), QPointF(12, 14));
        painter.drawLine(QPointF(20, 8), QPointF(20, 14));
        painter.drawLine(QPointF(12, 22), QPointF(20, 22));
        break;
    case ShotWindow::Action::Cancel:
        painter.setPen(QPen(red, 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(QPointF(10, 10), QPointF(22, 22));
        painter.drawLine(QPointF(22, 10), QPointF(10, 22));
        break;
    }

    painter.end();
    return QIcon(pixmap);
}

QString desktopEntryValue(const QStringList &lines, const QString &key)
{
    bool inDesktopEntry = false;
    const QString prefix = key + QLatin1Char('=');
    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith(QLatin1Char('#'))) {
            continue;
        }
        if (trimmed.startsWith(QLatin1Char('[')) && trimmed.endsWith(QLatin1Char(']'))) {
            inDesktopEntry = trimmed == QStringLiteral("[Desktop Entry]");
            continue;
        }
        if (inDesktopEntry && trimmed.startsWith(prefix)) {
            return trimmed.mid(prefix.size()).trimmed();
        }
    }
    return {};
}

bool desktopEntryBool(const QStringList &lines, const QString &key)
{
    const QString value = desktopEntryValue(lines, key).toLower();
    return value == QStringLiteral("true") || value == QStringLiteral("1");
}

bool desktopEntrySupportsImage(const QStringList &lines)
{
    const QStringList mimeTypes = desktopEntryValue(lines, QStringLiteral("MimeType"))
        .split(QLatin1Char(';'), Qt::SkipEmptyParts);
    for (const QString &mimeType : mimeTypes) {
        const QString normalized = mimeType.trimmed().toLower();
        if (normalized == QStringLiteral("image/png") || normalized == QStringLiteral("image/*")
            || normalized.startsWith(QStringLiteral("image/"))) {
            return true;
        }
    }
    return false;
}

QStringList desktopSearchDirs()
{
    QStringList dataDirs;
    dataDirs << QDir::home().filePath(QStringLiteral(".local/share"));

    const QString envDataDirs = QProcessEnvironment::systemEnvironment().value(
        QStringLiteral("XDG_DATA_DIRS"),
        QStringLiteral("/usr/local/share:/usr/share"));
    dataDirs << envDataDirs.split(QLatin1Char(':'), Qt::SkipEmptyParts);
    dataDirs.removeDuplicates();

    QStringList appDirs;
    for (const QString &dataDir : dataDirs) {
        appDirs << QDir(dataDir).filePath(QStringLiteral("applications"));
    }
    appDirs.removeDuplicates();
    return appDirs;
}

QStringList expandDesktopExec(const ShotWindow::DesktopApp &app, const QString &imagePath)
{
    QStringList command = QProcess::splitCommand(app.exec);
    if (command.isEmpty()) {
        return {};
    }

    const QString fileUrl = QUrl::fromLocalFile(imagePath).toString();
    bool usedFileField = false;
    QStringList expanded;
    for (QString argument : command) {
        if (argument == QStringLiteral("%i")) {
            continue;
        }

        bool appendArgument = true;
        if (argument.contains(QLatin1Char('%'))) {
            if (argument.contains(QStringLiteral("%f")) || argument.contains(QStringLiteral("%F"))) {
                argument.replace(QStringLiteral("%f"), imagePath);
                argument.replace(QStringLiteral("%F"), imagePath);
                usedFileField = true;
            }
            if (argument.contains(QStringLiteral("%u")) || argument.contains(QStringLiteral("%U"))) {
                argument.replace(QStringLiteral("%u"), fileUrl);
                argument.replace(QStringLiteral("%U"), fileUrl);
                usedFileField = true;
            }
            argument.replace(QStringLiteral("%c"), app.name);
            argument.replace(QStringLiteral("%k"), app.desktopPath);
            argument.replace(QStringLiteral("%%"), QStringLiteral("%"));

            static const QStringList unsupportedFields = {
                QStringLiteral("%d"), QStringLiteral("%D"), QStringLiteral("%n"), QStringLiteral("%N"),
                QStringLiteral("%v"), QStringLiteral("%m"),
            };
            for (const QString &field : unsupportedFields) {
                argument.remove(field);
            }
            appendArgument = !argument.trimmed().isEmpty();
        }

        if (appendArgument) {
            expanded.append(argument);
        }
    }

    if (!usedFileField) {
        expanded.append(imagePath);
    }
    return expanded;
}

} // namespace

ShotWindow::ShotWindow(QImage frozenFrame, QString outputName, QWidget *parent)
    : QWidget(parent)
    , m_frozenFrame(std::move(frozenFrame))
    , m_outputName(std::move(outputName))
{
    setWindowTitle(QStringLiteral("Mark Shot"));
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    m_toolbar = new QWidget(this);
    m_toolbar->setObjectName(QStringLiteral("shotToolbar"));
    m_toolbar->setStyleSheet(panelStyleSheet());
    m_toolbar->installEventFilter(this);

    auto *layout = new QHBoxLayout(m_toolbar);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(7);

    layout->addWidget(addToolbarButton(Action::ToolMove, QStringLiteral("V")));
    layout->addWidget(addToolbarButton(Action::ToolPen, QStringLiteral("P")));
    layout->addWidget(addToolbarButton(Action::ToolLine, QStringLiteral("L")));
    layout->addWidget(addToolbarButton(Action::ToolHighlighter, QStringLiteral("H")));
    layout->addWidget(addToolbarButton(Action::ToolRectangle, QStringLiteral("R")));
    layout->addWidget(addToolbarButton(Action::ToolEllipse, QStringLiteral("E")));
    layout->addWidget(addToolbarButton(Action::ToolArrow, QStringLiteral("A")));
    layout->addWidget(addToolbarButton(Action::ToolText, QStringLiteral("T")));
    layout->addWidget(addToolbarButton(Action::ToolNumber, QStringLiteral("N")));
    layout->addWidget(addToolbarButton(Action::ToolMosaic, QStringLiteral("M")));
    layout->addWidget(addToolbarButton(Action::Undo, QStringLiteral("Ctrl+Z")));
    layout->addWidget(addToolbarButton(Action::Redo, QStringLiteral("Ctrl+Shift+Z")));
    for (Action action : {Action::OpenWith, Action::Copy, Action::Save, Action::Cancel}) {
        const QString shortcut = action == Action::OpenWith ? QStringLiteral("Open")
            : action == Action::Copy                 ? QStringLiteral("Ctrl+C")
            : action == Action::Save                 ? QStringLiteral("Ctrl+S")
                                                     : QStringLiteral("Esc");
        QPushButton *button = addToolbarButton(action, shortcut);
        button->hide();
        m_fullscreenActionButtons.append(button);
        layout->addWidget(button);
    }
    m_toolbar->hide();

    m_actionToolbar = new QWidget(this);
    m_actionToolbar->setObjectName(QStringLiteral("actionToolbar"));
    m_actionToolbar->setStyleSheet(m_toolbar->styleSheet());
    auto *actionLayout = new QVBoxLayout(m_actionToolbar);
    actionLayout->setContentsMargins(10, 10, 10, 10);
    actionLayout->setSpacing(7);
    actionLayout->addWidget(addToolbarButton(Action::OpenWith, QStringLiteral("Open"), m_actionToolbar));
    actionLayout->addWidget(addToolbarButton(Action::Copy, QStringLiteral("Ctrl+C"), m_actionToolbar));
    actionLayout->addWidget(addToolbarButton(Action::Save, QStringLiteral("Ctrl+S"), m_actionToolbar));
    actionLayout->addWidget(addToolbarButton(Action::Cancel, QStringLiteral("Esc"), m_actionToolbar));
    m_actionToolbar->hide();

    m_openWithPanel = new QWidget(this);
    m_openWithPanel->setObjectName(QStringLiteral("openWithPanel"));
    m_openWithPanel->setStyleSheet(openWithPanelStyleSheet());
    auto *openLayout = new QVBoxLayout(m_openWithPanel);
    openLayout->setContentsMargins(12, 12, 12, 12);
    openLayout->setSpacing(7);
    m_openWithPanel->hide();

    m_colorPalette = new QWidget(this);
    m_colorPalette->setObjectName(QStringLiteral("colorPalette"));
    m_colorPalette->setStyleSheet(colorPaletteStyleSheet());
    for (const QColor &color : paletteColors()) {
        auto *button = new QPushButton(m_colorPalette);
        button->setFocusPolicy(Qt::NoFocus);
        button->setStyleSheet(QStringLiteral("background: %1;").arg(color.name()));
        connect(button, &QPushButton::clicked, this, [this, color] { setCurrentColor(color); });
    }
    m_colorPalettePreview = new QWidget(m_colorPalette);
    m_colorPalettePreview->setObjectName(QStringLiteral("colorPalettePreview"));
    m_colorPalette->hide();
    updateColorPalettePreview();

    m_textEditor = new QTextEdit(this);
    m_textEditor->setObjectName(QStringLiteral("textEditor"));
    m_textEditor->setPlaceholderText(QStringLiteral("Type text"));
    m_textEditor->setStyleSheet(textEditorStyleSheet(QColor(94, 234, 212), 24));
    m_textEditor->setAcceptRichText(false);
    m_textEditor->setTabChangesFocus(false);
    m_textEditor->setFrameShape(QFrame::NoFrame);
    m_textEditor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEditor->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEditor->viewport()->setAutoFillBackground(false);
    m_textEditor->setToolTip(QStringLiteral("Enter inserts newline, click outside commits, Esc cancels"));
    m_textEditor->hide();
    m_textEditor->installEventFilter(this);
}

bool ShotWindow::configureLayerShell(QScreen *screen)
{
    if (screen) {
        setScreen(screen);
    } else {
        resize(m_frozenFrame.size());
    }

    setAttribute(Qt::WA_NativeWindow);
    winId();

    QWindow *nativeWindow = windowHandle();
    if (!nativeWindow) {
        return false;
    }

    if (screen) {
        nativeWindow->setScreen(screen);
    }

    LayerShellQt::Window *layerWindow = LayerShellQt::Window::get(nativeWindow);
    if (!layerWindow) {
        return false;
    }

    LayerShellQt::Window::Anchors anchors = LayerShellQt::Window::AnchorTop;
    anchors |= LayerShellQt::Window::AnchorBottom;
    anchors |= LayerShellQt::Window::AnchorLeft;
    anchors |= LayerShellQt::Window::AnchorRight;

    layerWindow->setScope(QStringLiteral("mark-shot"));
    layerWindow->setLayer(LayerShellQt::Window::LayerOverlay);
    layerWindow->setAnchors(anchors);
    layerWindow->setMargins({});
    layerWindow->setExclusiveZone(-1);
    layerWindow->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityExclusive);
    layerWindow->setActivateOnShow(true);
    layerWindow->setCloseOnDismissed(true);
    if (screen) {
        layerWindow->setScreen(screen);
        layerWindow->setDesiredSize({});
    } else {
        layerWindow->setWantsToBeOnActiveScreen(true);
        layerWindow->setDesiredSize({});
    }

    return true;
}

void ShotWindow::startFullscreenAnnotation()
{
    commitTextEditor();
    if (m_colorPalette) {
        m_colorPalette->hide();
    }

    m_mode = Mode::Editing;
    m_dragging = false;
    m_fullscreenAnnotation = true;
    m_toolbarDragging = false;
    m_toolbarUserPlaced = false;
    m_selection = QRectF(QPointF(0, 0), QSizeF(m_frozenFrame.size()));
    m_annotations.clear();
    m_redoStack.clear();
    m_draft.reset();
    m_nextNumber = 1;
    if (m_openWithPanel) {
        m_openWithPanel->hide();
    }
    setTool(Tool::Pen);
    if (m_toolbar) {
        setFullscreenActionButtonsVisible(true);
        m_toolbar->show();
    }
    if (m_actionToolbar) {
        m_actionToolbar->hide();
    }
    updateToolbarGeometry();
    update();
}

QPushButton *ShotWindow::addToolbarButton(Action action, const QString &shortcutText, QWidget *parentToolbar)
{
    QWidget *toolbar = parentToolbar ? parentToolbar : m_toolbar;
    auto *button = new QPushButton(toolbar);
    button->setIcon(makeToolIcon(action));
    button->setIconSize(QSize(26, 26));
    button->setFocusPolicy(Qt::NoFocus);
    button->setToolTip(QStringLiteral("%1 (%2)").arg(actionName(action), shortcutText));
    button->setProperty("action", actionName(action));
    if (!parentToolbar && action == Action::ToolMove) {
        button->installEventFilter(this);
    }
    if (action == Action::Save) {
        button->setProperty("role", QStringLiteral("primary"));
    } else if (action == Action::Cancel) {
        button->setProperty("role", QStringLiteral("danger"));
    } else if (action == Action::OpenWith || action == Action::Copy) {
        button->setProperty("role", QStringLiteral("secondary"));
    }

    if (action == Action::ToolMove) {
        connect(button, &QPushButton::clicked, this, [this] { setTool(Tool::Move); });
    } else if (action == Action::ToolPen) {
        connect(button, &QPushButton::clicked, this, [this] { setTool(Tool::Pen); });
    } else if (action == Action::ToolLine) {
        connect(button, &QPushButton::clicked, this, [this] { setTool(Tool::Line); });
    } else if (action == Action::ToolHighlighter) {
        connect(button, &QPushButton::clicked, this, [this] { setTool(Tool::Highlighter); });
    } else if (action == Action::ToolRectangle) {
        connect(button, &QPushButton::clicked, this, [this] { setTool(Tool::Rectangle); });
    } else if (action == Action::ToolEllipse) {
        connect(button, &QPushButton::clicked, this, [this] { setTool(Tool::Ellipse); });
    } else if (action == Action::ToolArrow) {
        connect(button, &QPushButton::clicked, this, [this] { setTool(Tool::Arrow); });
    } else if (action == Action::ToolText) {
        connect(button, &QPushButton::clicked, this, [this] { setTool(Tool::Text); });
    } else if (action == Action::ToolNumber) {
        connect(button, &QPushButton::clicked, this, [this] { setTool(Tool::Number); });
    } else if (action == Action::ToolMosaic) {
        connect(button, &QPushButton::clicked, this, [this] { setTool(Tool::Mosaic); });
    } else if (action == Action::Undo) {
        connect(button, &QPushButton::clicked, this, [this] {
            if (!m_annotations.isEmpty()) {
                Annotation annotation = m_annotations.takeLast();
                if (annotation.tool == Tool::Number && m_nextNumber > 1) {
                    --m_nextNumber;
                }
                m_redoStack.append(annotation);
                update();
            }
        });
    } else if (action == Action::Redo) {
        connect(button, &QPushButton::clicked, this, [this] { redoAnnotation(); });
    } else if (action == Action::OpenWith) {
        connect(button, &QPushButton::clicked, this, [this] { toggleOpenWithPanel(); });
    } else if (action == Action::Copy) {
        connect(button, &QPushButton::clicked, this, [this] { copySelection(); });
    } else if (action == Action::Save) {
        connect(button, &QPushButton::clicked, this, [this] { saveSelection(); });
    } else if (action == Action::Cancel) {
        connect(button, &QPushButton::clicked, this, [this] { close(); });
    }

    return button;
}

QVector<ShotWindow::DesktopApp> ShotWindow::imageDesktopApps() const
{
    QVector<DesktopApp> apps;
    QStringList seenPaths;

    for (const QString &appDir : desktopSearchDirs()) {
        if (!QDir(appDir).exists()) {
            continue;
        }

        QDirIterator iterator(appDir, {QStringLiteral("*.desktop")}, QDir::Files, QDirIterator::Subdirectories);
        while (iterator.hasNext()) {
            const QString desktopPath = iterator.next();
            if (seenPaths.contains(desktopPath)) {
                continue;
            }
            seenPaths.append(desktopPath);

            QFile file(desktopPath);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                continue;
            }

            const QStringList lines = QString::fromUtf8(file.readAll()).split(QLatin1Char('\n'));
            if (desktopEntryValue(lines, QStringLiteral("Type")) != QStringLiteral("Application")) {
                continue;
            }
            if (desktopEntryBool(lines, QStringLiteral("Hidden"))
                || desktopEntryBool(lines, QStringLiteral("NoDisplay"))
                || !desktopEntrySupportsImage(lines)) {
                continue;
            }

            const QString exec = desktopEntryValue(lines, QStringLiteral("Exec"));
            const QString name = desktopEntryValue(lines, QStringLiteral("Name"));
            if (exec.isEmpty() || name.isEmpty()) {
                continue;
            }

            apps.append({name, desktopPath, exec});
        }
    }

    std::sort(apps.begin(), apps.end(), [](const DesktopApp &left, const DesktopApp &right) {
        return QString::localeAwareCompare(left.name, right.name) < 0;
    });
    return apps;
}

bool ShotWindow::eventFilter(QObject *watched, QEvent *event)
{
    const bool isFullscreenMoveButton = m_fullscreenAnnotation
        && watched->property("action").toString() == actionName(Action::ToolMove);
    if (isFullscreenMoveButton) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                auto *eventWidget = qobject_cast<QWidget *>(watched);
                if (!eventWidget) {
                    return false;
                }
                m_dragging = true;
                m_toolbarDragging = true;
                m_toolbarDragStart = eventWidget->mapTo(this, mouseEvent->pos());
                m_toolbarBeforeDrag = m_toolbar->geometry();
                setCursor(Qt::SizeAllCursor);
                return true;
            }
        } else if (event->type() == QEvent::MouseMove && m_toolbarDragging) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            auto *eventWidget = qobject_cast<QWidget *>(watched);
            if (!eventWidget) {
                return false;
            }
            const QPoint delta = eventWidget->mapTo(this, mouseEvent->pos()) - m_toolbarDragStart;
            m_toolbarUserPlaced = true;
            m_toolbar->setGeometry(clampedToolbarGeometry(m_toolbarBeforeDrag.translated(delta)));
            updateOpenWithPanelGeometry();
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease && m_toolbarDragging) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = false;
                m_toolbarDragging = false;
                updateCursor();
                updateOpenWithPanelGeometry();
                return true;
            }
        }
    }

    if (watched == m_textEditor && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            m_draft.reset();
            m_textEditor->hide();
            m_textEditor->clear();
            setFocus(Qt::OtherFocusReason);
            update();
            return true;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void ShotWindow::setFullscreenActionButtonsVisible(bool visible)
{
    for (QPushButton *button : std::as_const(m_fullscreenActionButtons)) {
        if (button) {
            button->setVisible(visible);
        }
    }
}

void ShotWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(0, 0, 0));
    painter.drawImage(m_frozenImageRect, m_frozenFrame);

    const QRectF selection = normalizedSelection();
    QPainterPath dimPath;
    dimPath.addRect(rect());
    if (hasUsableSelection()) {
        dimPath.addRect(imageRectToWidget(selection));
        painter.fillPath(dimPath, QColor(2, 6, 12, 128));
    } else {
        painter.fillRect(rect(), QColor(2, 6, 12, 88));
    }

    if (hasUsableSelection()) {
        const QRectF widgetSelection = imageRectToWidget(selection);
        painter.save();
        for (const Annotation &annotation : m_annotations) {
            drawAnnotation(painter, annotation, true);
        }
        if (m_draft.has_value()) {
            drawAnnotation(painter, *m_draft, true);
        }
        painter.restore();

        painter.setPen(QPen(QColor(94, 234, 212), 2.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(widgetSelection, 3.0, 3.0);

        if (m_tool == Tool::Move && !m_fullscreenAnnotation) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(94, 234, 212));
            const QVector<QPointF> handles = {
                widgetSelection.topLeft(), QPointF(widgetSelection.center().x(), widgetSelection.top()), widgetSelection.topRight(),
                QPointF(widgetSelection.left(), widgetSelection.center().y()), QPointF(widgetSelection.right(), widgetSelection.center().y()),
                widgetSelection.bottomLeft(), QPointF(widgetSelection.center().x(), widgetSelection.bottom()), widgetSelection.bottomRight(),
            };
            for (const QPointF &handle : handles) {
                painter.drawRoundedRect(QRectF(handle.x() - 4.0, handle.y() - 4.0, 8.0, 8.0), 2.0, 2.0);
            }
        }

        const bool selectionInfoVisible = m_selectionDrag != SelectionDrag::None
            || (m_showSelectionInfo && m_selectionInfoTimer.isValid() && m_selectionInfoTimer.elapsed() <= 1000);
        if (selectionInfoVisible) {
            const QString sizeText = QStringLiteral("%1 x %2").arg(qRound(selection.width())).arg(qRound(selection.height()));
            painter.setFont(QFont(QStringLiteral("Sans Serif"), 11, QFont::DemiBold));
            const QFontMetrics metrics(painter.font());
            const QRectF labelRect(widgetSelection.left() + 10.0,
                                   widgetSelection.top() + 10.0,
                                   metrics.horizontalAdvance(sizeText) + 22.0,
                                   metrics.height() + 12.0);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(8, 13, 19, 220));
            painter.drawRoundedRect(labelRect, 10.0, 10.0);
            painter.setPen(QColor(204, 251, 241, 238));
            painter.drawText(labelRect, Qt::AlignCenter, sizeText);
        } else if (m_showSelectionInfo) {
            m_showSelectionInfo = false;
        }
    }

    if (!hasUsableSelection()) {
        const QString hint = QStringLiteral("Drag to select   Esc cancels");
        painter.setFont(QFont(QStringLiteral("Sans Serif"), 15, QFont::DemiBold));
        const QFontMetrics metrics(painter.font());
        const QRectF hintRect((width() - metrics.horizontalAdvance(hint) - 44.0) / 2.0,
                              (height() - metrics.height() - 24.0) / 2.0,
                              metrics.horizontalAdvance(hint) + 44.0,
                              metrics.height() + 24.0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(8, 13, 19, 222));
        painter.drawRoundedRect(hintRect, 18.0, 18.0);
        painter.setPen(QColor(204, 251, 241, 240));
        painter.drawText(hintRect, Qt::AlignCenter, hint);
    }

    drawWheelPreview(painter);
}

void ShotWindow::resizeEvent(QResizeEvent *)
{
    updateFrozenImageRect();
    if (m_colorPalette && m_colorPalette->isVisible()) {
        updateColorPaletteGeometry(m_colorPaletteAnchor);
    }
    updateTextEditorGeometry();
    updateToolbarGeometry();
    updateActionToolbarGeometry();
    updateOpenWithPanelGeometry();
}

void ShotWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        if (event->button() == Qt::RightButton && m_mode == Mode::Editing) {
            toggleColorPalette(event->pos());
        }
        return;
    }

    const QPointF imagePoint = widgetToImage(event->position());
    if (m_openWithPanel && m_openWithPanel->isVisible()
        && !m_openWithPanel->geometry().contains(event->pos())
        && (!m_actionToolbar || !m_actionToolbar->geometry().contains(event->pos()))
        && (!m_toolbar || !m_toolbar->geometry().contains(event->pos()))) {
        m_openWithPanel->hide();
    }
    if (m_textEditor && m_textEditor->isVisible() && !m_textEditor->geometry().contains(event->pos())) {
        commitTextEditor();
    }

    if (m_mode == Mode::Selecting) {
        if (m_colorPalette) {
            m_colorPalette->hide();
        }
        beginSelection(imagePoint);
        return;
    }

    if (!m_frozenImageRect.contains(event->position())) {
        return;
    }

    if (m_tool == Tool::Move && !m_fullscreenAnnotation) {
        m_selectionDrag = selectionDragAt(imagePoint);
        if (m_selectionDrag == SelectionDrag::None) {
            updateCursor();
            return;
        }
        m_dragging = true;
        m_dragStart = imagePoint;
        m_selectionBeforeDrag = normalizedSelection();
        revealSelectionInfo();
        updateCursor();
        update();
        return;
    }

    if (m_tool == Tool::Text) {
        commitTextEditor();
        beginTextAnnotation(imagePoint);
        return;
    }

    if (m_tool == Tool::Number) {
        Annotation annotation;
        annotation.tool = Tool::Number;
        annotation.points.append(imagePoint);
        annotation.number = m_nextNumber++;
        annotation.color = m_currentColor;
        annotation.width = m_shapeWidth;
        m_annotations.append(annotation);
        m_redoStack.clear();
        update();
        return;
    }

    m_dragging = true;
    m_dragStart = imagePoint;
    Annotation annotation;
    annotation.tool = m_tool;
    annotation.color = m_currentColor;
    annotation.width = currentToolWidth();
    if (m_tool == Tool::Pen || m_tool == Tool::Highlighter) {
        annotation.points.append(imagePoint);
    } else if (m_tool == Tool::Mosaic) {
        annotation.width = m_mosaicBlockSize;
        annotation.rect = QRectF(imagePoint, imagePoint);
        annotation.points.append(imagePoint);
        annotation.points.append(imagePoint);
    } else {
        annotation.rect = QRectF(imagePoint, imagePoint);
        annotation.points.append(imagePoint);
        annotation.points.append(imagePoint);
    }
    m_draft = annotation;
    update();
}

void ShotWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_showWheelPreview && m_wheelPreviewTimer.isValid() && m_wheelPreviewTimer.elapsed() <= 900) {
        m_wheelPreviewPosition = event->position();
        update();
    } else if (m_showWheelPreview) {
        m_showWheelPreview = false;
        updateCursor();
        update();
    }

    const QPointF imagePoint = widgetToImage(event->position());
    if (m_mode == Mode::Selecting && m_dragging) {
        m_selection = normalizedRect(m_selectionStart, imagePoint);
        revealSelectionInfo();
        update();
        return;
    }

    if (m_fullscreenAnnotation && m_toolbarDragging) {
        const QPoint delta = event->pos() - m_toolbarDragStart;
        QRect toolbarGeometry = m_toolbarBeforeDrag.translated(delta);
        if (m_toolbar) {
            m_toolbarUserPlaced = true;
            m_toolbar->setGeometry(clampedToolbarGeometry(toolbarGeometry));
        }
        updateOpenWithPanelGeometry();
        return;
    }

    if (m_mode == Mode::Editing && m_tool == Tool::Move && !m_fullscreenAnnotation && !m_dragging) {
        const SelectionDrag hoverDrag = selectionDragAt(imagePoint);
        switch (hoverDrag) {
        case SelectionDrag::Left:
        case SelectionDrag::Right:
            setCursor(Qt::SizeHorCursor);
            break;
        case SelectionDrag::Top:
        case SelectionDrag::Bottom:
            setCursor(Qt::SizeVerCursor);
            break;
        case SelectionDrag::TopLeft:
        case SelectionDrag::BottomRight:
            setCursor(Qt::SizeFDiagCursor);
            break;
        case SelectionDrag::TopRight:
        case SelectionDrag::BottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            break;
        case SelectionDrag::Move:
            setCursor(Qt::SizeAllCursor);
            break;
        case SelectionDrag::None:
            setCursor(Qt::CrossCursor);
            break;
        }
        return;
    }

    if (m_mode == Mode::Editing && m_tool == Tool::Move && !m_fullscreenAnnotation && m_dragging && m_selectionDrag != SelectionDrag::None) {
        const QPointF clamped = clampImagePoint(imagePoint);
        const QRectF start = m_selectionBeforeDrag;
        const qreal maxWidth = m_frozenFrame.width();
        const qreal maxHeight = m_frozenFrame.height();
        qreal left = start.left();
        qreal top = start.top();
        qreal right = start.right();
        qreal bottom = start.bottom();

        if (m_selectionDrag == SelectionDrag::Move) {
            const QPointF delta = clamped - m_dragStart;
            left = std::clamp(start.left() + delta.x(), 0.0, std::max<qreal>(0.0, maxWidth - start.width()));
            top = std::clamp(start.top() + delta.y(), 0.0, std::max<qreal>(0.0, maxHeight - start.height()));
            right = left + start.width();
            bottom = top + start.height();
        } else {
            if (m_selectionDrag == SelectionDrag::Left || m_selectionDrag == SelectionDrag::TopLeft
                || m_selectionDrag == SelectionDrag::BottomLeft) {
                left = std::clamp(clamped.x(), 0.0, right - kMinSelectionSize);
            }
            if (m_selectionDrag == SelectionDrag::Right || m_selectionDrag == SelectionDrag::TopRight
                || m_selectionDrag == SelectionDrag::BottomRight) {
                right = std::clamp(clamped.x(), left + kMinSelectionSize, maxWidth);
            }
            if (m_selectionDrag == SelectionDrag::Top || m_selectionDrag == SelectionDrag::TopLeft
                || m_selectionDrag == SelectionDrag::TopRight) {
                top = std::clamp(clamped.y(), 0.0, bottom - kMinSelectionSize);
            }
            if (m_selectionDrag == SelectionDrag::Bottom || m_selectionDrag == SelectionDrag::BottomLeft
                || m_selectionDrag == SelectionDrag::BottomRight) {
                bottom = std::clamp(clamped.y(), top + kMinSelectionSize, maxHeight);
            }
        }

        m_selection = QRectF(QPointF(left, top), QPointF(right, bottom)).normalized();
        revealSelectionInfo();
        updateToolbarGeometry();
        updateActionToolbarGeometry();
        updateOpenWithPanelGeometry();
        updateTextEditorGeometry();
        update();
        return;
    }

    if (m_mode != Mode::Editing || !m_dragging || !m_draft.has_value()) {
        return;
    }

    const QPointF clamped = clampImagePoint(imagePoint);
    if (m_draft->tool == Tool::Pen || m_draft->tool == Tool::Highlighter) {
        m_draft->points.append(clamped);
    } else {
        if ((m_draft->tool == Tool::Rectangle || m_draft->tool == Tool::Ellipse)
            && event->modifiers().testFlag(Qt::ControlModifier)) {
            m_draft->rect = constrainedRect(m_dragStart, clamped);
        } else {
            m_draft->rect = normalizedRect(m_dragStart, clamped);
        }
        if (m_draft->points.size() >= 2) {
            m_draft->points[1] = (m_draft->tool == Tool::Rectangle || m_draft->tool == Tool::Ellipse)
                ? m_draft->rect.bottomRight()
                : clamped;
        }
    }
    update();
}

void ShotWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || !m_dragging) {
        return;
    }

    m_dragging = false;
    if (m_toolbarDragging) {
        m_toolbarDragging = false;
        updateCursor();
        updateOpenWithPanelGeometry();
        return;
    }

    if (m_mode == Mode::Selecting) {
        m_selection = normalizedSelection();
        if (!hasUsableSelection()) {
            m_selection = {};
            update();
            return;
        }
        m_mode = Mode::Editing;
        m_fullscreenAnnotation = false;
        m_toolbarUserPlaced = false;
        setTool(Tool::Pen);
        setFullscreenActionButtonsVisible(false);
        m_toolbar->show();
        m_actionToolbar->show();
        revealSelectionInfo();
        updateToolbarGeometry();
        updateActionToolbarGeometry();
        update();
        return;
    }

    if (m_tool == Tool::Move && !m_fullscreenAnnotation && m_selectionDrag != SelectionDrag::None) {
        m_selection = normalizedSelection();
        m_selectionDrag = SelectionDrag::None;
        revealSelectionInfo();
        updateCursor();
        updateToolbarGeometry();
        updateActionToolbarGeometry();
        updateOpenWithPanelGeometry();
        update();
        return;
    }

    commitDraft();
}

void ShotWindow::wheelEvent(QWheelEvent *event)
{
    const int steps = event->angleDelta().y() / 120;
    if (steps == 0 || m_mode != Mode::Editing) {
        QWidget::wheelEvent(event);
        return;
    }

    if (m_tool == Tool::Mosaic) {
        m_mosaicBlockSize = std::clamp(m_mosaicBlockSize + steps * 2.0, kMinMosaicBlockSize, kMaxMosaicBlockSize);
    } else if (m_tool == Tool::Pen || m_tool == Tool::Highlighter) {
        m_penWidth = std::clamp(m_penWidth + steps * 1.0, kMinStrokeWidth, kMaxStrokeWidth);
    } else {
        m_shapeWidth = std::clamp(m_shapeWidth + steps * 1.0, kMinStrokeWidth, kMaxStrokeWidth);
    }

    if (m_draft.has_value()) {
        m_draft->width = currentToolWidth();
    }
    m_showWheelPreview = true;
    m_wheelPreviewPosition = event->position();
    m_wheelPreviewTimer.restart();
    updateCursor();
    updateColorPalettePreview();
    event->accept();
    update();
}

void ShotWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        return;
    }

    if (event->matches(QKeySequence::Copy)) {
        commitTextEditor();
        copySelection();
        return;
    }

    if (event->matches(QKeySequence::Save)) {
        commitTextEditor();
        saveSelection();
        return;
    }

    if (event->matches(QKeySequence::Undo)) {
        if (!m_annotations.isEmpty()) {
            Annotation annotation = m_annotations.takeLast();
            if (annotation.tool == Tool::Number && m_nextNumber > 1) {
                --m_nextNumber;
            }
            m_redoStack.append(annotation);
            update();
        }
        return;
    }

    if (event->matches(QKeySequence::Redo)) {
        redoAnnotation();
        return;
    }

    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
        saveSelection();
        break;
    case Qt::Key_V:
        setTool(Tool::Move);
        break;
    case Qt::Key_P:
        setTool(Tool::Pen);
        break;
    case Qt::Key_L:
        setTool(Tool::Line);
        break;
    case Qt::Key_H:
        setTool(Tool::Highlighter);
        break;
    case Qt::Key_R:
        setTool(Tool::Rectangle);
        break;
    case Qt::Key_E:
        setTool(Tool::Ellipse);
        break;
    case Qt::Key_A:
        setTool(Tool::Arrow);
        break;
    case Qt::Key_T:
        setTool(Tool::Text);
        break;
    case Qt::Key_N:
        setTool(Tool::Number);
        break;
    case Qt::Key_M:
        setTool(Tool::Mosaic);
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void ShotWindow::beginSelection(QPointF imagePoint)
{
    m_dragging = true;
    m_fullscreenAnnotation = false;
    m_toolbarDragging = false;
    m_toolbarUserPlaced = false;
    m_selectionDrag = SelectionDrag::None;
    m_selectionStart = imagePoint;
    m_selection = QRectF(imagePoint, imagePoint);
    if (m_textEditor) {
        m_textEditor->hide();
        m_textEditor->clear();
    }
    if (m_openWithPanel) {
        m_openWithPanel->hide();
    }
    setFullscreenActionButtonsVisible(false);
    m_annotations.clear();
    m_redoStack.clear();
    m_draft.reset();
    m_nextNumber = 1;
    revealSelectionInfo();
    update();
}

void ShotWindow::commitDraft()
{
    if (!m_draft.has_value()) {
        return;
    }

    if ((m_draft->tool == Tool::Pen || m_draft->tool == Tool::Highlighter) && m_draft->points.size() < 2) {
        m_draft.reset();
        update();
        return;
    }

    if ((m_draft->tool == Tool::Line || m_draft->tool == Tool::Arrow) && m_draft->points.size() >= 2
        && QLineF(m_draft->points.first(), m_draft->points.last()).length() < 2.0) {
        m_draft.reset();
        update();
        return;
    }

    if (m_draft->tool != Tool::Pen && m_draft->tool != Tool::Highlighter && m_draft->tool != Tool::Line
        && m_draft->tool != Tool::Arrow && m_draft->tool != Tool::Text
        && (m_draft->rect.width() < 2.0 || m_draft->rect.height() < 2.0)) {
        m_draft.reset();
        update();
        return;
    }

    m_annotations.append(*m_draft);
    m_redoStack.clear();
    m_draft.reset();
    update();
}

void ShotWindow::setTool(Tool tool)
{
    commitTextEditor();
    m_selectionDrag = SelectionDrag::None;
    m_tool = tool;
    updateCursor();
    updateToolbarState();
    update();
}

void ShotWindow::updateCursor()
{
    if (m_showWheelPreview && m_wheelPreviewTimer.isValid() && m_wheelPreviewTimer.elapsed() <= 900) {
        setCursor(Qt::BlankCursor);
        return;
    }

    if (m_tool == Tool::Move && !m_fullscreenAnnotation) {
        switch (m_selectionDrag) {
        case SelectionDrag::Left:
        case SelectionDrag::Right:
            setCursor(Qt::SizeHorCursor);
            return;
        case SelectionDrag::Top:
        case SelectionDrag::Bottom:
            setCursor(Qt::SizeVerCursor);
            return;
        case SelectionDrag::TopLeft:
        case SelectionDrag::BottomRight:
            setCursor(Qt::SizeFDiagCursor);
            return;
        case SelectionDrag::TopRight:
        case SelectionDrag::BottomLeft:
            setCursor(Qt::SizeBDiagCursor);
            return;
        case SelectionDrag::Move:
            setCursor(Qt::SizeAllCursor);
            return;
        case SelectionDrag::None:
            setCursor(Qt::CrossCursor);
            return;
        }
    }

    setCursor(m_tool == Tool::Text ? Qt::IBeamCursor : Qt::CrossCursor);
}

bool ShotWindow::hasUsableSelection() const
{
    const QRectF selection = normalizedSelection();
    return selection.width() >= kMinSelectionSize && selection.height() >= kMinSelectionSize;
}

ShotWindow::SelectionDrag ShotWindow::selectionDragAt(QPointF imagePoint) const
{
    const QRectF selection = normalizedSelection();
    if (selection.isEmpty() || m_frozenImageRect.isEmpty()) {
        return SelectionDrag::None;
    }

    const qreal imageTolerance = 10.0 * m_frozenFrame.width() / std::max<qreal>(1.0, m_frozenImageRect.width());
    if (!selection.adjusted(-imageTolerance, -imageTolerance, imageTolerance, imageTolerance).contains(imagePoint)) {
        return SelectionDrag::None;
    }

    const bool nearLeft = std::abs(imagePoint.x() - selection.left()) <= imageTolerance;
    const bool nearRight = std::abs(imagePoint.x() - selection.right()) <= imageTolerance;
    const bool nearTop = std::abs(imagePoint.y() - selection.top()) <= imageTolerance;
    const bool nearBottom = std::abs(imagePoint.y() - selection.bottom()) <= imageTolerance;

    if (nearLeft && nearTop) {
        return SelectionDrag::TopLeft;
    }
    if (nearRight && nearTop) {
        return SelectionDrag::TopRight;
    }
    if (nearLeft && nearBottom) {
        return SelectionDrag::BottomLeft;
    }
    if (nearRight && nearBottom) {
        return SelectionDrag::BottomRight;
    }
    if (nearLeft) {
        return SelectionDrag::Left;
    }
    if (nearRight) {
        return SelectionDrag::Right;
    }
    if (nearTop) {
        return SelectionDrag::Top;
    }
    if (nearBottom) {
        return SelectionDrag::Bottom;
    }
    return selection.contains(imagePoint) ? SelectionDrag::Move : SelectionDrag::None;
}

qreal ShotWindow::currentToolWidth() const
{
    switch (m_tool) {
    case Tool::Move:
        return m_shapeWidth;
    case Tool::Pen:
        return m_penWidth;
    case Tool::Highlighter:
        return m_penWidth * 2.0;
    case Tool::Line:
    case Tool::Arrow:
    case Tool::Rectangle:
    case Tool::Ellipse:
    case Tool::Text:
    case Tool::Number:
        return m_shapeWidth;
    case Tool::Mosaic:
        return m_mosaicBlockSize;
    }

    return m_shapeWidth;
}

qreal ShotWindow::currentToolPreviewSize() const
{
    const qreal scale = !m_frozenImageRect.isEmpty()
        ? m_frozenImageRect.width() / std::max(1, m_frozenFrame.width())
        : 1.0;

    switch (m_tool) {
    case Tool::Move:
        return 8.0;
    case Tool::Pen:
    case Tool::Line:
    case Tool::Arrow:
    case Tool::Rectangle:
    case Tool::Ellipse:
        return std::max<qreal>(1.5, currentToolWidth() * scale);
    case Tool::Highlighter:
        return std::max<qreal>(6.0, currentToolWidth() * scale);
    case Tool::Text:
        return std::max<qreal>(10.0, (19.0 + currentToolWidth()) * scale);
    case Tool::Number:
        return std::max<qreal>(26.0, (15.0 + currentToolWidth()) * scale * 2.0);
    case Tool::Mosaic:
        return std::max<qreal>(2.0, currentToolWidth() * scale);
    }

    return std::max<qreal>(1.5, currentToolWidth() * scale);
}

void ShotWindow::setCurrentColor(QColor color)
{
    if (!color.isValid()) {
        return;
    }

    m_currentColor = color;
    if (m_draft.has_value()) {
        m_draft->color = color;
    }
    if (m_colorPalette) {
        m_colorPalette->hide();
    }
    updateColorPalettePreview();
    update();
}

void ShotWindow::revealSelectionInfo()
{
    m_showSelectionInfo = true;
    m_selectionInfoTimer.restart();
    QTimer::singleShot(1000, this, [this] {
        if (m_selectionDrag == SelectionDrag::None
            && m_selectionInfoTimer.isValid()
            && m_selectionInfoTimer.elapsed() >= 1000) {
            m_showSelectionInfo = false;
            update();
        }
    });
}

QRectF ShotWindow::normalizedSelection() const
{
    return m_selection.normalized().intersected(QRectF(QPointF(0, 0), QSizeF(m_frozenFrame.size())));
}

QPointF ShotWindow::widgetToImage(QPointF point) const
{
    if (m_frozenImageRect.isEmpty() || m_frozenFrame.isNull()) {
        return {};
    }

    const qreal x = (point.x() - m_frozenImageRect.left()) * m_frozenFrame.width() / m_frozenImageRect.width();
    const qreal y = (point.y() - m_frozenImageRect.top()) * m_frozenFrame.height() / m_frozenImageRect.height();
    return clampImagePoint({x, y});
}

QPointF ShotWindow::imageToWidget(QPointF point) const
{
    if (m_frozenImageRect.isEmpty() || m_frozenFrame.isNull()) {
        return {};
    }

    const qreal x = m_frozenImageRect.left() + point.x() * m_frozenImageRect.width() / m_frozenFrame.width();
    const qreal y = m_frozenImageRect.top() + point.y() * m_frozenImageRect.height() / m_frozenFrame.height();
    return {x, y};
}

QPointF ShotWindow::clampImagePoint(QPointF point) const
{
    return {
        std::clamp(point.x(), 0.0, static_cast<qreal>(std::max(0, m_frozenFrame.width() - 1))),
        std::clamp(point.y(), 0.0, static_cast<qreal>(std::max(0, m_frozenFrame.height() - 1))),
    };
}

QString ShotWindow::currentToolName() const
{
    switch (m_tool) {
    case Tool::Move:
        return QStringLiteral("Move");
    case Tool::Pen:
        return QStringLiteral("Pen");
    case Tool::Line:
        return QStringLiteral("Line");
    case Tool::Highlighter:
        return QStringLiteral("Highlighter");
    case Tool::Rectangle:
        return QStringLiteral("Rect");
    case Tool::Ellipse:
        return QStringLiteral("Ellipse");
    case Tool::Arrow:
        return QStringLiteral("Arrow");
    case Tool::Text:
        return QStringLiteral("Text");
    case Tool::Number:
        return QStringLiteral("Number");
    case Tool::Mosaic:
        return QStringLiteral("Mosaic");
    }

    return QStringLiteral("Tool");
}

QImage ShotWindow::mosaicImage(QRect sourceRect, int blockSize) const
{
    sourceRect = sourceRect.normalized().intersected(QRect(QPoint(0, 0), m_frozenFrame.size()));
    if (sourceRect.isEmpty()) {
        return {};
    }

    blockSize = std::clamp(blockSize, 2, 96);
    const QImage source = m_frozenFrame.copy(sourceRect).convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QImage output(source.size(), QImage::Format_ARGB32_Premultiplied);
    output.fill(Qt::transparent);

    QPainter blockPainter(&output);
    blockPainter.setPen(Qt::NoPen);
    blockPainter.setRenderHint(QPainter::Antialiasing, false);

    for (int y = 0; y < source.height(); y += blockSize) {
        const int blockHeight = std::min(blockSize, source.height() - y);
        for (int x = 0; x < source.width(); x += blockSize) {
            const int blockWidth = std::min(blockSize, source.width() - x);
            quint64 red = 0;
            quint64 green = 0;
            quint64 blue = 0;
            quint64 alpha = 0;
            for (int py = y; py < y + blockHeight; ++py) {
                const QRgb *line = reinterpret_cast<const QRgb *>(source.constScanLine(py));
                for (int px = x; px < x + blockWidth; ++px) {
                    const QRgb pixel = line[px];
                    red += qRed(pixel);
                    green += qGreen(pixel);
                    blue += qBlue(pixel);
                    alpha += qAlpha(pixel);
                }
            }

            const int count = blockWidth * blockHeight;
            QColor average(qRound(static_cast<double>(red) / count),
                           qRound(static_cast<double>(green) / count),
                           qRound(static_cast<double>(blue) / count),
                           qRound(static_cast<double>(alpha) / count));
            blockPainter.setBrush(average);
            blockPainter.drawRect(QRect(x, y, blockWidth, blockHeight));
        }
    }

    blockPainter.end();
    return output;
}

QRectF ShotWindow::imageRectToWidget(QRectF rect) const
{
    const QPointF topLeft = imageToWidget(rect.topLeft());
    const QPointF bottomRight = imageToWidget(rect.bottomRight());
    return QRectF(topLeft, bottomRight).normalized();
}

QRectF ShotWindow::constrainedRect(QPointF start, QPointF end) const
{
    const qreal dx = end.x() - start.x();
    const qreal dy = end.y() - start.y();
    const qreal side = std::max(std::abs(dx), std::abs(dy));
    const QPointF constrainedEnd(start.x() + std::copysign(side, dx == 0.0 ? 1.0 : dx),
                                 start.y() + std::copysign(side, dy == 0.0 ? 1.0 : dy));
    return normalizedRect(start, clampImagePoint(constrainedEnd));
}

void ShotWindow::updateFrozenImageRect()
{
    if (m_frozenFrame.isNull()) {
        m_frozenImageRect = {};
        return;
    }

    QSizeF frameSize = m_frozenFrame.size();
    frameSize.scale(size(), Qt::KeepAspectRatio);
    const QPointF topLeft((width() - frameSize.width()) / 2.0, (height() - frameSize.height()) / 2.0);
    m_frozenImageRect = QRectF(topLeft, frameSize);
}

QRect ShotWindow::clampedToolbarGeometry(QRect toolbarGeometry) const
{
    toolbarGeometry.moveLeft(std::clamp(toolbarGeometry.left(), 8, std::max(8, width() - toolbarGeometry.width() - 8)));
    toolbarGeometry.moveTop(std::clamp(toolbarGeometry.top(), 8, std::max(8, height() - toolbarGeometry.height() - 8)));
    return toolbarGeometry;
}

void ShotWindow::updateToolbarGeometry()
{
    if (!m_toolbar || !hasUsableSelection()) {
        return;
    }

    m_toolbar->adjustSize();
    if (m_fullscreenAnnotation && m_toolbarUserPlaced) {
        const QSize toolbarSize = m_toolbar->sizeHint();
        QRect toolbarGeometry = m_toolbar->geometry();
        toolbarGeometry.setSize(toolbarSize);
        m_toolbar->setGeometry(clampedToolbarGeometry(toolbarGeometry));
        return;
    }

    const QRectF selection = imageRectToWidget(normalizedSelection());
    const QSize toolbarSize = m_toolbar->sizeHint();
    int x = qRound(selection.center().x() - toolbarSize.width() / 2.0);
    int y = qRound(selection.bottom() + kToolbarMargin);

    x = std::clamp(x, 8, std::max(8, width() - toolbarSize.width() - 8));
    if (y + toolbarSize.height() > height() - 8) {
        y = qRound(selection.top() - toolbarSize.height() - kToolbarMargin);
    }
    y = std::clamp(y, 8, std::max(8, height() - toolbarSize.height() - 8));
    m_toolbar->setGeometry(x, y, toolbarSize.width(), toolbarSize.height());
}

void ShotWindow::updateActionToolbarGeometry()
{
    if (!m_actionToolbar || !hasUsableSelection() || m_fullscreenAnnotation) {
        return;
    }

    m_actionToolbar->adjustSize();
    const QRectF selection = imageRectToWidget(normalizedSelection());
    const QSize toolbarSize = m_actionToolbar->sizeHint();
    int x = qRound(selection.right() + kToolbarMargin);
    int y = qRound(selection.center().y() - toolbarSize.height() / 2.0);

    if (x + toolbarSize.width() > width() - 8) {
        x = qRound(selection.left() - toolbarSize.width() - kToolbarMargin);
    }
    x = std::clamp(x, 8, std::max(8, width() - toolbarSize.width() - 8));
    y = std::clamp(y, 8, std::max(8, height() - toolbarSize.height() - 8));
    m_actionToolbar->setGeometry(x, y, toolbarSize.width(), toolbarSize.height());
}

void ShotWindow::toggleOpenWithPanel()
{
    commitTextEditor();
    if (!m_openWithPanel || !hasUsableSelection()) {
        return;
    }
    if (m_colorPalette) {
        m_colorPalette->hide();
    }

    if (m_openWithPanel->isVisible()) {
        m_openWithPanel->hide();
        return;
    }

    updateOpenWithPanel();
    updateOpenWithPanelGeometry();
    m_openWithPanel->show();
    m_openWithPanel->raise();
}

void ShotWindow::updateOpenWithPanel()
{
    if (!m_openWithPanel) {
        return;
    }

    QLayout *layout = m_openWithPanel->layout();
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            delete widget;
        }
        delete item;
    }

    auto *title = new QLabel(QStringLiteral("Open with"), m_openWithPanel);
    layout->addWidget(title);

    const QVector<DesktopApp> apps = imageDesktopApps();
    if (apps.isEmpty()) {
        auto *empty = new QLabel(QStringLiteral("No image desktop entries found"), m_openWithPanel);
        empty->setWordWrap(true);
        layout->addWidget(empty);
        m_openWithPanel->adjustSize();
        return;
    }

    auto *list = new QListWidget(m_openWithPanel);
    list->setFocusPolicy(Qt::NoFocus);
    list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    for (const DesktopApp &app : apps) {
        auto *item = new QListWidgetItem(app.name, list);
        item->setToolTip(app.desktopPath);
        item->setData(Qt::UserRole, app.desktopPath);
        item->setData(Qt::UserRole + 1, app.exec);
    }
    list->setFixedHeight(std::min(420, std::max(58, static_cast<int>(apps.size()) * 42)));
    connect(list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }
        DesktopApp app;
        app.name = item->text();
        app.desktopPath = item->data(Qt::UserRole).toString();
        app.exec = item->data(Qt::UserRole + 1).toString();
        openSelectionWithDesktop(app);
    });
    layout->addWidget(list);

    m_openWithPanel->adjustSize();
}

void ShotWindow::updateOpenWithPanelGeometry()
{
    if (!m_openWithPanel) {
        return;
    }

    m_openWithPanel->adjustSize();
    const QSize panelSize(std::min(340, std::max(280, m_openWithPanel->sizeHint().width())),
                          std::min(540, std::max(80, m_openWithPanel->sizeHint().height())));
    const QRect toolbarRect = m_fullscreenAnnotation && m_toolbar
        ? m_toolbar->geometry()
        : (m_actionToolbar ? m_actionToolbar->geometry() : QRect(width() - 64, height() / 2 - 80, 56, 160));
    int x = toolbarRect.left() - panelSize.width() - kToolbarMargin;
    int y = toolbarRect.top();
    if (x < 8) {
        x = toolbarRect.right() + kToolbarMargin;
    }
    x = std::clamp(x, 8, std::max(8, width() - panelSize.width() - 8));
    y = std::clamp(y, 8, std::max(8, height() - panelSize.height() - 8));
    m_openWithPanel->setGeometry(x, y, panelSize.width(), panelSize.height());
}

void ShotWindow::toggleColorPalette(QPoint position)
{
    commitTextEditor();
    if (m_openWithPanel) {
        m_openWithPanel->hide();
    }
    if (!m_colorPalette) {
        return;
    }

    m_colorPaletteAnchor = position;
    if (m_colorPalette->isVisible()) {
        m_colorPalette->hide();
    } else {
        updateColorPaletteGeometry(position);
        m_colorPalette->show();
        m_colorPalette->raise();
    }
    update();
}

void ShotWindow::updateColorPaletteGeometry(QPoint anchor)
{
    if (!m_colorPalette) {
        return;
    }

    const QSize paletteSize(178, 178);
    int x = anchor.x() - paletteSize.width() / 2;
    int y = anchor.y() - paletteSize.height() / 2;
    x = std::clamp(x, 8, std::max(8, width() - paletteSize.width() - 8));
    y = std::clamp(y, 8, std::max(8, height() - paletteSize.height() - 8));
    m_colorPalette->setGeometry(x, y, paletteSize.width(), paletteSize.height());

    const QPoint center(paletteSize.width() / 2, paletteSize.height() / 2);
    const qreal radius = 68.0;
    const auto buttons = m_colorPalette->findChildren<QPushButton *>(QString(), Qt::FindDirectChildrenOnly);
    for (int i = 0; i < buttons.size(); ++i) {
        const qreal angle = -M_PI / 2.0 + (2.0 * M_PI * i / std::max<qsizetype>(1, buttons.size()));
        const QPoint pos(qRound(center.x() + std::cos(angle) * radius - 15.0),
                         qRound(center.y() + std::sin(angle) * radius - 15.0));
        buttons.at(i)->setGeometry(QRect(pos, QSize(30, 30)));
    }
    updateColorPalettePreview();
}

void ShotWindow::updateColorPalettePreview()
{
    if (!m_colorPalettePreview) {
        return;
    }

    const int size = std::clamp(qRound(currentToolPreviewSize()), 8, 34);
    const QPoint center(89, 89);
    m_colorPalettePreview->setGeometry(center.x() - size / 2, center.y() - size / 2, size, size);
    m_colorPalettePreview->setStyleSheet(QStringLiteral(
        "QWidget#colorPalettePreview {"
        " background: %1;"
        " border: 0;"
        " border-radius: 3px;"
        "}").arg(m_currentColor.name()));
}

void ShotWindow::updateTextEditorGeometry()
{
    if (!m_textEditor || !m_textEditor->isVisible()) {
        return;
    }

    const QPointF topLeft = imageToWidget(m_textEditorImagePoint);
    const QRectF selection = imageRectToWidget(normalizedSelection());
    const int availableRight = std::max(150, qRound(selection.right() - topLeft.x() - 12));
    const int availableBottom = std::max(46, qRound(selection.bottom() - topLeft.y() - 12));
    const int editorWidth = std::min(360, std::max(130, availableRight));
    const int editorHeight = std::min(160, std::max(46, availableBottom));
    QRect editorRect(qRound(topLeft.x()), qRound(topLeft.y()), editorWidth, editorHeight);
    editorRect.moveLeft(std::clamp(editorRect.left(), 8, std::max(8, width() - editorRect.width() - 8)));
    editorRect.moveTop(std::clamp(editorRect.top(), 8, std::max(8, height() - editorRect.height() - 8)));
    m_textEditor->setGeometry(editorRect);
}

void ShotWindow::redoAnnotation()
{
    if (m_redoStack.isEmpty()) {
        return;
    }

    Annotation annotation = m_redoStack.takeLast();
    if (annotation.tool == Tool::Number) {
        m_nextNumber = std::max(m_nextNumber, annotation.number + 1);
    }
    m_annotations.append(annotation);
    update();
}

void ShotWindow::updateToolbarState()
{
    if (!m_toolbar) {
        return;
    }

    const QString active = currentToolName();
    const auto buttons = m_toolbar->findChildren<QPushButton *>();
    for (QPushButton *button : buttons) {
        const bool isActiveTool = button->property("action").toString() == active;
        button->setProperty("active", isActiveTool);
        button->style()->unpolish(button);
        button->style()->polish(button);
        button->update();
    }
}

void ShotWindow::drawAnnotation(QPainter &painter, const Annotation &annotation, bool widgetCoordinates) const
{
    auto mapPoint = [this, widgetCoordinates](QPointF point) {
        return widgetCoordinates ? imageToWidget(point) : point;
    };

    auto mapRect = [this, widgetCoordinates](QRectF rect) {
        return widgetCoordinates ? imageRectToWidget(rect) : rect;
    };

    const qreal scale = widgetCoordinates && !m_frozenImageRect.isEmpty()
        ? m_frozenImageRect.width() / std::max(1, m_frozenFrame.width())
        : 1.0;
    const qreal penWidth = std::max<qreal>(1.5, annotation.width * scale);

    QPen pen(annotation.color, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    switch (annotation.tool) {
    case Tool::Move:
        return;
    case Tool::Pen: {
        if (annotation.points.size() < 2) {
            return;
        }
        QPainterPath path(mapPoint(annotation.points.first()));
        for (int i = 1; i < annotation.points.size(); ++i) {
            path.lineTo(mapPoint(annotation.points.at(i)));
        }
        painter.drawPath(path);
        break;
    }
    case Tool::Highlighter: {
        if (annotation.points.size() < 2) {
            return;
        }
        QColor color = annotation.color;
        color.setAlpha(120);
        painter.save();
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setPen(QPen(color, std::max<qreal>(6.0, penWidth), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        QPainterPath path(mapPoint(annotation.points.first()));
        for (int i = 1; i < annotation.points.size(); ++i) {
            path.lineTo(mapPoint(annotation.points.at(i)));
        }
        painter.drawPath(path);
        painter.restore();
        break;
    }
    case Tool::Line:
        if (annotation.points.size() >= 2) {
            painter.drawLine(mapPoint(annotation.points.first()), mapPoint(annotation.points.last()));
        }
        break;
    case Tool::Rectangle:
        painter.drawRect(mapRect(annotation.rect));
        break;
    case Tool::Ellipse:
        painter.drawEllipse(mapRect(annotation.rect));
        break;
    case Tool::Arrow:
        if (annotation.points.size() >= 2) {
            drawArrow(painter, mapPoint(annotation.points.first()), mapPoint(annotation.points.last()), penWidth);
        }
        break;
    case Tool::Text: {
        QFont font(QStringLiteral("Sans Serif"), qRound((19.0 + annotation.width) * scale), QFont::DemiBold);
        QRectF textRect = annotation.rect.isEmpty()
            ? QRectF(mapPoint(annotation.points.value(0)), QSizeF(360.0 * scale, 140.0 * scale))
            : mapRect(annotation.rect);
        QTextOption option;
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        option.setAlignment(Qt::AlignLeft | Qt::AlignTop);
        painter.save();
        painter.setFont(font);
        painter.setPen(annotation.color);
        painter.drawText(textRect, annotation.text, option);
        painter.restore();
        break;
    }
    case Tool::Number:
        if (!annotation.points.isEmpty()) {
            drawNumber(painter, annotation.points.first(), annotation.number, annotation.color, annotation.width, widgetCoordinates);
        }
        break;
    case Tool::Mosaic:
        drawMosaic(painter, annotation.rect, annotation.width, widgetCoordinates);
        break;
    }
}

void ShotWindow::drawArrow(QPainter &painter, QPointF start, QPointF end, qreal width) const
{
    painter.drawLine(start, end);

    const QLineF line(start, end);
    if (line.length() < 1.0) {
        return;
    }

    const qreal angle = std::atan2(end.y() - start.y(), end.x() - start.x());
    const qreal arrowSize = std::max<qreal>(12.0, width * 5.0);
    const QPointF p1 = end - QPointF(std::cos(angle - M_PI / 6.0) * arrowSize,
                                     std::sin(angle - M_PI / 6.0) * arrowSize);
    const QPointF p2 = end - QPointF(std::cos(angle + M_PI / 6.0) * arrowSize,
                                     std::sin(angle + M_PI / 6.0) * arrowSize);

    QPolygonF head;
    head << end << p1 << p2;
    painter.setBrush(painter.pen().color());
    painter.drawPolygon(head);
}

void ShotWindow::drawWheelPreview(QPainter &painter)
{
    if (!m_showWheelPreview || !m_wheelPreviewTimer.isValid() || m_wheelPreviewTimer.elapsed() > 900) {
        m_showWheelPreview = false;
        updateCursor();
        return;
    }

    const qreal size = std::clamp(currentToolPreviewSize(), 2.0, 96.0);
    QRectF preview(m_wheelPreviewPosition.x() - size / 2.0,
                   m_wheelPreviewPosition.y() - size / 2.0,
                   size,
                   size);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_currentColor);
    painter.drawRect(preview);
    painter.restore();
}

void ShotWindow::drawNumber(QPainter &painter, QPointF imagePoint, int number, QColor color, qreal width, bool widgetCoordinates) const
{
    const QPointF center = widgetCoordinates ? imageToWidget(imagePoint) : imagePoint;
    const qreal scale = widgetCoordinates && !m_frozenImageRect.isEmpty()
        ? m_frozenImageRect.width() / std::max(1, m_frozenFrame.width())
        : 1.0;
    const qreal radius = std::max<qreal>(13.0, (15.0 + width) * scale);
    const QRectF bubble(center.x() - radius, center.y() - radius, radius * 2.0, radius * 2.0);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(255, 255, 255), std::max<qreal>(2.0, 2.0 * scale)));
    painter.setBrush(color);
    painter.drawEllipse(bubble);

    QFont font(QStringLiteral("Sans Serif"), qRound(16 * scale), QFont::Black);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(bubble, Qt::AlignCenter, QString::number(number));
    painter.restore();
}

void ShotWindow::drawMosaic(QPainter &painter, QRectF imageRect, qreal blockSize, bool widgetCoordinates) const
{
    QRect sourceRect = imageRect.normalized().toAlignedRect().intersected(QRect(QPoint(0, 0), m_frozenFrame.size()));
    if (sourceRect.isEmpty()) {
        return;
    }

    const QImage mosaic = mosaicImage(sourceRect, qRound(blockSize));
    if (mosaic.isNull()) {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawImage(widgetCoordinates ? imageRectToWidget(sourceRect) : QRectF(sourceRect), mosaic);
    painter.restore();
}

void ShotWindow::beginTextAnnotation(QPointF imagePoint)
{
    m_textEditorImagePoint = imagePoint;
    m_draft.reset();
    m_textEditor->clear();
    m_textEditor->setStyleSheet(textEditorStyleSheet(m_currentColor, qRound(20.0 + m_shapeWidth)));
    m_textEditor->show();
    m_textEditor->raise();
    updateTextEditorGeometry();
    m_textEditor->setFocus(Qt::MouseFocusReason);
    update();
}

void ShotWindow::commitTextEditor()
{
    if (m_committingText || !m_textEditor || !m_textEditor->isVisible()) {
        return;
    }

    m_committingText = true;
    const QString text = m_textEditor->toPlainText().trimmed();
    const QRect editorGeometry = m_textEditor->geometry();
    m_textEditor->hide();
    m_textEditor->clear();
    setFocus(Qt::OtherFocusReason);

    if (!text.isEmpty()) {
        Annotation annotation;
        annotation.tool = Tool::Text;
        annotation.points.append(m_textEditorImagePoint);
        annotation.rect = QRectF(widgetToImage(editorGeometry.topLeft()),
                                 widgetToImage(editorGeometry.bottomRight())).normalized();
        annotation.text = text;
        annotation.color = m_currentColor;
        annotation.width = m_shapeWidth;
        m_annotations.append(annotation);
        m_redoStack.clear();
    }

    m_committingText = false;
    update();
}

QString ShotWindow::saveSelectionToTempFile() const
{
    if (!hasUsableSelection()) {
        return {};
    }

    const QImage output = renderedSelection();
    if (output.isNull()) {
        return {};
    }

    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation).isEmpty()
        ? QDir::tempPath()
        : QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString filename = QStringLiteral("mark-shot-open-%1.png")
                                 .arg(QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMdd-hhmmss-zzz")));
    const QString path = QDir(tempDir).filePath(filename);
    return output.save(path, "PNG") ? path : QString();
}

void ShotWindow::openSelectionWithDesktop(const DesktopApp &app)
{
    commitTextEditor();
    if (m_openWithPanel) {
        m_openWithPanel->hide();
    }

    const QString imagePath = saveSelectionToTempFile();
    if (imagePath.isEmpty()) {
        return;
    }

    QStringList command = expandDesktopExec(app, imagePath);
    if (command.isEmpty()) {
        return;
    }

    const QString program = command.takeFirst();
    if (QProcess::startDetached(program, command)) {
        close();
    }
}

QImage ShotWindow::renderedSelection() const
{
    const QRect sourceBounds(QPoint(0, 0), m_frozenFrame.size());
    const QRect selectionRect = normalizedSelection().toAlignedRect().intersected(sourceBounds);
    if (selectionRect.isEmpty()) {
        return {};
    }

    QImage output = m_frozenFrame.copy(selectionRect).convertToFormat(QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&output);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.translate(-selectionRect.topLeft());
    for (const Annotation &annotation : m_annotations) {
        drawAnnotation(painter, annotation, false);
    }
    painter.end();
    return output;
}

QString ShotWindow::defaultSavePath() const
{
    QString pictures = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (pictures.isEmpty()) {
        pictures = QDir::homePath();
    }

    const QString filename = QStringLiteral("mark-shot-%1.png").arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-hhmmss")));
    return QDir(pictures).filePath(filename);
}

void ShotWindow::saveSelection()
{
    commitTextEditor();

    if (!hasUsableSelection()) {
        return;
    }

    const QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Save Screenshot"), defaultSavePath(), QStringLiteral("PNG Images (*.png)"));
    if (path.isEmpty()) {
        return;
    }

    QImage output = renderedSelection();
    if (!output.isNull() && output.save(path, "PNG")) {
        close();
    }
}

void ShotWindow::copySelection()
{
    commitTextEditor();

    if (!hasUsableSelection()) {
        return;
    }

    QImage output = renderedSelection();
    if (output.isNull()) {
        return;
    }

    QApplication::clipboard()->setImage(output);

    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    output.save(&buffer, "PNG");

    QProcess wlCopy;
    wlCopy.setProgram(QStringLiteral("wl-copy"));
    wlCopy.setArguments({QStringLiteral("--type"), QStringLiteral("image/png")});
    wlCopy.start(QIODevice::WriteOnly);
    if (wlCopy.waitForStarted(1000)) {
        wlCopy.write(png);
        wlCopy.closeWriteChannel();
        wlCopy.waitForFinished(2500);
    }

    close();
}
