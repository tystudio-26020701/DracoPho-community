#include "settings/settings_dialog.h"

#include "settings/settings_page_advanced.h"
#include "settings/settings_page_annotation.h"
#include "settings/settings_page_capture.h"
#include "settings/settings_page_general.h"
#include "settings/settings_page_integrations.h"
#include "settings/settings_page_pinned.h"
#include "settings/settings_page_scroll.h"
#include "settings/settings_page_shortcuts.h"
#include "settings/settings_page_storage.h"
#include "ui/i18n.h"
#include "ui/icons.h"
#include "ui/theme.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPointer>
#include <QPushButton>
#include <QScreen>
#include <QGuiApplication>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace markshot::settings {
namespace {

/// @brief 返回设置窗口样式表。
/// @return Qt 样式表文本。
QString settingsStyleSheet()
{
    return QStringLiteral(
        "QDialog#settingsDialog { background: #F8FAFC; color: #020617; }"
        "QLabel#settingsHeroTitle { color: #F8FAFC; font-size: 22px; font-weight: 800; }"
        "QLabel#settingsHeroText { color: #CBD5E1; font-size: 12px; }"
        "QLabel#settingsStatus { color: #475569; }"
        "QListWidget#settingsNavigation {"
        " background: #0F172A;"
        " border: 0;"
        " padding: 10px;"
        " outline: 0;"
        "}"
        "QListWidget#settingsNavigation::item {"
        " color: #CBD5E1;"
        " border-radius: 10px;"
        " padding: 10px 12px;"
        " margin: 3px 0;"
        "}"
        "QListWidget#settingsNavigation::item:hover { background: #1E293B; color: #F8FAFC; }"
        "QListWidget#settingsNavigation::item:selected { background: #0369A1; color: #FFFFFF; }"
        "QFrame#settingsCard {"
        " background: #FFFFFF;"
        " border: 1px solid #E2E8F0;"
        " border-radius: 14px;"
        "}"
        "QLabel#settingsCardTitle { color: #0F172A; font-size: 15px; font-weight: 800; }"
        "QLabel#settingsCardDescription { color: #64748B; font-size: 12px; }"
        "QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QKeySequenceEdit, QPlainTextEdit {"
        " min-height: 30px;"
        " border: 1px solid #CBD5E1;"
        " border-radius: 8px;"
        " padding: 2px 8px;"
        " background: #FFFFFF;"
        " color: #0F172A;"
        "}"
        "QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QKeySequenceEdit:focus, QPlainTextEdit:focus {"
        " border-color: #0369A1;"
        "}"
        "QCheckBox { color: #334155; spacing: 8px; }"
        "QCheckBox::indicator { width: 18px; height: 18px; }"
        "QPushButton {"
        " min-height: 32px;"
        " border-radius: 9px;"
        " border: 1px solid #CBD5E1;"
        " padding: 4px 14px;"
        " background: #FFFFFF;"
        " color: #0F172A;"
        " font-weight: 700;"
        "}"
        "QPushButton:hover { border-color: #0369A1; }"
        "QPushButton[role=\"primary\"] { background: #0369A1; border-color: #0369A1; color: #FFFFFF; }"
        "QPushButton[role=\"primary\"]:hover { background: #075985; }");
}

/// @brief 添加设置分类导航项。
/// @param list 导航列表。
/// @param text 显示文本。
void addNavigationItem(QListWidget *list, const QString &text)
{
    auto *item = new QListWidgetItem(text, list);
    item->setSizeHint(QSize(148, 40));
}

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

}  // namespace

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("settingsDialog"));
    setWindowTitle(MS_TR("Settings"));
    setWindowIcon(markshot::ui::applicationIcon());
    setMinimumSize(820, 600);
    resize(900, 640);
    setStyleSheet(settingsStyleSheet());

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    auto *body = new QWidget(this);
    auto *bodyLayout = new QHBoxLayout(body);
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->setSpacing(0);

    auto *sidebar = new QWidget(body);
    sidebar->setFixedWidth(210);
    sidebar->setStyleSheet(QStringLiteral("background: #0F172A;"));
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(18, 20, 18, 18);
    sidebarLayout->setSpacing(14);

    auto *title = new QLabel(QStringLiteral("Mark Shot"), sidebar);
    title->setObjectName(QStringLiteral("settingsHeroTitle"));
    sidebarLayout->addWidget(title);
    auto *subtitle = new QLabel(MS_TR("Settings Center"), sidebar);
    subtitle->setObjectName(QStringLiteral("settingsHeroText"));
    subtitle->setWordWrap(true);
    sidebarLayout->addWidget(subtitle);

    m_navigation = new QListWidget(sidebar);
    m_navigation->setObjectName(QStringLiteral("settingsNavigation"));
    m_navigation->setFrameShape(QFrame::NoFrame);
    m_navigation->setFocusPolicy(Qt::NoFocus);
    addNavigationItem(m_navigation, MS_TR("General"));
    addNavigationItem(m_navigation, MS_TR("Capture"));
    addNavigationItem(m_navigation, MS_TR("Shortcuts"));
    addNavigationItem(m_navigation, MS_TR("Annotation"));
    addNavigationItem(m_navigation, MS_TR("Pinned Image"));
    addNavigationItem(m_navigation, MS_TR("Integrations"));
    addNavigationItem(m_navigation, MS_TR("Scroll Capture"));
    addNavigationItem(m_navigation, MS_TR("Storage"));
    addNavigationItem(m_navigation, MS_TR("Advanced"));
    sidebarLayout->addWidget(m_navigation, 1);
    bodyLayout->addWidget(sidebar);

    m_stack = new QStackedWidget(body);
    m_generalPage = new SettingsPageGeneral(m_stack);
    m_capturePage = new SettingsPageCapture(m_stack);
    m_shortcutsPage = new SettingsPageShortcuts(m_stack);
    m_annotationPage = new SettingsPageAnnotation(m_stack);
    m_pinnedPage = new SettingsPagePinned(m_stack);
    m_integrationsPage = new SettingsPageIntegrations(m_stack);
    m_scrollPage = new SettingsPageScroll(m_stack);
    m_storagePage = new SettingsPageStorage(m_stack);
    m_advancedPage = new SettingsPageAdvanced(m_stack);
    addScrollablePage(m_stack, m_generalPage);
    addScrollablePage(m_stack, m_capturePage);
    addScrollablePage(m_stack, m_shortcutsPage);
    addScrollablePage(m_stack, m_annotationPage);
    addScrollablePage(m_stack, m_pinnedPage);
    addScrollablePage(m_stack, m_integrationsPage);
    addScrollablePage(m_stack, m_scrollPage);
    addScrollablePage(m_stack, m_storagePage);
    addScrollablePage(m_stack, m_advancedPage);
    bodyLayout->addWidget(m_stack, 1);
    rootLayout->addWidget(body, 1);

    auto *footer = new QWidget(this);
    footer->setStyleSheet(QStringLiteral("background: #FFFFFF; border-top: 1px solid #E2E8F0;"));
    auto *footerLayout = new QHBoxLayout(footer);
    footerLayout->setContentsMargins(18, 10, 18, 10);
    m_statusLabel = new QLabel(MS_TR("Some changes take effect after restarting Mark Shot."), footer);
    m_statusLabel->setObjectName(QStringLiteral("settingsStatus"));
    footerLayout->addWidget(m_statusLabel, 1);
    auto *buttons = new QDialogButtonBox(footer);
    QPushButton *applyButton = buttons->addButton(MS_TR("Apply"), QDialogButtonBox::ApplyRole);
    QPushButton *saveButton = buttons->addButton(MS_TR("Save"), QDialogButtonBox::AcceptRole);
    QPushButton *cancelButton = buttons->addButton(MS_TR("Cancel"), QDialogButtonBox::RejectRole);
    saveButton->setProperty("role", QStringLiteral("primary"));
    footerLayout->addWidget(buttons);
    rootLayout->addWidget(footer);

    connect(m_navigation, &QListWidget::currentRowChanged, m_stack, &QStackedWidget::setCurrentIndex);
    connect(applyButton, &QPushButton::clicked, this, [this] { saveConfig(false); });
    connect(saveButton, &QPushButton::clicked, this, [this] { saveConfig(true); });
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::close);

    m_navigation->setCurrentRow(0);
    loadConfig();
}

