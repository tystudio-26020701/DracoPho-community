#include "floating_ball.h"

#include "app_config_store.h"
#include "debug_log.h"
#include "recording/recording_session_manager.h"
#include "recording/recording_start_flow.h"
#include "settings/settings_dialog.h"
#include "ui/i18n.h"
#include "ui/icons.h"
#include "ui/theme.h"
#include "windows_integration.h"

#include <QAction>
#include <QApplication>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QTimer>

namespace markshot {
namespace {

/// @brief 悬浮球直径。
constexpr int kBallSize = 44;
/// @brief 悬浮球外阴影半径。
constexpr int kShadowRadius = 10;

/// @brief 读取已保存的悬浮球位置。
/// @return 已保存位置；无则返回无效点。
QPoint storedBallPosition()
{
    bool ok = false;
    const QJsonObject root = readAppConfigRoot(&ok);
    if (!ok) {
        return {};
    }
    const QJsonObject ball = root.value(QStringLiteral("floatingBall")).toObject();
    const QJsonValue position = ball.value(QStringLiteral("position"));
    if (position.isArray()) {
        const QJsonArray array = position.toArray();
        if (array.size() >= 2 && array.at(0).isDouble() && array.at(1).isDouble()) {
            return QPoint(array.at(0).toInt(), array.at(1).toInt());
        }
    }
    if (position.isObject()) {
        const QJsonObject object = position.toObject();
        if (object.value(QStringLiteral("x")).isDouble() && object.value(QStringLiteral("y")).isDouble()) {
            return QPoint(object.value(QStringLiteral("x")).toInt(), object.value(QStringLiteral("y")).toInt());
        }
    }
    return {};
}

/// @brief 计算默认悬浮球位置：鼠标所在屏幕（无鼠标屏则主屏）右下角，留出边距。
/// @return 默认位置（悬浮球左上角全局坐标）。
QPoint defaultBallPosition()
{
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return {};
    }
    const QRect available = screen->availableGeometry();
    const int margin = 16;
    return QPoint(available.right() - kBallSize - margin, available.bottom() - kBallSize - margin);
}

}  // namespace

FloatingBall::FloatingBall(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFixedSize(kBallSize + kShadowRadius * 2, kBallSize + kShadowRadius * 2);
    setCursor(Qt::PointingHandCursor);
    setToolTip(QStringLiteral("Mark Shot"));
    setObjectName(QStringLiteral("floatingBall"));

    markshot::windows::setExcludedFromTaskbar(this);

    // 单击弹出菜单、双击快速截图：用计时器区分两种手势，
    // 避免单击的菜单弹出窗口吞掉双击事件的第二击。
    m_clickTimer = new QTimer(this);
    m_clickTimer->setSingleShot(true);
    m_clickTimer->setInterval(220);
    connect(m_clickTimer, &QTimer::timeout, this, [this] {
        showBallMenu(m_pendingClickPos);
    });

    m_menu = new QMenu(this);
    m_menu->addAction(MS_TR("Capture"), this, [this] {
        if (m_captureCallback) {
            m_captureCallback();
        }
    });
    m_menu->addAction(MS_TR("Fullscreen Capture"), this, [this] {
        if (m_fullscreenCallback) {
            m_fullscreenCallback();
        }
    });
    m_menu->addAction(MS_TR("Start Recording"), this, [this] {
        startRecordingFromBall();
    });
    m_menu->addSeparator();
    m_menu->addAction(MS_TR("Settings"), this, [] {
        markshot::settings::showSettingsDialog();
    });
    m_menu->addSeparator();
    m_menu->addAction(MS_TR("Hide Floating Ball"), this, [this] {
        if (m_quitWhenHidden) {
            // 无托盘入口时，隐藏悬浮球等于失去唯一入口，直接退出应用。
            savePosition();
            qApp->quit();
            return;
        }
        hide();
    });
    m_menu->addAction(MS_TR("Quit"), qApp, [this] {
        savePosition();
        qApp->quit();
    });
}

void FloatingBall::setCaptureCallbacks(CaptureCallback capture, CaptureCallback fullscreen)
{
    m_captureCallback = std::move(capture);
    m_fullscreenCallback = std::move(fullscreen);
}

void FloatingBall::setRecordingRegionCallback(RecordingRegionCallback callback)
{
    m_recordingRegionCallback = std::move(callback);
}

void FloatingBall::setQuitWhenHidden(bool quitWhenHidden)
{
    m_quitWhenHidden = quitWhenHidden;
}

void FloatingBall::savePosition()
{
    const QPoint global = frameGeometry().topLeft();
    const QJsonArray array{QJsonValue(global.x()), QJsonValue(global.y())};
    QString error;
    if (!writeAppConfigValue(QStringList{QStringLiteral("floatingBall"), QStringLiteral("position")},
                             array,
                             &error)) {
        markshot::debugLog("floating", "cannot save position: %s", error.toUtf8().constData());
    }
}

