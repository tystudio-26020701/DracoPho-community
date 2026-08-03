#include "recording/recording_config_dialog.h"

#include "app_config_store.h"
#include "recording/audio/audio_capture_reader_factory.h"
#include "recording/recording_dialog_config.h"
#include "recording/recording_display_source.h"
#include "recording/recording_file_naming.h"
#include "settings/settings_design_tokens.h"
#include "settings/settings_ui_helpers.h"
#include "settings/settings_wheel_guard.h"
#include "ui/i18n.h"
#include "ui/interface_theme_config.h"
#include "ui/theme.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCursor>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QFont>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>

namespace markshot::recording {
namespace {

/**
 * 返回录制模式标题。
 * @param mode 录制模式。
 * @return 标题文本。
 */
QString titleForMode(RecordingMode mode)
{
    if (mode == RecordingMode::Gif) {
        return MS_TR("GIF Recording");
    }
    if (mode == RecordingMode::Webp) {
        return MS_TR("WebP Recording");
    }
    return MS_TR("Video Recording");
}

/**
 * 返回当前显示器。
 * @return 当前显示器，无法判断时返回主显示器。
 */
QScreen *currentScreen()
{
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    return screen ? screen : QGuiApplication::primaryScreen();
}

/**
 * 解析录制配置窗口应跟随的界面主题。
 * 与设置窗口一致：读取配置 ui.theme（system/dark/light），再解析实际明暗。
 * @return 实际应用的明暗主题模式。
 */
markshot::ui::UiThemeMode effectiveDialogTheme()
{
    bool ok = false;
    const QJsonObject root = markshot::readAppConfigRoot(&ok);
    const markshot::ui::UiThemeMode configured = ok
        ? markshot::ui::uiThemeModeFromConfigRoot(root)
        : markshot::ui::UiThemeMode::System;
    return markshot::ui::effectiveUiThemeMode(configured);
}

/**
 * 查找当前显示器在来源列表中的下标。
 * @param sources 显示器来源列表。
 * @return 当前显示器来源下标。
 */
int currentDisplaySourceIndex(const QVector<DisplaySource> &sources)
{
    QScreen *screen = currentScreen();
    if (!screen) {
        return sources.isEmpty() ? -1 : 0;
    }

    for (int i = 0; i < sources.size(); ++i) {
        const DisplaySource &source = sources.at(i);
        if (!source.allOutputs && source.screenName == screen->name()) {
            return i;
        }
    }
    for (int i = 0; i < sources.size(); ++i) {
        const DisplaySource &source = sources.at(i);
        if (!source.allOutputs && source.geometry == screen->geometry()) {
            return i;
        }
    }
    return sources.isEmpty() ? -1 : 0;
}

/**
 * 按持久化键查找显示器来源下标。
 * @param sources 显示器来源列表。
 * @param key 持久化键。
 * @return 匹配下标，找不到时返回 -1。
 */
int displaySourceIndexForKey(const QVector<DisplaySource> &sources, const QString &key)
{
    if (key.trimmed().isEmpty()) {
        return -1;
    }
    for (int i = 0; i < sources.size(); ++i) {
        if (recordingDisplayPersistenceKey(sources.at(i)) == key) {
            return i;
        }
    }
    return -1;
}

/**
 * 给帧率下拉框写入阶梯选项。
 * @param combo 帧率下拉框。
 * @param mode 录制模式。
 * @return 无返回值。
 */
void populateFrameRateOptions(QComboBox *combo, RecordingMode mode, int requestedFps = -1)
{
    if (!combo) {
        return;
    }
    combo->clear();
    const QVector<int> values = isAnimatedImageMode(mode)
        ? QVector<int>{6, 8, 10, 12, 15, 20, 24, 30}
        : QVector<int>{15, 24, 30, 48, 60};
    const int fallback = isAnimatedImageMode(mode) ? 12 : 30;
    for (int fps : values) {
        combo->addItem(MS_TR("%1 fps").arg(fps), fps);
    }
    // 持久化帧率可能不在阶梯选项内（例如旧配置的 120fps）：补一个自定义项，
    // 避免静默回退到默认值。
    if (requestedFps > 0 && !values.contains(requestedFps)) {
        combo->addItem(MS_TR("%1 fps").arg(requestedFps), requestedFps);
    }
    const int requestedIndex = combo->findData(requestedFps);
    const int fallbackIndex = combo->findData(fallback);
    combo->setCurrentIndex(requestedIndex >= 0 ? requestedIndex : (fallbackIndex >= 0 ? fallbackIndex : 0));
}

/**
 * 填充采集后端下拉框。
 * @param combo 采集后端下拉框。
 * @param requested 请求后端。
 * @return 无返回值。
 */
void populateBackendOptions(QComboBox *combo, RecordingCaptureBackend requested)
{
    if (!combo) {
        return;
    }
    combo->clear();
    combo->addItem(MS_TR("Auto"), static_cast<int>(RecordingCaptureBackend::Auto));
#if defined(_WIN32)
    combo->addItem(MS_TR("Windows Graphics Capture"), static_cast<int>(RecordingCaptureBackend::WindowsWgc));
#else
    combo->addItem(MS_TR("wlroots screencopy"), static_cast<int>(RecordingCaptureBackend::Wlroots));
    combo->addItem(MS_TR("PipeWire"), static_cast<int>(RecordingCaptureBackend::PipeWire));
#endif
    combo->addItem(MS_TR("Polling"), static_cast<int>(RecordingCaptureBackend::Polling));
    const int index = combo->findData(static_cast<int>(requested));
    combo->setCurrentIndex(index >= 0 ? index : 0);
}

/**
 * 从下拉框数据读取录制模式。
 * @param combo 录制模式下拉框。
 * @param fallback 默认录制模式。
 * @return 录制模式。
 */
RecordingMode modeFromCombo(const QComboBox *combo, RecordingMode fallback)
{
    bool ok = false;
    const int value = combo ? combo->currentData().toInt(&ok) : 0;
    if (!ok) {
        return fallback;
    }
    if (value == static_cast<int>(RecordingMode::Webp)) {
        return RecordingMode::Webp;
    }
    return value == static_cast<int>(RecordingMode::Video)
        ? RecordingMode::Video
        : RecordingMode::Gif;
}

/**
 * 从下拉框数据读取采集后端。
 * @param combo 采集后端下拉框。
 * @return 采集后端。
 */
RecordingCaptureBackend backendFromCombo(const QComboBox *combo)
{
    bool ok = false;
    const int value = combo ? combo->currentData().toInt(&ok) : 0;
    if (!ok) {
        return RecordingCaptureBackend::Auto;
    }
    switch (static_cast<RecordingCaptureBackend>(value)) {
    case RecordingCaptureBackend::Wlroots:
    case RecordingCaptureBackend::PipeWire:
    case RecordingCaptureBackend::WindowsWgc:
    case RecordingCaptureBackend::Polling:
        return static_cast<RecordingCaptureBackend>(value);
    case RecordingCaptureBackend::Auto:
        break;
    }
    return RecordingCaptureBackend::Auto;
}

}  // namespace

RecordingConfigDialog::RecordingConfigDialog(RecordingMode mode, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
    , m_sources(availableDisplaySources())
{
    const RecordingDialogConfig persisted = configuredRecordingDialogConfig(m_mode);
    m_videoFps = persisted.videoFps;
    m_gifFps = persisted.gifFps;
    setWindowTitle(titleForMode(m_mode));
    setMinimumSize(460, 400);
    // 主题跟随用户设置：与设置窗口共享设计 token，按 ui.theme 选择明暗样式
    // 与调色板，保证复选框勾选、下拉弹层等原生绘制也随主题一致。
    const markshot::ui::UiThemeMode effectiveTheme = effectiveDialogTheme();
    setPalette(markshot::settings::tokens::settingsPalette(effectiveTheme));
    setStyleSheet(markshot::theme::recordingDialogStyleSheet(
        effectiveTheme != markshot::ui::UiThemeMode::Light));

    // 滚轮防护：与设置窗口一致，未聚焦的下拉框不再被滚轮误改内容。
    markshot::settings::installSettingsWheelGuard(this);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 18);
    root->setSpacing(16);

    m_title = new QLabel(titleForMode(m_mode), this);
    m_title->setObjectName(QStringLiteral("recordingDialogTitle"));
    m_title->setFont(markshot::theme::uiFont(16, QFont::DemiBold));
    root->addWidget(m_title);

    auto *form = new QFormLayout;
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(12);
    root->addLayout(form);

    // 下拉框全部走设置窗口的行辅助函数：设置光标、禁用上下文菜单，
    // 并安装控件级滚轮抑制——无论聚焦与否，滚轮都绝不篡改选中项。
    m_modeSelector = markshot::settings::addComboRow(form, MS_TR("Recording Type"));
    m_modeSelector->setObjectName(QStringLiteral("recordingModeSelector"));
    m_modeSelector->addItem(QStringLiteral("GIF"), static_cast<int>(RecordingMode::Gif));
    m_modeSelector->addItem(MS_TR("WebP"), static_cast<int>(RecordingMode::Webp));
    m_modeSelector->addItem(MS_TR("Video"), static_cast<int>(RecordingMode::Video));
    const int modeIndex = m_modeSelector->findData(static_cast<int>(m_mode));
    m_modeSelector->setCurrentIndex(modeIndex >= 0 ? modeIndex : 0);

    m_fps = markshot::settings::addComboRow(form, MS_TR("Frame Rate"));
    m_fps->setObjectName(QStringLiteral("recordingFps"));
    populateFrameRateOptions(m_fps, m_mode, fpsForMode(m_mode));

    m_audio = markshot::settings::addSwitchRow(form,
                                               MS_TR("Audio"),
                                               MS_TR("Record system default audio input"));
    m_audio->setObjectName(QStringLiteral("recordingAudioCheck"));
    m_audio->setChecked(persisted.includeAudio);

    m_display = markshot::settings::addComboRow(form, MS_TR("Display"));
    m_display->setObjectName(QStringLiteral("recordingDisplay"));
    for (int i = 0; i < m_sources.size(); ++i) {
        const DisplaySource &source = m_sources.at(i);
        const QString subtitle = QStringLiteral("%1 x %2").arg(source.geometry.width()).arg(source.geometry.height());
        m_display->addItem(QStringLiteral("%1  ·  %2").arg(source.title, subtitle), i);
    }
    const int savedSourceIndex = displaySourceIndexForKey(m_sources, persisted.displayKey);
    const int currentSourceIndex = savedSourceIndex >= 0 ? savedSourceIndex : currentDisplaySourceIndex(m_sources);
    if (currentSourceIndex >= 0) {
        const int comboIndex = m_display->findData(currentSourceIndex);
        m_display->setCurrentIndex(comboIndex >= 0 ? comboIndex : 0);
    }

    m_backend = markshot::settings::addComboRow(form, MS_TR("Recording Backend"));
    m_backend->setObjectName(QStringLiteral("recordingBackend"));
    populateBackendOptions(m_backend, persisted.backend);

    m_scope = markshot::settings::addComboRow(form, MS_TR("Capture Area"));
    m_scope->setObjectName(QStringLiteral("recordingScope"));
    m_scope->addItem(MS_TR("Record selected display"), static_cast<int>(RecordingScope::Display));
    m_scope->addItem(MS_TR("Select region after this dialog"), static_cast<int>(RecordingScope::Region));
    const int scopeIndex = m_scope->findData(static_cast<int>(persisted.scope));
    m_scope->setCurrentIndex(scopeIndex >= 0 ? scopeIndex : 0);

    m_outputPath = new QLineEdit(persisted.outputPath.isEmpty()
                                     ? defaultRecordingPath(m_mode)
                                     : normalizedRecordingPath(persisted.outputPath, m_mode),
                                 this);
    m_outputPath->setObjectName(QStringLiteral("recordingOutputPath"));
    m_outputPathTouched = false;
    auto *outputRow = new QWidget(this);
    auto *outputLayout = new QHBoxLayout(outputRow);
    outputLayout->setContentsMargins(0, 0, 0, 0);
    outputLayout->setSpacing(8);
    auto *outputBrowse = new QPushButton(MS_TR("Browse"), outputRow);
    outputBrowse->setObjectName(QStringLiteral("recordingBrowseButton"));
    outputLayout->addWidget(m_outputPath, 1);
    outputLayout->addWidget(outputBrowse);
    form->addRow(MS_TR("Output"), outputRow);

    // 拉伸项吸收窗口放大/最大化后的多余纵向空间，避免表单行被拉伸变形；
    // 与设置页"内容 + 页脚按钮"的布局一致，按钮始终贴底。
    root->addStretch();

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Ok, this);
    buttons->button(QDialogButtonBox::Ok)->setText(MS_TR("Start"));
    buttons->button(QDialogButtonBox::Cancel)->setText(MS_TR("Cancel"));
    root->addWidget(buttons);

