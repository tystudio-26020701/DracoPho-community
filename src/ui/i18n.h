#pragma once

#include <QObject>
#include <QString>

namespace markshot::i18n {

enum class Language {
    English,
    Chinese,            // 简体中文
    TraditionalChinese, // 繁體中文
    Japanese,           // 日本語
    Korean,             // 한국어
    Russian,            // Русский
    Italian,            // Italiano
    Arabic,             // العربية (RTL)
    French,             // Français
    German,             // Deutsch
    Spanish,            // Español
    Portuguese,         // Português
};

/// @brief 语言切换通知器。
///
/// 语言在设置页切换后会立即应用，但常驻 UI（系统托盘菜单、悬浮球菜单、
/// 已打开的录制配置窗口等）的文案是在构造时固化的。这些窗口订阅
/// languageChanged 信号即可在切换后立即重新翻译，无需重启应用。
class LanguageChangeNotifier final : public QObject {
    Q_OBJECT

public:
    /// @brief 返回全局唯一的语言切换通知器。
    /// @return 通知器实例。
    static LanguageChangeNotifier &instance();

signals:
    /// @brief 语言已切换（每次 setLanguage 生效后发出）。
    void languageChanged();

private:
    explicit LanguageChangeNotifier(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
};

// Detects the UI language from the MARK_SHOT_LANG environment variable, then
// falls back to the system locale. Call once after QApplication is created.
void initialize();

void setLanguage(Language language);
Language language();

// Returns the localized text for the given English source string. Unknown
// strings are returned unchanged, so the English text doubles as the lookup
// key and any missing translation falls back cleanly.
QString translate(const QString &source);

// Returns the display name of a language in its own language (used by the
// Interface Language selector, e.g. "English", "简体中文", "日本語").
QString languageDisplayName(Language language);

}  // namespace markshot::i18n

// Convenience wrapper for translating compile-time literals at call sites.
#define MS_TR(text) (::markshot::i18n::translate(QStringLiteral(text)))
