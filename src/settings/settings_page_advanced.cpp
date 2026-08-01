#include "settings/settings_page_advanced.h"

#include "settings/settings_ui_helpers.h"
#include "ui/i18n.h"

#include <QCheckBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace markshot::settings {

SettingsPageAdvanced::SettingsPageAdvanced(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = createSettingsPageLayout(this);

    QFrame *debugCard = createSettingsCard(MS_TR("Debug"),
                                           MS_TR("Configure diagnostic logging for troubleshooting."),
                                           this);
    QFormLayout *debugForm = settingsCardForm(debugCard);
    m_debugEnabled = addSwitchRow(debugForm, MS_TR("Debug Logging"), MS_TR("Enable debug log output."));
    m_debugLogPath = addTextRow(debugForm, MS_TR("Debug Log Path"), QStringLiteral("~/mark-shot-debug.log"));
    addCardRestoreButton(debugCard, [this] {
        m_debugEnabled->setChecked(m_saved.advanced.debugEnabled);
        m_debugLogPath->setText(m_saved.advanced.debugLogPath);
    });
    layout->addWidget(debugCard);

    QFrame *windowCard = createSettingsCard(MS_TR("Window Detection"),
                                            MS_TR("Configure the external helper used to detect windows under the selection."),
                                            this);
    QFormLayout *windowForm = settingsCardForm(windowCard);
    m_windowDetectionEnabled = addSwitchRow(windowForm,
                                            MS_TR("Window Detection Enabled"),
                                            MS_TR("Run the configured helper before region selection."));
    m_windowDetectionCommand = addTextRow(windowForm,
                                          MS_TR("Window Detection Command"),
                                          QStringLiteral("mark-shot-window-detection-niri"));
    m_windowDetectionWorkingDirectory = addTextRow(windowForm, MS_TR("Working Directory"), QStringLiteral("~/"));
    m_windowDetectionTimeoutMs = addSpinRow(windowForm, MS_TR("Window Detection Timeout"), 100, 30000, QStringLiteral(" ms"));
    m_windowDetectionEnv = addPlainTextRow(windowForm,
                                           MS_TR("Window Detection Environment"),
                                           QStringLiteral("KEY=value"));
    addCardRestoreButton(windowCard, [this] {
        m_windowDetectionEnabled->setChecked(m_saved.advanced.windowDetectionEnabled);
        m_windowDetectionCommand->setText(m_saved.advanced.windowDetectionCommand);
        m_windowDetectionWorkingDirectory->setText(m_saved.advanced.windowDetectionWorkingDirectory);
        m_windowDetectionTimeoutMs->setValue(m_saved.advanced.windowDetectionTimeoutMs);
        m_windowDetectionEnv->setPlainText(envMapToText(m_saved.advanced.windowDetectionEnv));
    });
    layout->addWidget(windowCard);

    QFrame *envCard = createSettingsCard(MS_TR("Application Environment"),
                                         MS_TR("Environment variables applied when Mark Shot starts."),
                                         this);
    QFormLayout *envForm = settingsCardForm(envCard);
    m_appEnv = addPlainTextRow(envForm, MS_TR("Application Environment"), QStringLiteral("KEY=value"));
    addCardRestoreButton(envCard, [this] {
        m_appEnv->setPlainText(envMapToText(m_saved.advanced.appEnv));
    });
    layout->addWidget(envCard);

    addPageRestoreButton(layout, [this] { setConfig(m_saved); });

    auto *resetRow = new QWidget(this);
    auto *resetLayout = new QHBoxLayout(resetRow);
    resetLayout->setContentsMargins(0, 0, 0, 0);
    resetLayout->addStretch();
    auto *resetButton = new QPushButton(MS_TR("Restore Original Settings"), resetRow);
    resetButton->setObjectName(QStringLiteral("settingsDanger"));
    resetButton->setCursor(Qt::PointingHandCursor);
    resetButton->setToolTip(MS_TR("Reset every setting back to the factory defaults. This cannot be undone."));
    resetLayout->addWidget(resetButton);
    layout->addWidget(resetRow);
    connect(resetButton, &QPushButton::clicked, this, &SettingsPageAdvanced::restoreOriginalSettings);

    layout->addStretch();
}

void SettingsPageAdvanced::setConfig(const SettingsConfig &config)
{
    m_saved = config;
    m_debugEnabled->setChecked(config.advanced.debugEnabled);
    m_debugLogPath->setText(config.advanced.debugLogPath);
    m_windowDetectionEnabled->setChecked(config.advanced.windowDetectionEnabled);
    m_windowDetectionCommand->setText(config.advanced.windowDetectionCommand);
    m_windowDetectionWorkingDirectory->setText(config.advanced.windowDetectionWorkingDirectory);
    m_windowDetectionTimeoutMs->setValue(config.advanced.windowDetectionTimeoutMs);
    m_windowDetectionEnv->setPlainText(envMapToText(config.advanced.windowDetectionEnv));
    m_appEnv->setPlainText(envMapToText(config.advanced.appEnv));
}

void SettingsPageAdvanced::updateConfig(SettingsConfig *config) const
{
    if (!config) {
        return;
    }

    config->advanced.debugEnabled = m_debugEnabled->isChecked();
    config->advanced.debugLogPath = m_debugLogPath->text().trimmed();
    config->advanced.windowDetectionEnabled = m_windowDetectionEnabled->isChecked();
    config->advanced.windowDetectionCommand = m_windowDetectionCommand->text().trimmed();
    config->advanced.windowDetectionWorkingDirectory = m_windowDetectionWorkingDirectory->text().trimmed();
    config->advanced.windowDetectionTimeoutMs = m_windowDetectionTimeoutMs->value();
    config->advanced.windowDetectionEnv = envMapFromText(m_windowDetectionEnv->toPlainText());
    config->advanced.appEnv = envMapFromText(m_appEnv->toPlainText());
}

void SettingsPageAdvanced::setRestoreOriginalHandler(const std::function<void()> &handler)
{
    m_onRestoreOriginal = handler;
}

void SettingsPageAdvanced::restoreOriginalSettings()
{
    const auto answer = QMessageBox::question(
        this,
        MS_TR("Restore Original Settings"),
        MS_TR("Reset every setting to its factory default? This cannot be undone, and "
              "any unsaved changes will be lost."),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    if (answer != QMessageBox::Yes) {
        return;
    }

    QString error;
    if (!resetSettingsToDefaults(&error)) {
        QMessageBox::critical(this, MS_TR("Restore Original Settings"), MS_TR("Failed to reset settings: %1").arg(error));
        return;
    }

    if (m_onRestoreOriginal) {
        m_onRestoreOriginal();
    }
}


bool SettingsPageAdvanced::isModified() const
{
    SettingsConfig current = m_saved;
    updateConfig(&current);
    return !(current.advanced == m_saved.advanced);
}

}  // namespace markshot::settings
