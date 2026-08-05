#include "annotation_launch.h"
#include "capture_cursor_policy.h"
#include "capture_delay_config.h"
#include "capture_freeze_scope.h"
#include "capture_own_windows_policy.h"
#include "capture_session_launcher.h"
#include "capture_session_screen_utils.h"
#include "cli/headless_capture.h"
#include "cli/image_pin_launch.h"
#include "cli/recording_cli.h"
#include "cli/window_capture_cli.h"
#include "debug_log.h"
#include "delayed_capture.h"
#include "floating_ball.h"
#include "ipc/single_instance_ipc.h"
#include "recording/recording_dialog_config.h"
#include "recording/recording_display_source.h"
#include "recording/recording_file_naming.h"
#include "recording/recording_session_manager.h"
#include "settings/settings_dialog.h"
#include "shot_window.h"
#include "startup_behavior_config.h"
#include "startup_config.h"
#include "ui/icons.h"
#include "ui/i18n.h"
#include "ui/theme.h"
#include "window_detection.h"
#include "windows_integration.h"
#include "windows_tray_controller.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QEventLoop>
#include <QFileInfo>
#include <QFont>
#include <QGuiApplication>
#include <QImageReader>
#include <QLocalServer>
#include <QMessageBox>
#include <QPointer>
#include <QScreen>
#include <QStringList>
#include <QTextStream>
#include <QThread>
#include <QTimer>
#include <QVector>

#include <algorithm>
#include <memory>
#include <optional>
#include <utility>

namespace {

/**
 * 创建单实例 IPC 服务。
 * @param error 输出错误信息。
 * @return IPC 服务实例。
 */
std::unique_ptr<QLocalServer> createSingleInstanceServer(QString *error)
{
    return markshot::ipc::listenForSingleInstanceCommands(error);
}

} // namespace

