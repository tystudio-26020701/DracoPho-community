#pragma once

#include "settings/settings_config.h"
#include "settings/settings_ui_helpers.h"

#include <QWidget>

#include <functional>

class QCheckBox;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;

namespace markshot::settings {

class SettingsPageAdvanced final : public QWidget {
public:
    /// @brief 创建高级设置页。
    /// @param parent 父控件。
    explicit SettingsPageAdvanced(QWidget *parent = nullptr);

    /// @brief 将配置加载到页面控件。
    /// @param config 设置配置。
    void setConfig(const SettingsConfig &config);

    /// @brief 将页面控件值写回配置。
    /// @param config 需要更新的设置配置。
    void updateConfig(SettingsConfig *config) const;

    /// @brief 注册"还原原始设置"成功后的重载回调。
    /// @param handler 回调，通常用于让设置对话框重新读取配置。
    void setRestoreOriginalHandler(const std::function<void()> &handler);

private:
    /// @brief 执行"还原原始设置"：确认后还原出厂默认值并重载配置。
    void restoreOriginalSettings();

    QCheckBox *m_debugEnabled = nullptr;
    QLineEdit *m_debugLogPath = nullptr;
    QCheckBox *m_windowDetectionEnabled = nullptr;
    QLineEdit *m_windowDetectionCommand = nullptr;
    QLineEdit *m_windowDetectionWorkingDirectory = nullptr;
    QSpinBox *m_windowDetectionTimeoutMs = nullptr;
    QPlainTextEdit *m_windowDetectionEnv = nullptr;
    QPlainTextEdit *m_appEnv = nullptr;
    SettingsConfig m_saved;
    std::function<void()> m_onRestoreOriginal;
};

}  // namespace markshot::settings
