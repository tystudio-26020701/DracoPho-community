#include "shot_window.h"

#include "ui/color_picker.h"
#include "ui/icons.h"
#include "ui/theme.h"

#include <LayerShellQt/Window>

#include <QAbstractItemView>
#include <QApplication>
#include <QBuffer>
#include <QBrush>
#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
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
#include <QSignalBlocker>
#include <QSlider>
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

QRectF normalizedRect(QPointF a, QPointF b)
{
    return QRectF(a, b).normalized();
}

// Stylesheets, palette presets, action names, and toolbar icons now live in
// src/ui/theme.{h,cpp} and src/ui/icons.{h,cpp}.

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
    m_toolbar->setStyleSheet(markshot::theme::panelStyleSheet());
    m_toolbar->installEventFilter(this);

    auto *layout = new QHBoxLayout(m_toolbar);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(7);

    layout->addWidget(addToolbarButton(Action::ToolMove, QStringLiteral("V")));
    layout->addWidget(addToolbarButton(Action::ToolSelect, QStringLiteral("S")));
    layout->addWidget(addToolbarButton(Action::ToolPen, QStringLiteral("P")));
    layout->addWidget(addToolbarButton(Action::ToolLine, QStringLiteral("L")));
    layout->addWidget(addToolbarButton(Action::ToolHighlighter, QStringLiteral("H")));
    layout->addWidget(addToolbarButton(Action::ToolRectangle, QStringLiteral("R")));
    layout->addWidget(addToolbarButton(Action::ToolEllipse, QStringLiteral("E")));
    layout->addWidget(addToolbarButton(Action::ToolArrow, QStringLiteral("A")));
    layout->addWidget(addToolbarButton(Action::ToolText, QStringLiteral("T")));
    layout->addWidget(addToolbarButton(Action::ToolNumber, QStringLiteral("N")));
    layout->addWidget(addToolbarButton(Action::ToolMosaic, QStringLiteral("M")));
    layout->addWidget(addToolbarButton(Action::Clear, QStringLiteral("Clear")));
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

    m_annotationPropertyPanel = new QWidget(this);
    m_annotationPropertyPanel->setObjectName(QStringLiteral("annotationPropertyPanel"));
    m_annotationPropertyPanel->setStyleSheet(m_toolbar->styleSheet());
    auto *propertyLayout = new QHBoxLayout(m_annotationPropertyPanel);
    propertyLayout->setContentsMargins(10, 10, 10, 10);
    propertyLayout->setSpacing(7);
    m_annotationPropertyTitle = new QLabel(QStringLiteral("Object"), m_annotationPropertyPanel);
    propertyLayout->addWidget(m_annotationPropertyTitle);
    m_propertyWidthLabel = new QLabel(QStringLiteral("Width 2"), m_annotationPropertyPanel);
    propertyLayout->addWidget(m_propertyWidthLabel);
    m_propertyWidthSlider = new QSlider(Qt::Horizontal, m_annotationPropertyPanel);
    m_propertyWidthSlider->setFocusPolicy(Qt::NoFocus);
    m_propertyWidthSlider->setFixedWidth(120);
    m_propertyWidthSlider->setToolTip(QStringLiteral("Selected object width or size"));
    connect(m_propertyWidthSlider, &QSlider::valueChanged, this, [this](int value) { setSelectedAnnotationWidth(value); });
    propertyLayout->addWidget(m_propertyWidthSlider);
    m_propertyColorButton = new QPushButton(QStringLiteral("Color"), m_annotationPropertyPanel);
    m_propertyColorButton->setFocusPolicy(Qt::NoFocus);
    m_propertyColorButton->setToolTip(QStringLiteral("Change selected object color"));
    connect(m_propertyColorButton, &QPushButton::clicked, this, [this] { openSelectedAnnotationColorPalette(); });
    propertyLayout->addWidget(m_propertyColorButton);
    m_propertyFillButton = new QPushButton(QStringLiteral("Fill"), m_annotationPropertyPanel);
    m_propertyFillButton->setCheckable(true);
    m_propertyFillButton->setFocusPolicy(Qt::NoFocus);
    m_propertyFillButton->setToolTip(QStringLiteral("Toggle shape fill"));
    connect(m_propertyFillButton, &QPushButton::toggled, this, [this](bool checked) { setSelectedAnnotationFilled(checked); });
    propertyLayout->addWidget(m_propertyFillButton);
    m_propertyRadiusLabel = new QLabel(QStringLiteral("Radius 0"), m_annotationPropertyPanel);
    propertyLayout->addWidget(m_propertyRadiusLabel);
    m_propertyRadiusSlider = new QSlider(Qt::Horizontal, m_annotationPropertyPanel);
    m_propertyRadiusSlider->setFocusPolicy(Qt::NoFocus);
    m_propertyRadiusSlider->setRange(0, 48);
    m_propertyRadiusSlider->setFixedWidth(100);
    m_propertyRadiusSlider->setToolTip(QStringLiteral("Rectangle corner radius"));
    connect(m_propertyRadiusSlider, &QSlider::valueChanged, this, [this](int value) { setSelectedAnnotationCornerRadius(value); });
    propertyLayout->addWidget(m_propertyRadiusSlider);
    m_propertyFontButton = new QPushButton(QStringLiteral("Font"), m_annotationPropertyPanel);
    m_propertyFontButton->setFocusPolicy(Qt::NoFocus);
    m_propertyFontButton->setFixedWidth(160);
    m_propertyFontButton->setToolTip(QStringLiteral("Text font"));
    connect(m_propertyFontButton, &QPushButton::clicked, this, [this] { toggleSelectedTextFontPanel(); });
    propertyLayout->addWidget(m_propertyFontButton);
    m_propertyEditTextButton = new QPushButton(QStringLiteral("Edit"), m_annotationPropertyPanel);
    m_propertyEditTextButton->setFocusPolicy(Qt::NoFocus);
    m_propertyEditTextButton->setToolTip(QStringLiteral("Edit selected text"));
    connect(m_propertyEditTextButton, &QPushButton::clicked, this, [this] { beginEditingSelectedTextAnnotation(); });
    propertyLayout->addWidget(m_propertyEditTextButton);
    m_annotationPropertyPanel->hide();

    m_propertyColorDialogPanel = new QWidget(this);
    m_propertyColorDialogPanel->setObjectName(QStringLiteral("propertyColorDialogPanel"));
    m_propertyColorDialogPanel->setStyleSheet(markshot::theme::propertyColorDialogPanelStyleSheet());
    auto *propertyColorLayout = new QVBoxLayout(m_propertyColorDialogPanel);
    propertyColorLayout->setContentsMargins(12, 12, 12, 12);
    propertyColorLayout->setSpacing(0);
    m_propertyColorPicker = new markshot::ui::ColorPicker(m_propertyColorDialogPanel);
    m_propertyColorPicker->setColor(m_currentColor);
    connect(m_propertyColorPicker, &markshot::ui::ColorPicker::colorChanged, this,
            [this](const QColor &color) { applyPropertyColor(color); });
    propertyColorLayout->addWidget(m_propertyColorPicker);
    m_propertyColorDialogPanel->hide();

    m_propertyFontPanel = new QWidget(this);
    m_propertyFontPanel->setObjectName(QStringLiteral("propertyFontPanel"));
    m_propertyFontPanel->setStyleSheet(markshot::theme::openWithPanelStyleSheet());
    auto *fontPanelLayout = new QVBoxLayout(m_propertyFontPanel);
    fontPanelLayout->setContentsMargins(8, 8, 8, 8);
    fontPanelLayout->setSpacing(0);
    m_propertyFontList = new QListWidget(m_propertyFontPanel);
    m_propertyFontList->setFocusPolicy(Qt::NoFocus);
    m_propertyFontList->setUniformItemSizes(true);
    m_propertyFontList->setMinimumHeight(96);
    m_propertyFontList->setMaximumHeight(260);
    m_propertyFontList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_propertyFontList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    for (const QString &family : QFontDatabase::families()) {
        auto *item = new QListWidgetItem(family, m_propertyFontList);
        item->setData(Qt::UserRole, family);
        item->setFont(QFont(family, 12));
    }
    connect(m_propertyFontList, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }
        setSelectedTextFontFamily(item->data(Qt::UserRole).toString());
        if (m_propertyFontPanel) {
            m_propertyFontPanel->hide();
        }
    });
    fontPanelLayout->addWidget(m_propertyFontList);
    m_propertyFontPanel->hide();

    m_openWithPanel = new QWidget(this);
    m_openWithPanel->setObjectName(QStringLiteral("openWithPanel"));
    m_openWithPanel->setStyleSheet(markshot::theme::openWithPanelStyleSheet());
    auto *openLayout = new QVBoxLayout(m_openWithPanel);
    openLayout->setContentsMargins(12, 12, 12, 12);
    openLayout->setSpacing(7);
    m_openWithPanel->hide();

    m_colorPalette = new QWidget(this);
    m_colorPalette->setObjectName(QStringLiteral("colorPalette"));
    m_colorPalette->setStyleSheet(markshot::theme::colorPaletteStyleSheet());
    for (const QColor &color : markshot::theme::paletteColors()) {
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
    m_textEditor->setStyleSheet(markshot::theme::textEditorStyleSheet(QColor(94, 234, 212), 24));
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
    m_undoStack.clear();
    m_redoStack.clear();
    m_draft.reset();
    setSelectedAnnotations({});
    m_nextNumber = 1;
    m_nextAnnotationId = 1;
    if (m_openWithPanel) {
        m_openWithPanel->hide();
    }
    if (m_annotationPropertyPanel) {
        m_annotationPropertyPanel->hide();
    }
    if (m_propertyColorDialogPanel) {
        m_propertyColorDialogPanel->hide();
    }
    if (m_propertyFontPanel) {
        m_propertyFontPanel->hide();
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
    button->setIcon(markshot::ui::makeToolIcon(action));
    button->setIconSize(QSize(26, 26));
    button->setFocusPolicy(Qt::NoFocus);
    button->setToolTip(QStringLiteral("%1 (%2)").arg(markshot::ui::actionName(action), shortcutText));
    button->setProperty("action", markshot::ui::actionName(action));
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
    } else if (action == Action::ToolSelect) {
        connect(button, &QPushButton::clicked, this, [this] { setTool(Tool::Select); });
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
    } else if (action == Action::Clear) {
        connect(button, &QPushButton::clicked, this, [this] { clearAnnotations(); });
    } else if (action == Action::Undo) {
        connect(button, &QPushButton::clicked, this, [this] { undoAnnotationEdit(); });
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
        && watched->property("action").toString() == markshot::ui::actionName(Action::ToolMove);
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
            updateAnnotationPropertyPanelGeometry();
            return true;
        } else if (event->type() == QEvent::MouseButtonRelease && m_toolbarDragging) {
            auto *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                m_dragging = false;
                m_toolbarDragging = false;
                updateCursor();
                updateOpenWithPanelGeometry();
                updateAnnotationPropertyPanelGeometry();
                return true;
            }
        }
    }

    if (watched == m_textEditor && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            m_draft.reset();
            m_editingTextAnnotationId.reset();
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
            if (m_editingTextAnnotationId.has_value() && annotation.id == *m_editingTextAnnotationId) {
                continue;
            }
            drawAnnotation(painter, annotation, true);
        }
        if (m_draft.has_value()) {
            drawAnnotation(painter, *m_draft, true);
        }
        drawSelectedAnnotationFrame(painter);
        if (m_annotationSelectionBoxActive) {
            const QRectF box = imageRectToWidget(m_annotationSelectionBox.normalized());
            painter.setPen(QPen(QColor(45, 212, 191), 1.5, Qt::DashLine));
            painter.setBrush(QColor(45, 212, 191, 34));
            painter.drawRoundedRect(box, 4.0, 4.0);
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
    updateAnnotationPropertyPanelGeometry();
    updateOpenWithPanelGeometry();
}

void ShotWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        if (event->button() == Qt::MiddleButton && m_mode == Mode::Editing) {
            setTool(Tool::Select);
        }
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
    if (m_propertyColorDialogPanel && m_propertyColorDialogPanel->isVisible()
        && !m_propertyColorDialogPanel->geometry().contains(event->pos())
        && (!m_annotationPropertyPanel || !m_annotationPropertyPanel->geometry().contains(event->pos()))
        && (!m_toolbar || !m_toolbar->geometry().contains(event->pos()))) {
        m_propertyColorDialogPanel->hide();
    }
    if (m_propertyFontPanel && m_propertyFontPanel->isVisible()
        && !m_propertyFontPanel->geometry().contains(event->pos())
        && (!m_annotationPropertyPanel || !m_annotationPropertyPanel->geometry().contains(event->pos()))
        && (!m_toolbar || !m_toolbar->geometry().contains(event->pos()))) {
        m_propertyFontPanel->hide();
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

    if (m_tool == Tool::Select) {
        if (selectedAnnotationDeleteButtonRect().contains(event->position())) {
            deleteSelectedAnnotation();
            return;
        }

        const QVector<int> selectedIds = selectedAnnotationIds();
        if (selectedIds.size() > 1) {
            const SelectionDrag drag = annotationBoundsDragAt(imagePoint, selectedAnnotationsBounds());
            if (drag != SelectionDrag::None) {
                beginAnnotationDrag(selectedIds.first(), drag, imagePoint);
                return;
            }
        } else if (m_selectedAnnotationId.has_value()) {
            const SelectionDrag drag = annotationDragAt(imagePoint, *m_selectedAnnotationId);
            if (drag != SelectionDrag::None) {
                beginAnnotationDrag(*m_selectedAnnotationId, drag, imagePoint);
                return;
            }
        }

        const std::optional<int> hitAnnotationId = annotationAt(imagePoint);
        if (hitAnnotationId.has_value()) {
            const SelectionDrag drag = annotationDragAt(imagePoint, *hitAnnotationId);
            setSelectedAnnotations({*hitAnnotationId});
            beginAnnotationDrag(*hitAnnotationId, drag == SelectionDrag::None ? SelectionDrag::Move : drag, imagePoint);
            updateAnnotationPropertyPanel();
        } else {
            beginAnnotationSelectionBox(imagePoint);
        }
        return;
    }

    if (m_tool == Tool::Text) {
        commitTextEditor();
        beginTextAnnotation(imagePoint);
        return;
    }

    if (m_tool == Tool::Number) {
        pushHistorySnapshot();
        Annotation annotation;
        annotation.id = m_nextAnnotationId++;
        annotation.tool = Tool::Number;
        annotation.points.append(imagePoint);
        annotation.number = m_nextNumber++;
        annotation.color = m_currentColor;
        annotation.width = m_shapeWidth;
        m_annotations.append(annotation);
        update();
        return;
    }

    m_dragging = true;
    m_dragStart = imagePoint;
    Annotation annotation;
    annotation.tool = m_tool;
    annotation.color = m_currentColor;
    annotation.width = currentToolWidth();
    annotation.filled = m_shapeFilled;
    annotation.cornerRadius = m_tool == Tool::Rectangle ? m_rectangleCornerRadius : 0.0;
    annotation.fontFamily = m_textFontFamily;
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

    if (m_mode == Mode::Editing && m_tool == Tool::Select && m_dragging && m_annotationDrag != SelectionDrag::None) {
        updateAnnotationDrag(imagePoint);
        updateAnnotationPropertyPanelGeometry();
        return;
    }

    if (m_mode == Mode::Editing && m_tool == Tool::Select && m_dragging && m_annotationSelectionBoxActive) {
        updateAnnotationSelectionBox(imagePoint);
        return;
    }

    if (m_mode == Mode::Editing && m_tool == Tool::Select && !m_dragging) {
        if (selectedAnnotationIds().size() > 1) {
            m_annotationDrag = annotationBoundsDragAt(imagePoint, selectedAnnotationsBounds());
            if (m_annotationDrag != SelectionDrag::None) {
                updateCursor();
                return;
            }
        } else if (m_selectedAnnotationId.has_value()) {
            m_annotationDrag = annotationDragAt(imagePoint, *m_selectedAnnotationId);
            if (m_annotationDrag != SelectionDrag::None) {
                updateCursor();
                return;
            }
        }
        m_annotationDrag = annotationAt(imagePoint).has_value() ? SelectionDrag::Move : SelectionDrag::None;
        updateCursor();
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
        updateAnnotationPropertyPanelGeometry();
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

void ShotWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || m_mode != Mode::Editing) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    const QPointF imagePoint = widgetToImage(event->position());
    if (!m_frozenImageRect.contains(event->position())) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    const auto annotationId = annotationAt(imagePoint);
    if (!annotationId) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    const Annotation *annotation = annotationById(*annotationId);
    if (!annotation || annotation->tool != Tool::Text) {
        QWidget::mouseDoubleClickEvent(event);
        return;
    }

    const int targetId = *annotationId;

    // The single click that preceded this double-click already executed
    // mousePressEvent in the active tool's branch. Roll back its side
    // effects so the user does not see a stray duplicate annotation when
    // we transition into text editing.
    switch (m_tool) {
    case Tool::Number:
        // mousePressEvent appended a number annotation and captured a
        // history snapshot. Undo both so the new digit doesn't linger.
        if (!m_annotations.isEmpty() && m_annotations.last().tool == Tool::Number) {
            m_annotations.removeLast();
            if (m_nextNumber > 1) {
                --m_nextNumber;
            }
            if (m_nextAnnotationId > 1) {
                --m_nextAnnotationId;
            }
        }
        if (!m_undoStack.isEmpty()) {
            m_undoStack.removeLast();
        }
        break;
    case Tool::Pen:
    case Tool::Highlighter:
    case Tool::Line:
    case Tool::Rectangle:
    case Tool::Ellipse:
    case Tool::Arrow:
    case Tool::Mosaic:
        // First press created an in-flight draft; discard it so the
        // upcoming mouseReleaseEvent (which still fires for the second
        // click of the double-click) does not commit a tiny stamp.
        m_draft.reset();
        break;
    case Tool::Text:
        // First press opened a fresh, empty text editor at the click point.
        // setTool(Select) below will call commitTextEditor() and tear it
        // down without producing an empty annotation.
        break;
    case Tool::Move:
    case Tool::Select:
        // No draft to discard.
        break;
    }

    m_dragging = false;
    m_annotationDrag = SelectionDrag::None;
    m_annotationHistoryCaptured = false;

    if (m_tool != Tool::Select) {
        setTool(Tool::Select);
    }
    setSelectedAnnotations({targetId});
    beginEditingSelectedTextAnnotation();
    update();
    event->accept();
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
        updateAnnotationPropertyPanelGeometry();
        return;
    }

    if (m_tool == Tool::Select && m_annotationDrag != SelectionDrag::None) {
        m_annotationDrag = SelectionDrag::None;
        m_annotationHistoryCaptured = false;
        updateAnnotationPropertyPanel();
        updateCursor();
        update();
        return;
    }

    if (m_tool == Tool::Select && m_annotationSelectionBoxActive) {
        commitAnnotationSelectionBox();
        updateCursor();
        update();
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

    if (m_tool == Tool::Select && !selectedAnnotationIds().isEmpty()) {
        pushHistorySnapshot();
        for (int id : selectedAnnotationIds()) {
            if (Annotation *annotation = annotationById(id)) {
                if (annotation->tool == Tool::Mosaic) {
                    annotation->width = std::clamp(annotation->width + steps * 2.0, kMinMosaicBlockSize, kMaxMosaicBlockSize);
                } else {
                    annotation->width = std::clamp(annotation->width + steps * 1.0, kMinStrokeWidth, kMaxStrokeWidth);
                }
            }
        }
        updateColorPalettePreview();
        updateAnnotationPropertyPanel();
        event->accept();
        update();
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
    updateAnnotationPropertyPanel();
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
        undoAnnotationEdit();
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
    case Qt::Key_S:
        setTool(Tool::Select);
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
    if (m_annotationPropertyPanel) {
        m_annotationPropertyPanel->hide();
    }
    if (m_propertyColorDialogPanel) {
        m_propertyColorDialogPanel->hide();
    }
    if (m_propertyFontPanel) {
        m_propertyFontPanel->hide();
    }
    setFullscreenActionButtonsVisible(false);
    m_annotations.clear();
    m_undoStack.clear();
    m_redoStack.clear();
    m_draft.reset();
    setSelectedAnnotations({});
    m_nextNumber = 1;
    m_nextAnnotationId = 1;
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

    pushHistorySnapshot();
    if (m_draft->id == 0) {
        m_draft->id = m_nextAnnotationId++;
    }
    m_annotations.append(*m_draft);
    m_draft.reset();
    update();
}

void ShotWindow::setTool(Tool tool)
{
    commitTextEditor();
    m_selectionDrag = SelectionDrag::None;
    m_annotationDrag = SelectionDrag::None;
    m_annotationSelectionBoxActive = false;
    m_tool = tool;
    if (m_tool != Tool::Select) {
        setSelectedAnnotations({});
    }
    updateAnnotationPropertyPanel();
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

    if (m_tool == Tool::Select) {
        switch (m_annotationDrag) {
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
            setCursor(Qt::ArrowCursor);
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

ShotWindow::Annotation *ShotWindow::annotationById(int id)
{
    for (Annotation &annotation : m_annotations) {
        if (annotation.id == id) {
            return &annotation;
        }
    }
    return nullptr;
}

const ShotWindow::Annotation *ShotWindow::annotationById(int id) const
{
    for (const Annotation &annotation : m_annotations) {
        if (annotation.id == id) {
            return &annotation;
        }
    }
    return nullptr;
}

QRectF ShotWindow::annotationBounds(const Annotation &annotation) const
{
    auto pointsBounds = [&annotation] {
        if (annotation.points.isEmpty()) {
            return QRectF();
        }
        qreal left = annotation.points.first().x();
        qreal right = left;
        qreal top = annotation.points.first().y();
        qreal bottom = top;
        for (const QPointF &point : annotation.points) {
            left = std::min(left, point.x());
            right = std::max(right, point.x());
            top = std::min(top, point.y());
            bottom = std::max(bottom, point.y());
        }
        return QRectF(QPointF(left, top), QPointF(right, bottom)).normalized();
    };

    QRectF bounds;
    switch (annotation.tool) {
    case Tool::Move:
    case Tool::Select:
        return {};
    case Tool::Pen:
    case Tool::Highlighter:
    case Tool::Line:
    case Tool::Arrow:
        bounds = pointsBounds();
        bounds.adjust(-annotation.width, -annotation.width, annotation.width, annotation.width);
        break;
    case Tool::Rectangle:
    case Tool::Ellipse:
    case Tool::Mosaic:
    case Tool::Text:
        bounds = annotation.rect.normalized();
        break;
    case Tool::Number: {
        if (annotation.points.isEmpty()) {
            return {};
        }
        const qreal radius = std::max<qreal>(13.0, 15.0 + annotation.width);
        const QPointF center = annotation.points.first();
        bounds = QRectF(center.x() - radius, center.y() - radius, radius * 2.0, radius * 2.0);
        break;
    }
    }

    return bounds.normalized().intersected(QRectF(QPointF(0, 0), QSizeF(m_frozenFrame.size())));
}

QVector<int> ShotWindow::selectedAnnotationIds() const
{
    QVector<int> ids;
    for (int id : m_selectedAnnotationIds) {
        if (annotationById(id) && !ids.contains(id)) {
            ids.append(id);
        }
    }
    if (m_selectedAnnotationId.has_value() && annotationById(*m_selectedAnnotationId) && !ids.contains(*m_selectedAnnotationId)) {
        ids.append(*m_selectedAnnotationId);
    }
    return ids;
}

void ShotWindow::setSelectedAnnotations(QVector<int> annotationIds)
{
    QVector<int> validIds;
    for (int id : annotationIds) {
        if (annotationById(id) && !validIds.contains(id)) {
            validIds.append(id);
        }
    }
    m_selectedAnnotationIds = validIds;
    m_selectedAnnotationId = validIds.size() == 1
        ? std::optional<int>(validIds.first())
        : std::nullopt;
}

QRectF ShotWindow::selectedAnnotationsBounds() const
{
    QRectF bounds;
    for (int id : selectedAnnotationIds()) {
        const Annotation *annotation = annotationById(id);
        if (!annotation) {
            continue;
        }
        const QRectF annotationRect = annotationBounds(*annotation);
        if (annotationRect.isEmpty()) {
            continue;
        }
        bounds = bounds.isEmpty() ? annotationRect : bounds.united(annotationRect);
    }
    return bounds.normalized();
}

QVector<int> ShotWindow::annotationsInRect(QRectF imageRect) const
{
    imageRect = imageRect.normalized();
    QVector<int> ids;
    if (imageRect.width() < 2.0 || imageRect.height() < 2.0) {
        return ids;
    }
    for (const Annotation &annotation : m_annotations) {
        const QRectF bounds = annotationBounds(annotation);
        if (!bounds.isEmpty() && imageRect.intersects(bounds)) {
            ids.append(annotation.id);
        }
    }
    return ids;
}

ShotWindow::SelectionDrag ShotWindow::annotationBoundsDragAt(QPointF imagePoint, QRectF bounds) const
{
    bounds = bounds.normalized();
    if (bounds.isEmpty()) {
        return SelectionDrag::None;
    }

    const qreal imageTolerance = 10.0 * m_frozenFrame.width() / std::max<qreal>(1.0, m_frozenImageRect.width());
    const bool nearLeft = std::abs(imagePoint.x() - bounds.left()) <= imageTolerance;
    const bool nearRight = std::abs(imagePoint.x() - bounds.right()) <= imageTolerance;
    const bool nearTop = std::abs(imagePoint.y() - bounds.top()) <= imageTolerance;
    const bool nearBottom = std::abs(imagePoint.y() - bounds.bottom()) <= imageTolerance;

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
    return bounds.adjusted(-imageTolerance, -imageTolerance, imageTolerance, imageTolerance).contains(imagePoint)
        ? SelectionDrag::Move
        : SelectionDrag::None;
}

QVector<QPointF> ShotWindow::selectionHandlePoints(QRectF rect) const
{
    rect = rect.normalized();
    return {
        rect.topLeft(), QPointF(rect.center().x(), rect.top()), rect.topRight(),
        QPointF(rect.left(), rect.center().y()), QPointF(rect.right(), rect.center().y()),
        rect.bottomLeft(), QPointF(rect.center().x(), rect.bottom()), rect.bottomRight(),
    };
}

QRectF ShotWindow::selectedAnnotationDeleteButtonRect() const
{
    const QRectF bounds = imageRectToWidget(selectedAnnotationsBounds());
    if (bounds.isEmpty()) {
        return {};
    }
    constexpr qreal buttonSize = 20.0;
    const qreal x = std::clamp(bounds.right() + 8.0, 8.0, std::max<qreal>(8.0, width() - buttonSize - 8.0));
    const qreal y = std::clamp(bounds.top() - buttonSize - 8.0, 8.0, std::max<qreal>(8.0, height() - buttonSize - 8.0));
    return QRectF(x,
                  y,
                  buttonSize,
                  buttonSize);
}

QRectF ShotWindow::resizedBounds(QRectF start, SelectionDrag drag, QPointF imagePoint) const
{
    start = start.normalized();
    const QPointF clamped = clampImagePoint(imagePoint);
    qreal left = start.left();
    qreal top = start.top();
    qreal right = start.right();
    qreal bottom = start.bottom();
    const qreal maxWidth = m_frozenFrame.width();
    const qreal maxHeight = m_frozenFrame.height();

    if (drag == SelectionDrag::Left || drag == SelectionDrag::TopLeft || drag == SelectionDrag::BottomLeft) {
        left = std::clamp(clamped.x(), 0.0, right - kMinSelectionSize);
    }
    if (drag == SelectionDrag::Right || drag == SelectionDrag::TopRight || drag == SelectionDrag::BottomRight) {
        right = std::clamp(clamped.x(), left + kMinSelectionSize, maxWidth);
    }
    if (drag == SelectionDrag::Top || drag == SelectionDrag::TopLeft || drag == SelectionDrag::TopRight) {
        top = std::clamp(clamped.y(), 0.0, bottom - kMinSelectionSize);
    }
    if (drag == SelectionDrag::Bottom || drag == SelectionDrag::BottomLeft || drag == SelectionDrag::BottomRight) {
        bottom = std::clamp(clamped.y(), top + kMinSelectionSize, maxHeight);
    }

    return QRectF(QPointF(left, top), QPointF(right, bottom)).normalized();
}

ShotWindow::SelectionDrag ShotWindow::annotationDragAt(QPointF imagePoint, int annotationId) const
{
    const Annotation *annotation = annotationById(annotationId);
    if (!annotation) {
        return SelectionDrag::None;
    }

    const QRectF bounds = annotationBounds(*annotation);
    if (bounds.isEmpty()) {
        return SelectionDrag::None;
    }

    return annotationBoundsDragAt(imagePoint, bounds);
}

std::optional<int> ShotWindow::annotationAt(QPointF imagePoint) const
{
    const qreal imageTolerance = 8.0 * m_frozenFrame.width() / std::max<qreal>(1.0, m_frozenImageRect.width());
    for (int i = m_annotations.size() - 1; i >= 0; --i) {
        const Annotation &annotation = m_annotations.at(i);
        const QRectF bounds = annotationBounds(annotation).adjusted(-imageTolerance, -imageTolerance, imageTolerance, imageTolerance);
        if (bounds.contains(imagePoint)) {
            return annotation.id;
        }
    }
    return std::nullopt;
}

void ShotWindow::drawSelectedAnnotationFrame(QPainter &painter) const
{
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (selectedIds.isEmpty()) {
        return;
    }

    const QRectF bounds = imageRectToWidget(selectedAnnotationsBounds());
    if (bounds.isEmpty()) {
        return;
    }

    painter.save();
    painter.setPen(QPen(QColor(251, 146, 60), 2.0, Qt::DashLine));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(bounds, 4.0, 4.0);
    if (selectedIds.size() > 1) {
        painter.setPen(QPen(QColor(251, 146, 60, 150), 1.0, Qt::DashLine));
        for (int id : selectedIds) {
            if (const Annotation *annotation = annotationById(id)) {
                painter.drawRoundedRect(imageRectToWidget(annotationBounds(*annotation)), 3.0, 3.0);
            }
        }
    }
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(251, 146, 60));
    for (const QPointF &handle : selectionHandlePoints(bounds)) {
        painter.drawRoundedRect(QRectF(handle.x() - 4.5, handle.y() - 4.5, 9.0, 9.0), 2.0, 2.0);
    }
    const QRectF deleteButton = selectedAnnotationDeleteButtonRect();
    if (!deleteButton.isEmpty()) {
        painter.setBrush(QColor(239, 68, 68));
        painter.setPen(QPen(QColor(255, 255, 255), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawEllipse(deleteButton);
        painter.drawLine(deleteButton.center() + QPointF(-4.5, -4.5), deleteButton.center() + QPointF(4.5, 4.5));
        painter.drawLine(deleteButton.center() + QPointF(4.5, -4.5), deleteButton.center() + QPointF(-4.5, 4.5));
    }
    painter.restore();
}

void ShotWindow::moveAnnotation(Annotation &annotation, QPointF delta) const
{
    annotation.rect.translate(delta);
    for (QPointF &point : annotation.points) {
        point = clampImagePoint(point + delta);
    }
}

void ShotWindow::transformAnnotation(Annotation &annotation, QRectF oldBounds, QRectF newBounds) const
{
    oldBounds = oldBounds.normalized();
    newBounds = newBounds.normalized();
    if (oldBounds.width() <= 0.0 || oldBounds.height() <= 0.0) {
        moveAnnotation(annotation, newBounds.topLeft() - oldBounds.topLeft());
        return;
    }

    auto mapPoint = [this, oldBounds, newBounds](QPointF point) {
        const qreal xRatio = (point.x() - oldBounds.left()) / oldBounds.width();
        const qreal yRatio = (point.y() - oldBounds.top()) / oldBounds.height();
        return clampImagePoint(QPointF(newBounds.left() + xRatio * newBounds.width(),
                                      newBounds.top() + yRatio * newBounds.height()));
    };
    const qreal scaleFactor = std::max(newBounds.width() / oldBounds.width(),
                                       newBounds.height() / oldBounds.height());

    switch (annotation.tool) {
    case Tool::Move:
    case Tool::Select:
        return;
    case Tool::Rectangle:
    case Tool::Ellipse:
    case Tool::Mosaic:
    case Tool::Text:
        annotation.rect = QRectF(mapPoint(annotation.rect.normalized().topLeft()),
                                 mapPoint(annotation.rect.normalized().bottomRight())).normalized();
        break;
    case Tool::Pen:
    case Tool::Highlighter:
    case Tool::Line:
    case Tool::Arrow:
        for (QPointF &point : annotation.points) {
            point = mapPoint(point);
        }
        break;
    case Tool::Number:
        if (!annotation.points.isEmpty()) {
            annotation.points[0] = mapPoint(annotation.points.first());
            annotation.width = std::clamp(annotation.width * scaleFactor, kMinStrokeWidth, kMaxStrokeWidth);
        }
        break;
    }
}

void ShotWindow::beginAnnotationDrag(int annotationId, SelectionDrag drag, QPointF imagePoint)
{
    Annotation *annotation = annotationById(annotationId);
    if (!annotation || drag == SelectionDrag::None) {
        return;
    }
    if (!selectedAnnotationIds().contains(annotationId)) {
        setSelectedAnnotations({annotationId});
    }
    m_annotationDrag = drag;
    m_annotationBeforeDrag = *annotation;
    m_annotationsBeforeDrag.clear();
    for (int id : selectedAnnotationIds()) {
        if (const Annotation *selected = annotationById(id)) {
            m_annotationsBeforeDrag.append(*selected);
        }
    }
    m_annotationBoundsBeforeDrag = selectedAnnotationsBounds();
    m_dragStart = imagePoint;
    m_dragging = true;
    m_annotationHistoryCaptured = false;
    updateCursor();
    updateAnnotationPropertyPanel();
    update();
}

void ShotWindow::updateAnnotationDrag(QPointF imagePoint)
{
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (selectedIds.isEmpty() || m_annotationDrag == SelectionDrag::None) {
        return;
    }
    if (!m_annotationHistoryCaptured) {
        pushHistorySnapshot();
        m_annotationHistoryCaptured = true;
    }

    for (const Annotation &before : m_annotationsBeforeDrag) {
        if (Annotation *annotation = annotationById(before.id)) {
            *annotation = before;
        }
    }

    if (m_annotationDrag == SelectionDrag::Move) {
        const QRectF startBounds = m_annotationBoundsBeforeDrag;
        QPointF delta = clampImagePoint(imagePoint) - m_dragStart;
        delta.setX(std::clamp(delta.x(), -startBounds.left(), m_frozenFrame.width() - startBounds.right()));
        delta.setY(std::clamp(delta.y(), -startBounds.top(), m_frozenFrame.height() - startBounds.bottom()));
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id)) {
                moveAnnotation(*annotation, delta);
            }
        }
    } else {
        const QRectF newBounds = resizedBounds(m_annotationBoundsBeforeDrag, m_annotationDrag, imagePoint);
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id)) {
                transformAnnotation(*annotation, m_annotationBoundsBeforeDrag, newBounds);
            }
        }
    }
    update();
}

