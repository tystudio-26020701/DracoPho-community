#include "settings/settings_dialog.h"

#include "app_config_store.h"
#include "settings/settings_design_tokens.h"
#include "settings/settings_navigation.h"
#include "settings/settings_page_about.h"
#include "settings/settings_page_advanced.h"
#include "settings/settings_page_annotation.h"
#include "settings/settings_page_capture.h"
#include "settings/settings_page_general.h"
#include "settings/settings_page_integrations.h"
#include "settings/settings_page_pinned.h"
#include "settings/settings_page_plugins.h"
#include "settings/settings_page_scroll.h"
#include "settings/settings_page_shortcuts.h"
#include "settings/settings_page_storage.h"
#include "settings/settings_wheel_guard.h"
#include "ui/i18n.h"
#include "ui/icons.h"
#include "ui/interface_language_config.h"
#include "ui/interface_theme_config.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QPointer>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QScreen>
#include <QGuiApplication>
#include <QScrollArea>
#include <QStackedWidget>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

namespace markshot::settings {
namespace {

/// @brief 将设置页包装成可滚动页面。
/// @param stack 目标堆叠控件。
/// @param page 需要显示的设置页。
void addScrollablePage(QStackedWidget *stack, QWidget *page)
{
    auto *area = new QScrollArea(stack);
    area->setFrameShape(QFrame::NoFrame);
    area->setWidgetResizable(true);
    area->setWidget(page);
    stack->addWidget(area);
}

/**
 * 读取配置中的设置界面主题。
 * @return 配置的界面主题模式。
 */
markshot::ui::UiThemeMode configuredSettingsThemeMode()
{
    bool ok = false;
    const QJsonObject root = markshot::readAppConfigRoot(&ok);
    return ok ? markshot::ui::uiThemeModeFromConfigRoot(root)
              : markshot::ui::UiThemeMode::System;
}

/// @brief 事件类型集合中是否有需要刷新脏标记的类型。
/// @param type 事件类型。
/// @return 需要刷新时返回 true。
bool isDirtyRelevantEvent(QEvent::Type type)
{
    switch (type) {
    case QEvent::FocusOut:
    case QEvent::KeyRelease:
    case QEvent::MouseButtonRelease:
    case QEvent::Wheel:
        return true;
    default:
        return false;
    }
}

}  // namespace

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("settingsDialog"));
    setWindowTitle(MS_TR("Settings"));
    setWindowIcon(markshot::ui::applicationIcon());
    setMinimumSize(820, 600);
    resize(900, 640);

    applyTheme(configuredSettingsThemeMode());

    // 滚轮防护：未聚焦的下拉框/数值框不再被滚轮误改内容，页面照常滚动。
    installSettingsWheelGuard(this);
    // 脏状态跟踪：输入变化后刷新"未保存修改"标记。
    installDirtyTracker();

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *body = new QWidget(this);
    m_bodyLayout = new QHBoxLayout(body);
    m_bodyLayout->setContentsMargins(0, 0, 0, 0);
    m_bodyLayout->setSpacing(0);

    // 侧栏导航：标题区 + 分组分类列表
    m_navigation = new SettingsNavigation(body);
    m_bodyLayout->addWidget(m_navigation);

    // 内容栈：设置页 + 关于页
    m_stack = new QStackedWidget(body);
    createPagesAndPopulate();

    m_bodyLayout->addWidget(m_stack, 1);
    rootLayout->addWidget(body, 1);

    auto *footer = new QFrame(this);
    footer->setObjectName(QStringLiteral("settingsFooter"));
    auto *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(18, 10, 18, 10);
    m_statusLabel = new QLabel(MS_TR("Some changes take effect after restarting Mark Shot."), footer);
    m_statusLabel->setObjectName(QStringLiteral("settingsStatus"));
    footerLayout->addWidget(m_statusLabel, 1);
    auto *buttons = new QDialogButtonBox(footer);
    m_applyButton = buttons->addButton(MS_TR("Apply"), QDialogButtonBox::ApplyRole);
    m_saveButton = buttons->addButton(MS_TR("Save"), QDialogButtonBox::AcceptRole);
    m_cancelButton = buttons->addButton(MS_TR("Cancel"), QDialogButtonBox::RejectRole);
    m_saveButton->setProperty("role", QStringLiteral("primary"));
    footerLayout->addWidget(buttons);
    rootLayout->addWidget(footer);

    // 1. 导航切换驱动内容栈翻页
    connectNavigationSignals();
    // 2. 底部按钮：应用 / 保存 / 取消
    connect(m_applyButton, &QPushButton::clicked, this, [this] { saveConfig(false); });
    connect(m_saveButton, &QPushButton::clicked, this, [this] { saveConfig(true); });
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::close);

    m_navigation->setCurrentLogicalRow(0);
    loadConfig();
}

