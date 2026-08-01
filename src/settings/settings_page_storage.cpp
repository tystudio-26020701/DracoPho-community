#include "settings/settings_page_storage.h"

#include "settings/settings_ui_helpers.h"
#include "ui/i18n.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QWidget>
#include <QVBoxLayout>

namespace markshot::settings {
namespace {

/// @brief 开启无头模式剪贴板写权限前展示的固定口令。
/// 防呆设计：用户必须逐字输入该口令并点击"确定"才能开启，避免脚本或
/// 误触把剪贴板写权限悄悄打开。
constexpr const char *kHeadlessClipboardPassphrase = "mark-shot-headless-clipboard";

/**
 * 添加目录选择表单项。
 * @param form 表单布局。
 * @param label 标签文本。
 * @param parent 父控件。
 * @return 目录输入控件。
 */
QLineEdit *addDirectoryRow(QFormLayout *form, const QString &label, QWidget *parent)
{
    auto *row = new QWidget(parent);
    auto *layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    auto *edit = new QLineEdit(row);
    auto *browse = new QPushButton(MS_TR("Browse"), row);
    layout->addWidget(edit, 1);
    layout->addWidget(browse);
    form->addRow(label, row);

    QObject::connect(browse, &QPushButton::clicked, row, [edit, parent] {
        const QString directory = QFileDialog::getExistingDirectory(parent,
                                                                    MS_TR("Select Folder"),
                                                                    edit->text().trimmed());
        if (!directory.isEmpty()) {
            edit->setText(directory);
        }
    });
    return edit;
}

/**
 * 弹出"输入口令确认开启剪贴板写权限"对话框。
 * @param parent 父控件。
 * @return 用户输入了口令并点击确定时返回 true。
 */
bool confirmHeadlessClipboardAccess(QWidget *parent)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(MS_TR("Enable Headless Clipboard Access"));
    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 18, 20, 18);
    layout->setSpacing(12);

    auto *prompt = new QLabel(MS_TR("Headless mode may write into your system clipboard. "
                                    "To confirm you understand the risk, type the passphrase "
                                    "shown below exactly and press Confirm."),
                              &dialog);
    prompt->setWordWrap(true);
    layout->addWidget(prompt);

    auto *passphrase = new QLabel(QString::fromLatin1(kHeadlessClipboardPassphrase), &dialog);
    passphrase->setObjectName(QStringLiteral("headlessPassphrase"));
    passphrase->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
    passphrase->setAlignment(Qt::AlignCenter);
    passphrase->setStyleSheet(QStringLiteral(
        "QLabel#headlessPassphrase {"
        " background: #0F172A;"
        " color: #5EEAD4;"
        " border: 1px solid #334155;"
        " border-radius: 8px;"
        " padding: 8px 12px;"
        " font-family: monospace;"
        " font-weight: 600;"
        "}"));
    layout->addWidget(passphrase);

