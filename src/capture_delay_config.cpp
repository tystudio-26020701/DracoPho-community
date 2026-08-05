#include "capture_delay_config.h"

#include "app_config_store.h"
#include "config_value.h"

#include <algorithm>

namespace {

QJsonValue delayValue(const QJsonObject &root)
{
    const QJsonObject capture =
        markshot::config::firstNonEmptyObjectValue(root,
                                                   {QStringLiteral("capture"),
                                                    QStringLiteral("screenshot"),
                                                    QStringLiteral("screenCapture")});
    const QJsonValue nestedValue =
        markshot::config::valueForKeys(capture,
                                       {QStringLiteral("delaySeconds"),
                                        QStringLiteral("captureDelaySeconds")});
    if (!nestedValue.isUndefined() && !nestedValue.isNull()) {
        return nestedValue;
    }
    return QJsonValue();
}

}  // namespace

namespace markshot {

/// @brief 延时秒数的上限（与 CLI --delay 的 0-3600 契约一致）。
constexpr int kMaxCaptureDelaySeconds = 3600;

int defaultCaptureDelaySeconds()
{
    return 0;
}

int captureDelaySecondsFromConfigRoot(const QJsonObject &root)
{
    const std::optional<int> value = config::intValue(delayValue(root));
    if (!value.has_value()) {
        return defaultCaptureDelaySeconds();
    }
    return std::clamp(*value, 0, kMaxCaptureDelaySeconds);
}

int configuredCaptureDelaySeconds()
{
    bool ok = false;
    const QJsonObject root = readAppConfigRoot(&ok);
    if (!ok) {
        return defaultCaptureDelaySeconds();
    }
    return captureDelaySecondsFromConfigRoot(root);
}

}  // namespace markshot
