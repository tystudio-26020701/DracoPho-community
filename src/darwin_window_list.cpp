#include "darwin_window_list.h"

#if defined(Q_OS_DARWIN)

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CGGeometry.h>
#include <CoreGraphics/CGWindow.h>

namespace markshot {
namespace {

QString cfStringToQString(CFStringRef value)
{
    if (!value) {
        return QString();
    }
    const CFIndex length = CFStringGetLength(value);
    const CFIndex maxSize = CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8);
    QByteArray buffer(static_cast<int>(maxSize + 1), '\0');
    if (!CFStringGetCString(value, buffer.data(), static_cast<CFIndex>(buffer.size()), kCFStringEncodingUTF8)) {
        return QString();
    }
    return QString::fromUtf8(buffer.constData());
}

std::optional<int> cfIntForKey(CFDictionaryRef dictionary, CFStringRef key)
{
    CFNumberRef number = static_cast<CFNumberRef>(CFDictionaryGetValue(dictionary, key));
    int value = 0;
    if (!number || !CFNumberGetValue(number, kCFNumberIntType, &value)) {
        return std::nullopt;
    }
    return value;
}

} // namespace

QVector<WindowInfo> enumerateDarwinWindowInfos(bool includeHidden)
{
    Q_UNUSED(includeHidden);
    QVector<WindowInfo> result;

    const CFArrayRef windowList = CGWindowListCopyWindowInfo(
        kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
        kCGNullWindowID);
    if (!windowList) {
        return result;
    }

    const CFIndex count = CFArrayGetCount(windowList);
    int z = 0;
    for (CFIndex i = 0; i < count; ++i) {
        const CFDictionaryRef info =
            static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(windowList, i));
        if (!info) {
            continue;
        }

        const std::optional<int> layer = cfIntForKey(info, kCGWindowLayer);
        // 只保留普通窗口层（0）；菜单/Dock/悬浮层对截图选窗无意义。
        if (!layer.has_value() || layer.value() != 0) {
            continue;
        }

        const std::optional<int> windowNumber = cfIntForKey(info, kCGWindowNumber);
        if (!windowNumber.has_value()) {
            continue;
        }

        WindowInfo entry;
        entry.id = QStringLiteral("0x%1").arg(static_cast<uint>(windowNumber.value()), 0, 16);
        entry.title = cfStringToQString(static_cast<CFStringRef>(CFDictionaryGetValue(info, kCGWindowName)));
        entry.className = cfStringToQString(static_cast<CFStringRef>(CFDictionaryGetValue(info, kCGWindowOwnerName)));
        entry.instance = entry.className;
        const std::optional<int> pid = cfIntForKey(info, kCGWindowOwnerPID);
        entry.pid = pid.has_value() ? pid.value() : -1;

        const CFDictionaryRef bounds =
            static_cast<CFDictionaryRef>(CFDictionaryGetValue(info, kCGWindowBounds));
        if (bounds) {
            CGRect rect = CGRectNull;
            if (CGRectMakeWithDictionaryRepresentation(bounds, &rect)) {
                entry.rect = QRect(qRound(rect.origin.x),
                                   qRound(rect.origin.y),
                                   qRound(rect.size.width),
                                   qRound(rect.size.height));
            }
        }

        if (entry.rect.isEmpty()) {
            continue;
        }
        entry.zOrder = z++;
        result.append(entry);
    }

    CFRelease(windowList);
    return result;
}

} // namespace markshot

#endif // Q_OS_DARWIN