    auto *input = new QLineEdit(&dialog);
    input->setPlaceholderText(MS_TR("Type the passphrase"));
    input->setClearButtonEnabled(true);
    layout->addWidget(input);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(MS_TR("Confirm"));
    buttons->button(QDialogButtonBox::Cancel)->setText(MS_TR("Cancel"));
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    input->setFocus();
    while (dialog.exec() == QDialog::Accepted) {
        if (input->text() == QString::fromLatin1(kHeadlessClipboardPassphrase)) {
            return true;
        }
        QMessageBox::warning(&dialog,
                             MS_TR("Wrong Passphrase"),
                             MS_TR("The passphrase does not match. Clipboard access stays disabled."));
        input->clear();
        input->setFocus();
    }
    return false;
}

}  // namespace

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
    addCardRestoreButton(saveCard, [this] {
        m_savePathTemplate->setText(m_saved.storage.savePathTemplate);
    });
    layout->addWidget(saveCard);

    QFrame *recordingCard = createSettingsCard(MS_TR("Recording Output"),
                                               MS_TR("Configure where video and GIF recordings are saved."),
                                               this);
    QFormLayout *recordingForm = settingsCardForm(recordingCard);
    m_recordingVideoDirectory = addDirectoryRow(recordingForm, MS_TR("Video Directory"), recordingCard);
    m_recordingGifDirectory = addDirectoryRow(recordingForm, MS_TR("GIF Directory"), recordingCard);
    addCardRestoreButton(recordingCard, [this] {
        m_recordingVideoDirectory->setText(m_saved.storage.recordingVideoDirectory);
        m_recordingGifDirectory->setText(m_saved.storage.recordingGifDirectory);
    });
    layout->addWidget(recordingCard);

    QFrame *clipboardCard = createSettingsCard(MS_TR("Clipboard Image"),
                                               MS_TR("Choose how copied images are placed into the clipboard."),
                                               this);
    QFormLayout *clipboardForm = settingsCardForm(clipboardCard);
    m_clipboardMode = addComboRow(clipboardForm, MS_TR("Clipboard Mode"));
    m_clipboardMode->addItem(MS_TR("PNG Image"), static_cast<int>(ClipboardImageMode::ImagePng));
    m_clipboardMode->addItem(MS_TR("File URL"), static_cast<int>(ClipboardImageMode::Url));
    m_clipboardMode->addItem(MS_TR("Auto by Size"), static_cast<int>(ClipboardImageMode::Threshold));
    m_clipboardThresholdM = addSpinRow(clipboardForm, MS_TR("Threshold"), 1, 1024, QStringLiteral(" MiB"));
    addCardRestoreButton(clipboardCard, [this] {
        const int modeIndex =
            m_clipboardMode->findData(static_cast<int>(m_saved.storage.clipboardImageMode));
        m_clipboardMode->setCurrentIndex(modeIndex >= 0 ? modeIndex : 0);
        m_clipboardThresholdM->setValue(m_saved.storage.clipboardThresholdM);
    });
    layout->addWidget(clipboardCard);

    QFrame *headlessCard = createSettingsCard(MS_TR("Headless Mode"),
                                              MS_TR("Control where window captures go when Mark Shot runs without a UI. "
                                                    "The clipboard is never modified unless you explicitly enable it here."),
                                              this);
    QFormLayout *headlessForm = settingsCardForm(headlessCard);
    m_headlessDestination = addComboRow(headlessForm, MS_TR("Default Capture Destination"));
    m_headlessDestination->addItem(MS_TR("Return inline (base64, no files)"),
                                   static_cast<int>(HeadlessCaptureDestination::Inline));
    m_headlessDestination->addItem(MS_TR("Stage temporary files"),
                                   static_cast<int>(HeadlessCaptureDestination::Stage));
    m_headlessClipboardAllowed = addSwitchRow(headlessForm,
                                              MS_TR("Allow Clipboard Modification"),
                                              MS_TR("Permit headless window captures to write into the system clipboard. "
                                                    "Disabled by default; enabling requires a confirmation passphrase."));
    connect(m_headlessClipboardAllowed, &QCheckBox::clicked, this, [this](bool checked) {
        if (!checked) {
            return;
        }
        if (!confirmHeadlessClipboardAccess(this)) {
            const QSignalBlocker blocker(m_headlessClipboardAllowed);
            m_headlessClipboardAllowed->setChecked(false);
            return;
        }
    });
    addCardRestoreButton(headlessCard, [this] {
        const int headlessIndex =
            m_headlessDestination->findData(static_cast<int>(m_saved.storage.headlessDefaultDestination));
        m_headlessDestination->setCurrentIndex(headlessIndex >= 0 ? headlessIndex : 0);
        const QSignalBlocker blocker(m_headlessClipboardAllowed);
        m_headlessClipboardAllowed->setChecked(m_saved.storage.headlessClipboardAllowed);
    });
    layout->addWidget(headlessCard);

    QFrame *exportCard = createSettingsCard(MS_TR("Screenshot Export Appearance"),
                                            MS_TR("Add a macOS-style transparent canvas and soft shadow to shared screenshots."),
                                            this);
    QFormLayout *exportForm = settingsCardForm(exportCard);
    m_exportImageFrameEnabled = addSwitchRow(exportForm,
                                             MS_TR("Mac-style Frame"),
                                             MS_TR("Apply only to saved, copied, uploaded, Open With, and extension-command images."));
    m_exportPadding = addSpinRow(exportForm, MS_TR("Transparent Padding"), 0, 256, QStringLiteral(" px"));
    m_exportCornerRadius = addSpinRow(exportForm, MS_TR("Corner Radius"), 0, 128, QStringLiteral(" px"));
    m_exportShadowRadius = addSpinRow(exportForm, MS_TR("Shadow Blur"), 0, 128, QStringLiteral(" px"));
    m_exportShadowOffsetY = addSpinRow(exportForm, MS_TR("Shadow Drop"), 0, 128, QStringLiteral(" px"));
    m_exportShadowOpacity = addDoubleRow(exportForm, MS_TR("Shadow Opacity"), 0.0, 1.0, 2);
    m_exportShadowOpacity->setSingleStep(0.05);
    addCardRestoreButton(exportCard, [this] {
        m_exportImageFrameEnabled->setChecked(m_saved.storage.exportImageEffect.enabled);
        m_exportPadding->setValue(m_saved.storage.exportImageEffect.padding);
        m_exportCornerRadius->setValue(qRound(m_saved.storage.exportImageEffect.cornerRadius));
        m_exportShadowRadius->setValue(m_saved.storage.exportImageEffect.shadowRadius);
        m_exportShadowOffsetY->setValue(m_saved.storage.exportImageEffect.shadowOffsetY);
        m_exportShadowOpacity->setValue(m_saved.storage.exportImageEffect.shadowOpacity);
    });
    layout->addWidget(exportCard);

    addPageRestoreButton(layout, [this] { setConfig(m_saved); });
    layout->addStretch();
}

