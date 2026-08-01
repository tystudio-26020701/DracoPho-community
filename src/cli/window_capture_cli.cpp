#include "cli/window_capture_cli.h"

#include "app_config_store.h"
#include "capture_session_screen_utils.h"
#include "clipboard_image.h"
#include "headless_capture_config.h"
#include "screen_capture.h"
#include "window_detection.h"

#include <QBuffer>
#include <QCommandLineOption>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QImageWriter>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QScreen>
#include <QTextStream>

#include <optional>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

namespace markshot::cli {
namespace {

// Sub-directory used for the "stage" capture destination.
constexpr const char *kStageDirectoryName = "mark-shot-staging";

enum class CaptureDestination {
    Inline,   // embed base64 PNG in the JSON output, no files, no clipboard
    File,     // write PNG files into --capture-to
    Stage,    // write PNG files into a staging directory under the system temp
    Clipboard // copy to the system clipboard; with several images the last one wins
};

struct WindowSelection {
    QString selector;                 // original --window value
    QString windowSpec;               // window part (before '@')
    std::optional<QRect> subRect;     // component sub-region relative to the window
};

bool isX11SessionLike()
{
#if defined(Q_OS_WIN)
    return false;
#else
    const QString sessionType = qEnvironmentVariable("XDG_SESSION_TYPE").toLower();
    if (sessionType == QLatin1String("x11")) {
        return true;
    }
    if (sessionType == QLatin1String("wayland")) {
        return false;
    }
    return !qEnvironmentVariable("DISPLAY").isEmpty();
#endif
}

// Collects the windows used by --list-windows / --window. Prefers the
// configured per-compositor detection script (GNOME / KDE / Hyprland / niri
// Wayland), and falls back to the in-process X11 enumeration.
QVector<WindowInfo> collectWindowInfos(QString *source)
{
    if (source) {
        source->clear();
    }

    const QVector<WindowInfo> scripted =
        collectConfiguredWindowInfos(markshot::capture_session::virtualScreensGeometry(), QString(), true);
    if (!scripted.isEmpty()) {
        if (source) {
            *source = QStringLiteral("compositor-script");
        }
        return scripted;
    }

    if (isX11SessionLike()) {
        const QVector<WindowInfo> x11 = enumerateX11WindowInfos();
        if (source) {
            *source = QStringLiteral("x11");
        }
        return x11;
    }

    if (source) {
        *source = QStringLiteral("none");
    }
    return {};
}

QString platformName()
{
#if defined(Q_OS_WIN)
    return QStringLiteral("windows");
#else
    if (isX11SessionLike()) {
        return QStringLiteral("x11");
    }
    return QStringLiteral("wayland");
#endif
}

QJsonObject windowJson(const WindowInfo &info, int index)
{
    QJsonObject entry;
    entry.insert(QStringLiteral("index"), index);
    if (!info.id.isEmpty()) {
        entry.insert(QStringLiteral("id"), info.id);
    }
    if (!info.title.isEmpty()) {
        entry.insert(QStringLiteral("title"), info.title);
    }
    if (!info.className.isEmpty()) {
        entry.insert(QStringLiteral("class"), info.className);
    }
    if (!info.instance.isEmpty()) {
        entry.insert(QStringLiteral("instance"), info.instance);
    }
    if (!info.monitor.isEmpty()) {
        entry.insert(QStringLiteral("monitor"), info.monitor);
    }
    if (!info.workspace.isEmpty()) {
        entry.insert(QStringLiteral("workspace"), info.workspace);
    }
    entry.insert(QStringLiteral("x"), info.rect.x());
    entry.insert(QStringLiteral("y"), info.rect.y());
    entry.insert(QStringLiteral("width"), info.rect.width());
    entry.insert(QStringLiteral("height"), info.rect.height());
    if (info.zOrder.has_value()) {
        entry.insert(QStringLiteral("zOrder"), *info.zOrder);
    }
    return entry;
}

std::optional<QRect> parseSubRect(const QString &text)
{
    const QStringList parts = text.split(QLatin1Char(','));
    if (parts.size() != 4) {
        return std::nullopt;
    }
    bool ok[4] = {false, false, false, false};
    const int x = parts.at(0).trimmed().toInt(&ok[0]);
    const int y = parts.at(1).trimmed().toInt(&ok[1]);
    const int width = parts.at(2).trimmed().toInt(&ok[2]);
    const int height = parts.at(3).trimmed().toInt(&ok[3]);
    if (!ok[0] || !ok[1] || !ok[2] || !ok[3] || width <= 0 || height <= 0) {
        return std::nullopt;
    }
    return QRect(x, y, width, height);
}

bool parseSelections(const QStringList &values, QVector<WindowSelection> *selections, QString *error)
{
    if (!selections) {
        return false;
    }
    for (const QString &value : values) {
        const int atIndex = value.indexOf(QLatin1Char('@'));
        WindowSelection selection;
        selection.selector = value.trimmed();
        selection.windowSpec = atIndex >= 0
            ? value.left(atIndex).trimmed()
            : value.trimmed();
        if (selection.windowSpec.isEmpty()) {
            if (error) {
                *error = QStringLiteral("empty window selector in \"%1\"").arg(value);
            }
            return false;
        }
        if (atIndex >= 0) {
            const std::optional<QRect> subRect = parseSubRect(value.mid(atIndex + 1).trimmed());
            if (!subRect.has_value()) {
                if (error) {
                    *error = QStringLiteral(
                        "invalid component sub-region in \"%1\": expected x,y,width,height after '@'").arg(value);
                }
                return false;
            }
            selection.subRect = subRect;
        }
        selections->append(selection);
    }
    return true;
}

// Matches a single window spec against a window info according to the mode.
bool windowMatches(const WindowInfo &info, int index, const QString &spec, const QString &mode)
{
    if (mode == QLatin1String("id")) {
        return !info.id.isEmpty() && info.id == spec;
    }
    if (mode == QLatin1String("title")) {
        return !info.title.isEmpty()
            && (info.title == spec || info.title.contains(spec, Qt::CaseInsensitive));
    }
    if (mode == QLatin1String("class")) {
        return !info.className.isEmpty()
            && (info.className == spec
                || info.className.contains(spec, Qt::CaseInsensitive)
                || info.instance.contains(spec, Qt::CaseInsensitive));
    }
    if (mode == QLatin1String("index")) {
        bool ok = false;
        const int wanted = spec.toInt(&ok);
        return ok && wanted == index;
    }

    // auto: exact id -> exact title -> exact class -> index -> substrings.
    if (!info.id.isEmpty() && info.id == spec) {
        return true;
    }
    if (info.title == spec) {
        return true;
    }
    if (info.className == spec || info.instance == spec) {
        return true;
    }
    bool indexOk = false;
    const int wantedIndex = spec.toInt(&indexOk);
    if (indexOk && wantedIndex == index) {
        return true;
    }
    if (!info.title.isEmpty() && info.title.contains(spec, Qt::CaseInsensitive)) {
        return true;
    }
    if (!info.className.isEmpty() && info.className.contains(spec, Qt::CaseInsensitive)) {
        return true;
    }
    return !info.instance.isEmpty() && info.instance.contains(spec, Qt::CaseInsensitive);
}

// Builds the capture rectangle for a window: the whole window, or a component
// sub-region (offset within the window) when the selector carried "@x,y,w,h".
QRect captureRectForWindow(const WindowInfo &info, const std::optional<QRect> &subRect)
{
    if (!subRect.has_value()) {
        return info.rect.normalized();
    }
    QRect rect(info.rect.x() + subRect->x(),
               info.rect.y() + subRect->y(),
               subRect->width(),
               subRect->height());
    rect = rect.intersected(info.rect.normalized());
    return rect;
}

QString stagingDirectory()
{
    // 固定路径可能被同一台机器上的其他本地用户预先创建（CWE-377）：目录若
    // 不属于当前用户则退回带 uid 的私有目录，避免截图被他人读取或暂存失败。
    const QString tempRoot = QDir::tempPath();
    const QString primary = QDir(tempRoot).filePath(QLatin1String(kStageDirectoryName));
    QDir().mkpath(primary);
#if defined(Q_OS_UNIX)
    const QFileInfo info(primary);
    if (!info.isDir() || info.ownerId() != static_cast<uint>(::getuid())) {
        const QString privateDirectory = QDir(tempRoot).filePath(
            QStringLiteral("mark-shot-staging-%1").arg(static_cast<qulonglong>(::getuid())));
        QDir().mkpath(privateDirectory);
        return privateDirectory;
    }
#endif
    return primary;
}

QString writePng(const QString &directory,
                 const QString &stem,
                 const QImage &image,
                 QString *error)
{
    QDir().mkpath(directory);
    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
    const QString path = QDir(directory).filePath(QStringLiteral("%1-%2.png").arg(stem, stamp));
    QImageWriter writer(path, QByteArrayLiteral("png"));
    if (!writer.write(image)) {
        if (error) {
            *error = QStringLiteral("failed to write %1: %2").arg(path, writer.errorString());
        }
        QFile::remove(path);
        return {};
    }
    // 截图可能包含敏感内容：收紧为仅属主可读写，避免被同机其他用户读取。
    QFile::setPermissions(path, QFile::ReadOwner | QFile::WriteOwner);
    return path;
}

// Turns a window identity into a file name stem.
QString windowStem(const WindowInfo &info, int index, const QString &outputName)
{
    QString base = info.title;
    if (base.isEmpty()) {
        base = info.id;
    }
    if (base.isEmpty()) {
        base = info.className;
    }
    if (base.isEmpty()) {
        base = QStringLiteral("window-%1").arg(index);
    }
    base = base.toLower();
    base.replace(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}]+")), QLatin1String("-"));
    while (base.startsWith(QLatin1Char('-'))) {
        base.remove(0, 1);
    }
    while (base.endsWith(QLatin1Char('-'))) {
        base.chop(1);
    }
    if (base.isEmpty()) {
        base = QStringLiteral("window-%1").arg(index);
    }
    if (!outputName.isEmpty()) {
        // 自定义输出名同样做净化，避免 "../" 或绝对路径把文件写到目标目录之外。
        QString name = outputName;
        name.replace(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}]+")), QLatin1String("-"));
        while (name.startsWith(QLatin1Char('-'))) {
            name.remove(0, 1);
        }
        while (name.endsWith(QLatin1Char('-'))) {
            name.chop(1);
        }
        if (!name.isEmpty()) {
            base = name + QLatin1Char('-') + base;
        }
    }
    return base;
}

