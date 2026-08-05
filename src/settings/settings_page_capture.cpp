#include "settings/settings_page_capture.h"

#include "settings/settings_ui_helpers.h"
#include "ui/i18n.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QVBoxLayout>

namespace markshot::settings {

SettingsPageCapture::SettingsPageCapture(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = createSettingsPageLayout(this);
    QFrame *captureCard = createSettingsCard(MS_TR("Capture"),
                                             MS_TR("Adjust how the frozen screenshot is captured before annotation starts."),
                                             this);
    QFormLayout *form = settingsCardForm(captureCard);
    m_includeCursor = addSwitchRow(form,
                                   MS_TR("Include Cursor"),
                                   MS_TR("Capture the mouse cursor in the frozen image when supported."));
    m_freezeScope = addComboRow(form, MS_TR("Freeze Scope"));
    m_freezeScope->addItem(MS_TR("All Screens"), static_cast<int>(CaptureFreezeScope::AllScreens));
    m_freezeScope->addItem(MS_TR("Cursor Screen"), static_cast<int>(CaptureFreezeScope::CursorScreen));
    m_kdeKwinScreenshot = addSwitchRow(form,
                                       MS_TR("KDE KWin Screenshot"),
                                       MS_TR("Use KWin ScreenShot2 on KDE Wayland when available."));
    m_hideOwnWindows = addSwitchRow(form,
                                    MS_TR("Hide DracoPho Windows While Capturing"),
                                    MS_TR("Hide own windows from screenshots. Turn off to include them."));
    m_delaySeconds = addComboRow(form, MS_TR("Capture Delay"));
    m_delaySeconds->addItem(MS_TR("Immediately"), 0);
    m_delaySeconds->addItem(MS_TR("1 Second"), 1);
    m_delaySeconds->addItem(MS_TR("3 Seconds"), 3);
    m_delaySeconds->addItem(MS_TR("5 Seconds"), 5);
    m_delaySeconds->addItem(MS_TR("10 Seconds"), 10);
    addCardRestoreButton(captureCard, [this] {
        m_includeCursor->setChecked(m_saved.capture.includeCursor);
        const int index = m_freezeScope->findData(static_cast<int>(m_saved.capture.freezeScope));
        m_freezeScope->setCurrentIndex(index >= 0 ? index : 0);
        m_kdeKwinScreenshot->setChecked(m_saved.capture.kdeKwinScreenshotEnabled);
        m_hideOwnWindows->setChecked(m_saved.capture.hideOwnWindows);
        const int delayIndex = m_delaySeconds->findData(m_saved.capture.delaySeconds);
        m_delaySeconds->setCurrentIndex(delayIndex >= 0 ? delayIndex : 0);
    });
    layout->addWidget(captureCard);
    addPageRestoreButton(layout, [this] { setConfig(m_saved); });
    layout->addStretch();
}

void SettingsPageCapture::setConfig(const SettingsConfig &config)
{
    m_saved = config;
    m_includeCursor->setChecked(config.capture.includeCursor);
    const int index = m_freezeScope->findData(static_cast<int>(config.capture.freezeScope));
    m_freezeScope->setCurrentIndex(index >= 0 ? index : 0);
    m_kdeKwinScreenshot->setChecked(config.capture.kdeKwinScreenshotEnabled);
    m_hideOwnWindows->setChecked(config.capture.hideOwnWindows);
    const int delayIndex = m_delaySeconds->findData(config.capture.delaySeconds);
    m_delaySeconds->setCurrentIndex(delayIndex >= 0 ? delayIndex : 0);
}

void SettingsPageCapture::updateConfig(SettingsConfig *config) const
{
    if (!config) {
        return;
    }

    config->capture.includeCursor = m_includeCursor->isChecked();
    config->capture.freezeScope =
        static_cast<CaptureFreezeScope>(m_freezeScope->currentData().toInt());
    config->capture.kdeKwinScreenshotEnabled = m_kdeKwinScreenshot->isChecked();
    config->capture.hideOwnWindows = m_hideOwnWindows->isChecked();
    config->capture.delaySeconds = m_delaySeconds->currentData().toInt();
}

bool SettingsPageCapture::isModified() const
{
    SettingsConfig current = m_saved;
    updateConfig(&current);
    return !(current.capture == m_saved.capture);
}

}  // namespace markshot::settings
