#pragma once

#include "settings/settings_config.h"
#include "settings/settings_ui_helpers.h"

#include <QList>
#include <QWidget>

#include <array>

class QFormLayout;

namespace markshot::settings {

class SettingsPageShortcuts final : public QWidget {
public:
    /// @brief 创建快捷键设置页。
    /// @param parent 父控件。
    explicit SettingsPageShortcuts(QWidget *parent = nullptr);

    /// @brief 将配置加载到页面控件。
    /// @param config 设置配置。
    void setConfig(const SettingsConfig &config);

    /// @brief 将页面控件值写回配置。
    /// @param config 需要更新的设置配置。
    void updateConfig(SettingsConfig *config) const;

    /// @brief 判断页面是否有未保存的修改。
    /// @return 有未保存修改时返回 true。
    bool isModified() const;

private:
    /// @brief 初始化工具快捷键输入项。
    /// @param form 目标表单布局。
    void addToolShortcutRows(QFormLayout *form);

    /// @brief 初始化动作快捷键输入项。
    /// @param form 目标表单布局。
    void addActionShortcutRows(QFormLayout *form);

    /// @brief 连接所有快捷键输入项的冲突检测。
    /// @return 无返回值。
    void connectShortcutConflictChecks();

    /// @brief 全量重扫所有快捷键项的重复冲突并刷新提示。
    /// 程序化 setKeySequence（加载/还原）不触发 keySequenceChanged，
    /// 因此需要在此类路径结束时显式调用。
    void refreshShortcutConflicts();

    /// @brief 收集页面内所有快捷键输入控件。
    /// @return 快捷键输入控件列表。
    QList<ShortcutKeySequenceEdit *> allShortcutEdits() const;

    std::array<ShortcutKeySequenceEdit *, static_cast<int>(ShotWindow::Tool::Laser) + 1> m_toolEdits = {};
    std::array<ShortcutKeySequenceEdit *, static_cast<int>(ShotWindow::Action::Cancel) + 1> m_actionEdits = {};
    ShortcutKeySequenceEdit *m_startupColorPicker = nullptr;
    ShortcutKeySequenceEdit *m_startupRuler = nullptr;
    ShortcutKeySequenceEdit *m_startupCodeScanner = nullptr;
    ShortcutKeySequenceEdit *m_startupDisplayCapture = nullptr;
    ShortcutKeySequenceEdit *m_startupGifRecorder = nullptr;
    ShortcutKeySequenceEdit *m_startupVideoRecorder = nullptr;
    SettingsConfig m_saved;
};

}  // namespace markshot::settings
