#pragma once

#include "settings/settings_config.h"

#include <QWidget>

namespace markshot::settings {

/// @brief 关于页：展示产品信息、维护方、开源仓库与上游致谢。
class SettingsPageAbout final : public QWidget {
public:
    /// @brief 创建关于页。
    /// @param parent 父控件。
    explicit SettingsPageAbout(QWidget *parent = nullptr);

    /// @brief 占位实现（关于页没有可配置项）。
    /// @param config 设置配置。
    void setConfig(const SettingsConfig &config);

    /// @brief 占位实现（关于页没有可配置项）。
    /// @param config 设置配置。
    void updateConfig(SettingsConfig *config) const;

    /// @brief 关于页从不包含未保存的修改。
    /// @return 恒为 false。
    bool isModified() const;
};

}  // namespace markshot::settings
