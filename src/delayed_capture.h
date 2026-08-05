#pragma once

#include <QColor>
#include <QWidget>

#include <functional>

class QKeyEvent;
class QPaintEvent;
class QTimer;

namespace markshot {

/// @brief 延时截图倒计时遮罩。
///
/// 全屏半透明覆盖层，倒计时结束后自动隐藏并调用截图回调；用户按 Esc
/// 可取消。与截图自身 UI 一样在抓屏时从画面中排除（Windows 走
/// WDA_EXCLUDEFROMCAPTURE，其余平台靠 hide() 后等待合成器重绘）。
class DelayedCaptureOverlay final : public QWidget {
    Q_OBJECT

public:
    using Callback = std::function<void()>;

    /// @brief 创建倒计时遮罩（不自动显示）。
    /// @param seconds 倒计时秒数（>0）。
    /// @param onCapture 倒计时结束时的截图回调。
    /// @param onCancelled 用户按 Esc 取消时的回调（可选）。
    explicit DelayedCaptureOverlay(int seconds,
                                   Callback onCapture,
                                   Callback onCancelled = {});
    ~DelayedCaptureOverlay() override;

    /// @brief 显示遮罩并启动倒计时。
    void start();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    /// @brief 每秒递减并重绘剩余秒数。
    void onTick();

    /// @brief 倒计时结束：隐藏自身并调用截图回调。
    void finish();

    /// @brief 用户取消：隐藏自身并调用取消回调。
    void cancel();

    int m_remaining = 0;
    Callback m_onCapture;
    Callback m_onCancelled;
    QTimer *m_timer = nullptr;
    QColor m_background;
    QColor m_text;
};

/// @brief 运行一次延时截图。
///
/// seconds 大于 0 时显示全屏倒计时遮罩，倒计时结束（或用户取消）后
/// 自动清理遮罩并调用对应回调；seconds 小于等于 0 时立即调用 onCapture。
/// 所有回调均在主线程、遮罩隐藏之后触发。
/// @param seconds 延时秒数。
/// @param onCapture 延时结束后的截图回调。
/// @param onCancelled Esc 取消回调（可选）。
void runDelayedCapture(int seconds,
                       std::function<void()> onCapture,
                       std::function<void()> onCancelled = {});

}  // namespace markshot
