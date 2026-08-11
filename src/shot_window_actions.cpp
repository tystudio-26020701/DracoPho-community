#include "shot_window_module.h"

#include "annotation_launch.h"
#include "history/capture_history.h"
#include "notifications/app_notifications.h"

#include "app_config_store.h"
#include "export_image_effect.h"
#include "providers/ocr/ocr_provider_factory.h"
#include "providers/provider_task.h"
#include "save_path_config.h"

namespace cfg = markshot::config;
namespace shortcuts = markshot::shortcut;
using namespace markshot::shot;

std::optional<int> ShotWindow::windowIndexAtImagePoint(const QPointF &imagePoint) const
{
    std::optional<int> best;
    const QPoint imgPt = imagePoint.toPoint();
    bool useZOrder = false;
    for (int i = 0; i < m_windowInfos.size(); ++i) {
        const markshot::WindowInfo &info = m_windowInfos.at(i);
        if (!info.rect.contains(imgPt)) {
            continue;
        }
        if (info.zOrder.has_value()) {
            useZOrder = true;
        }
        if (!best.has_value()) {
            best = i;
            continue;
        }
        if (useZOrder) {
            const int infoZ = info.zOrder.value_or(-1);
            const int bestZ = m_windowInfos.at(*best).zOrder.value_or(-1);
            if (infoZ > bestZ) {
                best = i;
            }
        } else {
            const qint64 area = static_cast<qint64>(info.rect.width()) * info.rect.height();
            const qint64 bestArea =
                static_cast<qint64>(m_windowInfos.at(*best).rect.width()) * m_windowInfos.at(*best).rect.height();
            if (area < bestArea) {
                best = i;
            }
        }
    }
    return best;
}

void ShotWindow::performWindowCapture(const markshot::WindowInfo &window)
{
    commitTextEditor();

    QImage captured;
    QString error;
    // 窗口对象抓取：X11 从合成命名 pixmap 读取（遮挡/最小化也真实），
    // Windows 走 PrintWindow(PW_RENDERFULLCONTENT)，KWin Wayland 由 KWin
    // 直接渲染窗口缓冲；这些都不弹起窗口、不抢焦点。无对象路径或失败时
    // 回退为从冻结帧裁剪窗口矩形（Wayland 无合成器接口时的可见内容）。
    captured = captureWindowObjectContent(window, false, &error);
    if (captured.isNull()) {
        const QRect sourceBounds(QPoint(0, 0), m_frozenFrame.size());
        const QRect rect = window.rect.intersected(sourceBounds);
        if (!rect.isEmpty()) {
            captured = m_frozenFrame.copy(rect);
        }
    }
    if (captured.isNull()) {
        if (error.isEmpty()) {
            error = MS_TR("Window capture failed");
        }
        showToast(error);
        return;
    }

    QString name = window.title;
    if (name.isEmpty()) {
        name = m_outputName;
    }
    if (name.isEmpty()) {
        name = QStringLiteral("window");
    }
    ShotWindow *newWindow = markshot::openImageForAnnotation(captured, name);
    if (newWindow) {
        newWindow->setDefaultTools(m_defaultTool, m_fullscreenDefaultTool);
    }
    close();
}

void ShotWindow::preselectWindowCaptureTool()
{
    if (m_mode == Mode::Selecting) {
        setStartupTool(StartupTool::WindowCapture);
    }
}

bool ShotWindow::windowCaptureToolActive() const
{
    return m_startupTool == StartupTool::WindowCapture;
}