QByteArray pngBytes(const QImage &image)
{
    QByteArray png;
    QBuffer buffer(&png);
    if (buffer.open(QIODevice::WriteOnly) && image.save(&buffer, "PNG")) {
        return png;
    }
    return {};
}

// Captures a single window (or a component sub-region of it) and routes the
// image to the requested destination, returning a JSON summary entry.
QJsonObject captureOne(const WindowInfo &info,
                       int index,
                       const QString &selector,
                       CaptureDestination destination,
                       const QString &outputDir,
                       const QString &outputName,
                       bool includeCursor,
                       const std::optional<QRect> &subRect,
                       QTextStream *err)
{
    QJsonObject entry;
    entry.insert(QStringLiteral("selector"), selector);
    entry.insert(QStringLiteral("index"), index);
    if (!info.id.isEmpty()) {
        entry.insert(QStringLiteral("id"), info.id);
    }
    if (!info.title.isEmpty()) {
        entry.insert(QStringLiteral("title"), info.title);
    }
    if (!info.className.isEmpty()) {
        entry.insert(QStringLiteral("class"), info.className);
    }

    const QRect captureRect = captureRectForWindow(info, subRect);
    entry.insert(QStringLiteral("x"), captureRect.x());
    entry.insert(QStringLiteral("y"), captureRect.y());
    entry.insert(QStringLiteral("width"), captureRect.width());
    entry.insert(QStringLiteral("height"), captureRect.height());

    CaptureRequest request;
    request.sourceGeometry = captureRect;
    request.includeCursor = includeCursor;
    request.allowInteractivePortal = false;
    request.hideOwnWindows = false;
    request.allOutputs = false;

    const CaptureResult result = captureScreenFrame(request);
    if (result.image.isNull()) {
        if (err) {
            *err << result.error << '\n';
        }
        entry.insert(QStringLiteral("path"), QJsonValue::Null);
        entry.insert(QStringLiteral("width"), 0);
        entry.insert(QStringLiteral("height"), 0);
        entry.insert(QStringLiteral("error"), result.error);
        return entry;
    }

    entry.insert(QStringLiteral("width"), result.image.width());
    entry.insert(QStringLiteral("height"), result.image.height());

    switch (destination) {
    case CaptureDestination::Inline: {
        const QByteArray png = pngBytes(result.image);
        if (png.isEmpty()) {
            entry.insert(QStringLiteral("error"), QStringLiteral("failed to encode PNG"));
        } else {
            entry.insert(QStringLiteral("data"), QString::fromLatin1(png.toBase64()));
            entry.insert(QStringLiteral("error"), QJsonValue::Null);
        }
        entry.insert(QStringLiteral("path"), QJsonValue::Null);
        break;
    }
    case CaptureDestination::Clipboard: {
        // 无头进程走专用无阻塞剪贴板路径，避免 Wayland QClipboard::setImage
        // 的组合往返阻塞短命进程。
        const bool copied = copyImageToClipboardHeadless(result.image);
        entry.insert(QStringLiteral("path"), QJsonValue::Null);
        entry.insert(QStringLiteral("error"), copied
            ? QJsonValue::Null
            : QJsonValue(QStringLiteral("failed to copy image to the clipboard")));
        break;
    }
    case CaptureDestination::File:
    case CaptureDestination::Stage: {
        const QString directory =
            destination == CaptureDestination::File ? outputDir : stagingDirectory();
        QString writeError;
        const QString path = writePng(directory,
                                      windowStem(info, index, outputName),
                                      result.image,
                                      &writeError);
        if (path.isEmpty()) {
            if (err) {
                *err << writeError << '\n';
            }
            entry.insert(QStringLiteral("path"), QJsonValue::Null);
            entry.insert(QStringLiteral("error"), writeError);
        } else {
            entry.insert(QStringLiteral("path"), path);
            entry.insert(QStringLiteral("error"), QJsonValue::Null);
        }
        break;
    }
    }
    return entry;
}

