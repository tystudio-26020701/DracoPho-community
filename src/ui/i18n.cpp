#include "ui/i18n.h"

#include "app_config_store.h"
#include "ui/i18n_tables.h"
#include "ui/interface_language_config.h"

#include <QGuiApplication>
#include <QHash>
#include <QProcessEnvironment>

namespace markshot::i18n {

namespace {

/// @brief Global language setting for the application interface.
Language g_language = Language::English;

/// @brief Parses a language enum from a raw string value.
/// @param raw The raw language name string.
/// @return The parsed Language enum value.
Language languageFromString(const QString &raw)
{
    const QString value = raw.trimmed().toLower();
    if (value.startsWith(QStringLiteral("zh-tw")) || value == QStringLiteral("zh_hk")
        || value == QStringLiteral("zh-hk") || value == QStringLiteral("zh_tw")
        || value == QStringLiteral("traditional")) {
        return Language::TraditionalChinese;
    }
    if (value.startsWith(QStringLiteral("zh")) || value == QStringLiteral("chinese")
        || value == QStringLiteral("cn")) {
        return Language::Chinese;
    }
    if (value == QStringLiteral("ja") || value == QStringLiteral("japanese")) {
        return Language::Japanese;
    }
    if (value == QStringLiteral("ko") || value == QStringLiteral("korean")) {
        return Language::Korean;
    }
    if (value == QStringLiteral("ru") || value == QStringLiteral("russian")) {
        return Language::Russian;
    }
    if (value == QStringLiteral("it") || value == QStringLiteral("italian")) {
        return Language::Italian;
    }
    if (value == QStringLiteral("ar") || value == QStringLiteral("arabic")) {
        return Language::Arabic;
    }
    if (value == QStringLiteral("fr") || value == QStringLiteral("french")) {
        return Language::French;
    }
    if (value == QStringLiteral("de") || value == QStringLiteral("german")) {
        return Language::German;
    }
    if (value == QStringLiteral("es") || value == QStringLiteral("spanish")) {
        return Language::Spanish;
    }
    if (value == QStringLiteral("pt") || value == QStringLiteral("portuguese")
        || value == QStringLiteral("pt_br") || value == QStringLiteral("pt-br")) {
        return Language::Portuguese;
    }
    return Language::English;
}

/// @brief Automatically detects the target system language using environment variables and system locale.
/// @return The detected Language enum value.
Language detectLanguage()
{
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString override = env.value(QStringLiteral("MARK_SHOT_LANG")).trimmed();
    if (!override.isEmpty()) {
        return languageFromString(override);
    }

    bool ok = false;
    const QJsonObject root = readAppConfigRoot(&ok);
    const markshot::ui::UiLanguageMode mode = ok
        ? markshot::ui::uiLanguageModeFromConfigRoot(root)
        : markshot::ui::UiLanguageMode::System;
    return markshot::ui::languageForUiLanguageMode(mode);
}

/// @brief 应用当前语言的文本方向（阿拉伯语为 RTL）。
void applyLayoutDirection(Language language)
{
    if (QGuiApplication *app = qApp) {
        app->setLayoutDirection(language == Language::Arabic ? Qt::RightToLeft : Qt::LeftToRight);
    }
}

/// @brief 返回当前语言对应的翻译表。
/// @return 翻译表指针；英文（源语言）时返回空指针，translate 直接返回原文。
const QHash<QString, QString> *currentTable()
{
    switch (g_language) {
    case Language::Chinese:
        return &tableZhCN();
    case Language::TraditionalChinese:
        return &tableZhTW();
    case Language::Japanese:
        return &tableJa();
    case Language::Korean:
        return &tableKo();
    case Language::Russian:
        return &tableRu();
    case Language::Italian:
        return &tableIt();
    case Language::Arabic:
        return &tableAr();
    case Language::French:
        return &tableFr();
    case Language::German:
        return &tableDe();
    case Language::Spanish:
        return &tableEs();
    case Language::Portuguese:
        return &tablePt();
    case Language::English:
        break;
    }
    return nullptr;
}

}  // namespace

void initialize()
{
    g_language = detectLanguage();
    applyLayoutDirection(g_language);
}

void setLanguage(Language language)
{
    g_language = language;
    applyLayoutDirection(language);
}

Language language()
{
    return g_language;
}

QString translate(const QString &source)
{
    if (const QHash<QString, QString> *table = currentTable()) {
        const auto it = table->constFind(source);
        if (it != table->constEnd()) {
            return it.value();
        }
    }
    return source;
}

QString languageDisplayName(Language language)
{
    switch (language) {
    case Language::English:
        return QStringLiteral("English");
    case Language::Chinese:
        return QStringLiteral("简体中文");
    case Language::TraditionalChinese:
        return QStringLiteral("繁體中文");
    case Language::Japanese:
        return QStringLiteral("日本語");
    case Language::Korean:
        return QStringLiteral("한국어");
    case Language::Russian:
        return QStringLiteral("Русский");
    case Language::Italian:
        return QStringLiteral("Italiano");
    case Language::Arabic:
        return QStringLiteral("العربية");
    case Language::French:
        return QStringLiteral("Français");
    case Language::German:
        return QStringLiteral("Deutsch");
    case Language::Spanish:
        return QStringLiteral("Español");
    case Language::Portuguese:
        return QStringLiteral("Português");
    }
    return QStringLiteral("English");
}

}  // namespace markshot::i18n