void SettingsPageStorage::setConfig(const SettingsConfig &config)
{
    m_saved = config;
    m_savePathTemplate->setText(config.storage.savePathTemplate);
    m_recordingVideoDirectory->setText(config.storage.recordingVideoDirectory);
    m_recordingGifDirectory->setText(config.storage.recordingGifDirectory);
    const int modeIndex = m_clipboardMode->findData(static_cast<int>(config.storage.clipboardImageMode));
    m_clipboardMode->setCurrentIndex(modeIndex >= 0 ? modeIndex : 0);
    m_clipboardThresholdM->setValue(config.storage.clipboardThresholdM);
    const int headlessIndex =
        m_headlessDestination->findData(static_cast<int>(config.storage.headlessDefaultDestination));
    m_headlessDestination->setCurrentIndex(headlessIndex >= 0 ? headlessIndex : 0);
    m_headlessClipboardAllowed->setChecked(config.storage.headlessClipboardAllowed);
    m_exportImageFrameEnabled->setChecked(config.storage.exportImageEffect.enabled);
    m_exportPadding->setValue(config.storage.exportImageEffect.padding);
    m_exportCornerRadius->setValue(qRound(config.storage.exportImageEffect.cornerRadius));
    m_exportShadowRadius->setValue(config.storage.exportImageEffect.shadowRadius);
    m_exportShadowOffsetY->setValue(config.storage.exportImageEffect.shadowOffsetY);
    m_exportShadowOpacity->setValue(config.storage.exportImageEffect.shadowOpacity);
}

void SettingsPageStorage::updateConfig(SettingsConfig *config) const
{
    if (!config) {
        return;
    }

    config->storage.savePathTemplate = m_savePathTemplate->text().trimmed();
    config->storage.recordingVideoDirectory = m_recordingVideoDirectory->text().trimmed();
    config->storage.recordingGifDirectory = m_recordingGifDirectory->text().trimmed();
    config->storage.clipboardImageMode =
        static_cast<ClipboardImageMode>(m_clipboardMode->currentData().toInt());
    config->storage.clipboardThresholdM = m_clipboardThresholdM->value();
    config->storage.headlessDefaultDestination =
        static_cast<HeadlessCaptureDestination>(m_headlessDestination->currentData().toInt());
    config->storage.headlessClipboardAllowed = m_headlessClipboardAllowed->isChecked();
    config->storage.exportImageEffect.enabled = m_exportImageFrameEnabled->isChecked();
    config->storage.exportImageEffect.padding = m_exportPadding->value();
    config->storage.exportImageEffect.cornerRadius = m_exportCornerRadius->value();
    config->storage.exportImageEffect.shadowRadius = m_exportShadowRadius->value();
    config->storage.exportImageEffect.shadowOffsetY = m_exportShadowOffsetY->value();
    config->storage.exportImageEffect.shadowOpacity = m_exportShadowOpacity->value();
}

}  // namespace markshot::settings