SettingsDialog::~SettingsDialog()
{
    // 与 WheelGuard 对称：析构时移除应用级事件过滤器，避免悬垂指针。
    if (QCoreApplication *app = QCoreApplication::instance()) {
        app->removeEventFilter(this);
    }
}

void SettingsDialog::connectNavigationSignals()
{
    connect(m_navigation, &SettingsNavigation::navigationChanged, m_stack, &QStackedWidget::setCurrentIndex);
    connect(m_navigation, &SettingsNavigation::navigationChanged, this, [this](int) {
        refreshDirtyState();
    });
}

void SettingsDialog::rebuildNavigation()
{
    if (m_navigation) {
        m_navigation->deleteLater();
    }
    m_navigation = new SettingsNavigation(this);
    m_bodyLayout->insertWidget(0, m_navigation);
    connectNavigationSignals();
}

void SettingsDialog::retranslateFooter()
{
    if (m_applyButton) {
        m_applyButton->setText(MS_TR("Apply"));
    }
    if (m_saveButton) {
        m_saveButton->setText(MS_TR("Save"));
    }
    if (m_cancelButton) {
        m_cancelButton->setText(MS_TR("Cancel"));
    }
}

void SettingsDialog::createPagesAndPopulate()
{
    m_generalPage = new SettingsPageGeneral(m_stack);
    m_capturePage = new SettingsPageCapture(m_stack);
    m_shortcutsPage = new SettingsPageShortcuts(m_stack);
    m_annotationPage = new SettingsPageAnnotation(m_stack);
    m_pinnedPage = new SettingsPagePinned(m_stack);
    m_integrationsPage = new SettingsPageIntegrations(m_stack);
    m_pluginsPage = new SettingsPagePlugins(m_stack);
    m_scrollPage = new SettingsPageScroll(m_stack);
    m_storagePage = new SettingsPageStorage(m_stack);
    m_advancedPage = new SettingsPageAdvanced(m_stack);
    m_aboutPage = new SettingsPageAbout(m_stack);
    // 高级页"还原原始设置"成功后，重新读取配置并刷新全部页面。
    m_advancedPage->setRestoreOriginalHandler([this] { loadConfig(); });
    // 通用页语言切换：即时应用到设置窗口（其他窗口重启后生效）。
    m_generalPage->setLanguageModeHandler([this](markshot::ui::UiLanguageMode mode) {
        applyLanguageMode(mode);
    });
    addScrollablePage(m_stack, m_generalPage);
    addScrollablePage(m_stack, m_capturePage);
    addScrollablePage(m_stack, m_shortcutsPage);
    addScrollablePage(m_stack, m_annotationPage);
    addScrollablePage(m_stack, m_pinnedPage);
    addScrollablePage(m_stack, m_integrationsPage);
    addScrollablePage(m_stack, m_pluginsPage);
    addScrollablePage(m_stack, m_scrollPage);
    addScrollablePage(m_stack, m_storagePage);
    addScrollablePage(m_stack, m_advancedPage);
    addScrollablePage(m_stack, m_aboutPage);
}