void ShotWindow::beginAnnotationSelectionBox(QPointF imagePoint)
{
    setSelectedAnnotations({});
    m_annotationDrag = SelectionDrag::None;
    m_annotationSelectionBoxActive = true;
    m_dragging = true;
    m_dragStart = clampImagePoint(imagePoint);
    m_annotationSelectionBox = QRectF(m_dragStart, m_dragStart);
    updateAnnotationPropertyPanel();
    updateCursor();
    update();
}

void ShotWindow::updateAnnotationSelectionBox(QPointF imagePoint)
{
    m_annotationSelectionBox = QRectF(m_dragStart, clampImagePoint(imagePoint)).normalized();
    update();
}

void ShotWindow::commitAnnotationSelectionBox()
{
    m_annotationSelectionBoxActive = false;
    setSelectedAnnotations(annotationsInRect(m_annotationSelectionBox));
    m_annotationSelectionBox = {};
    updateAnnotationPropertyPanel();
}

ShotWindow::HistorySnapshot ShotWindow::currentHistorySnapshot() const
{
    return {m_annotations, m_selectedAnnotationId, selectedAnnotationIds(), m_nextNumber, m_nextAnnotationId};
}

void ShotWindow::restoreHistorySnapshot(const HistorySnapshot &snapshot)
{
    m_annotations = snapshot.annotations;
    setSelectedAnnotations(snapshot.selectedAnnotationIds.isEmpty()
                               ? (snapshot.selectedAnnotationId.has_value() ? QVector<int>{*snapshot.selectedAnnotationId} : QVector<int>{})
                               : snapshot.selectedAnnotationIds);
    m_nextNumber = snapshot.nextNumber;
    m_nextAnnotationId = snapshot.nextAnnotationId;
    m_draft.reset();
    m_annotationDrag = SelectionDrag::None;
    m_annotationSelectionBoxActive = false;
    m_annotationHistoryCaptured = false;
    updateAnnotationPropertyPanel();
    updateCursor();
    update();
}

