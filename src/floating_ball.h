#pragma once

#include "recording/recording_options.h"

#include <QPoint>
#include <QWidget>

#include <functional>

class QMenu;
class QMouseEvent;
class QPaintEvent;
class QTimer;

namespace markshot {

/// @brief 悬浮球（类 PixPin 桌面快捷入口）。
///
/// 始终置顶的小型圆形窗口，可拖动改变位置；左键单击/右键弹出快捷菜单，
/// 双击快速截图。截图会话进行中自动隐藏，会话结束后自动恢复显示。
/// 位置持久化到应用配置。
class FloatingBall final : public QWidget {
    Q_OBJECT

public:
    using CaptureCallback = std::function<void()>;
    using RecordingRegionCallback = std::function<void(recording::RecordingOptions)>;

    /// @brief 创建悬浮球。
    /// @param parent 父控件。
    explicit FloatingBall(QWidget *parent = nullptr);

    /// @brief 设置截图回调。
    /// @param capture 区域截图回调。
    /// @param fullscreen 全屏截图回调。
    void setCaptureCallbacks(CaptureCallback capture, CaptureCallback fullscreen);

    /// @brief 设置录制区域回调。
    /// @param callback 区域录制回调。
    void setRecordingRegionCallback(RecordingRegionCallback callback);

    /// @brief 设置"隐藏悬浮球"后的行为。
    ///
    /// 当没有托盘入口（ball-only 组合）时，隐藏悬浮球会失去唯一入口，
    /// 此时隐藏动作改为退出应用；有托盘时仅隐藏悬浮球本身。
    /// @param quitWhenHidden 隐藏悬浮球时是否直接退出应用。
    void setQuitWhenHidden(bool quitWhenHidden);

    /// @brief 保存当前位置到配置。
    void savePosition();

    /// @brief 在屏幕上定位悬浮球（默认屏幕右下角，使用已保存位置时跳过）。
    void placeOnScreen();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    /// @brief 显示悬浮球快捷菜单。
    /// @param globalPos 菜单出现位置（全局坐标）。
    void showBallMenu(const QPoint &globalPos);

    /// @brief 从悬浮球启动录制（复用托盘菜单的录制流程）。
    void startRecordingFromBall();

    CaptureCallback m_captureCallback;
    CaptureCallback m_fullscreenCallback;
    RecordingRegionCallback m_recordingRegionCallback;
    QMenu *m_menu = nullptr;
    QTimer *m_clickTimer = nullptr;
    QPoint m_pendingClickPos;
    QPoint m_dragOffset;
    bool m_quitWhenHidden = false;
    bool m_dragging = false;
    bool m_moved = false;
    bool m_hovered = false;
};

}  // namespace markshot
