#include "cli/headless_capture_options.h"

namespace markshot::cli {

QString screenStageDirectoryName()
{
    return QStringLiteral("dracoPho-staging");
}

std::optional<int> parseHeadlessDelaySeconds(const QString &value)
{
    bool ok = false;
    const int seconds = value.trimmed().toInt(&ok);
    if (!ok || seconds < 0 || seconds > 3600) {
        return std::nullopt;
    }
    return seconds;
}

std::optional<ScreenCaptureDestination> parseScreenDestination(const QString &name)
{
    const QString normalized = name.trimmed().toLower();
    if (normalized == QLatin1String("file")) {
        return ScreenCaptureDestination::File;
    }
    if (normalized == QLatin1String("inline")) {
        return ScreenCaptureDestination::Inline;
    }
    if (normalized == QLatin1String("stage")) {
        return ScreenCaptureDestination::Stage;
    }
    return std::nullopt;
}

QString headlessInteractiveConflict(const QCommandLineParser &parser)
{
    // --capture-window 启动交互式全屏悬停选择界面；无头选项会改变它的语义
    // 或被静默忽略。此处逐项点名，让脚本/智能体一次拿到可执行的修正建议。
    static const struct {
        const char *flag;
        const char *advice;
    } kConflicts[] = {
        {"capture-to",
         "use \"--window <selector> --capture-destination file --capture-to <dir>\" for headless window capture"},
        {"capture-screen",
         "\"--capture-screen\" is a headless trigger; interactive window capture does not take it"},
        {"region", "use \"--capture-to <path> --region x,y,w,h\" for headless region capture"},
        {"display", "use \"--capture-to <dir> --display <name>\" for headless display capture"},
        {"all-outputs", "use \"--capture-to <dir> --all-outputs\" for headless multi-display capture"},
        {"list-displays", "run \"--list-displays\" on its own for the JSON display list"},
        {"window", "interactive window capture takes no \"--window\" selector; drop it or drop \"--capture-window\""},
        {"list-windows", "run \"--list-windows\" on its own for the JSON window list"},
        {"window-by", "\"--window-by\" only applies to headless \"--window\" captures"},
        {"capture-destination", "\"--capture-destination\" only applies to headless \"--window\"/\"--capture-to\"/\"--capture-screen\" captures"},
        {"output-name", "\"--output-name\" only applies to headless captures"},
        {"include-cursor", "\"--include-cursor\" only applies to headless captures"},
    };
    for (const auto &conflict : kConflicts) {
        if (parser.isSet(QLatin1String(conflict.flag))) {
            return QStringLiteral("--capture-window cannot be combined with --%1. %2")
                .arg(QLatin1String(conflict.flag), QLatin1String(conflict.advice));
        }
    }
    return QString();
}

QString doctorOptionConflict(const QCommandLineParser &parser)
{
    // --doctor 是纯只读自检：与任何捕获、录制、窗口或文件参数组合都属于
    // 用法错误（此前 doctor 会静默优先，吞掉其余选项）。
    static const char *kExclusiveFlags[] = {
        "capture-to",       "capture-screen",    "region",            "display",
        "all-outputs",      "list-displays",     "window",            "list-windows",
        "window-by",        "capture-destination", "output-name",     "include-cursor",
        "delay",            "capture-window",    "record-region",     "record-display",
        "record-duration",  "record-output",     "record-fps",        "record-format",
        "record-audio",     "record-wait-json",  "recording-status",  "stop-recording",
    };
    for (const char *flag : kExclusiveFlags) {
        if (parser.isSet(QLatin1String(flag))) {
            return QStringLiteral("--doctor cannot be combined with --%1. "
                                  "Run --doctor on its own for the environment self-check.")
                .arg(QLatin1String(flag));
        }
    }
    if (!parser.positionalArguments().isEmpty()) {
        return QStringLiteral("--doctor cannot be combined with an image file argument.");
    }
    return QString();
}

} // namespace markshot::cli