void ShotWindow::pushHistorySnapshot()
{
    m_undoStack.append(currentHistorySnapshot());
    if (m_undoStack.size() > 100) {
        m_undoStack.removeFirst();
    }
    m_redoStack.clear();
}

void ShotWindow::undoAnnotationEdit()
{
    if (m_undoStack.isEmpty()) {
        return;
    }
    const HistorySnapshot current = currentHistorySnapshot();
    const HistorySnapshot previous = m_undoStack.takeLast();
    m_redoStack.append(current);
    restoreHistorySnapshot(previous);
}

qreal ShotWindow::currentToolWidth() const
{
    switch (m_tool) {
    case Tool::Move:
    case Tool::Select:
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
    case Tool::Select:
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
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (m_tool == Tool::Select && !selectedIds.isEmpty()) {
        pushHistorySnapshot();
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id)) {
                annotation->color = color;
            }
        }
        updateAnnotationPropertyPanel();
    }
    if (m_draft.has_value()) {
        m_draft->color = color;
    }
    if (m_colorPalette) {
        m_colorPalette->hide();
    }
    updateColorPalettePreview();
    updateAnnotationPropertyPanel();
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
    case Tool::Select:
        return QStringLiteral("Select");
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
        updateAnnotationPropertyPanelGeometry();
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
    updateAnnotationPropertyPanelGeometry();
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