    connect(outputBrowse, &QPushButton::clicked, this, [this] { browseOutputPath(); });
    connect(m_scope, &QComboBox::currentIndexChanged, this, [this] { updateDisplayControls(); });
    connect(m_modeSelector, &QComboBox::currentIndexChanged, this, [this] {
        const RecordingMode nextMode = modeFromCombo(m_modeSelector, m_mode);
        if (nextMode == m_mode) {
            return;
        }
        storeCurrentFpsForMode(m_mode);
        const bool preserveOutput = m_outputPathTouched
            && m_outputPath
            && !m_outputPath->text().trimmed().isEmpty();
        m_mode = nextMode;
        setWindowTitle(titleForMode(m_mode));
        if (m_title) {
            m_title->setText(titleForMode(m_mode));
        }
        populateFrameRateOptions(m_fps, m_mode, fpsForMode(m_mode));
        updateAudioControls();
        if (m_outputPath) {
            m_outputPath->setText(preserveOutput
                                      ? normalizedRecordingPath(m_outputPath->text(), m_mode)
                                      : defaultRecordingPath(m_mode));
        }
    });
    connect(m_outputPath, &QLineEdit::textEdited, this, [this] {
        m_outputPathTouched = true;
    });
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    updateAudioControls();
    updateDisplayControls();
}