void FloatingBall::placeOnScreen()
{
    const QPoint stored = storedBallPosition();
    QScreen *screen = QGuiApplication::screenAt(stored);
    if (stored.isNull() || !screen) {
        move(defaultBallPosition());
        return;
    }

    // 已保存位置若不在任何当前屏幕内（显示器拔出等），回退到默认位置。
    const QRect bounds = screen->availableGeometry().adjusted(0, 0, -kBallSize - 16, -kBallSize - 16);
    if (bounds.contains(stored)) {
        move(stored);
    } else {
        move(defaultBallPosition());
    }
}

void FloatingBall::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF ballRect(kShadowRadius, kShadowRadius, kBallSize, kBallSize);
    const QColor accent = theme::kAccent;

    // 外阴影。
    QPainterPath shadowPath;
    shadowPath.addEllipse(ballRect);
    painter.setPen(Qt::NoPen);
    for (int i = kShadowRadius; i > 0; --i) {
        const qreal alpha = 0.10 * (1.0 - static_cast<qreal>(i - 1) / kShadowRadius);
        painter.setBrush(QColor(0, 0, 0, static_cast<int>(255 * alpha)));
        painter.drawEllipse(ballRect.adjusted(-i, -i, i, i));
    }

    // 球体渐变。
    const QRectF body = ballRect.adjusted(1.5, 1.5, -1.5, -1.5);
    QLinearGradient gradient(body.topLeft(), body.bottomRight());
    gradient.setColorAt(0.0, accent.lighter(125));
    gradient.setColorAt(0.55, accent);
    gradient.setColorAt(1.0, accent.darker(120));
    painter.setBrush(gradient);
    painter.setPen(m_hovered ? QPen(QColor(255, 255, 255, 200), 2.0) : QPen(QColor(255, 255, 255, 90), 1.0));
    painter.drawEllipse(body);

    // 中心图标。
    const QPixmap iconPixmap = markshot::ui::applicationIcon().pixmap(26, 26);
    if (!iconPixmap.isNull()) {
        const QPointF iconCenter = body.center();
        const QRectF iconRect(iconCenter.x() - iconPixmap.width() / 2.0,
                              iconCenter.y() - iconPixmap.height() / 2.0,
                              iconPixmap.width(),
                              iconPixmap.height());
        painter.drawPixmap(iconRect.toRect(), iconPixmap);
    }
}

void FloatingBall::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_moved = false;
        m_dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void FloatingBall::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        const QPoint target = event->globalPosition().toPoint() - m_dragOffset;
        const QPoint delta = target - frameGeometry().topLeft();
        if (delta.manhattanLength() >= 4) {
            m_moved = true;
        }
        move(target);
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void FloatingBall::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        event->accept();
        if (!m_moved) {
            // 单击：延迟弹出菜单，等待可能的双击。
            m_pendingClickPos = event->globalPosition().toPoint();
            m_clickTimer->start();
        } else {
            savePosition();
        }
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void FloatingBall::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 双击：取消单击菜单，直接快速截图。
        m_clickTimer->stop();
        m_dragging = false;
        event->accept();
        if (m_captureCallback) {
            m_captureCallback();
        }
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void FloatingBall::enterEvent(QEnterEvent *event)
{
    m_hovered = true;
    update();
    QWidget::enterEvent(event);
}

void FloatingBall::leaveEvent(QEvent *event)
{
    m_hovered = false;
    update();
    QWidget::leaveEvent(event);
}

void FloatingBall::showBallMenu(const QPoint &globalPos)
{
    // 悬浮球菜单每次现建现用（菜单关闭后不持有动作），动作复用 m_menu。
    QMenu menu;
    for (QAction *action : m_menu->actions()) {
        if (action->isSeparator()) {
            menu.addSeparator();
        } else {
            QAction *cloned = menu.addAction(action->text());
            cloned->setEnabled(action->isEnabled());
            QObject::connect(cloned, &QAction::triggered, action, [action] {
                if (action->isEnabled()) {
                    action->trigger();
                }
            });
        }
    }
    menu.exec(globalPos);
}

void FloatingBall::startRecordingFromBall()
{
    auto &manager = recording::RecordingSessionManager::instance();
    if (manager.status().active) {
        return;
    }

    recording::RecordingStartFlowRequest request;
    request.initialMode = recording::RecordingMode::Video;
    request.stayOnTop = true;
    request.startDisplayRecording = [&manager](recording::RecordingOptions options) {
        QString error;
        if (!manager.start(options, QApplication::instance(), &error)) {
            // 悬浮球无托盘通知可用，录制失败必须弹窗告知，避免静默无反应。
            QMessageBox::warning(nullptr,
                                 QStringLiteral("Mark Shot"),
                                 error.isEmpty() ? MS_TR("Recording failed to start") : error);
        }
    };
    request.selectRegionRecording = [this](recording::RecordingOptions options) {
        if (m_recordingRegionCallback) {
            m_recordingRegionCallback(std::move(options));
            return;
        }
        QMessageBox::warning(nullptr,
                             QStringLiteral("Mark Shot"),
                             MS_TR("Failed to start capture session."));
    };
    request.showError = [](const QString &message) {
        QMessageBox::warning(nullptr, QStringLiteral("Mark Shot"), message);
    };

    recording::runRecordingStartFlow(request);
}

}  // namespace markshot