void ShotWindow::updateAnnotationPropertyPanel()
{
    if (!m_annotationPropertyPanel) {
        return;
    }

    const QVector<int> selectedIds = selectedAnnotationIds();
    const Annotation *annotation = selectedIds.size() == 1
        ? annotationById(selectedIds.first())
        : nullptr;
    const Annotation *firstSelectedAnnotation = !selectedIds.isEmpty()
        ? annotationById(selectedIds.first())
        : nullptr;
    const bool groupSelection = m_tool == Tool::Select && selectedIds.size() > 1;
    const bool editingAnnotation = m_tool == Tool::Select && !selectedIds.isEmpty();
    const bool editingTool = m_mode == Mode::Editing
        && m_tool != Tool::Move
        && m_tool != Tool::Select;
    if (!editingAnnotation && !editingTool) {
        m_annotationPropertyPanel->hide();
        if (m_propertyColorDialogPanel) {
            m_propertyColorDialogPanel->hide();
        }
        if (m_propertyFontPanel) {
            m_propertyFontPanel->hide();
        }
        return;
    }

    QString title = QStringLiteral("Object");
    const Tool panelTool = groupSelection ? Tool::Select : (annotation ? annotation->tool : m_tool);
    const QColor panelColor = firstSelectedAnnotation ? firstSelectedAnnotation->color : m_currentColor;
    const qreal panelWidth = firstSelectedAnnotation ? firstSelectedAnnotation->width : currentToolWidth();
    const bool panelFilled = annotation ? annotation->filled : m_shapeFilled;
    const qreal panelRadius = annotation ? annotation->cornerRadius : m_rectangleCornerRadius;
    const QString panelFontFamily = annotation ? annotation->fontFamily : m_textFontFamily;

    switch (panelTool) {
    case Tool::Move:
    case Tool::Select:
        title = QStringLiteral("Object");
        break;
    case Tool::Pen:
        title = QStringLiteral("Pen");
        break;
    case Tool::Highlighter:
        title = QStringLiteral("Highlighter");
        break;
    case Tool::Line:
        title = QStringLiteral("Line");
        break;
    case Tool::Rectangle:
        title = QStringLiteral("Rect");
        break;
    case Tool::Ellipse:
        title = QStringLiteral("Ellipse");
        break;
    case Tool::Arrow:
        title = QStringLiteral("Arrow");
        break;
    case Tool::Text:
        title = QStringLiteral("Text");
        break;
    case Tool::Number:
        title = QStringLiteral("Number");
        break;
    case Tool::Mosaic:
        title = QStringLiteral("Mosaic");
        break;
    }

    if (m_annotationPropertyTitle) {
        m_annotationPropertyTitle->setText(groupSelection
                                               ? QStringLiteral("Group %1").arg(selectedIds.size())
                                               : title);
    }
    if (m_propertyEditTextButton) {
        m_propertyEditTextButton->setVisible(!groupSelection && editingAnnotation && panelTool == Tool::Text);
    }
    if (m_propertyFontButton) {
        m_propertyFontButton->setVisible(!groupSelection && panelTool == Tool::Text);
        if (!groupSelection && panelTool == Tool::Text) {
            const QString family = panelFontFamily.isEmpty() ? QStringLiteral("Sans Serif") : panelFontFamily;
            m_propertyFontButton->setText(QStringLiteral("Font"));
            m_propertyFontButton->setToolTip(family);
            if (m_propertyFontList) {
                const auto matches = m_propertyFontList->findItems(family, Qt::MatchExactly);
                if (!matches.isEmpty()) {
                    m_propertyFontList->setCurrentItem(matches.first());
                    m_propertyFontList->scrollToItem(matches.first(), QAbstractItemView::PositionAtCenter);
                }
            }
        } else if (m_propertyFontPanel) {
            m_propertyFontPanel->hide();
            m_propertyFontButton->setToolTip(QStringLiteral("Text font"));
        }
    }
    if (m_propertyFillButton) {
        const bool supportsFill = !groupSelection && (panelTool == Tool::Rectangle || panelTool == Tool::Ellipse);
        m_propertyFillButton->setVisible(supportsFill);
        const QSignalBlocker blocker(m_propertyFillButton);
        m_propertyFillButton->setChecked(panelFilled);
    }
    if (m_propertyRadiusLabel) {
        m_propertyRadiusLabel->setVisible(!groupSelection && panelTool == Tool::Rectangle);
        m_propertyRadiusLabel->setText(QStringLiteral("Radius %1").arg(qRound(panelRadius)));
    }
    if (m_propertyRadiusSlider) {
        m_propertyRadiusSlider->setVisible(!groupSelection && panelTool == Tool::Rectangle);
        const QSignalBlocker blocker(m_propertyRadiusSlider);
        m_propertyRadiusSlider->setValue(qRound(panelRadius));
    }
    if (m_propertyWidthLabel) {
        m_propertyWidthLabel->setText(QStringLiteral("Width %1").arg(qRound(panelWidth)));
    }
    if (m_propertyWidthSlider) {
        const QSignalBlocker blocker(m_propertyWidthSlider);
        if (panelTool == Tool::Mosaic) {
            m_propertyWidthSlider->setRange(qRound(kMinMosaicBlockSize), qRound(kMaxMosaicBlockSize));
        } else {
            m_propertyWidthSlider->setRange(qRound(kMinStrokeWidth), qRound(kMaxStrokeWidth));
        }
        m_propertyWidthSlider->setValue(qRound(panelWidth));
    }
    if (m_propertyColorButton) {
        m_propertyColorButton->setStyleSheet(markshot::theme::propertyColorButtonStyleSheet(panelColor));
        m_propertyColorButton->setVisible(panelTool != Tool::Mosaic);
        if (panelTool == Tool::Mosaic && m_propertyColorDialogPanel) {
            m_propertyColorDialogPanel->hide();
        }
    }
    if (m_propertyColorPicker && m_propertyColorDialogPanel && m_propertyColorDialogPanel->isVisible()) {
        const QSignalBlocker blocker(m_propertyColorPicker);
        m_propertyColorPicker->setColor(panelColor);
    }

    m_annotationPropertyPanel->show();
    if (QLayout *panelLayout = m_annotationPropertyPanel->layout()) {
        panelLayout->activate();
    }
    updateAnnotationPropertyPanelGeometry();
    m_annotationPropertyPanel->raise();
    if (m_propertyColorDialogPanel && m_propertyColorDialogPanel->isVisible()) {
        updatePropertyColorDialogGeometry();
        m_propertyColorDialogPanel->raise();
    }
}

