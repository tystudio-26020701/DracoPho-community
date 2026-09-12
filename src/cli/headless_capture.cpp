#include "cli/headless_capture.h"

#include "recording/recording_display_source.h"
#include "screen_capture.h"

#include <QBuffer>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImageWriter>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScreen>
#include <QTextStream>
#include <QThread>

#include <cstdio>
#include <optional>

namespace markshot::cli {
namespace {

// Parses "x,y,w,h" into a QRect. Returns nullopt when malformed or empty.
std::optional<QRect> parseRegion(const QString &value)
{
    const QStringList parts = value.split(QLatin1Char(','));
    if (parts.size() != 4) {
        return std::nullopt;
    }
    bool xOk = false;
    bool yOk = false;
    bool wOk = false;
    bool hOk = false;
    const int x = parts.at(0).trimmed().toInt(&xOk);
    const int y = parts.at(1).trimmed().toInt(&yOk);
    const int w = parts.at(2).trimmed().toInt(&wOk);
    const int h = parts.at(3).trimmed().toInt(&hOk);
    if (!xOk || !yOk || !wOk || !hOk || w <= 0 || h <= 0) {
        return std::nullopt;
    }
    return QRect(x, y, w, h);
}

QByteArray displaysJson()
{
    QJsonArray displays;
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        if (!screen) {
            continue;
        }
        QJsonObject entry;
        entry.insert(QStringLiteral("name"), screen->name());
        // 持久化键：可直接用于 --record-display（裸名、screen:、output: 前缀
        // 均会被归一化，此处给出规范形式 output:<name>）。
        entry.insert(QStringLiteral("key"),
                     markshot::recording::normalizeRecordingDisplayId(screen->name()));
        const QRect geometry = screen->geometry();
        entry.insert(QStringLiteral("x"), geometry.x());
        entry.insert(QStringLiteral("y"), geometry.y());
        entry.insert(QStringLiteral("width"), geometry.width());
        entry.insert(QStringLiteral("height"), geometry.height());
        entry.insert(QStringLiteral("dpr"), screen->devicePixelRatio());
        entry.insert(QStringLiteral("primary"), screen == QGuiApplication::primaryScreen());
        displays.append(entry);
    }
    QJsonObject root;
    root.insert(QStringLiteral("v"), 1);
    root.insert(QStringLiteral("displays"), displays);
    return QJsonDocument(root).toJson(QJsonDocument::Compact);
}

// Resolves the final output path. When the user passes a directory (or a path
// ending with a separator that does not yet exist), a timestamped file name is
// generated inside it.
QString resolveOutputPath(const QString &captureTo, const QString &outputName, QString *error)
{
    QFileInfo info(captureTo);
    const bool looksLikeDirectory = info.isDir()
        || (captureTo.endsWith(QLatin1Char('/')) || captureTo.endsWith(QLatin1Char('\\')));
    if (looksLikeDirectory) {
        QDir().mkpath(info.absoluteFilePath());
        const QString stamp =
            QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
        QString fileName = QStringLiteral("dracoPho-%1.png").arg(stamp);
        if (!outputName.isEmpty()) {
            fileName = QStringLiteral("dracoPho-%1-%2.png").arg(outputName, stamp);
        }
        return QDir(info.absoluteFilePath()).filePath(fileName);
    }
    if (info.absoluteFilePath().isEmpty()) {
        if (error) {
            *error = QStringLiteral("empty output path");
        }
        return QString();
    }
    return info.absoluteFilePath();
}

// Returns true when captureTo looks like an existing directory (the only form
// supported for multi-display capture).
bool isDirectoryTarget(const QString &captureTo)
{
    return QFileInfo(captureTo).isDir();
}

// Builds the base CaptureRequest for headless mode. Shared by the single and
// multi-display paths.
CaptureRequest baseCaptureRequest(const QCommandLineParser &parser)
{
    CaptureRequest request;
    request.allOutputs = parser.isSet(QStringLiteral("all-outputs"));
    request.includeCursor = parser.isSet(QStringLiteral("include-cursor"));
    request.allowInteractivePortal = false;
    request.hideOwnWindows = false;
    return request;
}

// Returns the geometry of the named display, or a default-constructed QRect
// when no such display exists.
QRect displayGeometry(const QString &displayName)
{
    const QList<QScreen *> screens = QGuiApplication::screens();
    for (QScreen *screen : screens) {
        if (screen && screen->name() == displayName) {
            return screen->geometry();
        }
    }
    return {};
}

// Verifies that every requested display name exists. Returns the first
// offending name, or an empty string when all names are valid.
QString firstUnknownDisplay(const QStringList &displayNames)
{
    for (const QString &displayName : displayNames) {
        if (displayGeometry(displayName).isNull()) {
            return displayName;
        }
    }
    return {};
}

// Applies a display name: prefers that output and, when no explicit region is
// given, crops to the output geometry so portal backends capture that monitor
// instead of the whole virtual desktop.
void applyDisplayToRequest(CaptureRequest *request, const QString &displayName)
{
    if (displayName.isEmpty() || !request) {
        return;
    }
    request->preferredOutputName = displayName;
    if (!request->sourceGeometry.isValid()) {
        const QRect geometry = displayGeometry(displayName);
        if (!geometry.isNull()) {
            request->sourceGeometry = geometry;
        }
    }
}

// Captures one frame to a PNG and returns a compact JSON summary object.
// On failure an object with a null path and an error message is returned.
QJsonObject captureOneToFile(const CaptureRequest &request,
                             const QString &captureTo,
                             const QString &baseName,
                             QTextStream *err)
{
    const CaptureResult result = captureScreenFrame(request);

    if (result.image.isNull()) {
        if (err) {
            *err << result.error << '\n';
        }
        return {{QStringLiteral("path"), QJsonValue::Null},
                {QStringLiteral("width"), 0},
                {QStringLiteral("height"), 0},
                {QStringLiteral("output"), QJsonValue::Null},
                {QStringLiteral("error"), result.error}};
    }

    QString resolveError;
    const QString outputPath = resolveOutputPath(captureTo, baseName, &resolveError);
    if (outputPath.isEmpty()) {
        if (err) {
            *err << resolveError << '\n';
        }
        return {{QStringLiteral("path"), QJsonValue::Null},
                {QStringLiteral("width"), 0},
                {QStringLiteral("height"), 0},
                {QStringLiteral("output"), QJsonValue::Null},
                {QStringLiteral("error"), resolveError}};
    }

    QImageWriter writer(outputPath, QByteArrayLiteral("png"));
    if (!writer.write(result.image)) {
        const QString writeError = writer.errorString();
        QFile::remove(outputPath);
        if (err) {
            *err << "failed to write capture to " << outputPath << ": " << writeError << '\n';
        }
        return {{QStringLiteral("path"), QJsonValue::Null},
                {QStringLiteral("width"), 0},
                {QStringLiteral("height"), 0},
                {QStringLiteral("output"), QJsonValue::Null},
                {QStringLiteral("error"), writeError}};
    }

    // Propagate the requested display name when the backend did not fill it
    // (QScreen-based backends return an empty outputName).
    QString effectiveOutput = result.outputName;
    if (effectiveOutput.isEmpty() && !request.preferredOutputName.isEmpty()) {
        effectiveOutput = request.preferredOutputName;
    }

    return {{QStringLiteral("path"), outputPath},
            {QStringLiteral("width"), result.image.width()},
            {QStringLiteral("height"), result.image.height()},
            {QStringLiteral("output"), effectiveOutput.isEmpty() ? QJsonValue::Null : QJsonValue(effectiveOutput)},
            {QStringLiteral("error"), QJsonValue::Null}};
}

// Captures one frame and returns it inline as base64 PNG, keeping the image
// out of the filesystem entirely. Shape mirrors captureOneToFile (with "data"
// instead of "path") so agents can treat both destinations uniformly.
QJsonObject captureOneToInline(const CaptureRequest &request, QTextStream *err)
{
    const CaptureResult result = captureScreenFrame(request);

    if (result.image.isNull()) {
        if (err) {
            *err << result.error << '\n';
        }
        return {{QStringLiteral("path"), QJsonValue::Null},
                {QStringLiteral("width"), 0},
                {QStringLiteral("height"), 0},
                {QStringLiteral("data"), QJsonValue::Null},
                {QStringLiteral("output"), QJsonValue::Null},
                {QStringLiteral("error"), result.error}};
    }

    QByteArray png;
    QBuffer buffer(&png);
    buffer.open(QIODevice::WriteOnly);
    if (!result.image.save(&buffer, "PNG")) {
        if (err) {
            *err << "failed to encode capture as PNG\n";
        }
        return {{QStringLiteral("path"), QJsonValue::Null},
                {QStringLiteral("width"), 0},
                {QStringLiteral("height"), 0},
                {QStringLiteral("data"), QJsonValue::Null},
                {QStringLiteral("output"), QJsonValue::Null},
                {QStringLiteral("error"), QStringLiteral("failed to encode capture as PNG")}};
    }

    QString effectiveOutput = result.outputName;
    if (effectiveOutput.isEmpty() && !request.preferredOutputName.isEmpty()) {
        effectiveOutput = request.preferredOutputName;
    }

    return {{QStringLiteral("path"), QJsonValue::Null},
            {QStringLiteral("width"), result.image.width()},
            {QStringLiteral("height"), result.image.height()},
            {QStringLiteral("data"), QString::fromLatin1(png.toBase64())},
            {QStringLiteral("output"), effectiveOutput.isEmpty() ? QJsonValue::Null : QJsonValue(effectiveOutput)},
            {QStringLiteral("error"), QJsonValue::Null}};
}

// Waits the requested headless delay in short chunks so the process stays
// responsive to termination signals. Unlike the interactive countdown overlay
// this is a plain wait with no window and no Esc handling.
void waitForHeadlessDelay(int seconds, QTextStream &err)
{
    if (seconds <= 0) {
        return;
    }
    err << "waiting " << seconds << "s before capture (--delay)\n";
    err.flush();
    for (int waited = 0; waited < seconds * 10; ++waited) {
        QThread::msleep(100);
    }
}

} // namespace