/// @brief Main entry point of the application.
/// @param argc The count of command line arguments.
/// @param argv The array of command line arguments.
/// @return Exit code of the application.
int main(int argc, char *argv[])
{
    markshot::applyConfiguredEnvironment();

    QGuiApplication::setDesktopFileName(QStringLiteral("mark-shot"));
    markshot::disableQtPortalServicesForHostApp();

    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("mark-shot"));
    QApplication::setApplicationDisplayName(QStringLiteral("DracoPho"));
    QApplication::setApplicationVersion(QStringLiteral(MARK_SHOT_VERSION));
    QApplication::setWindowIcon(markshot::ui::applicationIcon());
    QFont applicationFont = app.font();
    applicationFont.setFamily(markshot::theme::uiFontFamily());
    app.setFont(applicationFont);

    markshot::i18n::initialize();

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Screenshot selection and annotation tool."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QStringLiteral("file"), QStringLiteral("Open an existing image file for annotation instead of capturing the screen."), QStringLiteral("[file]"));
    QCommandLineOption editorOption(QStringLiteral("editor"),
                                    QStringLiteral("Open the DracoPho image editor with an empty canvas. Use Open (Ctrl+O) or drag & drop to import an image for annotation editing."));
    QCommandLineOption allOutputsOption({QStringLiteral("all-outputs"), QStringLiteral("all-output")},
                                        QStringLiteral("Capture all outputs instead of the current Qt screen."));
    QCommandLineOption xdgWindowOption(QStringLiteral("xdg-window"), QStringLiteral("Use a regular fullscreen xdg window instead of layer-shell."));
    QCommandLineOption fullscreenAnnotationOption({QStringLiteral("fullscreen"), QStringLiteral("full-screen")},
                                                  QStringLiteral("Skip region selection and annotate the full captured frame."));
    QCommandLineOption trayOption(QStringLiteral("tray"),
                                  QStringLiteral("Keep running in the system tray and register global hotkeys when supported."));
    QCommandLineOption captureOption(QStringLiteral("capture"),
                                     QStringLiteral("Capture once even when tray autostart is enabled."));
    QCommandLineOption pinImageOption(QStringLiteral("pin-image"),
                                      QStringLiteral("Open an image file directly as a pinned sticker."),
                                      QStringLiteral("path"));
    QCommandLineOption recordingStatusOption(QStringLiteral("recording-status"),
                                             QStringLiteral("Print the current recording status as JSON."));
    QCommandLineOption stopRecordingOption(QStringLiteral("stop-recording"),
                                           QStringLiteral("Stop the active recording through the running instance."));
    QCommandLineOption recordRegionOption(QStringLiteral("record-region"),
                                          QStringLiteral("Record a screen region for the given duration (see --record-duration). Geometry as x,y,width,height."),
                                          QStringLiteral("geometry"));
    QCommandLineOption recordDisplayOption(QStringLiteral("record-display"),
                                           QStringLiteral("Record a display by id (see --list-displays output: a raw screen name such as \"DP-1\", the \"output:DP-1\" key, or \"all\")."),
                                           QStringLiteral("id"));
    QCommandLineOption recordDurationOption(QStringLiteral("record-duration"),
                                            QStringLiteral("Recording duration in seconds; 0 records until --stop-recording."),
                                            QStringLiteral("seconds"));
    QCommandLineOption recordOutputOption(QStringLiteral("record-output"),
                                          QStringLiteral("Output file path for the recording."),
                                          QStringLiteral("path"));
    QCommandLineOption recordFpsOption(QStringLiteral("record-fps"),
                                       QStringLiteral("Frame rate for the recording (default 15)."),
                                       QStringLiteral("fps"));
    QCommandLineOption recordFormatOption(QStringLiteral("record-format"),
                                          QStringLiteral("Recording format: mp4 (default), gif or webp."),
                                          QStringLiteral("format"));
    QCommandLineOption recordAudioOption(QStringLiteral("record-audio"),
                                         QStringLiteral("Include system audio in the recording."));
    QCommandLineOption recordWaitOption(QStringLiteral("record-wait-json"),
                                        QStringLiteral("Wait for the recording to finish, then print the final status as JSON."));
    QCommandLineOption defaultToolOption(QStringLiteral("default-tool"),
                                         QStringLiteral("Set the default annotation tool after a selected region. Also seeds fullscreen mode unless overridden. Supported: %1.")
                                             .arg(ShotWindow::supportedToolNames().join(QStringLiteral(", "))),
                                         QStringLiteral("tool"));
    QCommandLineOption fullscreenDefaultToolOption(QStringLiteral("fullscreen-default-tool"),
                                                   QStringLiteral("Set the default annotation tool for fullscreen annotation mode. Supported: %1.")
                                                       .arg(ShotWindow::supportedToolNames().join(QStringLiteral(", "))),
                                                   QStringLiteral("tool"));
    QCommandLineOption fileDefaultToolOption(QStringLiteral("file-default-tool"),
                                             QStringLiteral("Set the default annotation tool when opening an existing image file. Supported: %1.")
                                                 .arg(ShotWindow::supportedToolNames().join(QStringLiteral(", "))),
                                             QStringLiteral("tool"));
    QCommandLineOption defaultColorOption(QStringLiteral("default-color"),
                                          QStringLiteral("Set the default annotation color. Supported formats: #RRGGBB or #RRGGBBAA."),
                                          QStringLiteral("color"));
    QCommandLineOption debugOption(QStringLiteral("debug"),
                                   QStringLiteral("Enable debug logging."));
    QCommandLineOption noDebugOption(QStringLiteral("no-debug"),
                                     QStringLiteral("Disable debug logging even if config or environment enables it."));
    QCommandLineOption debugLogOption(QStringLiteral("debug-log"),
                                      QStringLiteral("Write debug logs to the specified file path."),
                                      QStringLiteral("path"));
    QCommandLineOption delayOption(QStringLiteral("delay"),
                                   QStringLiteral("Wait the given number of seconds before capturing (countdown overlay, Esc to cancel)."),
                                   QStringLiteral("seconds"));
    QCommandLineOption windowCaptureOption(QStringLiteral("capture-window"),
                                           QStringLiteral("Start an interactive window capture: hover to highlight a window, click to capture it (X11 reads occluded/minimized window content; elsewhere the window's screen region is captured)."));
    parser.addOption(allOutputsOption);
    parser.addOption(xdgWindowOption);
    parser.addOption(fullscreenAnnotationOption);
    parser.addOption(editorOption);
    parser.addOption(trayOption);
    parser.addOption(captureOption);
    parser.addOption(pinImageOption);
    parser.addOption(recordingStatusOption);
    parser.addOption(stopRecordingOption);
    parser.addOption(recordRegionOption);
    parser.addOption(recordDisplayOption);
    parser.addOption(recordDurationOption);
    parser.addOption(recordOutputOption);
    parser.addOption(recordFpsOption);
    parser.addOption(recordFormatOption);
    parser.addOption(recordAudioOption);
    parser.addOption(recordWaitOption);
    parser.addOption(defaultToolOption);
    parser.addOption(fullscreenDefaultToolOption);
    parser.addOption(fileDefaultToolOption);
    parser.addOption(defaultColorOption);
    parser.addOption(debugOption);
    parser.addOption(noDebugOption);
    parser.addOption(debugLogOption);
    parser.addOption(delayOption);
    parser.addOption(windowCaptureOption);
    markshot::cli::addHeadlessCaptureOptions(&parser);
    markshot::cli::addWindowCaptureOptions(&parser);
    parser.process(app);

    if (parser.isSet(stopRecordingOption)) {
        return markshot::cli::stopRecordingFromCommandLine();
    }
    if (parser.isSet(recordingStatusOption)) {
        return markshot::cli::printRecordingStatus();
    }

    // 无人值守录制：由运行实例执行，返回 JSON 状态。
    if (parser.isSet(recordRegionOption) || parser.isSet(recordDisplayOption)) {
        markshot::cli::CliRecordingRequest request;
        request.geometryText = parser.value(recordRegionOption).trimmed();
        request.displayKey = parser.value(recordDisplayOption).trimmed();
        request.outputPath = parser.value(recordOutputOption).trimmed();
        request.format = parser.value(recordFormatOption).trimmed();
        if (!parser.isSet(recordFpsOption)) {
            request.fps = 15;
        } else {
            request.fps = std::clamp(parser.value(recordFpsOption).toInt(), 1, 120);
        }
        request.includeAudio = parser.isSet(recordAudioOption);
        // 用 qint64 解析并夹紧到 24h 上限，避免 int 溢出进 QTimer 区间。
        const qint64 durationSeconds =
            std::clamp<qint64>(parser.value(recordDurationOption).toLongLong(), 0, 24 * 3600);
        request.durationMs = static_cast<int>(std::min<qint64>(durationSeconds * 1000, INT_MAX));
        request.waitForFinish = parser.isSet(recordWaitOption);

        if (request.outputPath.isEmpty()) {
            QTextStream errorStream(stderr);
            errorStream << "mark-shot: --record-output is required for recording\n";
            return 1;
        }
        if (request.displayKey.isEmpty() && request.geometryText.isEmpty()) {
            QTextStream errorStream(stderr);
            errorStream << "mark-shot: one of --record-display or --record-region is required for recording\n";
            return 1;
        }
        // 录制格式白名单校验：非法值立即报错，绝不静默降级为 mp4。
        if (!request.format.isEmpty()
            && request.format != QLatin1String("mp4")
            && request.format != QLatin1String("gif")
            && request.format != QLatin1String("webp")) {
            QTextStream errorStream(stderr);
            errorStream << "mark-shot: unsupported --record-format \"" << request.format
                        << "\" (expected mp4, gif or webp)\n";
            return 1;
        }
        // --record-wait-json 等待录制自行结束；无限录制（duration=0）只能靠
        // --stop-recording 结束，二者组合必然超时，直接拒绝并说明。
        if (request.waitForFinish && request.durationMs <= 0) {
            QTextStream errorStream(stderr);
            errorStream << "mark-shot: --record-wait-json requires --record-duration > 0 "
                           "(infinite recording only ends via --stop-recording)\n";
            return 1;
        }
        // --record-region 必须为 4 个整数（x,y,width,height）。
        if (!request.geometryText.isEmpty()) {
            const QStringList parts = request.geometryText.split(QLatin1Char(','), Qt::SkipEmptyParts);
            bool valid = parts.size() == 4;
            if (valid) {
                for (const QString &part : parts) {
                    bool partOk = false;
                    part.trimmed().toInt(&partOk);
                    valid = valid && partOk;
                }
            }
            if (!valid) {
                QTextStream errorStream(stderr);
                errorStream << "mark-shot: --record-region expects x,y,width,height integers\n";
                return 1;
            }
        }
        return markshot::cli::startRecordingFromCommandLine(request);
    }

    // 无头模式必须绝不弹窗、绝不创建任何窗口（包括图片编辑窗口）。在进入
    // 任何可能弹出 QMessageBox 或创建窗口的代码之前，先完成无头分发并退出。
    markshot::ensureAppConfigFile();

    markshot::DebugRuntimeConfig debugConfig = markshot::configuredDebugRuntimeConfig();
    if (parser.isSet(debugOption)) {
        debugConfig.enabled = true;
    }
    if (parser.isSet(noDebugOption)) {
        debugConfig.enabled = false;
    }
    if (parser.isSet(debugLogOption)) {
        const QString optionPath = parser.value(debugLogOption).trimmed();
        if (!optionPath.isEmpty()) {
            debugConfig.logPath = markshot::expandedConfigPath(optionPath);
        }
        if (!parser.isSet(noDebugOption)) {
            debugConfig.enabled = true;
        }
    }
    markshot::configureDebugLogging(debugConfig.enabled, debugConfig.logPath);
    markshot::debugLog("config",
                       "debug enabled path=%s",
                       markshot::debugLogPath().toUtf8().constData());

    // 无头分发：返回 >=0 表示已处理并应立即退出；-1 表示继续交互式启动。
    const int windowExitCode = markshot::cli::runWindowCaptureIfRequested(parser);
    if (windowExitCode >= 0) {
        return windowExitCode;
    }
    const int headlessExitCode = markshot::cli::runHeadlessCaptureIfRequested(parser);
    if (headlessExitCode >= 0) {
        return headlessExitCode;
    }

    // 从这里开始才允许交互式 UI 与对话框。
    const QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.size() > 1) {
        QMessageBox::critical(nullptr, QStringLiteral("DracoPho"), MS_TR("Only one image file can be opened at a time."));
        return 1;
    }

    if (parser.isSet(debugOption) && parser.isSet(noDebugOption)) {
        QMessageBox::critical(nullptr,
                              QStringLiteral("DracoPho"),
                              MS_TR("--debug and --no-debug cannot be used together."));
        return 1;
    }

    // 窗口捕获与全屏标注互斥：全屏标注会立即进入编辑态，窗口捕获工具将失效。
    if (parser.isSet(windowCaptureOption) && parser.isSet(fullscreenAnnotationOption)) {
        QMessageBox::critical(nullptr,
                              QStringLiteral("DracoPho"),
                              MS_TR("--capture-window and --fullscreen cannot be used together."));
        return 1;
    }

    QString configDefaultToolWarning;
    markshot::DefaultTools defaultTools = markshot::configuredDefaultTools(&configDefaultToolWarning);
    auto parseRuntimeTool = [](const QString &optionValue) {
        return ShotWindow::toolFromName(optionValue);
    };
    if (parser.isSet(defaultToolOption)) {
        const QString optionValue = parser.value(defaultToolOption);
        const std::optional<ShotWindow::Tool> parsedTool = parseRuntimeTool(optionValue);
        if (!parsedTool.has_value()) {
            QMessageBox::critical(nullptr,
                                  QStringLiteral("DracoPho"),
                                  MS_TR("Unsupported default tool: %1\nSupported tools: %2")
                                      .arg(optionValue, ShotWindow::supportedToolNames().join(QStringLiteral(", "))));
            return 1;
        }
        defaultTools.normal = *parsedTool;
        defaultTools.fullscreen = *parsedTool;
        defaultTools.file = *parsedTool;
        configDefaultToolWarning.clear();
    }
    if (parser.isSet(fullscreenDefaultToolOption)) {
        const QString optionValue = parser.value(fullscreenDefaultToolOption);
        const std::optional<ShotWindow::Tool> parsedTool = parseRuntimeTool(optionValue);
        if (!parsedTool.has_value()) {
            QMessageBox::critical(nullptr,
                                  QStringLiteral("DracoPho"),
                                  MS_TR("Unsupported fullscreen default tool: %1\nSupported tools: %2")
                                      .arg(optionValue, ShotWindow::supportedToolNames().join(QStringLiteral(", "))));
            return 1;
        }
        defaultTools.fullscreen = *parsedTool;
        defaultTools.file = *parsedTool;
    }
    if (parser.isSet(fileDefaultToolOption)) {
        const QString optionValue = parser.value(fileDefaultToolOption);
        const std::optional<ShotWindow::Tool> parsedTool = parseRuntimeTool(optionValue);
        if (!parsedTool.has_value()) {
            QMessageBox::critical(nullptr,
                                  QStringLiteral("DracoPho"),
                                  MS_TR("Unsupported file default tool: %1\nSupported tools: %2")
                                      .arg(optionValue, ShotWindow::supportedToolNames().join(QStringLiteral(", "))));
            return 1;
        }
        defaultTools.file = *parsedTool;
    }
    if (parser.isSet(defaultColorOption)) {
        const QString optionValue = parser.value(defaultColorOption);
        const std::optional<QColor> parsedColor = markshot::colorFromString(optionValue);
        if (!parsedColor.has_value()) {
            QMessageBox::critical(nullptr,
                                  QStringLiteral("DracoPho"),
                                  MS_TR("Unsupported default color: %1\nSupported color formats: #RRGGBB or #RRGGBBAA")
                                      .arg(optionValue));
            return 1;
        }
        defaultTools.color = *parsedColor;
        defaultTools.colorSource = markshot::DefaultColorSource::CommandLine;
    }
    if (!configDefaultToolWarning.isEmpty()) {
        QMessageBox::warning(nullptr, QStringLiteral("DracoPho"), configDefaultToolWarning);
    }

    const QString imagePath = positionalArguments.isEmpty() ? QString() : positionalArguments.first();
    if (parser.isSet(pinImageOption)) {
        if (!positionalArguments.isEmpty()) {
            QMessageBox::critical(nullptr,
                                  QStringLiteral("DracoPho"),
                                  MS_TR("Only one image file can be opened at a time."));
            return 1;
        }

        QString error;
        QWidget *window = markshot::cli::launchPinnedImageFromPath(parser.value(pinImageOption), &error);
        if (!window) {
            QMessageBox::critical(nullptr, QStringLiteral("DracoPho"), error);
            return 1;
        }
        return QApplication::exec();
    }

    const bool fileMode = !imagePath.isEmpty();
    const bool editorMode = parser.isSet(editorOption);
    if (fileMode || editorMode) {
        QImage image;
        QString fileName;
        if (fileMode) {
            QFileInfo imageFile(imagePath);
            if (!imageFile.exists() || !imageFile.isFile()) {
                QMessageBox::critical(nullptr, QStringLiteral("DracoPho"), MS_TR("Image file does not exist: %1").arg(imagePath));
                return 1;
            }

            QImageReader reader(imageFile.absoluteFilePath());
            reader.setAutoTransform(true);
            image = reader.read();
            if (image.isNull()) {
                QMessageBox::critical(nullptr,
                                      QStringLiteral("DracoPho"),
                                      MS_TR("Failed to load image: %1\n%2").arg(imageFile.absoluteFilePath(), reader.errorString()));
                return 1;
            }
            fileName = imageFile.fileName();
        }

        ShotWindow *window = new ShotWindow(image, fileName);
        window->setDefaultTools(defaultTools.normal, defaultTools.file);
        if (markshot::shouldApplyDefaultColor(defaultTools)) {
            window->setDefaultColor(defaultTools.color);
        }
        window->setStandaloneEditorMode(editorMode);
        QScreen *screen = markshot::focusedScreen();
        if (screen) {
            window->setScreen(screen);
        }
        window->setWindowFlags(Qt::Window);
        if (image.isNull()) {
            // 独立编辑器空画布：使用一个常规大小的编辑窗口。
            const QRect availableGeometry = screen
                ? screen->availableGeometry()
                : QRect(QPoint(0, 0), QSize(1200, 800));
            const QSize editorSize(qMax(480, qRound(availableGeometry.width() * 0.8)),
                                   qMax(360, qRound(availableGeometry.height() * 0.8)));
            // 先设最小尺寸再 resize：resize 会在最小尺寸之上生效。
            window->setMinimumSize(400, 300);
            window->resize(editorSize);
            window->setWindowTitle(MS_TR("DracoPho Image Editor"));
        } else {
            const QRect windowGeometry = markshot::centeredImageWindowGeometry(image.size(), screen);
            if (windowGeometry.isValid() && !windowGeometry.isEmpty()) {
                window->setGeometry(windowGeometry);
            }
            window->setWindowTitle(fileName);
        }
        window->setImageNavigationEnabled(true);
        window->show();
        window->raise();
        window->activateWindow();
        if (!image.isNull()) {
            QTimer::singleShot(0, window, [window] {
                window->startFullscreenAnnotation();
            });
        }
        return QApplication::exec();
    }

    const bool allOutputs = parser.isSet(allOutputsOption);
    const bool useRegularWindow = parser.isSet(xdgWindowOption);
    const bool fullscreenAnnotation = parser.isSet(fullscreenAnnotationOption);
    const bool windowCaptureRequested = parser.isSet(windowCaptureOption);
    const markshot::WindowsTrayController::Config trayConfig = markshot::WindowsTrayController::readConfig();
    const bool explicitCaptureRequest =
        parser.isSet(captureOption) || parser.isSet(allOutputsOption) || parser.isSet(fullscreenAnnotationOption)
        || windowCaptureRequested;

    // 解析启动行为（多选组合）：直接截图 / 托盘图标 / 悬浮球 / 设置窗口。
    // 显式 CLI 请求（--capture/--all-outputs/--fullscreen/--tray）始终优先于配置。
    const markshot::StartupBehaviorConfig startupBehavior = markshot::configuredStartupBehavior();

    bool wantCapture = false;
    bool wantTray = false;
    bool wantFloatingBall = false;
    bool wantSettingsWindow = false;

    if (explicitCaptureRequest) {
        wantCapture = true;
        if (startupBehavior.configured) {
            wantTray = startupBehavior.tray;
            wantFloatingBall = startupBehavior.floatingBall;
        } else {
            wantTray = trayConfig.autoStart;
            wantFloatingBall = false;
        }
        wantSettingsWindow = false;
    } else if (parser.isSet(trayOption)) {
        wantTray = true;
        wantFloatingBall = startupBehavior.configured && startupBehavior.floatingBall;
    } else if (startupBehavior.configured) {
        wantCapture = startupBehavior.directCapture;
        wantTray = startupBehavior.tray;
        wantFloatingBall = startupBehavior.floatingBall;
        wantSettingsWindow = startupBehavior.settingsWindow;
    } else {
        // 旧版配置（无 startup.modes）：直接截图已不再是默认行为。
        // 回退为后台托盘运行，避免点击图标后意外弹出截图界面。
        wantTray = true;
    }

    // 防御：不允许出现"启动后无任何入口"的组合（如手写空 modes 数组）。
    if (!wantCapture && !wantTray && !wantFloatingBall && !wantSettingsWindow) {
        wantTray = true;
    }

    const bool keepAlive = wantTray || wantFloatingBall;
    if (keepAlive) {
        QApplication::setQuitOnLastWindowClosed(false);
    }

    // 延时截图：CLI --delay 显式指定时覆盖配置，否则读取配置中的 capture.delaySeconds。
    // 延时仅在普通截图（区域/全屏）时生效，区域录制不走倒计时遮罩。
    int captureDelaySeconds = markshot::configuredCaptureDelaySeconds();
    if (parser.isSet(delayOption)) {
        bool delayOk = false;
        const int cliDelay = parser.value(delayOption).trimmed().toInt(&delayOk);
        if (!delayOk || cliDelay < 0 || cliDelay > 3600) {
            QMessageBox::critical(nullptr,
                                  QStringLiteral("DracoPho"),
                                  MS_TR("Invalid --delay value (expected 0-3600)."));
            return 1;
        }
        captureDelaySeconds = cliDelay;
    }

    markshot::ipc::SingleInstanceCommand duplicateCommand;
    duplicateCommand.capture = wantCapture;
    duplicateCommand.fullscreen = fullscreenAnnotation;
    duplicateCommand.allOutputs = allOutputs;
    duplicateCommand.delaySeconds = captureDelaySeconds;
    duplicateCommand.windowCapture = windowCaptureRequested;
    if (markshot::ipc::sendSingleInstanceCommand(duplicateCommand, nullptr, nullptr)) {
        return 0;
    }

    QString singleInstanceError;
    std::unique_ptr<QLocalServer> singleInstanceServer = createSingleInstanceServer(&singleInstanceError);
    if (!singleInstanceServer) {
        if (markshot::ipc::sendSingleInstanceCommand(duplicateCommand, nullptr, nullptr)) {
            return 0;
        }
        QLocalServer::removeServer(markshot::ipc::singleInstanceServerName());
        singleInstanceServer = createSingleInstanceServer(&singleInstanceError);
    }
    if (!singleInstanceServer) {
        QMessageBox::critical(nullptr,
                              QStringLiteral("DracoPho"),
                              MS_TR("Failed to start single-instance guard: %1").arg(singleInstanceError));
        return 1;
    }

    bool captureActive = false;
    markshot::FloatingBall *floatingBall = nullptr;
    auto captureNow = [&app,
                       &captureActive,
                       &floatingBall,
                       useRegularWindow](bool startFullscreen,
                                         bool requestAllOutputs,
                                         std::optional<markshot::recording::RecordingOptions> regionRecordingOptions = std::nullopt,
                                         bool windowCaptureMode = false) -> bool {
        if (captureActive) {
            return true;
        }

        const bool hideOwn = markshot::configuredHideOwnWindowsDuringCapture();
        // 截图会话期间隐藏本软件自身 UI（悬浮球、设置窗口、已打开的弹出菜单），
        // 避免它们进入截图画面/遮挡选区。贴图窗口是用户内容，不属于本软件 UI，
        // 因此不隐藏。会话结束后统一恢复；用户主动隐藏的状态不被覆盖。
        //
        // Windows 上抓屏/合成器按 WDA_EXCLUDEFROMCAPTURE 标记排除本软件窗口，
        // 各窗口构造期已默认打标；此处按当前配置把本软件全部顶层窗口的标记
        // 重新校正一次，使 capture.hideOwnWindows=false 诚实生效（关闭后本软件
        // 窗口可被截入画面）。其余平台该调用为空操作，仅应用层 hide 生效。
        if (floatingBall) {
            markshot::windows::setExcludedFromCapture(floatingBall, hideOwn);
        }
        for (QWidget *widget : QApplication::topLevelWidgets()) {
            markshot::windows::setExcludedFromCapture(widget, hideOwn);
        }

        const bool ballHiddenByUser = floatingBall && floatingBall->isHiddenByUser();
        if (hideOwn) {
            if (floatingBall) {
                floatingBall->hide();
            }
            markshot::settings::hideSettingsWindowForCapture();
            // 热键触发截图时托盘菜单/悬浮球菜单可能仍打开：先关闭所有弹出窗口，
            // 避免 QMenu popup（Qt::Popup 顶层窗口）进入截图画面；录制配置对话框
            // 等非 popup 业务顶层窗口也一并隐藏（objectName 白名单）；tooltip
            // （Qt::ToolTip 顶层窗口）同样处理。
            for (QWidget *widget : QApplication::topLevelWidgets()) {
                const Qt::WindowType type = widget->windowType();
                if (widget->isVisible()
                    && (type == Qt::Popup || type == Qt::ToolTip
                        || widget->objectName() == QLatin1String("recordingConfigDialog"))) {
                    widget->hide();
                }
            }
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            // GNOME/wlroots 等无合成器级"排除调用者窗口"接口的平台只能靠隐藏 +
            // 等待合成器重绘：hide() 只是把 unmap 请求交给合成器，立即抓屏可能仍
            // 捕获到尚未消失的自身窗口。等待约两帧再进抓屏。KWin Wayland 走
            // hide-caller-windows（合成层排除）与 Windows 走 WDA_EXCLUDEFROMCAPTURE，
            // 无需此等待；该等待会阻塞主线程约 50ms，故只在确有必要的平台生效。
#if !defined(Q_OS_WIN)
            const bool kdeWayland = markshot::capture_session::isWaylandPlatform()
                && qEnvironmentVariable("XDG_CURRENT_DESKTOP").contains(QStringLiteral("KDE"),
                                                                        Qt::CaseInsensitive);
            if (!kdeWayland
                && (qEnvironmentVariableIsSet("DISPLAY") || qEnvironmentVariableIsSet("WAYLAND_DISPLAY"))) {
                QThread::msleep(50);
            }
#endif
        } else {
            // 开关关闭：本软件自身 UI 保持可见、可入画，仅需避免弹出菜单
            // 等瞬态 popup 干扰（菜单本身不属"本软件持久 UI"）。
            for (QWidget *widget : QApplication::topLevelWidgets()) {
                if (widget->isVisible() && widget->windowType() == Qt::Popup) {
                    widget->hide();
                }
            }
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }

        QString captureError;
        markshot::DefaultTools defaultTools = markshot::configuredDefaultTools(nullptr);
        QVector<QPointer<ShotWindow>> windows =
            markshot::showCaptureSession(&app,
                                         requestAllOutputs,
                                         markshot::configuredCaptureFreezeScope(),
                                         markshot::configuredCaptureIncludeCursor(),
                                         hideOwn,
                                         useRegularWindow,
                                         startFullscreen,
                                         defaultTools,
                                         &captureError,
                                         std::move(regionRecordingOptions),
                                         windowCaptureMode);
        if (windows.isEmpty()) {
            if (floatingBall && !ballHiddenByUser) {
                floatingBall->show();
            }
            markshot::settings::restoreSettingsWindowAfterCapture();
            QMessageBox::critical(nullptr,
                                  QStringLiteral("DracoPho"),
                                  captureError.isEmpty() ? MS_TR("Failed to start capture session.") : captureError);
            return false;
        }

        captureActive = true;
        return true;
    };

    // 倒计时进行中标记：延时截图期间置位，避免托盘/悬浮球/热键/IPC 在
    // 倒计时未结束前再次触发第二个倒计时遮罩（叠加遮罩且可能进入截图画面）。
    bool delayedCaptureActive = false;
    // 启动一次倒计时，统一管理进行中标记；取消回调同样清除标记。
    auto startCountdown = [&delayedCaptureActive](int seconds,
                                                  std::function<void()> onComplete) {
        if (delayedCaptureActive) {
            return;
        }
        delayedCaptureActive = true;
        markshot::runDelayedCapture(
            seconds,
            [&delayedCaptureActive, onComplete] {
                delayedCaptureActive = false;
                onComplete();
            },
            [&delayedCaptureActive] {
                delayedCaptureActive = false;
            });
    };

    // 延时截图入口：配置或 --delay 指定秒数大于 0 时，先显示倒计时遮罩，
    // 倒计时结束（或取消）后再进入真实截图流程。区域录制不走延时。
    // delayOverride 由 IPC 命令携带（运行实例处理外部 --delay 请求时使用）。
    auto launchCapture = [&captureNow,
                          &captureActive,
                          &captureDelaySeconds,
                          &startCountdown](bool startFullscreen,
                                           bool requestAllOutputs,
                                           std::optional<markshot::recording::RecordingOptions> regionRecordingOptions = std::nullopt,
                                           std::optional<int> delayOverride = std::nullopt,
                                           bool windowCaptureMode = false) -> bool {
        if (captureActive) {
            return true;
        }
        // 窗口捕获模式不走倒计时遮罩，直接进入窗口选择覆盖层。
        if (windowCaptureMode) {
            return captureNow(startFullscreen, requestAllOutputs, std::move(regionRecordingOptions), true);
        }
        const int effectiveDelay = delayOverride.value_or(captureDelaySeconds);
        if (effectiveDelay > 0 && !regionRecordingOptions.has_value()) {
            startCountdown(effectiveDelay,
                           [captureNow, startFullscreen, requestAllOutputs] {
                               captureNow(startFullscreen, requestAllOutputs);
                           });
            return true;
        }
        return captureNow(startFullscreen, requestAllOutputs, std::move(regionRecordingOptions));
    };

    // 延时截图（托盘/悬浮球子菜单）：以菜单选择的秒数显式触发，覆盖配置默认值。
    auto launchTimedCapture = [&app, &captureActive, &startCountdown, captureNow](int seconds) {
        if (captureActive) {
            return;
        }
        startCountdown(seconds, [&app, captureNow] {
            QTimer::singleShot(0, &app, [captureNow] {
                captureNow(false, false);
            });
        });
    };

    // 会话级恢复：整场截图会话（含"显示器快速截取"编辑目标时替换出的新窗口）
    // 的最后一个截图覆盖窗口销毁时，统一恢复被临时隐藏的本软件 UI。连接一次、
    // 跨多次截图复用。若按"最初那批窗口的销毁数"判断，会话内替换窗口会提前
    // 恢复悬浮球/设置窗口，把它们重新压回仍全屏打开的截图覆盖层之上。
    QObject::connect(&markshot::CaptureSessionMonitor::instance(),
                     &markshot::CaptureSessionMonitor::sessionEnded,
                     &app,
                     [&captureActive, &floatingBall] {
                         captureActive = false;
                         // 用户主动隐藏的悬浮球不被截图会话重新唤起。
                         if (floatingBall && !floatingBall->isHiddenByUser()) {
                             floatingBall->show();
                         }
                         markshot::settings::restoreSettingsWindowAfterCapture();
                     });

    auto &recordingManager = markshot::recording::RecordingSessionManager::instance();
    // 限时录制的自动停止定时器：任何录制结束/停止时都必须取消，
    // 否则上一个录制遗留的定时器会误停新开启的录制。
    QPointer<QTimer> recordingAutoStopTimer;
    QObject::connect(&recordingManager,
                     &markshot::recording::RecordingSessionManager::recordingFinished,
                     &app,
                     [&recordingAutoStopTimer](bool, const QString &, const QString &) {
                         if (recordingAutoStopTimer) {
                             recordingAutoStopTimer->stop();
                             recordingAutoStopTimer->deleteLater();
                             recordingAutoStopTimer = nullptr;
                         }
                     });
    markshot::ipc::installSingleInstanceCommandHandler(
        singleInstanceServer.get(),
        &app,
        [&app, launchCapture, &recordingManager, &recordingAutoStopTimer](const markshot::ipc::SingleInstanceCommand &command) {
            markshot::ipc::SingleInstanceResponse response;
            response.handled = true;

            if (command.stopRecording) {
                QString error;
                response.stopped = recordingManager.stop(&error);
                response.message = response.stopped
                    ? QStringLiteral("stop requested")
                    : (error.isEmpty() ? QStringLiteral("no active recording") : error);
                response.recording = recordingManager.status();
                return response;
            }

            if (command.recordingStatus) {
                response.recording = recordingManager.status();
                response.message = response.recording.active
                    ? QStringLiteral("recording active")
                    : QStringLiteral("recording inactive");
                return response;
            }

            if (command.startRecording) {
                markshot::recording::RecordingOptions options;
                // 无人值守录制（CLI/MCP 经 IPC 触发）：静默执行——不弹桌面通知、
                // 不发起交互式 portal 授权，全程对用户无感、不抢焦点。
                options.silent = true;
                if (command.recordFormat == QStringLiteral("gif")) {
                    options.mode = markshot::recording::RecordingMode::Gif;
                } else if (command.recordFormat == QStringLiteral("webp")) {
                    options.mode = markshot::recording::RecordingMode::Webp;
                } else if (command.recordFormat == QStringLiteral("mp4")
                           || command.recordFormat.isEmpty()) {
                    options.mode = markshot::recording::RecordingMode::Video;
                } else {
                    response.message = QStringLiteral("unsupported recording format: %1 (expected mp4, gif or webp)")
                                           .arg(command.recordFormat);
                    response.recording = recordingManager.status();
                    return response;
                }
                options.scope = markshot::recording::RecordingScope::Region;
                options.fps = std::clamp(command.recordFps, 1, 120);
                options.includeAudio = command.recordIncludeAudio;
                options.outputPath = markshot::recording::normalizedRecordingPath(
                    command.recordOutputPath, options.mode);

                const QVector<markshot::recording::DisplaySource> sources =
                    markshot::recording::availableDisplaySources();
                if (!command.recordDisplayKey.trimmed().isEmpty()) {
                    // 归一化显示器标识（裸屏名 "DP-1"、"screen:DP-1"、"output:DP-1"、
                    // "all" 均可），使 CLI 帮助文本承诺的用法与 MCP 路径一致。
                    const QString key = markshot::recording::normalizeRecordingDisplayId(
                        command.recordDisplayKey);
                    int matched = -1;
                    for (int i = 0; i < sources.size(); ++i) {
                        if (markshot::recording::recordingDisplayPersistenceKey(sources.at(i)) == key) {
                            matched = i;
                            break;
                        }
                    }
                    if (matched < 0) {
                        response.message = QStringLiteral("display id not found: %1").arg(key);
                        response.recording = recordingManager.status();
                        return response;
                    }
                    options.display = sources.at(matched);
                    options.captureGeometry = options.display.geometry;
                } else {
                    const QStringList parts = command.recordGeometryText.split(QLatin1Char(','), Qt::SkipEmptyParts);
                    if (parts.size() == 4) {
                        bool allOk = true;
                        for (const QString &part : parts) {
                            bool partOk = false;
                            part.trimmed().toInt(&partOk);
                            allOk = allOk && partOk;
                        }
                        if (!allOk) {
                            response.message = QStringLiteral("recording geometry is invalid: expected x,y,width,height integers");
                            response.recording = recordingManager.status();
                            return response;
                        }
                        options.captureGeometry = QRect(parts.at(0).toInt(),
                                                        parts.at(1).toInt(),
                                                        parts.at(2).toInt(),
                                                        parts.at(3).toInt()).normalized();
                    }
                }

                if (options.captureGeometry.isEmpty()) {
                    response.message = QStringLiteral("recording geometry is invalid");
                    response.recording = recordingManager.status();
                    return response;
                }
                // 区域必须落在真实显示器的虚拟桌面内：拒绝完全越界的区域，
                // 避免把不可捕获的几何交给采集后端。注意显示器可能位于主屏
                // 左/上方（虚拟桌面坐标可为负），因此只做相交校验、不断言
                // 坐标非负。
                QRect virtualDesktop;
                for (const markshot::recording::DisplaySource &source : sources) {
                    virtualDesktop = virtualDesktop.isNull()
                        ? source.geometry
                        : virtualDesktop.united(source.geometry);
                }
                const QRect region = options.captureGeometry.normalized();
                if (!virtualDesktop.isNull() && !virtualDesktop.intersects(region)) {
                    response.message = QStringLiteral("recording geometry is outside all displays");
                    response.recording = recordingManager.status();
                    return response;
                }

                QString startError;
                if (!recordingManager.start(options, &app, &startError)) {
                    response.message = startError.isEmpty()
                        ? QStringLiteral("failed to start recording")
                        : startError;
                    response.recording = recordingManager.status();
                    return response;
                }
                response.recordingStarted = true;
                response.message = QStringLiteral("recording started");
                // 限时录制：到时自动停止。定时器与当前录制会话绑定，录制提前
                // 结束（stop/失败）时会通过 recordingFinished 取消，避免误停
                // 后续开启的录制。时长由 IPC 传来，夹紧到 24h 上限。
                const int durationMs = std::clamp(command.recordDurationMs, 0, 24 * 3600 * 1000);
                if (durationMs > 0) {
                    if (recordingAutoStopTimer) {
                        recordingAutoStopTimer->stop();
                        recordingAutoStopTimer->deleteLater();
                    }
                    auto *timer = new QTimer(&app);
                    timer->setSingleShot(true);
                    timer->setInterval(durationMs);
                    QObject::connect(timer, &QTimer::timeout, &app, [&recordingManager] {
                        QString stopError;
                        recordingManager.stop(&stopError);
                    });
                    recordingAutoStopTimer = timer;
                    timer->start();
                }
                response.recording = recordingManager.status();
                return response;
            }

            if (command.capture) {
                QTimer::singleShot(0, &app, [launchCapture, command] {
                    launchCapture(command.fullscreen,
                                  command.allOutputs,
                                  std::nullopt,
                                  command.delaySeconds,
                                  command.windowCapture);
                });
                response.message = QStringLiteral("capture requested");
            } else {
                response.message = QStringLiteral("running");
            }
            response.recording = recordingManager.status();
            return response;
        });

    if (wantFloatingBall) {
        floatingBall = new markshot::FloatingBall();
        // 没有托盘入口时，隐藏悬浮球改为退出应用，避免失去唯一入口。
        floatingBall->setQuitWhenHidden(!wantTray);
        floatingBall->setCaptureCallbacks([launchCapture, allOutputs] { launchCapture(false, allOutputs); },
                                          [launchCapture, allOutputs] { launchCapture(true, allOutputs); });
        floatingBall->setTimedCaptureCallback(launchTimedCapture);
        floatingBall->setWindowCaptureCallback([launchCapture, allOutputs] {
            launchCapture(false, allOutputs, std::nullopt, std::nullopt, true);
        });
        floatingBall->setRecordingRegionCallback([launchCapture, allOutputs](markshot::recording::RecordingOptions options) {
            launchCapture(false, allOutputs, std::move(options));
        });
        floatingBall->placeOnScreen();
        floatingBall->show();
    }

    // 托盘控制器：wantTray 时显示托盘图标；悬浮球-only 且启用全局快捷键时
    // 以隐藏模式创建，仅注册热键（否则热键随托盘缺失而静默失效）。
    const bool wantHotkeys = trayConfig.hotkeysEnabled && (wantTray || wantFloatingBall);
    if (wantTray || wantHotkeys) {
        auto *trayController = new markshot::WindowsTrayController(&app, trayConfig, &app, wantTray);
        trayController->setCaptureCallbacks([launchCapture, allOutputs] { launchCapture(false, allOutputs); },
                                            [launchCapture, allOutputs] { launchCapture(true, allOutputs); });
        trayController->setTimedCaptureCallback(launchTimedCapture);
        trayController->setWindowCaptureCallback([launchCapture, allOutputs] {
            launchCapture(false, allOutputs, std::nullopt, std::nullopt, true);
        });
        trayController->setRecordingRegionCallback([launchCapture, allOutputs](markshot::recording::RecordingOptions options) {
            launchCapture(false, allOutputs, std::move(options));
        });
        // 托盘"显示/隐藏悬浮球"开关：必须在 start() 之前设置，start() 构建菜单。
        if (floatingBall) {
            trayController->setFloatingBallVisibilityControl(
                [floatingBall] { floatingBall->toggleByUser(); },
                [floatingBall] { return floatingBall->isVisible(); });
        }
        if (!trayController->start()) {
            QMessageBox::critical(nullptr, QStringLiteral("DracoPho"), trayController->errorString());
            return 1;
        }
    }

    if (wantSettingsWindow) {
        QTimer::singleShot(0, &app, [&captureActive] {
            // 若同时配置了直接截图，截图会话已优先开始，设置窗口留待从
            // 截图工具栏/托盘/悬浮球入口打开，避免覆盖截图界面。
            if (!captureActive) {
                markshot::settings::showSettingsDialog();
            }
        });
    }

    if (wantCapture) {
        if (!launchCapture(fullscreenAnnotation,
                           allOutputs,
                           std::nullopt,
                           std::nullopt,
                           windowCaptureRequested)) {
            return 1;
        }
    }

    return QApplication::exec();
}