RecordingOptions RecordingConfigDialog::options() const
{
    RecordingOptions result;
    result.mode = m_mode;
    bool fpsOk = false;
    const int fallbackFps = isAnimatedImageMode(m_mode) ? 12 : 30;
    const int selectedFps = m_fps ? m_fps->currentData().toInt(&fpsOk) : fallbackFps;
    const int effectiveFps = fpsOk ? selectedFps : fallbackFps;
    result.fps = effectiveFps;
    // 记录两个模式各自的帧率：当前模式取下拉框实时值，另一模式取内存中
    // 最后一次切换时保存的值，避免"切模式后直接开始"丢掉另一模式的选择。
    result.videoFps = m_mode == RecordingMode::Video ? effectiveFps : m_videoFps;
    result.gifFps = isAnimatedImageMode(m_mode) ? effectiveFps : m_gifFps;
    result.includeAudio = m_audio && m_audio->isEnabled() && m_audio->isChecked();
    result.captureBackend = backendFromCombo(m_backend);
    result.scope = static_cast<RecordingScope>(m_scope ? m_scope->currentData().toInt() : static_cast<int>(RecordingScope::Region));
    result.outputPath = normalizedRecordingPath(m_outputPath ? m_outputPath->text() : QString(), m_mode);

    const int sourceIndex = m_display ? m_display->currentData().toInt() : -1;
    if (sourceIndex >= 0 && sourceIndex < m_sources.size()) {
        result.display = m_sources.at(sourceIndex);
    }
    if (result.scope == RecordingScope::Display) {
        result.captureGeometry = result.display.geometry;
    }
    return result;
}

