#include "ui/interface_language_config.h"

#include <QJsonValue>
#include <QLocale>

namespace markshot::ui {

UiLanguageMode uiLanguageModeFromString(const QString &raw)
{
    const QString value = raw.trimmed().toLower();
    if (value == QStringLiteral("english") || value == QStringLiteral("en")
        || value == QStringLiteral("en_us") || value == QStringLiteral("en-us")) {
        return UiLanguageMode::English;
    }
    if (value == QStringLiteral("zh_tw") || value == QStringLiteral("zh-tw")
        || value == QStringLiteral("zh_hk") || value == QStringLiteral("zh-hk")
        || value == QStringLiteral("traditional_chinese")
        || value == QStringLiteral("traditional")) {
        return UiLanguageMode::TraditionalChinese;
    }
    if (value == QStringLiteral("chinese") || value == QStringLiteral("zh")
        || value == QStringLiteral("zh_cn") || value == QStringLiteral("zh-cn")
        || value == QStringLiteral("cn")) {
        return UiLanguageMode::Chinese;
    }
    if (value == QStringLiteral("japanese") || value == QStringLiteral("ja")) {
        return UiLanguageMode::Japanese;
    }
    if (value == QStringLiteral("korean") || value == QStringLiteral("ko")
        || value == QStringLiteral("ko_kr") || value == QStringLiteral("ko-kr")) {
        return UiLanguageMode::Korean;
    }
    if (value == QStringLiteral("russian") || value == QStringLiteral("ru")) {
        return UiLanguageMode::Russian;
    }
    if (value == QStringLiteral("italian") || value == QStringLiteral("it")) {
        return UiLanguageMode::Italian;
    }
    if (value == QStringLiteral("arabic") || value == QStringLiteral("ar")) {
        return UiLanguageMode::Arabic;
    }
    if (value == QStringLiteral("french") || value == QStringLiteral("fr")) {
        return UiLanguageMode::French;
    }
    if (value == QStringLiteral("german") || value == QStringLiteral("de")) {
        return UiLanguageMode::German;
    }
    if (value == QStringLiteral("spanish") || value == QStringLiteral("es")) {
        return UiLanguageMode::Spanish;
    }
    if (value == QStringLiteral("portuguese") || value == QStringLiteral("pt")
        || value == QStringLiteral("pt_br") || value == QStringLiteral("pt-br")) {
        return UiLanguageMode::Portuguese;
    }
    return UiLanguageMode::System;
}

QString uiLanguageModeName(UiLanguageMode mode)
{
    switch (mode) {
    case UiLanguageMode::English:
        return QStringLiteral("english");
    case UiLanguageMode::Chinese:
        return QStringLiteral("chinese");
    case UiLanguageMode::TraditionalChinese:
        return QStringLiteral("zh_tw");
    case UiLanguageMode::Japanese:
        return QStringLiteral("japanese");
    case UiLanguageMode::Korean:
        return QStringLiteral("korean");
    case UiLanguageMode::Russian:
        return QStringLiteral("russian");
    case UiLanguageMode::Italian:
        return QStringLiteral("italian");
    case UiLanguageMode::Arabic:
        return QStringLiteral("arabic");
    case UiLanguageMode::French:
        return QStringLiteral("french");
    case UiLanguageMode::German:
        return QStringLiteral("german");
    case UiLanguageMode::Spanish:
        return QStringLiteral("spanish");
    case UiLanguageMode::Portuguese:
        return QStringLiteral("portuguese");
    case UiLanguageMode::System:
        break;
    }
    return QStringLiteral("system");
}

UiLanguageMode uiLanguageModeFromConfigRoot(const QJsonObject &root)
{
    const QJsonValue uiValue = root.value(QStringLiteral("ui"));
    if (uiValue.isObject()) {
        const QString value = uiValue.toObject().value(QStringLiteral("language")).toString();
        if (!value.trimmed().isEmpty()) {
            return uiLanguageModeFromString(value);
        }
    }

    const QString legacyValue = root.value(QStringLiteral("language")).toString();
    return uiLanguageModeFromString(legacyValue);
}

markshot::i18n::Language languageForUiLanguageMode(UiLanguageMode mode)
{
    switch (mode) {
    case UiLanguageMode::English:
        return markshot::i18n::Language::English;
    case UiLanguageMode::Chinese:
        return markshot::i18n::Language::Chinese;
    case UiLanguageMode::TraditionalChinese:
        return markshot::i18n::Language::TraditionalChinese;
    case UiLanguageMode::Japanese:
        return markshot::i18n::Language::Japanese;
    case UiLanguageMode::Korean:
        return markshot::i18n::Language::Korean;
    case UiLanguageMode::Russian:
        return markshot::i18n::Language::Russian;
    case UiLanguageMode::Italian:
        return markshot::i18n::Language::Italian;
    case UiLanguageMode::Arabic:
        return markshot::i18n::Language::Arabic;
    case UiLanguageMode::French:
        return markshot::i18n::Language::French;
    case UiLanguageMode::German:
        return markshot::i18n::Language::German;
    case UiLanguageMode::Spanish:
        return markshot::i18n::Language::Spanish;
    case UiLanguageMode::Portuguese:
        return markshot::i18n::Language::Portuguese;
    case UiLanguageMode::System:
        break;
    }

    switch (QLocale::system().language()) {
    case QLocale::Chinese: {
        // 繁体中文区域（台湾/香港/澳门）使用繁体，其余简体。
        const QLocale::Country country = QLocale::system().country();
        if (country == QLocale::Taiwan || country == QLocale::HongKong
            || country == QLocale::Macau) {
            return markshot::i18n::Language::TraditionalChinese;
        }
        return markshot::i18n::Language::Chinese;
    }
    case QLocale::Japanese:
        return markshot::i18n::Language::Japanese;
    case QLocale::Korean:
        return markshot::i18n::Language::Korean;
    case QLocale::Russian:
        return markshot::i18n::Language::Russian;
    case QLocale::Italian:
        return markshot::i18n::Language::Italian;
    case QLocale::Arabic:
        return markshot::i18n::Language::Arabic;
    case QLocale::French:
        return markshot::i18n::Language::French;
    case QLocale::German:
        return markshot::i18n::Language::German;
    case QLocale::Spanish:
        return markshot::i18n::Language::Spanish;
    case QLocale::Portuguese:
        return markshot::i18n::Language::Portuguese;
    default:
        return markshot::i18n::Language::English;
    }
}

}  // namespace markshot::ui
