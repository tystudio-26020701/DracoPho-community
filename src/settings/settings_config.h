#pragma once

#include "capture_freeze_scope.h"
#include "clipboard_image_config.h"
#include "export_image_effect.h"
#include "headless_capture_config.h"
#include "shortcut_config.h"
#include "shot_window.h"
#include "ui/interface_language_config.h"
#include "ui/interface_theme_config.h"

#include <QColor>
#include <QKeySequence>
#include <QMap>
#include <QString>

namespace markshot::settings {

struct GeneralSettings {
    markshot::ui::UiLanguageMode uiLanguageMode = markshot::ui::UiLanguageMode::System;
    markshot::ui::UiThemeMode uiThemeMode = markshot::ui::UiThemeMode::System;
    bool trayEnabled = false;
    bool launchOnStartup = false;
    bool hotkeysEnabled = true;
    QKeySequence captureHotkey = QKeySequence(QStringLiteral("Ctrl+Alt+S"));
    QKeySequence fullscreenHotkey;
};

struct CaptureSettings {
    bool includeCursor = false;
    CaptureFreezeScope freezeScope = CaptureFreezeScope::AllScreens;
    bool kdeKwinScreenshotEnabled = true;
    bool hideOwnWindows = true;
};

struct ShortcutSettings {
    shortcut::ActionShortcuts actions;
    shortcut::ToolShortcuts tools;
    QKeySequence startupColorPicker;
    QKeySequence startupRuler;
    QKeySequence startupCodeScanner;
    QKeySequence startupDisplayCapture;
    QKeySequence startupGifRecorder;
    QKeySequence startupVideoRecorder;
};

struct AnnotationSettings {
    ShotWindow::Tool normalTool = ShotWindow::Tool::Pen;
    ShotWindow::Tool fullscreenTool = ShotWindow::Tool::Pen;
    ShotWindow::Tool fileTool = ShotWindow::Tool::Pen;
    QColor defaultColor = QColor(255, 77, 77);
    int toolbarIconSize = 22;
    int toolbarFontSize = 11;
};

struct PinnedSettings {
    bool alwaysOnTop = true;
    bool textSelectionCopyEnabled = true;
    bool ocrEnabled = true;
    bool autoOcr = false;
    QString ocrBackend = QStringLiteral("rapidocr");
    QString ocrProvider = QStringLiteral("auto");
    QString ocrCommand;
    int ocrTimeoutMs = 30000;
    bool autoTranslateAfterOcr = false;
    QString translationCommand;
    QString translationProvider = QStringLiteral("auto");
    QString translationTargetLanguage = QStringLiteral("Simplified Chinese");
    int translationTimeoutMs = 60000;
    bool borderEnabled = true;
    QColor borderColor;
    double borderWidth = 2.0;
};

struct StorageSettings {
    QString savePathTemplate;
    QString recordingVideoDirectory;
    QString recordingGifDirectory;
    ClipboardImageMode clipboardImageMode = ClipboardImageMode::ImagePng;
    int clipboardThresholdM = 4;
    ExportImageEffectConfig exportImageEffect;
    HeadlessCaptureDestination headlessDefaultDestination = HeadlessCaptureDestination::Inline;
    bool headlessClipboardAllowed = false;
};

struct ScrollSettings {
    bool frameEnabled = true;
    bool hidePreviewDuringCapture = false;
    int frameGap = 5;
    int previewGap = 5;
};

struct IntegrationSettings {
    QString codeScanCommand;
    QString codeScanProvider = QStringLiteral("auto");
    int codeScanTimeoutMs = 15000;
    QString uploadCommand;
    int uploadTimeoutMs = 30000;
    QMap<QString, QString> uploadEnv;
    bool ocrResultPanelEnabled = true;
    QString translationApiBase = QStringLiteral("https://api.openai.com/v1");
    QString translationApiKeyEnv = QStringLiteral("OPENAI_API_KEY");
    QString translationApiKey;
    QString translationModel = QStringLiteral("gpt-4o-mini");
    double translationTemperature = 0.2;
    QString translationSystemPrompt;
};

struct AdvancedSettings {
    bool debugEnabled = false;
    QString debugLogPath;
    bool windowDetectionEnabled = true;
    QString windowDetectionCommand;
    QString windowDetectionWorkingDirectory;
    int windowDetectionTimeoutMs = 1000;
    QMap<QString, QString> windowDetectionEnv;
    QMap<QString, QString> appEnv;
};

struct SettingsConfig {
    GeneralSettings general;
    CaptureSettings capture;
    ShortcutSettings shortcuts;
    AnnotationSettings annotation;
    PinnedSettings pinned;
    StorageSettings storage;
    ScrollSettings scroll;
    IntegrationSettings integrations;
    AdvancedSettings advanced;
};

inline bool operator==(const GeneralSettings &a, const GeneralSettings &b)
{
    return a.uiLanguageMode == b.uiLanguageMode && a.uiThemeMode == b.uiThemeMode
        && a.trayEnabled == b.trayEnabled && a.launchOnStartup == b.launchOnStartup
        && a.hotkeysEnabled == b.hotkeysEnabled && a.captureHotkey == b.captureHotkey
        && a.fullscreenHotkey == b.fullscreenHotkey;
}

inline bool operator==(const CaptureSettings &a, const CaptureSettings &b)
{
    return a.includeCursor == b.includeCursor && a.freezeScope == b.freezeScope
        && a.kdeKwinScreenshotEnabled == b.kdeKwinScreenshotEnabled
        && a.hideOwnWindows == b.hideOwnWindows;
}

inline bool operator==(const ShortcutSettings &a, const ShortcutSettings &b)
{
    return a.actions == b.actions && a.tools == b.tools
        && a.startupColorPicker == b.startupColorPicker && a.startupRuler == b.startupRuler
        && a.startupCodeScanner == b.startupCodeScanner
        && a.startupDisplayCapture == b.startupDisplayCapture
        && a.startupGifRecorder == b.startupGifRecorder
        && a.startupVideoRecorder == b.startupVideoRecorder;
}

inline bool operator==(const AnnotationSettings &a, const AnnotationSettings &b)
{
    return a.normalTool == b.normalTool && a.fullscreenTool == b.fullscreenTool
        && a.fileTool == b.fileTool && a.defaultColor == b.defaultColor
        && a.toolbarIconSize == b.toolbarIconSize && a.toolbarFontSize == b.toolbarFontSize;
}

inline bool operator==(const PinnedSettings &a, const PinnedSettings &b)
{
    return a.alwaysOnTop == b.alwaysOnTop
        && a.textSelectionCopyEnabled == b.textSelectionCopyEnabled
        && a.ocrEnabled == b.ocrEnabled && a.autoOcr == b.autoOcr
        && a.ocrBackend == b.ocrBackend && a.ocrProvider == b.ocrProvider
        && a.ocrCommand == b.ocrCommand && a.ocrTimeoutMs == b.ocrTimeoutMs
        && a.autoTranslateAfterOcr == b.autoTranslateAfterOcr
        && a.translationCommand == b.translationCommand
        && a.translationProvider == b.translationProvider
        && a.translationTargetLanguage == b.translationTargetLanguage
        && a.translationTimeoutMs == b.translationTimeoutMs
        && a.borderEnabled == b.borderEnabled && a.borderColor == b.borderColor
        && a.borderWidth == b.borderWidth;
}

inline bool operator==(const StorageSettings &a, const StorageSettings &b)
{
    return a.savePathTemplate == b.savePathTemplate
        && a.recordingVideoDirectory == b.recordingVideoDirectory
        && a.recordingGifDirectory == b.recordingGifDirectory
        && a.clipboardImageMode == b.clipboardImageMode
        && a.clipboardThresholdM == b.clipboardThresholdM
        && a.exportImageEffect == b.exportImageEffect
        && a.headlessDefaultDestination == b.headlessDefaultDestination
        && a.headlessClipboardAllowed == b.headlessClipboardAllowed;
}

inline bool operator==(const ScrollSettings &a, const ScrollSettings &b)
{
    return a.frameEnabled == b.frameEnabled
        && a.hidePreviewDuringCapture == b.hidePreviewDuringCapture
        && a.frameGap == b.frameGap && a.previewGap == b.previewGap;
}

inline bool operator==(const IntegrationSettings &a, const IntegrationSettings &b)
{
    return a.codeScanCommand == b.codeScanCommand
        && a.codeScanProvider == b.codeScanProvider
        && a.codeScanTimeoutMs == b.codeScanTimeoutMs
        && a.uploadCommand == b.uploadCommand
        && a.uploadTimeoutMs == b.uploadTimeoutMs && a.uploadEnv == b.uploadEnv
        && a.ocrResultPanelEnabled == b.ocrResultPanelEnabled
        && a.translationApiBase == b.translationApiBase
        && a.translationApiKeyEnv == b.translationApiKeyEnv
        && a.translationApiKey == b.translationApiKey
        && a.translationModel == b.translationModel
        && a.translationTemperature == b.translationTemperature
        && a.translationSystemPrompt == b.translationSystemPrompt;
}

inline bool operator==(const AdvancedSettings &a, const AdvancedSettings &b)
{
    return a.debugEnabled == b.debugEnabled && a.debugLogPath == b.debugLogPath
        && a.windowDetectionEnabled == b.windowDetectionEnabled
        && a.windowDetectionCommand == b.windowDetectionCommand
        && a.windowDetectionWorkingDirectory == b.windowDetectionWorkingDirectory
        && a.windowDetectionTimeoutMs == b.windowDetectionTimeoutMs
        && a.windowDetectionEnv == b.windowDetectionEnv && a.appEnv == b.appEnv;
}

/// @brief 读取设置界面使用的应用配置。
/// @param error 读取失败时输出错误信息。
/// @return 当前配置与默认值合并后的设置结构。
SettingsConfig readSettingsConfig(QString *error = nullptr);

/// @brief 保存设置界面修改后的应用配置。
/// @param config 需要保存的设置结构。
/// @param error 保存失败时输出错误信息。
/// @return 保存成功返回 true，否则返回 false。
bool writeSettingsConfig(const SettingsConfig &config, QString *error = nullptr);

/// @brief 将应用配置还原为出厂默认值。
/// @param error 还原失败时输出错误信息。
/// @return 还原成功返回 true，否则返回 false。
bool resetSettingsToDefaults(QString *error = nullptr);

/// @brief 返回工具枚举的规范配置名称。
/// @param tool 标注工具。
/// @return 可写入配置文件的工具名称。
QString toolConfigName(ShotWindow::Tool tool);

/// @brief 返回剪贴板图片策略的规范配置名称。
/// @param mode 剪贴板图片策略。
/// @return 可写入配置文件的策略名称。
QString clipboardImageModeName(ClipboardImageMode mode);

/// @brief 返回动作枚举的规范配置名称。
/// @param action 动作枚举。
/// @return 可写入配置文件的动作名称。
QString actionConfigName(ShotWindow::Action action);

/// @brief 将环境变量映射转换为多行文本。
/// @param values 环境变量映射。
/// @return KEY=VALUE 形式的多行文本。
QString envMapToText(const QMap<QString, QString> &values);

/// @brief 将多行文本解析为环境变量映射。
/// @param text KEY=VALUE 形式的多行文本。
/// @return 环境变量映射。
QMap<QString, QString> envMapFromText(const QString &text);

}  // namespace markshot::settings