bool parseDestination(const QString &name, CaptureDestination *destination)
{
    if (!destination) {
        return false;
    }
    const QString normalized = name.toLower();
    if (normalized == QLatin1String("inline")) {
        *destination = CaptureDestination::Inline;
        return true;
    }
    if (normalized == QLatin1String("file")) {
        *destination = CaptureDestination::File;
        return true;
    }
    if (normalized == QLatin1String("stage")) {
        *destination = CaptureDestination::Stage;
        return true;
    }
    if (normalized == QLatin1String("clipboard")) {
        *destination = CaptureDestination::Clipboard;
        return true;
    }
    return false;
}

/// @brief 将配置中的无头默认去向转换为本地去向枚举。
/// @param destination 配置枚举。
/// @return 本地去向枚举。
CaptureDestination captureDestinationForConfig(HeadlessCaptureDestination destination)
{
    return destination == HeadlessCaptureDestination::Stage
        ? CaptureDestination::Stage
        : CaptureDestination::Inline;
}

/// @brief 返回去向的展示名称。
/// @param destination 去向枚举。
/// @return 展示名称。
QString captureDestinationName(CaptureDestination destination)
{
    switch (destination) {
    case CaptureDestination::Inline:
        return QStringLiteral("inline");
    case CaptureDestination::File:
        return QStringLiteral("file");
    case CaptureDestination::Stage:
        return QStringLiteral("stage");
    case CaptureDestination::Clipboard:
        return QStringLiteral("clipboard");
    }
    return QStringLiteral("inline");
}

} // namespace