void addHeadlessCaptureOptions(QCommandLineParser *parser)
{
    QCommandLineOption captureToOption(QStringLiteral("capture-to"),
                                       QStringLiteral("Capture the screen and write it to the given file or directory without showing the UI."),
                                       QStringLiteral("path"));
    QCommandLineOption regionOption(QStringLiteral("region"),
                                    QStringLiteral("Capture only the region x,y,width,height in logical screen coordinates."),
                                    QStringLiteral("x,y,w,h"));
    QCommandLineOption displayOption(QStringLiteral("display"),
                                     QStringLiteral("Capture a specific output by monitor name. May be repeated to capture several monitors at once."),
                                     QStringLiteral("name"));
    QCommandLineOption includeCursorOption(QStringLiteral("include-cursor"),
                                           QStringLiteral("Draw the mouse cursor into the captured image."));
    QCommandLineOption listDisplaysOption(QStringLiteral("list-displays"),
                                          QStringLiteral("Print the available outputs as JSON and exit."));
    QCommandLineOption captureScreenOption(QStringLiteral("capture-screen"),
                                           QStringLiteral("Capture the screen without a caller-supplied path; requires an explicit "
                                                          "--capture-destination inline (base64 in JSON, no files) or stage (temporary "
                                                          "staging directory). Combine with --region/--display/--all-outputs to select "
                                                          "what to capture (default: the primary screen)."));
    QCommandLineOption outputNameOption(QStringLiteral("output-name"),
                                        QStringLiteral("Base file name (without extension) used when the capture path is a directory."),
                                        QStringLiteral("name"));
    parser->addOption(captureToOption);
    parser->addOption(captureScreenOption);
    parser->addOption(regionOption);
    parser->addOption(displayOption);
    parser->addOption(includeCursorOption);
    parser->addOption(listDisplaysOption);
    parser->addOption(outputNameOption);
}

