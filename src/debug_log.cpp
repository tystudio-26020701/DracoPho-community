#include "debug_log.h"

#include <QByteArray>
#include <QDateTime>
#include <QDir>
#include <QString>

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace markshot {

namespace {

/// @brief Evaluates whether debug logging is enabled based on the environment.
/// @return True if debug logging is enabled, false otherwise.
bool computeDebugEnabled()
{
    const char *raw = std::getenv("DEBUG");
    if (!raw || raw[0] == '\0') {
        return false;
    }
    // Accept the common truthy spellings; treat "0", "false", "off", "no" and an
    // empty value as disabled so `DEBUG=0` does not accidentally turn it on.
    if (std::strcmp(raw, "0") == 0) {
        return false;
    }
    const QByteArray value = QByteArray(raw).trimmed().toLower();
    return value != "false" && value != "off" && value != "no" && !value.isEmpty();
}

/// @brief Retrieves the file path for debug logs.
/// @return A pointer to a character array representing the log file path.
const char *logFilePath()
{
    // Allow redirecting the log file; default keeps the historical scroll log
    // path so existing tooling that tails it keeps working.
    static const QByteArray path = [] {
        const char *override = std::getenv("MARK_SHOT_DEBUG_LOG");
        if (override && override[0] != '\0') {
            return QByteArray(override);
        }
        return QDir::temp().filePath(QStringLiteral("mark-shot-scroll.log")).toLocal8Bit();
    }();
    return path.constData();
}

}  // namespace

bool debugEnabled()
{
    static const bool enabled = computeDebugEnabled();
    return enabled;
}

/// @brief Helper to write formatted debug log messages to stderr and/or the debug log file.
/// @param category The logging category or tag.
/// @param format The format string for the log message.
/// @param args The variable arguments list.
void debugLogV(const char *category, const char *format, va_list args)
{
    if (!debugEnabled()) {
        return;
    }

    const QByteArray stamp =
        QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz")).toUtf8();
    const char *tag = category ? category : "log";

    // Render the message once so the same text reaches both sinks.
    char message[2048];
    std::vsnprintf(message, sizeof(message), format, args);

    if (FILE *file = std::fopen(logFilePath(), "a")) {
        std::fprintf(file, "%s [%s] %s\n", stamp.constData(), tag, message);
        std::fclose(file);
    }
    std::fprintf(stderr, "[mark-shot][%s] %s\n", tag, message);
    std::fflush(stderr);
}

void debugLog(const char *category, const char *format, ...)
{
    if (!debugEnabled()) {
        return;
    }
    va_list args;
    va_start(args, format);
    debugLogV(category, format, args);
    va_end(args);
}

}  // namespace markshot