void addWindowCaptureOptions(QCommandLineParser *parser)
{
    QCommandLineOption listWindowsOption(QStringLiteral("list-windows"),
                                         QStringLiteral("List the visible windows (id, title, class, geometry) as JSON and exit."));
    QCommandLineOption windowOption(QStringLiteral("window"),
                                    QStringLiteral("Capture the window(s) matching the given selector. May be repeated to capture several windows at once. "
                                                   "A selector may carry a component sub-region: \"<selector>@x,y,width,height\" captures only that part of the window."),
                                    QStringLiteral("selector"));
    QCommandLineOption windowByOption(QStringLiteral("window-by"),
                                      QStringLiteral("How to interpret each --window selector: auto, id, title, class or index (default: auto)."),
                                      QStringLiteral("mode"));
    QCommandLineOption destinationOption(QStringLiteral("capture-destination"),
                                         QStringLiteral("Where captured window images go: inline (base64 in the JSON output, no files, clipboard untouched), "
                                                        "file (PNG files into --capture-to), stage (PNG files into a temporary staging directory) or "
                                                        "clipboard (system clipboard; the last image wins). Default: the value configured in "
                                                        "Settings > Storage > Headless Mode (inline unless changed). The clipboard is never "
                                                        "modified unless \"clipboard\" is explicitly selected AND clipboard writes are enabled "
                                                        "in Settings > Storage > Headless Mode; a blocked clipboard request falls back to the "
                                                        "configured default destination and exits with code 1 so scripts can detect it."),
                                         QStringLiteral("mode"));
    parser->addOption(listWindowsOption);
    parser->addOption(windowOption);
    parser->addOption(windowByOption);
    parser->addOption(destinationOption);
}