int runHeadlessCaptureIfRequested(const QCommandLineParser &parser)
{
    QTextStream out(stdout);
    QTextStream err(stderr);

    const bool wantListDisplays = parser.isSet(QStringLiteral("list-displays"));
    // B 契约：触发器只有 --capture-to（写文件）与 --capture-screen（显式无路径
    // 截屏，必须搭配 --capture-destination inline|stage）。--capture-destination
    // 及其余无头参数一律是修饰符：修饰符离开触发器单独出现 = 用法错误退出 2，
    // 绝不静默落回交互式启动。
    const bool wantScreen = parser.isSet(QStringLiteral("capture-screen"));
    const bool wantCapture = parser.isSet(QStringLiteral("capture-to")) || wantScreen;
    if (!wantListDisplays && !wantCapture) {
        if (parser.isSet(QStringLiteral("region"))
            || parser.isSet(QStringLiteral("display"))
            || parser.isSet(QStringLiteral("all-outputs"))
            || parser.isSet(QStringLiteral("capture-destination"))
            || parser.isSet(QStringLiteral("output-name"))
            || parser.isSet(QStringLiteral("include-cursor"))) {
            err << "no capture operation requested: headless modifiers need a trigger. "
                   "Use --capture-to <path> for file output, or --capture-screen with "
                   "--capture-destination inline|stage, or --window <selector> for "
                   "window captures.\n";
            return 2;
        }
        return -1;
    }
    if (wantScreen && parser.isSet(QStringLiteral("capture-to"))) {
        err << "--capture-screen and --capture-to are mutually exclusive: "
               "--capture-to writes a file, --capture-screen needs "
               "--capture-destination inline|stage.\n";
        return 2;
    }

    if (wantListDisplays) {
        out << displaysJson() << '\n';
        out.flush();
        if (wantCapture) {
            err << "--list-displays and --capture-to cannot be combined.\n";
            return 1;
        }
        return 0;
    }

    if (parser.positionalArguments().size() > 0) {
        err << "--capture-to cannot be combined with an image file argument.\n";
        return 1;
    }

    const QStringList displayNames = parser.values(QStringLiteral("display"));
    const QString captureTo = parser.value(QStringLiteral("capture-to"));
    const QString outputName = parser.value(QStringLiteral("output-name")).trimmed();
    const bool hasRegion = parser.isSet(QStringLiteral("region"));
    const bool allOutputs = parser.isSet(QStringLiteral("all-outputs"));

    // Reject semantically conflicting option combinations early so the user
    // gets a clear error instead of silently capturing the wrong pixels.
    if (allOutputs && !displayNames.isEmpty()) {
        err << "--all-outputs cannot be combined with --display.\n";
        return 1;
    }
    if (hasRegion && displayNames.size() > 1) {
        err << "--region cannot be combined with multiple --display options.\n";
        return 1;
    }
    if (displayNames.size() > 1 && !isDirectoryTarget(captureTo)) {
        err << "--capture-to must be an existing directory when capturing multiple displays.\n";
        return 1;
    }
    const QString unknownDisplay = firstUnknownDisplay(displayNames);
    if (!unknownDisplay.isEmpty()) {
        err << "unknown display: " << unknownDisplay << ". Use --list-displays to see valid names.\n";
        return 1;
    }

    // 无头延时：--delay 在无头链路中是纯等待（无倒计时遮罩）。此前它被静默
    // 忽略，脚本/智能体会误以为延时已生效；现在非法值报错、合法值真实生效。
    if (parser.isSet(QStringLiteral("delay"))) {
        const std::optional<int> delay =
            parseHeadlessDelaySeconds(parser.value(QStringLiteral("delay")));
        if (!delay.has_value()) {
            err << "--delay expects an integer number of seconds in [0, 3600].\n";
            return 2;
        }
        waitForHeadlessDelay(delay.value(), err);
    }

    // B 契约去向矩阵：
    //   --capture-to                     → file（唯一写文件方式）
    //   --capture-screen                 → 必须显式 --capture-destination inline|stage
    //   inline × --capture-to/--output-name 互斥；stage × --capture-to 冗余拒绝
    ScreenCaptureDestination screenDestination = ScreenCaptureDestination::File;
    const bool hasDestination = parser.isSet(QStringLiteral("capture-destination"));
    if (hasDestination) {
        const QString value = parser.value(QStringLiteral("capture-destination"));
        if (value.trimmed().toLower() == QLatin1String("clipboard")) {
            err << "--capture-destination clipboard is only available for window captures "
                   "(use --window <selector> --capture-destination clipboard).\n";
            return 2;
        }
        const std::optional<ScreenCaptureDestination> parsed =
            parseScreenDestination(value);
        if (!parsed.has_value()) {
            err << "invalid --capture-destination \"" << value
                << "\" (expected inline, file or stage for screen captures).\n";
            return 2;
        }
        screenDestination = parsed.value();
    }
    if (wantScreen) {
        if (!hasDestination) {
            err << "--capture-screen requires an explicit --capture-destination inline or stage.\n";
            return 2;
        }
        if (screenDestination == ScreenCaptureDestination::File) {
            err << "--capture-screen does not take a path: use --capture-to <path> for "
                   "file output, or --capture-destination inline|stage with --capture-screen.\n";
            return 2;
        }
    } else if (screenDestination == ScreenCaptureDestination::File && captureTo.isEmpty()) {
        err << "--capture-destination file requires --capture-to <path>.\n";
        return 2;
    }
    if (screenDestination == ScreenCaptureDestination::Inline
        && parser.isSet(QStringLiteral("output-name"))) {
        err << "--capture-destination inline writes no files; drop --output-name.\n";
        return 2;
    }
    if (screenDestination == ScreenCaptureDestination::Stage
        && parser.isSet(QStringLiteral("capture-to"))) {
        err << "--capture-destination stage writes to the staging directory while "
               "--capture-to already names a path; keep exactly one.\n";
        return 2;
    }
    QString effectiveCaptureTo = captureTo;
    if (screenDestination == ScreenCaptureDestination::Stage) {
        effectiveCaptureTo = QDir(QDir::tempPath()).filePath(screenStageDirectoryName());
        // 先确保暂存目录存在：resolveOutputPath 依赖 isDir() 判定生成
        // 时间戳文件名，缺失时会把这个路径误当作目标文件本身。
        QDir().mkpath(effectiveCaptureTo);
    }

    CaptureRequest request = baseCaptureRequest(parser);
    request.allOutputs = allOutputs;

    if (hasRegion) {
        const std::optional<QRect> region = parseRegion(parser.value(QStringLiteral("region")));
        if (!region.has_value()) {
            err << "--region expects a comma-separated rectangle x,y,width,height.\n";
            return 1;
        }
        if (allOutputs) {
            err << "--region cannot be combined with --all-outputs.\n";
            return 1;
        }
        request.sourceGeometry = region.value();
    }

    // Multiple --display: capture each monitor to its own PNG and print a
    // {"captures":[...]} JSON array. This enables multi-screen selection for
    // agents and scripts.
    const QString destinationName = screenDestination == ScreenCaptureDestination::Inline
        ? QStringLiteral("inline")
        : (screenDestination == ScreenCaptureDestination::Stage ? QStringLiteral("stage")
                                                                : QStringLiteral("file"));
    if (displayNames.size() > 1) {
        QJsonArray captures;
        bool anyFailed = false;
        for (const QString &displayName : displayNames) {
            CaptureRequest displayRequest = request;
            applyDisplayToRequest(&displayRequest, displayName);
            const QString baseName = outputName.isEmpty()
                ? displayName
                : QStringLiteral("%1-%2").arg(outputName, displayName);
            const QJsonObject one = screenDestination == ScreenCaptureDestination::Inline
                ? captureOneToInline(displayRequest, &err)
                : captureOneToFile(displayRequest, effectiveCaptureTo, baseName, &err);
            if (one.value(QStringLiteral("error")).isString()) {
                anyFailed = true;
            }
            captures.append(one);
        }
        out << QJsonDocument(QJsonObject{{QStringLiteral("v"), 1},
                                         {QStringLiteral("destination"), destinationName},
                                         {QStringLiteral("captures"), captures}})
                   .toJson(QJsonDocument::Compact)
            << '\n';
        out.flush();
        return anyFailed ? 1 : 0;
    }

    // Single display (or none): keep the original compact single-object JSON.
    if (!displayNames.isEmpty()) {
        applyDisplayToRequest(&request, displayNames.first());
    }

    const QJsonObject single = screenDestination == ScreenCaptureDestination::Inline
        ? captureOneToInline(request, &err)
        : captureOneToFile(request, effectiveCaptureTo, outputName, &err);
    QJsonObject singleRoot = single;
    singleRoot.insert(QStringLiteral("v"), 1);
    singleRoot.insert(QStringLiteral("destination"), destinationName);
    out << QJsonDocument(singleRoot).toJson(QJsonDocument::Compact) << '\n';
    out.flush();
    return single.value(QStringLiteral("error")).isString() ? 1 : 0;
}

} // namespace markshot::cli