void SettingsDialog::installDirtyTracker()
{
    if (QCoreApplication *app = QCoreApplication::instance()) {
        app->installEventFilter(this);
    }
}

bool SettingsDialog::eventFilter(QObject *watched, QEvent *event)
{
    const QEvent::Type type = event->type();
    if (m_rebuilding || !isDirtyRelevantEvent(type)) {
        return QDialog::eventFilter(watched, event);
    }
    auto *widget = qobject_cast<QWidget *>(watched);
    if (!widget || widget->window() != window()) {
        return QDialog::eventFilter(watched, event);
    }
    // 延迟到事件处理完成后统一刷新；合并同一轮内的多次输入，最多排一个刷新。
    if (!m_dirtyRefreshPending) {
        m_dirtyRefreshPending = true;
        QTimer::singleShot(0, this, [this] {
            m_dirtyRefreshPending = false;
            if (!m_rebuilding) {
                refreshDirtyState();
            }
        });
    }
    return QDialog::eventFilter(watched, event);
}

void SettingsDialog::loadConfig()
{
    QString error;
    m_config = readSettingsConfig(&error);
    m_pendingLanguageMode.reset();
    // 语言回读：保证打开设置窗口时语言与配置一致。但 MARK_SHOT_LANG
    // 环境变量是会话级覆盖，打开设置窗口不应改动整个会话的语言。
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (env.value(QStringLiteral("MARK_SHOT_LANG")).trimmed().isEmpty()) {
        markshot::i18n::setLanguage(markshot::ui::languageForUiLanguageMode(m_config.general.uiLanguageMode));
    }
    applyConfigToPages(m_config);
    if (!error.isEmpty()) {
        m_statusLabel->setText(error);
    }
    applyTheme(m_config.general.uiThemeMode);
    refreshDirtyState();
}

void SettingsDialog::applyConfigToPages(const SettingsConfig &config)
{
    SettingsConfig effective = config;
    if (m_pendingLanguageMode.has_value()) {
        effective.general.uiLanguageMode = *m_pendingLanguageMode;
    }
    m_generalPage->setConfig(effective);
    m_capturePage->setConfig(effective);
    m_shortcutsPage->setConfig(effective);
    m_annotationPage->setConfig(effective);
    m_pinnedPage->setConfig(effective);
    m_integrationsPage->setConfig(effective);
    m_pluginsPage->setConfig(effective);
    m_scrollPage->setConfig(effective);
    m_storagePage->setConfig(effective);
    m_advancedPage->setConfig(effective);
    m_aboutPage->setConfig(effective);
}

SettingsConfig SettingsDialog::collectConfig() const
{
    SettingsConfig config = m_config;
    if (m_pendingLanguageMode.has_value()) {
        config.general.uiLanguageMode = *m_pendingLanguageMode;
    }
    m_generalPage->updateConfig(&config);
    m_capturePage->updateConfig(&config);
    m_shortcutsPage->updateConfig(&config);
    m_annotationPage->updateConfig(&config);
    m_pinnedPage->updateConfig(&config);
    m_pluginsPage->updateConfig(&config);
    m_scrollPage->updateConfig(&config);
    m_storagePage->updateConfig(&config);
    m_integrationsPage->updateConfig(&config);
    m_advancedPage->updateConfig(&config);
    m_aboutPage->updateConfig(&config);
    return config;
}

bool SettingsDialog::saveConfig(bool closeAfterSave)
{
    SettingsConfig nextConfig = collectConfig();
    QString error;
    if (!writeSettingsConfig(nextConfig, &error)) {
        QMessageBox::critical(this, MS_TR("Settings"), MS_TR("Failed to save settings: %1").arg(error));
        return false;
    }

    m_config = nextConfig;
    m_pendingLanguageMode.reset();
    // 刷新各页"已保存"基线，保证 Apply 后"还原配置"还原的是刚保存的值。
    applyConfigToPages(nextConfig);
    applyTheme(m_config.general.uiThemeMode);
    m_statusLabel->setText(MS_TR("Settings saved. Some changes take effect after restarting Mark Shot."));
    refreshDirtyState();
    if (closeAfterSave) {
        close();
    }
    return true;
}

