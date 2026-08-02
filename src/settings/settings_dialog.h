#pragma once

#include "settings/settings_config.h"

#include <QDialog>
#include <QVector>

#include <optional>

class QCloseEvent;
class QEvent;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QStackedWidget;
class QWidget;

namespace markshot::settings {

class SettingsNavigation;
class SettingsPageAbout;
class SettingsPageAnnotation;
class SettingsPageAdvanced;
class SettingsPageCapture;
class SettingsPageGeneral;
class SettingsPageIntegrations;
class SettingsPagePinned;
class SettingsPagePlugins;
class SettingsPageScroll;
class SettingsPageShortcuts;
class SettingsPageStorage;

class SettingsDialog final : public QDialog {
public:
    /// @brief 创建设置窗口。
    /// @param parent 父控件。
    explicit SettingsDialog(QWidget *parent = nullptr);

    /// @brief 析构：从应用事件过滤器列表移除自身，避免悬垂指针。
    ~SettingsDialog() override;

    /// @brief 关闭窗口时的防呆确认（未保存修改时弹出保存/放弃/继续编辑）。
    /// @param event 关闭事件。
    void closeEvent(QCloseEvent *event) override;

    /// @brief 复用窗口重新打开时刷新：重读配置并按当前语言重建页面。
    void reloadForDisplay();

    /// @brief 请求刷新"未保存修改"标记（供设置页在无值信号路径——
    /// 如取色对话框改变颜色——完成后通知对话框刷新脏状态）。
    void notifyConfigChanged();

    /// @brief 处理 Escape 键（QDialog::reject），同样走未保存修改确认。
    void reject() override;

protected:
    /// @brief 捕获输入变化事件，用于刷新"未保存修改"标记。
    /// @param watched 事件源对象。
    /// @param event 事件。
    /// @return 是否已处理。
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    /// @brief 未保存修改确认的抉择。
    enum class CloseChoice {
        SaveAndClose,
        DiscardAndClose,
        KeepEditing,
    };

    /// @brief 从配置文件加载设置并更新所有页面。
    void loadConfig();

    /// @brief 将配置应用到全部设置页（并记录为各页的"已保存"基线）。
    /// @param config 需要应用的设置结构。
    void applyConfigToPages(const SettingsConfig &config);

    /// @brief 从所有页面收集控件值。
    /// @return 设置结构。
    SettingsConfig collectConfig() const;

    /// @brief 保存当前设置。
    /// @param closeAfterSave 保存成功后是否关闭窗口。
    /// @return 保存成功返回 true。
    bool saveConfig(bool closeAfterSave);

    /// @brief 应用设置界面主题。
    /// @param mode 配置中的界面主题模式。
    void applyTheme(markshot::ui::UiThemeMode mode);

    /// @brief 应用界面语言模式（即时生效于设置窗口，其他窗口重启后生效）。
    /// @param mode 用户选择的语言模式。
    void applyLanguageMode(markshot::ui::UiLanguageMode mode);

    /// @brief 按当前语言重建全部设置页（语言切换后重新翻译）。
    void rebuildPages();

    /// @brief 创建全部设置页并加入内容栈（构造与重建共用）。
    void createPagesAndPopulate();

    /// @brief 判断是否存在未保存的修改。
    /// @return 有未保存修改时返回 true。
    bool hasUnsavedChanges() const;

    /// @brief 刷新各页"未保存修改"标记、页脚提示与窗口标题。
    void refreshDirtyState();

    /// @brief 延迟调度一次脏状态刷新（合并同一轮内的多次输入）。
    void scheduleDirtyRefresh();

    /// @brief 连接全部设置控件的值变更信号到脏状态刷新。
    /// 覆盖下拉框/开关/数值框/文本框/快捷键框等控件，保证任何输入方式
    /// （含下拉框弹出层选择）都会触发脏状态刷新，避免状态栏滞后。
    void connectDirtyRefreshSignals();

    /// @brief 放弃所有未保存修改（含语言预览）并恢复已保存语言。
    void discardUnsavedChanges();

    /// @brief 弹出"未保存修改"三选一确认框。
    /// @return 用户的抉择。
    CloseChoice confirmUnsavedChanges();

    /// @brief 安装脏状态检测事件过滤器。
    void installDirtyTracker();

    /// @brief 重建侧栏导航（语言切换后重新翻译）。
    void rebuildNavigation();

    /// @brief 连接导航信号（构造与重建导航后共用）。
    void connectNavigationSignals();

    /// @brief 刷新页脚按钮文本（语言切换后重新翻译）。
    void retranslateFooter();

    SettingsNavigation *m_navigation = nullptr;
    QHBoxLayout *m_bodyLayout = nullptr;
    QStackedWidget *m_stack = nullptr;
    QLabel *m_statusLabel = nullptr;
    QPushButton *m_applyButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
    SettingsConfig m_config;
    std::optional<markshot::ui::UiLanguageMode> m_pendingLanguageMode;
    bool m_rebuilding = false;
    bool m_dirtyRefreshPending = false;
    QVector<bool> m_lastDirty;
    bool m_lastAnyDirty = false;

    SettingsPageGeneral *m_generalPage = nullptr;
    SettingsPageCapture *m_capturePage = nullptr;
    SettingsPageShortcuts *m_shortcutsPage = nullptr;
    SettingsPageAnnotation *m_annotationPage = nullptr;
    SettingsPagePinned *m_pinnedPage = nullptr;
    SettingsPageIntegrations *m_integrationsPage = nullptr;
    SettingsPagePlugins *m_pluginsPage = nullptr;
    SettingsPageScroll *m_scrollPage = nullptr;
    SettingsPageStorage *m_storagePage = nullptr;
    SettingsPageAdvanced *m_advancedPage = nullptr;
    SettingsPageAbout *m_aboutPage = nullptr;
};

/// @brief 显示全局设置窗口，重复调用会复用现有窗口。
/// @param parent 用于定位窗口的父控件。
void showSettingsDialog(QWidget *parent = nullptr);

}  // namespace markshot::settings