int runWindowCaptureIfRequested(const QCommandLineParser &parser)
{
    const bool wantList = parser.isSet(QStringLiteral("list-windows"));
    const QStringList selectors = parser.values(QStringLiteral("window"));
    const bool hasDestination = parser.isSet(QStringLiteral("capture-destination"));
    const bool hasWindowBy = parser.isSet(QStringLiteral("window-by"));
    // Any window-capture flag must be routed here and exit instead of falling
    // through to the interactive UI (which would pop the capture overlay).
    if (!wantList && selectors.isEmpty() && !hasDestination && !hasWindowBy) {
        return -1;
    }

    QTextStream out(stdout);
    QTextStream err(stderr);

    if (wantList && !selectors.isEmpty()) {
        err << "--list-windows and --window cannot be combined.\n";
        return 1;
    }
    if (wantList || !selectors.isEmpty()) {
        if (parser.isSet(QStringLiteral("region"))
            || parser.isSet(QStringLiteral("display"))
            || parser.isSet(QStringLiteral("all-outputs"))
            || parser.isSet(QStringLiteral("list-displays"))) {
            err << "window capture options cannot be combined with --region, --display, --all-outputs or --list-displays.\n";
            return 1;
        }
        if (!parser.positionalArguments().isEmpty()) {
            err << "window capture cannot be combined with an image file argument.\n";
            return 1;
        }
    }
    if (hasDestination && selectors.isEmpty()) {
        err << "--capture-destination requires --window.\n";
        return 1;
    }
    if (parser.isSet(QStringLiteral("window-by")) && selectors.isEmpty()) {
        err << "--window-by requires --window.\n";
        return 1;
    }

    const QString windowBy = parser.isSet(QStringLiteral("window-by"))
        ? parser.value(QStringLiteral("window-by")).toLower()
        : QStringLiteral("auto");
    if (windowBy != QLatin1String("auto")
        && windowBy != QLatin1String("id")
        && windowBy != QLatin1String("title")
        && windowBy != QLatin1String("class")
        && windowBy != QLatin1String("index")) {
        err << "invalid --window-by mode \"" << windowBy
            << "\" (expected auto, id, title, class or index).\n";
        return 1;
    }

    const QString captureTo = parser.value(QStringLiteral("capture-to")).trimmed();
    const QString outputName = parser.value(QStringLiteral("output-name")).trimmed();
    const bool includeCursor = parser.isSet(QStringLiteral("include-cursor"));

    QString source;
    const QVector<WindowInfo> windows = collectWindowInfos(&source);
    if (windows.isEmpty()) {
        err << "no windows detected (platform=" << platformName()
            << "). Check the window detection configuration in ~/.config/mark-shot/config.json "
               "and make sure the desktop session supports window enumeration.\n";
        return 1;
    }

    if (wantList) {
        QJsonArray entries;
        for (int i = 0; i < windows.size(); ++i) {
            entries.append(windowJson(windows.at(i), i));
        }
        QJsonObject root;
        root.insert(QStringLiteral("platform"), platformName());
        root.insert(QStringLiteral("source"), source);
        root.insert(QStringLiteral("count"), windows.size());
        root.insert(QStringLiteral("windows"), entries);
        out << QJsonDocument(root).toJson(QJsonDocument::Compact) << '\n';
        out.flush();
        return 0;
    }

    // 默认去向取自设置页"存储 > 无头模式"；未显式指定时绝不写入剪贴板。
    const HeadlessCaptureConfig headless =
        headlessCaptureConfigFromRoot(readAppConfigRoot());
    CaptureDestination destination = captureDestinationForConfig(headless.defaultDestination);
    if (hasDestination && !parseDestination(parser.value(QStringLiteral("capture-destination")), &destination)) {
        err << "invalid --capture-destination \"" << parser.value(QStringLiteral("capture-destination"))
            << "\" (expected inline, file, stage or clipboard).\n";
        return 1;
    }
    if (destination == CaptureDestination::File && captureTo.isEmpty()) {
        err << "--capture-destination file requires --capture-to <directory>.\n";
        return 1;
    }

    // 防呆：无头模式下剪贴板写权限默认关闭（设置页需输入口令才可开启）。
    // 显式请求 clipboard 但未获授权时，降级为配置的默认去向并给出警告；
    // 降级后退出码保持非零，让依赖剪贴板的脚本/Agent 能检测到并自行处理。
    bool clipboardBlocked = false;
    if (destination == CaptureDestination::Clipboard && !headless.clipboardAllowed) {
        clipboardBlocked = true;
        destination = captureDestinationForConfig(headless.defaultDestination);
        err << "warning: clipboard writes are disabled for headless mode. "
               "Enable them in Settings > Storage > Headless Mode if needed. "
               "Falling back to \""
            << captureDestinationName(destination) << "\".\n";
    }

    QVector<WindowSelection> selections;
    QString parseError;
    if (!parseSelections(selectors, &selections, &parseError)) {
        err << parseError << '\n';
        return 1;
    }

    QJsonArray captures;
    bool anyFailed = false;
    for (const WindowSelection &selection : selections) {
        int matchedCount = 0;
        for (int i = 0; i < windows.size(); ++i) {
            if (!windowMatches(windows.at(i), i, selection.windowSpec, windowBy)) {
                continue;
            }
            const QJsonObject one = captureOne(windows.at(i),
                                               i,
                                               selection.selector,
                                               destination,
                                               captureTo,
                                               outputName,
                                               includeCursor,
                                               selection.subRect,
                                               &err);
            if (one.value(QStringLiteral("error")).isString()) {
                anyFailed = true;
            }
            captures.append(one);
            ++matchedCount;
        }
        if (matchedCount == 0) {
            anyFailed = true;
            QJsonObject missing;
            missing.insert(QStringLiteral("selector"), selection.selector);
            missing.insert(QStringLiteral("path"), QJsonValue::Null);
            missing.insert(QStringLiteral("error"),
                           QStringLiteral("no window matched selector \"%1\"").arg(selection.windowSpec));
            captures.append(missing);
        }
    }

    QJsonObject root;
    root.insert(QStringLiteral("platform"), platformName());
    root.insert(QStringLiteral("destination"), captureDestinationName(destination));
    if (clipboardBlocked) {
        root.insert(QStringLiteral("warning"),
                    QStringLiteral("clipboard writes are disabled for headless mode; "
                                   "destination downgraded to %1")
                        .arg(captureDestinationName(destination)));
    }
    root.insert(QStringLiteral("captures"), captures);
    out << QJsonDocument(root).toJson(QJsonDocument::Compact) << '\n';
    out.flush();
    return (anyFailed || clipboardBlocked) ? 1 : 0;
}

} // namespace markshot::cli