void ShotWindow::updateAnnotationPropertyPanelGeometry()
{
    if (!m_annotationPropertyPanel) {
        return;
    }

    m_annotationPropertyPanel->adjustSize();
    const QSize panelSize = m_annotationPropertyPanel->sizeHint();
    const QRect toolbarRect = m_toolbar && m_toolbar->isVisible()
        ? m_toolbar->geometry()
        : QRect(8, 8, 0, 0);
    int x = toolbarRect.left();
    int y = toolbarRect.bottom() + kToolbarMargin;
    if (y + panelSize.height() > height() - 8) {
        y = toolbarRect.top() - panelSize.height() - kToolbarMargin;
    }
    if (x + panelSize.width() > width() - 8) {
        x = toolbarRect.right() - panelSize.width();
    }
    x = std::clamp(x, 8, std::max(8, width() - panelSize.width() - 8));
    y = std::clamp(y, 8, std::max(8, height() - panelSize.height() - 8));
    m_annotationPropertyPanel->setGeometry(x, y, panelSize.width(), panelSize.height());
    updatePropertyColorDialogGeometry();
    updatePropertyFontPanelGeometry();
}

void ShotWindow::updatePropertyColorDialogGeometry()
{
    if (!m_propertyColorDialogPanel || !m_annotationPropertyPanel) {
        return;
    }

    m_propertyColorDialogPanel->adjustSize();
    QSize panelSize = m_propertyColorDialogPanel->sizeHint();
    panelSize.setWidth(std::min(panelSize.width(), std::max(160, width() - 16)));
    panelSize.setHeight(std::min(panelSize.height(), std::max(180, height() - 16)));

    // Anchor on the color button's centre so the picker stays put when the
    // property panel resizes (e.g. fill/radius/font slots showing or hiding).
    // Falling back to the property panel keeps geometry valid when the
    // button is hidden (mosaic case).
    QPoint anchor;
    if (m_propertyColorButton && m_propertyColorButton->isVisible()) {
        anchor = m_propertyColorButton->mapTo(this, m_propertyColorButton->rect().center());
    } else {
        const QRect propertyRect = m_annotationPropertyPanel->geometry();
        anchor = QPoint(propertyRect.center().x(), propertyRect.bottom());
    }

    int x = anchor.x() - panelSize.width() / 2;
    int y = anchor.y() + 14;

    const QRect propertyRect = m_annotationPropertyPanel->geometry();
    if (y + panelSize.height() > height() - 8) {
        // Place above the property panel instead.
        y = propertyRect.top() - panelSize.height() - 8;
    }
    x = std::clamp(x, 8, std::max(8, width() - panelSize.width() - 8));
    y = std::clamp(y, 8, std::max(8, height() - panelSize.height() - 8));
    m_propertyColorDialogPanel->setGeometry(x, y, panelSize.width(), panelSize.height());
}

