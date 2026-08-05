#include "delayed_capture.h"

#include "capture_own_windows_policy.h"
#include "ui/i18n.h"
#include "ui/theme.h"
#include "windows_integration.h"

#include <QFont>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QScreen>
#include <QTimer>

namespace markshot {

namespace {

/// @brief 倒计时文字的内边距。
constexpr int kTextPadding = 48;

}  // namespace

DelayedCaptureOverlay::DelayedCaptureOverlay(int seconds,
                                             Callback onCapture,
                                             Callback onCancelled)
    : QWidget(nullptr, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , m_remaining(std::max(seconds, 1))
    , m_onCapture(std::move(onCapture))
    , m_onCancelled(std::move(onCancelled))
    , m_background(11, 15, 26, 214)
    , m_text(229, 231, 235)
{
    setObjectName(QStringLiteral("delayedCaptureOverlay"));
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::StrongFocus);
    // 与截图自身 UI 一致，倒计时遮罩也不进入抓屏画面。
    markshot::windows::setExcludedFromCapture(this);

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, &DelayedCaptureOverlay::onTick);
}

DelayedCaptureOverlay::~DelayedCaptureOverlay()
{
    if (m_timer && m_timer->isActive()) {
        m_timer->stop();
    }
}

void DelayedCaptureOverlay::start()
{
    // 覆盖全部显示器组成的虚拟桌面区域，多屏任意位置开始倒计时都可见。
    QRect virtualDesktop;
    for (QScreen *screen : QGuiApplication::screens()) {
        virtualDesktop = virtualDesktop.isNull() ? screen->geometry()
                                                 : virtualDesktop.united(screen->geometry());
    }
    if (virtualDesktop.isNull() && QGuiApplication::primaryScreen()) {
        virtualDesktop = QGuiApplication::primaryScreen()->geometry();
    }
    setGeometry(virtualDesktop);
    show();
    raise();
    activateWindow();
    setFocus(Qt::OtherFocusReason);
    m_timer->start();
    update();
}

void DelayedCaptureOverlay::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), m_background);

    const QString caption = MS_TR("Capturing in %1 second(s)...").arg(m_remaining);
    const QString hint = MS_TR("Press Esc to cancel");
    const QFont captionFont = theme::uiFont(64, QFont::DemiBold);
    const QFont hintFont = theme::uiFont(16, QFont::Normal);

    // 垂直居中整体（数字行 + 提示行），避免倒计时跳动。
    painter.setFont(captionFont);
    const QRect captionRect = painter.fontMetrics().boundingRect(caption);
    painter.setFont(hintFont);
    const QRect hintRect = painter.fontMetrics().boundingRect(hint);
    const int totalHeight = captionRect.height() + 18 + hintRect.height();
    const QPoint center = rect().center();
    const QRect captionArea(center.x() - captionRect.width() / 2 - kTextPadding,
                            center.y() - totalHeight / 2,
                            captionRect.width() + kTextPadding * 2,
                            captionRect.height());
    const QRect hintArea(center.x() - hintRect.width() / 2 - kTextPadding,
                         captionArea.bottom() + 18,
                         hintRect.width() + kTextPadding * 2,
                         hintRect.height());

    painter.setPen(m_text);
    painter.setFont(captionFont);
    painter.drawText(captionArea, Qt::AlignCenter, caption);
    painter.setPen(QColor(229, 231, 235, 170));
    painter.setFont(hintFont);
    painter.drawText(hintArea, Qt::AlignCenter, hint);
}

void DelayedCaptureOverlay::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        cancel();
        return;
    }
    QWidget::keyPressEvent(event);
}

void DelayedCaptureOverlay::onTick()
{
    --m_remaining;
    if (m_remaining <= 0) {
        finish();
        return;
    }
    update();
}

void DelayedCaptureOverlay::finish()
{
    hide();
    if (m_timer) {
        m_timer->stop();
    }
    Callback callback = m_onCapture;
    m_onCapture = {};
    m_onCancelled = {};
    if (callback) {
        callback();
    }
    close();
}

void DelayedCaptureOverlay::cancel()
{
    hide();
    if (m_timer) {
        m_timer->stop();
    }
    Callback callback = m_onCancelled;
    m_onCapture = {};
    m_onCancelled = {};
    if (callback) {
        callback();
    }
    close();
}

void runDelayedCapture(int seconds, std::function<void()> onCapture, std::function<void()> onCancelled)
{
    if (seconds <= 0) {
        if (onCapture) {
            onCapture();
        }
        return;
    }
    auto *overlay = new DelayedCaptureOverlay(seconds, std::move(onCapture), std::move(onCancelled));
    overlay->start();
}

}  // namespace markshot
