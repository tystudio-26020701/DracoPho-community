#pragma once

#include "settings/settings_config.h"

#include <QWidget>

class QComboBox;
class QLineEdit;
class QSpinBox;

namespace markshot::settings {

class SettingsPageStorage final : public QWidget {
public:
    /// @brief 创建保存与剪贴板设置页。
    /// @param parent 父控件。
    explicit SettingsPageStorage(QWidget *parent = nullptr);

    /// @brief 将配置加载到页面控件。
    /// @param config 设置配置。
    void setConfig(const SettingsConfig &config);

    /// @brief 将页面控件值写回配置。
    /// @param config 需要更新的设置配置。
    void updateConfig(SettingsConfig *config) const;

private:
    QLineEdit *m_savePathTemplate = nullptr;
    QComboBox *m_clipboardMode = nullptr;
    QSpinBox *m_clipboardThresholdM = nullptr;
};

}  // namespace markshot::settings