void SettingsDialog::loadConfig()
{
    QString error;
    m_config = readSettingsConfig(&error);
    m_generalPage->setConfig(m_config);
    m_capturePage->setConfig(m_config);
    m_shortcutsPage->setConfig(m_config);
    m_annotationPage->setConfig(m_config);
    m_pinnedPage->setConfig(m_config);
    m_integrationsPage->setConfig(m_config);
    m_scrollPage->setConfig(m_config);
    m_storagePage->setConfig(m_config);
    m_advancedPage->setConfig(m_config);
    if (!error.isEmpty()) {
        m_statusLabel->setText(error);
    }
}

SettingsConfig SettingsDialog::collectConfig() const
{
    SettingsConfig config = m_config;
    m_generalPage->updateConfig(&config);
    m_capturePage->updateConfig(&config);
    m_shortcutsPage->updateConfig(&config);
    m_annotationPage->updateConfig(&config);
    m_pinnedPage->updateConfig(&config);
    m_integrationsPage->updateConfig(&config);
    m_scrollPage->updateConfig(&config);
    m_storagePage->updateConfig(&config);
    m_advancedPage->updateConfig(&config);
    return config;
}

void SettingsDialog::saveConfig(bool closeAfterSave)
{
    SettingsConfig nextConfig = collectConfig();
    QString error;
    if (!writeSettingsConfig(nextConfig, &error)) {
        QMessageBox::critical(this, MS_TR("Settings"), MS_TR("Failed to save settings: %1").arg(error));
        return;
    }

    m_config = nextConfig;
    m_statusLabel->setText(MS_TR("Settings saved. Some changes take effect after restarting Mark Shot."));
    if (closeAfterSave) {
        close();
    }
}

void showSettingsDialog(QWidget *parent)
{
    static QPointer<SettingsDialog> dialog;
    if (!dialog) {
        dialog = new SettingsDialog(nullptr);
        dialog->setAttribute(Qt::WA_DeleteOnClose, true);
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
