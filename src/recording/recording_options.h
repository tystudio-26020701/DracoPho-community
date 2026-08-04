#pragma once

#include "recording/recording_capture_backend.h"

#include <QRect>
#include <QString>

namespace markshot::recording {

enum class RecordingMode {
    Gif,
    Video,
    Webp,
};

/**
 * 判断是否为动图录制模式（GIF / 动画 WebP）。
 * 所有"动图 vs 视频"分流都应走此谓词，避免各文件自行枚举造成漏改。
 * @param mode 录制模式。
 * @return 动图模式时返回 true。
 */
inline bool isAnimatedImageMode(RecordingMode mode)
{
    return mode == RecordingMode::Gif || mode == RecordingMode::Webp;
}

enum class RecordingScope {
    Display,
    Region,
};

struct DisplaySource {
    bool allOutputs = false;
    QString screenName;
    QString outputName;
    QString title;
    QRect geometry;
};

struct RecordingOptions {
    RecordingMode mode = RecordingMode::Gif;
    RecordingScope scope = RecordingScope::Region;
    int fps = 12;
    // Per-mode frame rates retained so a mode switch followed by "Start" does
    // not silently drop the frame rate chosen for the other mode.
    int videoFps = 30;
    int gifFps = 12;
    bool includeAudio = false;
    RecordingCaptureBackend captureBackend = RecordingCaptureBackend::Auto;
    DisplaySource display;
    QRect captureGeometry;
    QString outputPath;
    // 静默录制（无人值守 CLI / MCP 触发）：不弹桌面通知、不发起交互式 portal
    // 授权，保证对用户完全无感、不抢焦点。
    bool silent = false;
};

}  // namespace markshot::recording
