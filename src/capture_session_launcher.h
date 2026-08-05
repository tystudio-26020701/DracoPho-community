#pragma once

#include "capture_freeze_scope.h"
#include "recording/recording_options.h"
#include "shot_window.h"
#include "startup_config.h"

#include <QObject>
#include <QPointer>
#include <QSet>
#include <QVector>

#include <optional>

class QApplication;

namespace markshot {

/// @brief 截图会话全局监视器。
///
/// 跟踪当前所有存活的截图覆盖窗口（含"显示器快速截取"编辑目标时替换出的
/// 新窗口），最后一个窗口销毁时发出 sessionEnded 信号。截图发起方据此统一
/// 恢复被临时隐藏的本软件 UI，而不是按"最初那批窗口的销毁数"判断——否则
/// 在会话内替换窗口时（旧窗口销毁、新窗口仍全屏）会提前恢复悬浮球/设置
/// 窗口，把它们重新压回截图覆盖层之上。
class CaptureSessionMonitor final : public QObject {
    Q_OBJECT

public:
    static CaptureSessionMonitor &instance();

    /// @brief 登记一个截图覆盖窗口；同一窗口重复登记会被忽略。
    /// @param window 截图覆盖窗口。
    void registerWindow(QObject *window);

    /// @brief 当前是否仍有存活的截图覆盖窗口。
    /// @return 有存活窗口时返回 true。
    bool hasActiveWindows() const;

signals:
    /// @brief 最后一个截图覆盖窗口销毁（整场截图会话结束）。
    void sessionEnded();

private:
    explicit CaptureSessionMonitor(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    QSet<QObject *> m_liveWindows;
};

/// @brief 启动截图会话并显示对应冻结窗口。
/// @param app QApplication 实例。
/// @param allOutputs 是否把所有输出捕获为一张虚拟桌面图片。
/// @param freezeScope 普通区域截图的显示器冻结范围。
/// @param includeCursor 冻结图是否包含鼠标。
/// @param hideOwnWindows 是否让截屏后端隐藏 mark-shot 自身窗口。
/// @param useRegularWindow 是否使用普通窗口替代 layer-shell。
/// @param fullscreenAnnotation 是否直接进入全屏标注。
/// @param defaultTools 默认工具配置。
/// @param error 输出错误信息。
/// @param regionRecordingOptions 区域录制配置，为空时启动普通截图流程。
/// @param windowCaptureMode 是否以"窗口捕获"模式启动（预选窗口捕获工具）。
/// @return 创建出的截图窗口列表。
QVector<QPointer<ShotWindow>> showCaptureSession(QApplication *app,
                                                 bool allOutputs,
                                                 CaptureFreezeScope freezeScope,
                                                 bool includeCursor,
                                                 bool hideOwnWindows,
                                                 bool useRegularWindow,
                                                 bool fullscreenAnnotation,
                                                 const DefaultTools &defaultTools,
                                                 QString *error,
                                                 std::optional<recording::RecordingOptions> regionRecordingOptions = std::nullopt,
                                                 bool windowCaptureMode = false);

}  // namespace markshot
