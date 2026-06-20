#include "settings/settings_page_storage.h"

#include "settings/settings_ui_helpers.h"
#include "ui/i18n.h"

#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QLineEdit>
#include <QSpinBox>
#include <QVBoxLayout>

namespace markshot::settings {

SettingsPageStorage::SettingsPageStorage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = createSettingsPageLayout(this);

    QFrame *saveCard = createSettingsCard(MS_TR("Saving"),
                                          MS_TR("Configure the default file name template for saved screenshots."),
                                          this);
    QFormLayout *saveForm = settingsCardForm(saveCard);
    m_savePathTemplate = addTextRow(saveForm,
                                    MS_TR("Path Template"),
                                    QStringLiteral("{pictures}/mark-shot/mark-shot-{datetime}.png"));
    layout->addWidget(saveCard);

    QFrame *clipboardCard = createSettingsCard(MS_TR("Clipboard Image"),
                                               MS_TR("Choose how copied images are placed into the clipboard."),
                                               this);
    QFormLayout *clipboardForm = settingsCardForm(clipboardCard);
    m_clipboardMode = addComboRow(clipboardForm, MS_TR("Clipboard Mode"));
    m_clipboardMode->addItem(MS_TR("PNG Image"), static_cast<int>(ClipboardImageMode::ImagePng));
    m_clipboardMode->addItem(MS_TR("File URL"), static_cast<int>(ClipboardImageMode::Url));
    m_clipboardMode->addItem(MS_TR("Auto by Size"), static_cast<int>(ClipboardImageMode::Threshold));
    m_clipboardThresholdM = addSpinRow(clipboardForm, MS_TR("Threshold"), 1, 1024, QStringLiteral(" MiB"));
    layout->addWidget(clipboardCard);
    layout->addStretch();
}

void SettingsPageStorage::setConfig(const SettingsConfig &config)
{
    m_savePathTemplate->setText(config.storage.savePathTemplate);
    const int modeIndex = m_clipboardMode->findData(static_cast<int>(config.storage.clipboardImageMode));
    m_clipboardMode->setCurrentIndex(modeIndex >= 0 ? modeIndex : 0);
    m_clipboardThresholdM->setValue(config.storage.clipboardThresholdM);
}

void SettingsPageStorage::updateConfig(SettingsConfig *config) const
{
    if (!config) {
        return;
    }

    config->storage.savePathTemplate = m_savePathTemplate->text().trimmed();
    config->storage.clipboardImageMode =
        static_cast<ClipboardImageMode>(m_clipboardMode->currentData().toInt());
    config->storage.clipboardThresholdM = m_clipboardThresholdM->value();
}

}  // namespace markshot::settings