void RecordingConfigDialog::browseOutputPath()
{
    QString filter;
    if (m_mode == RecordingMode::Gif) {
        filter = MS_TR("GIF Images (*.gif)");
    } else if (m_mode == RecordingMode::Webp) {
        filter = MS_TR("WebP Images (*.webp)");
    } else {
        filter = MS_TR("MP4 Videos (*.mp4)");
    }
    const QString path = QFileDialog::getSaveFileName(this,
                                                      MS_TR("Save Recording"),
                                                      m_outputPath ? m_outputPath->text() : defaultRecordingPath(m_mode),
                                                      filter);
    if (!path.isEmpty() && m_outputPath) {
        m_outputPathTouched = true;
        m_outputPath->setText(normalizedRecordingPath(path, m_mode));
    }
}

void RecordingConfigDialog::updateAudioControls()
{
    if (!m_audio) {
        return;
    }
    const bool videoMode = m_mode == RecordingMode::Video;
    const bool audioAvailable = recordingAudioCaptureAvailable();
    m_audio->setEnabled(videoMode && audioAvailable);
    if (!videoMode) {
        m_audio->setChecked(false);
        m_audio->setToolTip(MS_TR("GIF recording does not include audio."));
    } else if (!audioAvailable) {
        m_audio->setChecked(false);
        m_audio->setToolTip(recordingAudioUnavailableText());
    } else {
        m_audio->setToolTip(QString());
    }
}

/**
 * 按录制范围更新显示器控件可用状态。
 * @return 无返回值。
 */
void RecordingConfigDialog::updateDisplayControls()
{
    if (!m_display || !m_scope) {
        return;
    }
    bool ok = false;
    const int value = m_scope->currentData().toInt(&ok);
    const bool regionScope = ok && value == static_cast<int>(RecordingScope::Region);
    m_display->setEnabled(!regionScope && !m_sources.isEmpty());
    m_display->setToolTip(regionScope ? MS_TR("A region is selected on screen after this dialog.") : QString());
}

/**
 * 读取指定录制模式的帧率状态。
 * @param mode 录制模式。
 * @return 帧率。
 */
int RecordingConfigDialog::fpsForMode(RecordingMode mode) const
{
    return isAnimatedImageMode(mode) ? m_gifFps : m_videoFps;
}

/**
 * 保存当前帧率下拉框状态到指定录制模式。
 * @param mode 录制模式。
 * @return 无返回值。
 */
void RecordingConfigDialog::storeCurrentFpsForMode(RecordingMode mode)
{
    if (!m_fps) {
        return;
    }
    bool ok = false;
    const int value = m_fps->currentData().toInt(&ok);
    if (!ok) {
        return;
    }
    if (isAnimatedImageMode(mode)) {
        m_gifFps = value;
        return;
    }
    m_videoFps = value;
}

}  // namespace markshot::recording