void ShotWindow::updatePropertyFontPanelGeometry()
{
    if (!m_propertyFontPanel || !m_annotationPropertyPanel || !m_propertyFontButton) {
        return;
    }

    const int visibleRows = std::min(10, m_propertyFontList ? std::max(1, m_propertyFontList->count()) : 1);
    const int rowHeight = m_propertyFontList ? std::max(24, m_propertyFontList->sizeHintForRow(0)) : 28;
    QSize panelSize(260, std::min(280, visibleRows * rowHeight + 18));
    panelSize.setWidth(std::min(panelSize.width(), std::max(180, width() - 16)));
    panelSize.setHeight(std::min(panelSize.height(), std::max(120, height() - 16)));

    QPoint anchor = m_propertyFontButton->mapTo(this, QPoint(m_propertyFontButton->width() / 2,
                                                            m_propertyFontButton->height()));
    int x = anchor.x() - panelSize.width() / 2;
    int y = anchor.y() + 10;
    const QRect propertyRect = m_annotationPropertyPanel->geometry();
    if (y + panelSize.height() > height() - 8) {
        y = propertyRect.top() - panelSize.height() - 8;
    }
    x = std::clamp(x, 8, std::max(8, width() - panelSize.width() - 8));
    y = std::clamp(y, 8, std::max(8, height() - panelSize.height() - 8));
    if (m_propertyFontList) {
        m_propertyFontList->setFixedHeight(std::max(80, panelSize.height() - 16));
    }
    m_propertyFontPanel->setFixedSize(panelSize);
    m_propertyFontPanel->setGeometry(x, y, panelSize.width(), panelSize.height());
}

void ShotWindow::adjustSelectedAnnotationWidth(qreal delta)
{
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (selectedIds.isEmpty()) {
        return;
    }

    pushHistorySnapshot();
    for (int id : selectedIds) {
        if (Annotation *annotation = annotationById(id)) {
            if (annotation->tool == Tool::Mosaic) {
                annotation->width = std::clamp(annotation->width + delta * 2.0, kMinMosaicBlockSize, kMaxMosaicBlockSize);
            } else {
                annotation->width = std::clamp(annotation->width + delta, kMinStrokeWidth, kMaxStrokeWidth);
            }
        }
    }
    updateAnnotationPropertyPanel();
    update();
}

void ShotWindow::setSelectedAnnotationWidth(int width)
{
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        bool changed = false;
        for (int id : selectedIds) {
            const Annotation *annotation = annotationById(id);
            if (annotation && qRound(annotation->width) != width) {
                changed = true;
                break;
            }
        }
        if (!changed) {
            return;
        }
        pushHistorySnapshot();
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id)) {
                annotation->width = width;
            }
        }
    } else {
        switch (m_tool) {
        case Tool::Pen:
        case Tool::Highlighter:
            m_penWidth = width;
            break;
        case Tool::Mosaic:
            m_mosaicBlockSize = width;
            break;
        case Tool::Move:
        case Tool::Select:
            return;
        case Tool::Line:
        case Tool::Rectangle:
        case Tool::Ellipse:
        case Tool::Arrow:
        case Tool::Text:
        case Tool::Number:
            m_shapeWidth = width;
            break;
        }
    }
    updateAnnotationPropertyPanel();
    updateColorPalettePreview();
    update();
}

void ShotWindow::setSelectedAnnotationFilled(bool filled)
{
    if (m_selectedAnnotationId.has_value()) {
        Annotation *annotation = annotationById(*m_selectedAnnotationId);
        if (!annotation || annotation->filled == filled) {
            return;
        }
        if (annotation->tool != Tool::Rectangle && annotation->tool != Tool::Ellipse) {
            return;
        }
        pushHistorySnapshot();
        annotation->filled = filled;
    } else {
        if (m_tool != Tool::Rectangle && m_tool != Tool::Ellipse) {
            return;
        }
        m_shapeFilled = filled;
    }
    updateAnnotationPropertyPanel();
    update();
}

void ShotWindow::setSelectedAnnotationCornerRadius(int radius)
{
    if (m_selectedAnnotationId.has_value()) {
        Annotation *annotation = annotationById(*m_selectedAnnotationId);
        if (!annotation || annotation->tool != Tool::Rectangle || qRound(annotation->cornerRadius) == radius) {
            return;
        }
        pushHistorySnapshot();
        annotation->cornerRadius = radius;
    } else {
        if (m_tool != Tool::Rectangle || qRound(m_rectangleCornerRadius) == radius) {
            return;
        }
        m_rectangleCornerRadius = radius;
    }
    updateAnnotationPropertyPanel();
    update();
}

void ShotWindow::deleteSelectedAnnotation()
{
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (selectedIds.isEmpty()) {
        return;
    }
    pushHistorySnapshot();
    for (int i = m_annotations.size() - 1; i >= 0; --i) {
        if (selectedIds.contains(m_annotations.at(i).id)) {
            m_annotations.removeAt(i);
        }
    }
    setSelectedAnnotations({});
    m_annotationDrag = SelectionDrag::None;
    updateAnnotationPropertyPanel();
    updateCursor();
    update();
}

void ShotWindow::openSelectedAnnotationColorPalette()
{
    if (!m_propertyColorDialogPanel || !m_propertyColorPicker || !m_annotationPropertyPanel) {
        return;
    }

    if (m_propertyColorDialogPanel->isVisible()) {
        m_propertyColorDialogPanel->hide();
        return;
    }

    if (m_colorPalette) {
        m_colorPalette->hide();
    }
    QColor color = m_currentColor;
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        if (const Annotation *annotation = annotationById(selectedIds.first())) {
            color = annotation->color;
        }
    }
    m_propertyColorEditHistoryCaptured = false;
    {
        const QSignalBlocker blocker(m_propertyColorPicker);
        m_propertyColorPicker->setColor(color);
    }
    updateAnnotationPropertyPanel();
    if (m_annotationPropertyPanel) {
        m_annotationPropertyPanel->show();
        m_annotationPropertyPanel->raise();
        if (QLayout *panelLayout = m_annotationPropertyPanel->layout()) {
            panelLayout->activate();
        }
        updateAnnotationPropertyPanelGeometry();
    }
    if (QLayout *colorLayout = m_propertyColorDialogPanel->layout()) {
        colorLayout->activate();
    }
    updatePropertyColorDialogGeometry();
    m_propertyColorDialogPanel->show();
    updatePropertyColorDialogGeometry();
    m_propertyColorDialogPanel->raise();
    QTimer::singleShot(0, this, [this] {
        if (m_propertyColorDialogPanel && m_propertyColorDialogPanel->isVisible()) {
            updatePropertyColorDialogGeometry();
            m_propertyColorDialogPanel->raise();
        }
    });
}

void ShotWindow::toggleSelectedTextFontPanel()
{
    if (!m_propertyFontPanel || !m_propertyFontList || !m_propertyFontButton) {
        return;
    }

    if (m_propertyFontPanel->isVisible()) {
        m_propertyFontPanel->hide();
        return;
    }

    if (m_propertyColorDialogPanel) {
        m_propertyColorDialogPanel->hide();
    }
    updateAnnotationPropertyPanel();
    if (QLayout *fontLayout = m_propertyFontPanel->layout()) {
        fontLayout->activate();
    }
    updatePropertyFontPanelGeometry();
    m_propertyFontPanel->show();
    updatePropertyFontPanelGeometry();
    m_propertyFontPanel->raise();
}

void ShotWindow::applyPropertyColor(QColor color)
{
    if (!color.isValid()) {
        return;
    }
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        if (!m_propertyColorEditHistoryCaptured) {
            pushHistorySnapshot();
            m_propertyColorEditHistoryCaptured = true;
        }
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id)) {
                annotation->color = color;
            }
        }
    } else {
        m_currentColor = color;
    }
    if (m_draft.has_value()) {
        m_draft->color = color;
    }
    updateColorPalettePreview();
    updateAnnotationPropertyPanel();
    update();
}

void ShotWindow::clearAnnotations()
{
    commitTextEditor();
    if (m_annotations.isEmpty() && !m_draft.has_value()) {
        return;
    }

    pushHistorySnapshot();
    m_annotations.clear();
    m_draft.reset();
    setSelectedAnnotations({});
    m_annotationDrag = SelectionDrag::None;
    m_annotationSelectionBoxActive = false;
    m_annotationHistoryCaptured = false;
    m_nextNumber = 1;
    m_nextAnnotationId = 1;
    if (m_propertyColorDialogPanel) {
        m_propertyColorDialogPanel->hide();
    }
    if (m_propertyFontPanel) {
        m_propertyFontPanel->hide();
    }
    updateAnnotationPropertyPanel();
    updateCursor();
    update();
}

