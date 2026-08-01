#include "headless_capture_config.h"

#include "config_value.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

namespace {

/// @brief 解析无头去向名称。
/// @param value 配置值。
/// @param fallback 解析失败时的默认值。
/// @return 无头去向枚举。
markshot::HeadlessCaptureDestination destinationFromString(const QString &value,
                                                           markshot::HeadlessCaptureDestination fallback)
{
    const QString normalized = markshot::config::normalizedKey(value);
    if (normalized == QStringLiteral("inline") || normalized == QStringLiteral("base64")) {
        return markshot::HeadlessCaptureDestination::Inline;
    }
    if (normalized == QStringLiteral("stage") || normalized == QStringLiteral("staging")
        || normalized == QStringLiteral("temp")) {
        return markshot::HeadlessCaptureDestination::Stage;
    }
    return fallback;
}

/// @brief 从配置根对象读取无头配置对象。
/// @param root 应用配置根对象。
/// @return 无头配置对象。
QJsonObject headlessObject(const QJsonObject &root)
{
    return markshot::config::firstObjectValue(
        root,
        {QStringLiteral("headless"), QStringLiteral("headlessMode"), QStringLiteral("cli")});
}

}  // namespace

namespace markshot {

QString headlessCaptureDestinationName(HeadlessCaptureDestination destination)
{
    switch (destination) {
    case HeadlessCaptureDestination::Inline:
        return QStringLiteral("inline");
    case HeadlessCaptureDestination::Stage:
        return QStringLiteral("stage");
    }
    return QStringLiteral("inline");
}

HeadlessCaptureConfig headlessCaptureConfigFromRoot(const QJsonObject &root)
{
    HeadlessCaptureConfig config;
    const QJsonObject headless = headlessObject(root);
    if (headless.isEmpty()) {
        return config;
    }

    config.defaultDestination =
        destinationFromString(config::valueForKeys(headless,
                                                   {QStringLiteral("defaultDestination"),
                                                    QStringLiteral("destination"),
                                                    QStringLiteral("defaultDestinationMode")})
                                  .toString(),
                              HeadlessCaptureDestination::Inline);

    if (const std::optional<bool> allowed = config::boolValue(
            config::valueForKeys(headless,
                                 {QStringLiteral("clipboardAllowed"),
                                  QStringLiteral("allowClipboard"),
                                  QStringLiteral("allowClipboardWrite"),
                                  QStringLiteral("clipboard")}))) {
        config.clipboardAllowed = *allowed;
    }
    return config;
}

void writeHeadlessCaptureConfig(QJsonObject *root, const HeadlessCaptureConfig &config)
{
    if (!root) {
        return;
    }
    QJsonObject headless = root->value(QStringLiteral("headless")).toObject();
    headless.insert(QStringLiteral("defaultDestination"),
                    headlessCaptureDestinationName(config.defaultDestination));
    headless.insert(QStringLiteral("clipboardAllowed"), config.clipboardAllowed);
    root->insert(QStringLiteral("headless"), headless);
}

}  // namespace markshot
