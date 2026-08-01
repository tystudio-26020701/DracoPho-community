#pragma once

#include "settings/settings_config.h"
#include "settings/settings_ui_helpers.h"

#include <QWidget>

#include <functional>

class QCheckBox;
class QComboBox;

namespace markshot::settings {

class SettingsPageGeneral final : public QWidget {
public:
    /// @brief 创建通用设置页。
    /// @param parent 父控件。
    explicit SettingsPageGeneral(QWidget *parent = nullptr);

    /// @brief 将配置加载到页面控件。
    /// @param config 设置配置。
    void setConfig(const SettingsConfig &config);

    /// @brief 将页面控件值写回配置。
    /// @param config 需要更新的设置配置。
    void updateConfig(SettingsConfig *config) const;

    /// @brief 判断页面是否有未保存的修改。
    /// @return 有未保存修改时返回 true。
    bool isModified() const;

    /// @brief 设置界面语言变化回调（由设置对话框用于即时切换语言）。
    /// @param handler 语言模式变化回调。
    void setLanguageModeHandler(const std::function<void(markshot::ui::UiLanguageMode)> &handler);

private:
    /// @brief 检测两个全局快捷键之间的冲突并给出提示。
    /// @param primary 刚发生变化的快捷键控件。
    /// @param other 另一个全局快捷键控件。
    void checkGlobalHotkeyConflict(QKeySequenceEdit *primary, QKeySequenceEdit *other);

    /// @brief 通知外部界面语言模式发生变化。
    void notifyLanguageModeChanged();

    QComboBox *m_uiLanguage = nullptr;
    QComboBox *m_uiTheme = nullptr;
    QCheckBox *m_trayEnabled = nullptr;
    QCheckBox *m_launchOnStartup = nullptr;
    QCheckBox *m_hotkeysEnabled = nullptr;
    ShortcutKeySequenceEdit *m_captureHotkey = nullptr;
    ShortcutKeySequenceEdit *m_fullscreenHotkey = nullptr;
    SettingsConfig m_saved;
    std::function<void(markshot::ui::UiLanguageMode)> m_onLanguageModeChanged;
};

}  // namespace markshot::settings