void ShotWindow::setSelectedTextFontFamily(const QString &fontFamily)
{
    if (fontFamily.isEmpty()) {
        return;
    }

    if (m_selectedAnnotationId.has_value()) {
        Annotation *annotation = annotationById(*m_selectedAnnotationId);
        if (!annotation || annotation->tool != Tool::Text || annotation->fontFamily == fontFamily) {
            return;
        }
        pushHistorySnapshot();
        annotation->fontFamily = fontFamily;
    } else {
        if (m_tool != Tool::Text || m_textFontFamily == fontFamily) {
            return;
        }
        m_textFontFamily = fontFamily;
        if (m_textEditor && m_textEditor->isVisible() && !m_editingTextAnnotationId.has_value()) {
            m_textEditor->setFont(QFont(m_textFontFamily, qRound(20.0 + m_shapeWidth), QFont::DemiBold));
        }
    }
    updateAnnotationPropertyPanel();
    update();
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
    if (m_editingTextAnnotationId.has_value()) {
        if (const Annotation *annotation = annotationById(*m_editingTextAnnotationId)) {
            QRect editorRect = imageRectToWidget(annotation->rect.normalized()).toAlignedRect().adjusted(0, 0, 1, 1);
            editorRect.moveLeft(std::clamp(editorRect.left(), 8, std::max(8, width() - editorRect.width() - 8)));
            editorRect.moveTop(std::clamp(editorRect.top(), 8, std::max(8, height() - editorRect.height() - 8)));
            m_textEditor->setGeometry(editorRect);
        }
        return;
    }

    const QPointF topLeft = imageToWidget(m_textEditorImagePoint);
    const QRectF selection = imageRectToWidget(normalizedSelection());
    const int availableRight = std::max(80, qRound(selection.right() - topLeft.x() - 12));
    const int availableBottom = std::max(36, qRound(selection.bottom() - topLeft.y() - 12));
    const int editorWidth = std::clamp(220, 96, availableRight);
    const int editorHeight = std::clamp(m_textEditor->fontMetrics().height() + 18, 38, availableBottom);
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

    const HistorySnapshot current = currentHistorySnapshot();
    const HistorySnapshot next = m_redoStack.takeLast();
    m_undoStack.append(current);
    restoreHistorySnapshot(next);
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

    painter.save();
    QPen pen(annotation.color, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    switch (annotation.tool) {
    case Tool::Move:
    case Tool::Select:
        break;
    case Tool::Pen: {
        if (annotation.points.size() < 2) {
            break;
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
            break;
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
    case Tool::Rectangle: {
        painter.setBrush(annotation.filled ? QBrush(annotation.color) : QBrush(Qt::NoBrush));
        const QRectF rect = mapRect(annotation.rect);
        const qreal radius = annotation.cornerRadius * scale;
        if (radius > 0.0) {
            painter.drawRoundedRect(rect, radius, radius);
        } else {
            painter.drawRect(rect);
        }
        break;
    }
    case Tool::Ellipse:
        painter.setBrush(annotation.filled ? QBrush(annotation.color) : QBrush(Qt::NoBrush));
        painter.drawEllipse(mapRect(annotation.rect));
        break;
    case Tool::Arrow:
        if (annotation.points.size() >= 2) {
            drawArrow(painter, mapPoint(annotation.points.first()), mapPoint(annotation.points.last()), penWidth);
        }
        break;
    case Tool::Text: {
        QFont font(annotation.fontFamily.isEmpty() ? QStringLiteral("Sans Serif") : annotation.fontFamily,
                   qRound((19.0 + annotation.width) * scale),
                   QFont::DemiBold);
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
    painter.restore();
}

void ShotWindow::drawArrow(QPainter &painter, QPointF start, QPointF end, qreal width) const
{
    const QLineF line(start, end);
    if (line.length() < 1.0) {
        return;
    }

    const QColor color = painter.pen().color();
    painter.save();
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(color, width, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter.drawLine(start, end);

    const qreal angle = std::atan2(end.y() - start.y(), end.x() - start.x());
    const qreal arrowSize = std::clamp(width * 6.0, 14.0, 38.0);
    constexpr qreal headAngle = M_PI / 7.5;
    const QPointF p1 = end - QPointF(std::cos(angle - headAngle) * arrowSize,
                                     std::sin(angle - headAngle) * arrowSize);
    const QPointF p2 = end - QPointF(std::cos(angle + headAngle) * arrowSize,
                                     std::sin(angle + headAngle) * arrowSize);

    QPainterPath head;
    head.moveTo(p1);
    head.lineTo(end);
    head.lineTo(p2);
    painter.setPen(QPen(color, width, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    painter.drawPath(head);
    painter.restore();
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
    m_editingTextAnnotationId.reset();
    m_textEditorImagePoint = imagePoint;
    m_draft.reset();
    m_textEditor->clear();
    m_textEditor->setStyleSheet(markshot::theme::textEditorStyleSheet(m_currentColor, qRound(20.0 + m_shapeWidth)));
    m_textEditor->setFont(QFont(m_textFontFamily, qRound(20.0 + m_shapeWidth), QFont::DemiBold));
    m_textEditor->show();
    m_textEditor->raise();
    updateTextEditorGeometry();
    m_textEditor->setFocus(Qt::MouseFocusReason);
    update();
}

void ShotWindow::beginEditingSelectedTextAnnotation()
{
    if (!m_selectedAnnotationId.has_value()) {
        return;
    }
    Annotation *annotation = annotationById(*m_selectedAnnotationId);
    if (!annotation || annotation->tool != Tool::Text) {
        return;
    }

    m_editingTextAnnotationId = annotation->id;
    m_textEditorImagePoint = annotation->rect.normalized().topLeft();
    m_draft.reset();
    m_textEditor->setPlainText(annotation->text);
    m_textEditor->setStyleSheet(markshot::theme::textEditorStyleSheet(annotation->color, qRound(20.0 + annotation->width)));
    m_textEditor->setFont(QFont(annotation->fontFamily.isEmpty() ? QStringLiteral("Sans Serif") : annotation->fontFamily,
                                qRound(20.0 + annotation->width),
                                QFont::DemiBold));
    if (m_annotationPropertyPanel) {
        m_annotationPropertyPanel->hide();
    }
    if (m_propertyColorDialogPanel) {
        m_propertyColorDialogPanel->hide();
    }
    if (m_propertyFontPanel) {
        m_propertyFontPanel->hide();
    }
    m_textEditor->show();
    m_textEditor->raise();
    const QRectF widgetRect = imageRectToWidget(annotation->rect.normalized());
    m_textEditor->setGeometry(widgetRect.toAlignedRect().adjusted(0, 0, 1, 1));
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

    if (m_editingTextAnnotationId.has_value()) {
        if (Annotation *annotation = annotationById(*m_editingTextAnnotationId)) {
            pushHistorySnapshot();
            annotation->text = text;
            annotation->rect = QRectF(widgetToImage(editorGeometry.topLeft()),
                                      widgetToImage(editorGeometry.bottomRight())).normalized();
            annotation->fontFamily = m_textEditor->font().family();
            if (!annotation->points.isEmpty()) {
                annotation->points[0] = annotation->rect.topLeft();
            }
        }
        m_editingTextAnnotationId.reset();
        m_committingText = false;
        updateAnnotationPropertyPanel();
        update();
        return;
    }

    if (!text.isEmpty()) {
        pushHistorySnapshot();
        Annotation annotation;
        annotation.id = m_nextAnnotationId++;
        annotation.tool = Tool::Text;
        annotation.points.append(m_textEditorImagePoint);
        annotation.rect = QRectF(widgetToImage(editorGeometry.topLeft()),
                                 widgetToImage(editorGeometry.bottomRight())).normalized();
        annotation.text = text;
        annotation.color = m_currentColor;
        annotation.width = m_shapeWidth;
        annotation.fontFamily = m_textEditor->font().family();
        m_textFontFamily = annotation.fontFamily;
        m_annotations.append(annotation);
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