void SettingsDialog::applyTheme(markshot::ui::UiThemeMode mode)
{
    const markshot::ui::UiThemeMode effectiveMode = markshot::ui::effectiveUiThemeMode(mode);
    qApp->setPalette(tokens::settingsPalette(effectiveMode));
    setStyleSheet(tokens::settingsStyleSheet(effectiveMode));
}

void SettingsDialog::applyLanguageMode(markshot::ui::UiLanguageMode mode)
{
    // 回到已保存的语言模式：清除语言预览状态（可能仍有未保存的其他修改）。
    const bool reverting = mode == m_config.general.uiLanguageMode;
    if (reverting) {
        m_pendingLanguageMode.reset();
    }
    // 与当前已生效语言一致时无需重建（例如重建后 setCurrentIndex 触发）。
    if (markshot::ui::languageForUiLanguageMode(mode) == markshot::i18n::language()) {
        return;
    }
    if (!reverting) {
        m_pendingLanguageMode = mode;
    }
    markshot::i18n::setLanguage(markshot::ui::languageForUiLanguageMode(mode));
    // 延迟到当前信号处理完成后重建，避免在组合框信号栈内销毁控件。
    QTimer::singleShot(0, this, [this] { rebuildPages(); });
}

void SettingsDialog::rebuildPages()
{
    if (m_rebuilding) {
        return;
    }
    m_rebuilding = true;

    const int currentIndex = m_stack->currentIndex();

    // 销毁旧页面与滚动容器。
    while (m_stack->count() > 0) {
        QWidget *widget = m_stack->widget(0);
        m_stack->removeWidget(widget);
        widget->deleteLater();
    }

    // 语言切换后侧栏与页脚按钮也需要重新翻译。
    rebuildNavigation();
    retranslateFooter();

    createPagesAndPopulate();

    // 以当前配置（含未保存的语言预览）重建，并恢复导航位置。
    applyConfigToPages(m_config);
    applyTheme(m_config.general.uiThemeMode);
    m_navigation->setCurrentLogicalRow(qBound(0, currentIndex, m_stack->count() - 1));
    m_rebuilding = false;

    refreshDirtyState();
}

bool SettingsDialog::hasUnsavedChanges() const
{
    if (m_pendingLanguageMode.has_value()) {
        return true;
    }
    return m_generalPage->isModified() || m_capturePage->isModified()
        || m_shortcutsPage->isModified() || m_annotationPage->isModified()
        || m_pinnedPage->isModified() || m_integrationsPage->isModified()
        || m_pluginsPage->isModified() || m_scrollPage->isModified()
        || m_storagePage->isModified() || m_advancedPage->isModified();
}

void SettingsDialog::refreshDirtyState()
{
    if (m_rebuilding) {
        return;
    }
    const QVector<bool> dirty = {
        m_generalPage->isModified(),
        m_capturePage->isModified(),
        m_shortcutsPage->isModified(),
        m_annotationPage->isModified(),
        m_pinnedPage->isModified(),
        m_integrationsPage->isModified(),
        m_pluginsPage->isModified(),
        m_scrollPage->isModified(),
        m_storagePage->isModified(),
        m_advancedPage->isModified(),
        m_aboutPage->isModified(),
    };
    const bool anyDirty = dirty.contains(true) || m_pendingLanguageMode.has_value();

    // 状态未变化时跳过 UI 更新，避免滚动/按键时反复重写导航与重刷样式。
    if (dirty == m_lastDirty && anyDirty == m_lastAnyDirty) {
        return;
    }
    m_lastDirty = dirty;
    m_lastAnyDirty = anyDirty;

    m_navigation->setDirtyFlags(dirty);

    m_statusLabel->setProperty("state", anyDirty ? QStringLiteral("warning") : QString());
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
    if (anyDirty) {
        m_statusLabel->setText(MS_TR("You have unsaved changes."));
        setWindowTitle(QStringLiteral("%1 *").arg(MS_TR("Settings")));
    } else {
        m_statusLabel->setText(MS_TR("Some changes take effect after restarting Mark Shot."));
        setWindowTitle(MS_TR("Settings"));
    }
}