void ShotWindow::runExtensionCommand(const ExtensionCommand &command)
{
    commitTextEditor();
    if (m_extensionPanel) {
        m_extensionPanel->hide();
    }
    if (m_openWithPanel) {
        m_openWithPanel->hide();
    }

    QString commandLine = command.command;
    if (commandLine.contains(QStringLiteral("{slurp}"))) {
        const QString geometry = slurpSelectionGeometry();
        if (geometry.isEmpty()) {
            return;
        }
        replaceExtensionSlurpPlaceholder(&commandLine, geometry);
    }

    bool replacedImagePlaceholder = false;
    QString imagePath;
    if (command.saveImage) {
        imagePath = saveSelectionToTempFile(true);
        if (imagePath.isEmpty()) {
            return;
        }
        replacedImagePlaceholder = replaceExtensionImagePlaceholders(&commandLine, imagePath);
        if (!replacedImagePlaceholder) {
            commandLine += QLatin1Char(' ');
            commandLine += shellQuote(imagePath);
        }
    }

    if (commandLine.trimmed().isEmpty()) {
        return;
    }

    const QString workingDirectory = command.workingDirectory.isEmpty()
        ? QString()
        : expandUserPath(command.workingDirectory);

    if (command.closeOnStart) {
        hide();
        QApplication::processEvents();
    }

    const bool started = QProcess::startDetached(markshot::commandShellProgram(),
                                                 markshot::commandShellArguments(commandLine),
                                                 workingDirectory);
    if (started && command.closeOnStart) {
        close();
        return;
    }

    if (!started && command.closeOnStart) {
        show();
        raise();
        activateWindow();
        updateToolbarGeometry();
        updateActionToolbarGeometry();
        updateExtensionPanelGeometry();
    }
}

void ShotWindow::startScrollCapture()
{
    commitTextEditor();
    if (!hasUsableSelection()) {
        return;
    }

    const QRect geometry = selectionGlobalRect();
    if (geometry.isEmpty()) {
        return;
    }

    if (isGnomeWaylandSession() && !hasGnomeScrollHelper()) {
        QMessageBox::information(
            this,
            MS_TR("Scroll Capture"),
            MS_TR("Scroll capture is not supported on GNOME Wayland."));
        return;
    }

    const QString outputName = m_outputName;
    const markshot::scroll::ScrollSessionUiConfig uiConfig = scrollSessionUiConfig();
    QScreen *targetScreen = screen();
    QPointer<ShotWindow> self(this);

    auto launchScrollWindow = [self, geometry, outputName, targetScreen, uiConfig] {
        auto *window =
            new markshot::scroll::ScrollSessionWindow(geometry, outputName, targetScreen, uiConfig);
        window->show();
        window->raise();
        window->activateWindow();
        if (self) {
            self->close();
        }
    };

#if defined(Q_OS_WIN)
    // WGC honors WDA_EXCLUDEFROMCAPTURE on both windows, so switch overlays
    // directly instead of blanking the desktop for the X11 compositor delay.
    launchScrollWindow();
#else
    // On X11, QScreen::grabWindow captures visible top-level windows. Hide the
    // selection UI and give the compositor one repaint before seeding the scroll
    // stitcher, otherwise the first frame can contain our own toolbar/overlay.
    hide();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    QTimer::singleShot(120, qApp, std::move(launchScrollWindow));
#endif
}

void ShotWindow::pinSelection()
{
    commitTextEditor();
    if (!hasUsableSelection()) {
        return;
    }

    QImage output = renderedSelection();
    if (output.isNull()) {
        return;
    }
    markshot::capture_history::recordCapture(output, m_outputName);
    const QRect logicalSelection = selectionGlobalRect();
    if (!logicalSelection.isEmpty()) {
        const qreal dprX = static_cast<qreal>(output.width()) / std::max(1, logicalSelection.width());
        const qreal dprY = static_cast<qreal>(output.height()) / std::max(1, logicalSelection.height());
        const qreal dpr = std::max<qreal>(1.0, (dprX + dprY) / 2.0);
        output.setDevicePixelRatio(dpr);
    }

    const std::optional<QPoint> pinnedTopLeft = logicalSelection.isEmpty()
        ? std::nullopt
        : std::optional<QPoint>(logicalSelection.topLeft());
    auto *window = createPinnedImageWindow(output, pinnedTopLeft);
    window->show();
    window->raise();
    window->activateWindow();
    if (!m_standaloneEditor) {
        close();
    }
}

