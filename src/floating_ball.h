#pragma once

#include "recording/recording_options.h"

#include <QPoint>
#include <QWidget>

#include <functional>

class QGraphicsOpacityEffect;
class QMenu;
class QMouseEvent;
class QPaintEvent;
class QTimer;

namespace markshot {

/// @brief 悬浮球（类 PixPin 桌面快捷入口）。
///
/// 始终置顶的小型圆形窗口，可拖动改变位置（Wayland 下走合成器系统拖动，
/// 其余平台回退到手拖）；释放时若靠近屏幕边缘则吸附贴边。空闲数秒后自动
/// 变为半透明，鼠标悬停时恢复不透明度，避免长时间遮挡用户操作。左键单击/
/// 右键弹出快捷菜单，双击快速截图。截图会话进行中自动隐藏，会话结束后自动
/// 恢复显示（用户主动隐藏的状态不会被会话覆盖）。位置持久化到应用配置。
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

    /// @brief 用户是否主动隐藏了悬浮球。
    ///
    /// 截图会话期间隐藏/恢复显示不应覆盖该状态；只有用户显式隐藏（菜单项、
    /// 托盘开关）才置位。
    bool isHiddenByUser() const;

    /// @brief 按用户意图切换悬浮球显示状态（托盘"显示/隐藏悬浮球"用）。
    void toggleByUser();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    /// @brief 显示悬浮球快捷菜单。
    /// @param globalPos 菜单出现位置（全局坐标）。
    void showBallMenu(const QPoint &globalPos);

    /// @brief 从悬浮球启动录制（复用托盘菜单的录制流程）。
    void startRecordingFromBall();

    /// @brief 暂停闲置淡出（拖动/菜单打开时调用），并恢复不透明度。
    void pauseIdleFade();

    /// @brief 启动闲置淡出计时器（离开悬浮球时调用）。
    void restartIdleFade();

    /// @brief 设置悬浮球整体不透明度（0..1）。
    /// @param opacity 不透明度。
    void setBallOpacity(qreal opacity);

    /// @brief 读取闲置淡出配置（秒数与目标不透明度）。
    /// @param fadeSeconds 输出淡出延迟秒数（0 表示禁用淡出）。
    /// @param idleOpacity 输出闲置时不透明度（0..1）。
    void idleFadeConfig(int *fadeSeconds, double *idleOpacity) const;

    /// @brief 计算边缘吸附后的位置。
    /// @param desired 期望位置（窗口左上角全局坐标）。
    /// @return 靠近屏幕边缘时被吸附调整后的位置。
    QPoint snappedPosition(const QPoint &desired) const;

    /// @brief 拖动结束后吸附到屏幕边缘并保存位置。
    void snapAndSavePosition();

    CaptureCallback m_captureCallback;
    CaptureCallback m_fullscreenCallback;
    RecordingRegionCallback m_recordingRegionCallback;
    QMenu *m_menu = nullptr;
    QTimer *m_clickTimer = nullptr;
    QTimer *m_fadeTimer = nullptr;
    QGraphicsOpacityEffect *m_opacityEffect = nullptr;
    QPoint m_pendingClickPos;
    QPoint m_dragOffset;
    QPoint m_pressGlobalPos;
    QPoint m_pressFrameTopLeft;
    bool m_quitWhenHidden = false;
    bool m_dragging = false;
    bool m_moved = false;
    bool m_hovered = false;
    bool m_hiddenByUser = false;
    bool m_systemMoveStarted = false;
};

}  // namespace markshot
