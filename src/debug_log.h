#pragma once

#include <cstdarg>

namespace markshot {

// True when the DEBUG environment variable is set to a truthy value. The result
// is computed once and cached. When enabled, debugLog writes detailed
// diagnostics to stderr and to /tmp/mark-shot-scroll.log so a scrolling capture
// session can be replayed after the fact.
bool debugEnabled();

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
