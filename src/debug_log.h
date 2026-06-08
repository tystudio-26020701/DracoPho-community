#pragma once

#include <cstdarg>

#include <QString>

namespace markshot {

// Configures debug logging after startup config and command-line options have
// been parsed. Before this is called, DEBUG and MARK_SHOT_DEBUG_LOG keep their
// historical behavior.
void configureDebugLogging(bool enabled, const QString &logPath = QString());

// True when debug logging is enabled by runtime configuration or by the DEBUG
// environment variable. When enabled, debugLog writes detailed diagnostics to
// stderr and to debugLogPath().
bool debugEnabled();
QString debugLogPath();

// printf-style diagnostic logging, gated by debugEnabled(). The category is a
// short tag (for example "capture", "session", "stitch") used to prefix each
// line so logs from different subsystems can be told apart.
void debugLog(const char *category, const char *format, ...)
#if defined(__GNUC__)
    __attribute__((format(printf, 2, 3)))
#endif
    ;

// va_list form, so existing variadic wrappers can forward to it without losing
// their own printf-format checking.
void debugLogV(const char *category, const char *format, va_list args);

}  // namespace markshot
