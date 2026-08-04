#pragma once

#include "recording/recording_options.h"

#include <QPoint>
#include <QWidget>

#include <functional>

class QMenu;
class QMouseEvent;
class QPaintEvent;
class QPropertyAnimation;
class QTimer;

namespace markshot {

/// @brief 悬浮球（类 PixPin 桌面快捷入口）。
///
/// 始终置顶的小型圆形窗口，可拖动改变位置（Wayland 下走合成器系统拖动，
/// 其余平台回退到手拖）。拖动释放时若靠近屏幕边缘则触发"边缘停靠"：
/// 球体向屏幕外方向平移一半（露出部分通过 setMask 限定输入区域），实现
/// "隐入靠边缘的一半"的视觉效果。该停靠仅在可程序化定位的平台可用
/// （X11/Windows/macOS/offscreen）；原生 Wayland 客户端无法获取自己的屏幕
/// 绝对坐标，基于坐标的吸附与偏移计算不可靠（会把球画到窗口外、产生鬼影
/// 拖尾），故 Wayland 上停靠不可用（协议限制），球体保持自由漂浮。
/// 悬停/按下滑出完整显示，移开后延迟隐回。闲置数秒后自动淡出为半透明
/// （paintEvent 自绘 alpha，平台无关），鼠标悬停/移动立即恢复不透明度。
/// 淡出判定由"交互时间戳 + 常驻 tick"驱动，不依赖 leaveEvent，杜绝"移开
/// 不淡出"。左键单击/右键弹出快捷菜单，双击快速截图。截图会话进行中自动
/// 隐藏，会话结束后自动恢复显示（用户主动隐藏的状态不会被会话覆盖）。
/// 位置持久化（仅限坐标可靠的平台，Wayland 上跳过以免陈旧坐标覆盖）。
class FloatingBall final : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal fadeAlpha READ fadeAlpha WRITE setFadeAlpha)

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

    /// @brief 当前不透明度（Q_PROPERTY，供淡入淡出动画驱动）。
    /// @return 不透明度（0..1）。
    qreal fadeAlpha() const;

    /// @brief 直接设置当前不透明度（Q_PROPERTY，动画目标）。
    /// @param alpha 不透明度（0..1）。
    void setFadeAlpha(qreal alpha);

    /// @brief 直接设置悬浮球整体不透明度（无动画；兼容既有调用点）。
    /// @param opacity 不透明度（0..1）。
    void setBallOpacity(qreal opacity);

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
    /// @brief 边缘停靠状态。
    enum class DockState {
        Floating, ///< 自由漂浮，未停靠
        Snapped,  ///< 停靠并隐入边缘（球体一半在屏幕外）
        Revealed, ///< 停靠且滑出（完全可见，等待隐回）
    };

    /// @brief 停靠的屏幕边缘。
    enum class DockEdge {
        None,
        Left,
        Right,
        Top,
        Bottom,
    };

    /// @brief 显示悬浮球快捷菜单。
    /// @param globalPos 菜单出现位置（全局坐标）。
    void showBallMenu(const QPoint &globalPos);

    /// @brief 从悬浮球启动录制（复用托盘菜单的录制流程）。
    void startRecordingFromBall();

    /// @brief 记录一次用户交互（刷新闲置计时），并恢复不透明度。
    void noteInteraction();

    /// @brief 常驻 tick：按"距最近交互的时间"决定淡出/恢复不透明。
    /// 这是闲置淡出的唯一判定（不依赖 leaveEvent，可自愈事件丢失）。
    void onFadeTick();

    /// @brief 平滑淡入/淡出到目标不透明度（QPropertyAnimation 驱动 fadeAlpha）。
    /// @param target 目标不透明度（0..1）。
    void fadeTo(qreal target);

    /// @brief 读取闲置淡出配置（秒数与目标不透明度）。
    /// @param fadeSeconds 输出淡出延迟秒数（0 表示禁用淡出）。
    /// @param idleOpacity 输出闲置时不透明度（0..1）。
    void idleFadeConfig(int *fadeSeconds, double *idleOpacity) const;

    /// @brief 边缘停靠功能是否启用（配置开关）。
    /// @return 启用时返回 true。
    bool edgeDockEnabled() const;

    /// @brief 当前会话是否支持程序化定位（X11/Windows/macOS 支持；
    /// 原生 Wayland 顶层窗口位置由合成器决定，move() 无效）。
    /// @return 支持时返回 true。
    bool positioningEnabled() const;

    /// @brief 读取边缘停靠隐入像素数（默认球半径，露出约一半）。
    /// @return 隐入像素数。
    int hiddenExtentPx() const;

    /// @brief 检测给定期望位置贴哪条屏幕边缘。
    /// @param desired 期望位置（窗口左上角全局坐标）。
    /// @return 未靠边时返回 DockEdge::None。
    DockEdge detectDockEdge(const QPoint &desired) const;

    /// @brief 停靠到边缘（进入 Snapped 状态：球体向屏幕外方向平移）。
    /// @param edge 停靠边缘。
    /// @return 平台不支持停靠（原生 Wayland，坐标不可靠）时返回 false。
    bool enterDocked(DockEdge edge);

    /// @brief 计算使球体在指定边缘"隐藏 extent 像素于屏幕外"所需的内容偏移。
    /// @param edge 停靠边缘。
    /// @param available 目标屏幕可用几何。
    /// @param offset 输出内容偏移。
    /// @return 传入的边缘。
    DockEdge computeDockOffset(DockEdge edge, const QRect &available, QPoint *offset) const;

    /// @brief 退出停靠态，回到自由漂浮（清除偏移与输入 mask）。
    void exitDockState();

    /// @brief 拖动结束的统一处理（mouseReleaseEvent 与拖动卡死自愈共用）。
    ///
    /// Wayland 系统移动（startSystemMove）结束时合成器接管手势，客户端可能
    /// 永远收不到 mouseReleaseEvent；此时必须由拖动卡死自愈补一次 finishDrag，
    /// 否则停靠/保存位置等拖动结束逻辑永不执行。
    void finishDrag();

    /// @brief 从停靠隐入状态滑出（完全可见）。
    void revealBall();

    /// @brief 从滑出状态隐回边缘。
    void autoHideBall();

    /// @brief 刷新停靠的视觉状态：应用/清除内容偏移与输入 mask。
    void applyDockVisuals();

    /// @brief 处理屏幕变化（拔插/分辨率/方向）：重算停靠或回退。
    void revalidateDockState();

    /// @brief 读取已保存位置是否仍可用（球与屏幕几何有交集即可）。
    /// @param topLeft 窗口左上角全局坐标。
    /// @return 可用时返回 true。
    bool positionWithinScreenBounds(const QPoint &topLeft) const;

    CaptureCallback m_captureCallback;
    CaptureCallback m_fullscreenCallback;
    RecordingRegionCallback m_recordingRegionCallback;
    QMenu *m_menu = nullptr;
    QTimer *m_clickTimer = nullptr;
    QTimer *m_fadeTickTimer = nullptr;
    QTimer *m_autoHideTimer = nullptr;
    QPropertyAnimation *m_fadeAnim = nullptr;
    QPoint m_pendingClickPos;
    QPoint m_dragOffset;
    QPoint m_pressGlobalPos;
    QPoint m_pressFrameTopLeft;
    qreal m_fadeAlpha = 1.0;
    qint64 m_lastInteractionMs = -1; ///< 最近一次交互的单调时钟毫秒（-1=从未交互）
    qint64 m_lastDragActivityMs = -1; ///< 最近一次拖动活动的单调时钟毫秒（系统移动超时自愈用）
    DockState m_dockState = DockState::Floating;
    DockEdge m_dockEdge = DockEdge::None;
    QPoint m_contentOffset; ///< 停靠隐入时球体绘制偏移（指向屏幕外方向）
    bool m_quitWhenHidden = false;
    bool m_dragging = false;
    bool m_moved = false;
    bool m_menuOpen = false;
    bool m_hiddenByUser = false;
    bool m_systemMoveStarted = false;
};

}  // namespace markshot