void ShotWindow::ocrCopySelection()
{
    commitTextEditor();
    if (!hasUsableSelection()) {
        return;
    }

    const QString tempPath = saveSelectionToTempFile();
    if (tempPath.isEmpty()) {
        return;
    }

    const PinnedWindowConfig config = pinnedWindowConfig();
    if (!config.ocrEnabled) {
        QFile::remove(tempPath);
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // 1. 组装 OCR 请求，provider 优先链由工厂解析
    markshot::providers::OcrTaskRequest request;
    request.imagePath = tempPath;
    request.backend = config.ocrBackend;
    request.provider = config.ocrProvider;
    if (!config.ocrCommand.isEmpty()) {
        QString commandLine = config.ocrCommand;
        const bool replaced = replaceExtensionImagePlaceholders(&commandLine, tempPath);
        if (!replaced) {
            commandLine += QLatin1Char(' ');
            commandLine += shellQuote(tempPath);
        }
        request.commandLine = commandLine;
    } else {
        request.helperProgram = helperProgramPath(QStringLiteral("dracoPho-ocr"));
    }

    // 2. 同步等待任务结果，行为与旧版阻塞式调用一致
    markshot::providers::ProviderTask *task = markshot::providers::createOcrTask(request, this);
    task->start(config.ocrTimeoutMs);
    const markshot::providers::TaskResult taskResult = task->waitForResult();
    task->deleteLater();

    QFile::remove(tempPath);
    QApplication::restoreOverrideCursor();

    if (taskResult.error == markshot::providers::TaskError::StartFailed) {
        showToast(config.ocrCommand.isEmpty()
                      ? MS_TR("OCR helper not found")
                      : MS_TR("OCR failed"));
        return;
    }
    if (taskResult.error == markshot::providers::TaskError::Timeout) {
        showToast(MS_TR("OCR timed out"));
        return;
    }

    const QByteArray output = taskResult.output;
    const QByteArray errorOutput = taskResult.errorOutput;
    if (!taskResult.ok) {
        showToast(config.ocrCommand.isEmpty()
                      && ocrOutputReportsMissingBackend(output, errorOutput, config.ocrBackend)
                      ? MS_TR("OCR backend not installed. Install rapidocr or tesseract.")
                      : MS_TR("OCR failed"));
        return;
    }

    const markshot::ocr::ParsedOutput parsedOcr = markshot::ocr::parseOutput(output);
    if (!parsedOcr.validJson) {
        showToast(config.ocrCommand.isEmpty()
                      && ocrOutputReportsMissingBackend(output, errorOutput, config.ocrBackend)
                      ? MS_TR("OCR backend not installed. Install rapidocr or tesseract.")
                      : MS_TR("OCR failed"));
        return;
    }

    if (parsedOcr.tokens.isEmpty()) {
        showToast(config.ocrCommand.isEmpty()
                      && ocrOutputReportsMissingBackend(output, errorOutput, config.ocrBackend)
                      ? MS_TR("OCR backend not installed. Install rapidocr or tesseract.")
                      : MS_TR("No text recognized"));
        return;
    }

    const QString result = markshot::ocr::tokensText(parsedOcr.tokens);

    if (ocrResultPanelEnabled()) {
        auto *window = createOcrResultWindow(result);
        window->show();
        window->raise();
        window->activateWindow();
        if (!m_standaloneEditor) {
            close();
        }
        return;
    }

    markshot::copyTextToClipboard(result);
    if (!sendDesktopNotification(QStringLiteral("DracoPho"), MS_TR("OCR text copied"), 2500)) {
        showToast(MS_TR("OCR text copied"));
    }
    if (!m_standaloneEditor) {
        QTimer::singleShot(150, this, [this] { close(); });
    }
}

void ShotWindow::showToast(const QString &text, int durationMs)
{
    auto *label = new QLabel(text, this);
    label->setAlignment(Qt::AlignCenter);
    label->setFont(markshot::theme::uiFont(12, QFont::DemiBold));
    label->setStyleSheet(QStringLiteral(
        "background: rgba(8, 13, 19, 220);"
        "color: rgba(204, 251, 241, 238);"
        "border-radius: 14px;"
        "padding: 8px 22px;"));
    label->adjustSize();
    label->move((width() - label->width()) / 2, height() - label->height() - 80);
    label->show();
    QTimer::singleShot(durationMs, label, &QObject::deleteLater);
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

QImage ShotWindow::exportSelectionImage() const
{
    const QImage output = renderedSelection();
    if (output.isNull()) {
        return {};
    }

    bool ok = false;
    const QJsonObject root = markshot::readAppConfigRoot(&ok);
    if (!ok) {
        return output;
    }
    return markshot::applyExportImageEffect(output,
                                            markshot::exportImageEffectConfigFromRoot(root));
}

QString ShotWindow::defaultSavePath() const
{
    const QRect sourceBounds(QPoint(0, 0), m_frozenFrame.size());
    markshot::SavePathContext context;
    context.timestamp = QDateTime::currentDateTime();
    context.selectionRect = normalizedSelection().toAlignedRect().intersected(sourceBounds);
    context.sourceGeometry = m_sourceGeometry;
    context.imageSize = m_frozenFrame.size();
    context.outputName = m_outputName;
    context.extension = QStringLiteral("png");

    bool ok = false;
    const QJsonObject root = markshot::readAppConfigRoot(&ok);
    return ok ? markshot::savePathFromConfigRoot(root, context) : markshot::defaultSavePath(context);
}

void ShotWindow::saveSelection()
{
    commitTextEditor();

    if (!hasUsableSelection()) {
        return;
    }

    const QImage output = exportSelectionImage();
    if (output.isNull()) {
        return;
    }

    const QString path = defaultSavePath();
    if (markshot::ensureSavePathDirectory(path) && output.save(path, "PNG")) {
        markshot::capture_history::recordCapture(output, m_outputName);
        const QString message = MS_TR("Saved to %1").arg(path);
        // Keyboard save should finish without another dialog round-trip.
        if (!markshot::notifications::notifyScreenshotSaved(path)) {
            showToast(message, 2500);
        }
        // 独立编辑器模式：保存后保持窗口打开，继续编辑。
        if (!m_standaloneEditor) {
            QTimer::singleShot(150, this, [this] { close(); });
        }
        return;
    }

    showToast(MS_TR("Save failed"), 2500);
}

void ShotWindow::saveSelectionAs()
{
    commitTextEditor();

    if (!hasUsableSelection()) {
        return;
    }

    const QImage output = exportSelectionImage();
    if (output.isNull()) {
        return;
    }

    if (m_openWithPanel) {
        m_openWithPanel->hide();
    }
    if (m_extensionPanel) {
        m_extensionPanel->hide();
    }
    if (m_colorPalette) {
        m_colorPalette->hide();
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

    hide();

    auto *dialog = new QFileDialog(nullptr, MS_TR("Save Screenshot"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setFileMode(QFileDialog::AnyFile);
    dialog->setNameFilter(MS_TR("PNG Images (*.png)"));
    dialog->setDefaultSuffix(QStringLiteral("png"));
    dialog->setOption(QFileDialog::DontUseNativeDialog, true);
    const QString initialPath = defaultSavePath();
    markshot::ensureSavePathDirectory(initialPath);
    dialog->selectFile(initialPath);

    connect(dialog, &QFileDialog::accepted, this, [this, dialog, output] {
        const QStringList files = dialog->selectedFiles();
        if (!files.isEmpty()
            && markshot::ensureSavePathDirectory(files.first())
            && output.save(files.first(), "PNG")) {
            markshot::capture_history::recordCapture(output, m_outputName);
            const QString message = MS_TR("Saved to %1").arg(files.first());
            // Prefer desktop notifications because the window may close immediately after saving.
            if (!markshot::notifications::notifyScreenshotSaved(files.first())) {
                showToast(message, 2500);
            }
            // 独立编辑器模式：另存为后保持窗口打开，继续编辑。
            if (!m_standaloneEditor) {
                QTimer::singleShot(150, this, [this] { close(); });
            }
            return;
        }
        showToast(MS_TR("Save failed"), 2500);
        show();
        raise();
        activateWindow();
        updateToolbarGeometry();
        updateActionToolbarGeometry();
    });
    connect(dialog, &QFileDialog::rejected, this, [this] {
        show();
        raise();
        activateWindow();
        updateToolbarGeometry();
        updateActionToolbarGeometry();
    });
    dialog->open();
}

void ShotWindow::copySelection()
{
    commitTextEditor();

    if (!hasUsableSelection()) {
        return;
    }

    QImage output = exportSelectionImage();
    if (output.isNull()) {
        return;
    }

    markshot::capture_history::recordCapture(output, m_outputName);
    markshot::copyImageToClipboard(output);

    if (!m_standaloneEditor) {
        close();
    }
}
