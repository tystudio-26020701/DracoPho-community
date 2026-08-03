#pragma once

#include "recording/recording_options.h"

#include <QString>

#include <functional>

class QWidget;

namespace markshot::recording {

struct RecordingStartFlowRequest {
    RecordingMode initialMode = RecordingMode::Video;
    QWidget *parent = nullptr;
    bool stayOnTop = false;
    std::function<void(RecordingOptions)> startDisplayRecording;
    std::function<void(RecordingOptions)> selectRegionRecording;
    std::function<void(const QString &)> showError;
    std::function<void()> cancelled;
};

/**
 * 运行录制启动流程。
 *
 * 与设置窗口一致：非模态创建对话框并以 show() 打开（WA_DeleteOnClose 自清理），
 * 对话框 accept / reject 后通过 request 回调异步分发结果。
 * @param request 启动流程回调和初始配置。
 */
void runRecordingStartFlow(const RecordingStartFlowRequest &request);

}  // namespace markshot::recording