void SettingsDialog::discardUnsavedChanges()
{
    if (m_pendingLanguageMode.has_value()) {
        m_pendingLanguageMode.reset();
        markshot::i18n::setLanguage(markshot::ui::languageForUiLanguageMode(m_config.general.uiLanguageMode));
        rebuildPages();
        return;
    }
    applyConfigToPages(m_config);
    refreshDirtyState();
}

void SettingsDialog::reloadForDisplay()
{
    loadConfig();
    rebuildPages();
}

SettingsDialog::CloseChoice SettingsDialog::confirmUnsavedChanges()
{
    QMessageBox box(this);
    box.setIcon(QMessageBox::Question);
    box.setWindowTitle(MS_TR("Unsaved Changes"));
    box.setText(MS_TR("You have unsaved changes."));
    QPushButton *saveButton = box.addButton(MS_TR("Save and Close"), QMessageBox::AcceptRole);
    QPushButton *discardButton = box.addButton(MS_TR("Discard and Close"), QMessageBox::DestructiveRole);
    QPushButton *keepButton = box.addButton(MS_TR("Keep Editing"), QMessageBox::RejectRole);
    box.setDefaultButton(saveButton);
    box.exec();

    if (box.clickedButton() == saveButton) {
        return CloseChoice::SaveAndClose;
    }
    if (box.clickedButton() == discardButton) {
        return CloseChoice::DiscardAndClose;
    }
    return CloseChoice::KeepEditing;
}

void SettingsDialog::closeEvent(QCloseEvent *event)
{
    if (!hasUnsavedChanges()) {
        event->accept();
        return;
    }

    switch (confirmUnsavedChanges()) {
    case CloseChoice::SaveAndClose:
        // 保存失败时不关闭窗口，避免丢失未保存的修改。
        if (saveConfig(true)) {
            event->accept();
        } else {
            event->ignore();
        }
        break;
    case CloseChoice::DiscardAndClose:
        discardUnsavedChanges();
        event->accept();
        break;
    case CloseChoice::KeepEditing:
        event->ignore();
        break;
    }
}

void SettingsDialog::reject()
{
    if (!hasUnsavedChanges()) {
        QDialog::reject();
        return;
    }

    switch (confirmUnsavedChanges()) {
    case CloseChoice::SaveAndClose:
        // saveConfig(true) 成功后内部 close() 会走 closeEvent（此时已干净）关闭窗口。
        saveConfig(true);
        break;
    case CloseChoice::DiscardAndClose:
        discardUnsavedChanges();
        QDialog::reject();
        break;
    case CloseChoice::KeepEditing:
        break;
    }
}

void showSettingsDialog(QWidget *parent)
{
    static QPointer<SettingsDialog> dialog;
    if (!dialog) {
        dialog = new SettingsDialog(nullptr);
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    } else if (!dialog->isVisible()) {
        // 复用窗口时刷新：重读配置并让页面跟随当前界面语言（重新翻译）。
        dialog->reloadForDisplay();
    }

    if (parent && parent->screen()) {
        const QRect available = parent->screen()->availableGeometry();
        dialog->move(available.center() - dialog->rect().center());
    } else if (QScreen *screen = QGuiApplication::primaryScreen()) {
        const QRect available = screen->availableGeometry();
        dialog->move(available.center() - dialog->rect().center());
    }

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

}  // namespace markshot::settings
