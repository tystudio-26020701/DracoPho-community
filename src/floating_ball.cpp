#include "floating_ball.h"

#include "app_config_store.h"
#include "debug_log.h"
#include "delayed_capture_options.h"
#include "history/capture_history_dialog.h"
#include "recording/recording_session_manager.h"
#include "recording/recording_start_flow.h"
#include "settings/settings_dialog.h"
#include "ui/i18n.h"
#include "ui/icons.h"
#include "ui/theme.h"
#include "windows_integration.h"

#include <QAction>
#include <QApplication>
#include <QEasingCurve>
#include <QElapsedTimer>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QRegion>
#include <QScreen>
#include <QTimer>
#include <QWindow>

#include <algorithm>
#include <cmath>

namespace markshot {
namespace {

/// @brief 悬浮球直径。
constexpr int kBallSize = 44;
/// @brief 悬浮球外阴影半径。
constexpr int kShadowRadius = 10;
/// @brief 距屏幕边缘多少像素内触发吸附停靠。
constexpr int kSnapMarginPixels = 24;
/// @brief 拖动判定最小位移（像素）。
constexpr int kDragThresholdPixels = 4;
/// @brief 默认闲置淡出延迟（秒）。
constexpr int kDefaultFadeSeconds = 3;
/// @brief 默认闲置不透明度。
constexpr double kDefaultIdleOpacity = 0.35;
/// @brief 默认边缘停靠隐入像素数（露出约一半球体）。
constexpr int kDefaultHiddenExtentPx = kBallSize / 2;
/// @brief 滑出后隐回延迟（毫秒，参考 Plank HideDelay）。
constexpr int kAutoHideDelayMs = 600;
/// @brief 闲置判定 tick 间隔（毫秒）。
constexpr int kFadeTickIntervalMs = 300;

/// @brief 当前单调时钟（毫秒）。
qint64 monotonicMs()
{
    static QElapsedTimer timer;
    static const bool initialized = [] {
        timer.start();
        return true;
    }();
    Q_UNUSED(initialized);
    return timer.elapsed();
}

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
    setToolTip(QStringLiteral("DracoPho"));
    setObjectName(QStringLiteral("floatingBall"));

    markshot::windows::setExcludedFromTaskbar(this);
    markshot::windows::setExcludedFromCapture(this);

    // 单击弹出菜单、双击快速截图：用计时器区分两种手势，
    // 避免单击的菜单弹出窗口吞掉双击事件的第二击。
    m_clickTimer = new QTimer(this);
    m_clickTimer->setSingleShot(true);
    m_clickTimer->setInterval(220);
    connect(m_clickTimer, &QTimer::timeout, this, [this] {
        showBallMenu(m_pendingClickPos);
    });

    // 闲置淡出用自绘 alpha（paintEvent 里 painter.setOpacity）实现，平台无关：
    // Wayland 不支持 setWindowOpacity，而 QGraphicsOpacityEffect 对顶层透明窗口
    // 存在"opacity 已设置但画面不重绘"的已知缺陷。淡入淡出由 QPropertyAnimation
    // 驱动 fadeAlpha 属性（实机已验证 Wayland 下平滑生效）。
    m_fadeAnim = new QPropertyAnimation(this, "fadeAlpha", this);
    m_fadeAnim->setDuration(180);
    m_fadeAnim->setEasingCurve(QEasingCurve::OutCubic);

    // 常驻闲置 tick：淡出/恢复的唯一判定。它依据"最近一次交互（悬停/移动/
    // 点击）的时间戳"决定是否进入闲置，完全不依赖 leaveEvent——即使合成器
    // 丢失 leave 事件（GNOME Wayland 部分场景），鼠标移开后时间戳不再刷新，
    // tick 超时后照样淡出，从根上杜绝"移开不淡出"。
    m_fadeTickTimer = new QTimer(this);
    m_fadeTickTimer->setInterval(kFadeTickIntervalMs);
    connect(m_fadeTickTimer, &QTimer::timeout, this, &FloatingBall::onFadeTick);

    // 停靠隐回延迟：滑出后移开，等一小段时间确认不会回来再隐入。
    m_autoHideTimer = new QTimer(this);
    m_autoHideTimer->setSingleShot(true);
    m_autoHideTimer->setInterval(kAutoHideDelayMs);
    connect(m_autoHideTimer, &QTimer::timeout, this, [this] {
        if (m_dockState == DockState::Revealed) {
            autoHideBall();
        }
    });

    // 屏幕拔插/分辨率/方向变化时重算停靠（球体隐入依赖屏幕边缘几何）。
    auto connectScreenSignals = [this](QScreen *screen) {
        if (!screen) {
            return;
        }
        connect(screen, &QScreen::geometryChanged, this, [this] { revalidateDockState(); });
        connect(screen, &QScreen::availableGeometryChanged, this, [this] { revalidateDockState(); });
    };
    for (QScreen *screen : QGuiApplication::screens()) {
        connectScreenSignals(screen);
    }
    connect(qGuiApp, &QGuiApplication::screenAdded, this, [this, connectScreenSignals](QScreen *screen) {
        connectScreenSignals(screen);
        revalidateDockState();
    });
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, [this](QScreen *) {
        revalidateDockState();
    });

    m_menu = new QMenu(this);
    m_captureAction = m_menu->addAction(MS_TR("Capture"), this, [this] {
        if (m_captureCallback) {
            m_captureCallback();
        }
    });
    m_fullscreenAction = m_menu->addAction(MS_TR("Fullscreen Capture"), this, [this] {
        if (m_fullscreenCallback) {
            m_fullscreenCallback();
        }
    });
    m_startRecordingAction = m_menu->addAction(MS_TR("Start Recording"), this, [this] {
        startRecordingFromBall();
    });
    m_menu->addSeparator();
    m_settingsAction = m_menu->addAction(MS_TR("Settings"), this, [] {
        markshot::settings::showSettingsDialog();
    });
    m_historyAction = m_menu->addAction(MS_TR("Capture History..."), this, [this] {
        auto *dialog = new markshot::capture_history::CaptureHistoryDialog();
        dialog->setNewCaptureCallback([this] {
            if (m_captureCallback) {
                m_captureCallback();
            }
        });
        dialog->show();
        dialog->raise();
        dialog->activateWindow();
    });
    m_menu->addSeparator();
    m_hideBallAction = m_menu->addAction(MS_TR("Hide Floating Ball"), this, [this] {
        if (m_quitWhenHidden) {
            // 无托盘入口时，隐藏悬浮球等于失去唯一入口，直接退出应用。
            savePosition();
            qApp->quit();
            return;
        }
        // 用户主动隐藏：置位持久状态，截图会话结束不得重新显示。
        m_hiddenByUser = true;
        hide();
    });
    m_quitAction = m_menu->addAction(MS_TR("Quit"), qApp, [this] {
        savePosition();
        qApp->quit();
    });

    // 语言切换即时生效：菜单文案在构造时固化，订阅通知后重新翻译。
    connect(&i18n::LanguageChangeNotifier::instance(),
            &i18n::LanguageChangeNotifier::languageChanged,
            this,
            &FloatingBall::retranslateUi);

    markshot::debugLog("floating", "ball constructed platform=%s",
                       QGuiApplication::platformName().toUtf8().constData());
}

void FloatingBall::setCaptureCallbacks(CaptureCallback capture, CaptureCallback fullscreen)
{
    m_captureCallback = std::move(capture);
    m_fullscreenCallback = std::move(fullscreen);
}

void FloatingBall::setTimedCaptureCallback(TimedCaptureCallback callback)
{
    m_timedCaptureCallback = std::move(callback);
    // 回调在构造之后才设置（悬浮球菜单在构造时固化），这里惰性构建
    // "延时截图"子菜单并插入到全屏截图之后、开始录制之前。
    if (!m_timedCaptureCallback) {
        return;
    }    if (m_delayedCaptureMenu) {
        m_menu->removeAction(m_delayedCaptureMenu->menuAction());
        m_delayedCaptureMenu->deleteLater();
        m_delayedCaptureMenu = nullptr;
    }
    m_delayedCaptureMenu = new QMenu(MS_TR("Delayed Capture"), m_menu);
    m_delayedCaptureMenu->setObjectName(QStringLiteral("floatingDelayedCaptureMenu"));
    m_delayedCaptureItems.clear();
    for (int seconds : kDelayedCapturePresets) {
        QAction *item = m_delayedCaptureMenu->addAction(delayedCapturePresetLabel(seconds));
        m_delayedCaptureItems.append(item);
        QObject::connect(item, &QAction::triggered, this, [this, seconds] {
            if (m_timedCaptureCallback) {
                m_timedCaptureCallback(seconds);
            }
        });
    }
    QAction *before = m_startRecordingAction;
    if (!before) {
        before = m_fullscreenAction;
    }
    if (before) {
        m_menu->insertMenu(before, m_delayedCaptureMenu);
    } else {
        m_menu->addMenu(m_delayedCaptureMenu);
    }
}

void FloatingBall::setRecordingRegionCallback(RecordingRegionCallback callback)
{
    m_recordingRegionCallback = std::move(callback);
}

void FloatingBall::retranslateUi()
{
    if (m_captureAction) {
        m_captureAction->setText(MS_TR("Capture"));
    }
    if (m_fullscreenAction) {
        m_fullscreenAction->setText(MS_TR("Fullscreen Capture"));
    }
    if (m_startRecordingAction) {
        m_startRecordingAction->setText(MS_TR("Start Recording"));
    }
    if (m_settingsAction) {
        m_settingsAction->setText(MS_TR("Settings"));
    }
    if (m_hideBallAction) {
        m_hideBallAction->setText(MS_TR("Hide Floating Ball"));
    }
    if (m_quitAction) {
        m_quitAction->setText(MS_TR("Quit"));
    }
    if (m_delayedCaptureMenu) {
        m_delayedCaptureMenu->setTitle(MS_TR("Delayed Capture"));
        const QStringList labels = delayedCapturePresetLabels();
        for (int i = 0; i < m_delayedCaptureItems.size() && i < labels.size(); ++i) {
            m_delayedCaptureItems.at(i)->setText(labels.at(i));
        }
    }
    // 悬浮球 tooltip 使用产品名，无需翻译。
}

void FloatingBall::setQuitWhenHidden(bool quitWhenHidden)
{
    m_quitWhenHidden = quitWhenHidden;
}

void FloatingBall::savePosition()
{
    // 原生 Wayland 下 frameGeometry()/pos() 不可靠（返回陈旧坐标），持久化
    // 会把已保存的正确位置覆盖成初始/错误值，导致下次启动位置错乱。停靠与
    // 位置持久化仅在有可靠绝对坐标的平台（X11/Windows/macOS）上进行。
    if (!positioningEnabled()) {
        return;
    }
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

    // 已保存位置若完全在屏幕外（显示器拔出等），回退到默认位置。
    if (positionWithinScreenBounds(stored)) {
        move(stored);
    } else {
        move(defaultBallPosition());
    }
}

bool FloatingBall::isHiddenByUser() const
{
    return m_hiddenByUser;
}

void FloatingBall::toggleByUser()
{
    if (m_hiddenByUser || !isVisible()) {
        m_hiddenByUser = false;
        placeOnScreen();
        setBallOpacity(1.0);
        show();
        return;
    }
    m_hiddenByUser = true;
    hide();
}

qreal FloatingBall::fadeAlpha() const
{
    return m_fadeAlpha;
}

void FloatingBall::setFadeAlpha(qreal alpha)
{
    m_fadeAlpha = qBound<qreal>(0.05, alpha, 1.0);
    update();
}

void FloatingBall::setBallOpacity(qreal opacity)
{
    setFadeAlpha(opacity);
}

void FloatingBall::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    // 自绘 alpha：整体（阴影/球体/图标）统一按当前不透明度合成。
    // 相比 QGraphicsOpacityEffect 无"属性变化不刷新"的缺陷，且平台无关。
    // 注意：Wayland 合成器对 ARGB buffer 会正确混合（无 opaque region），
    // 判断"是否半透明"应看像素 RGB 是否混入背景色，而非截图 PNG 的 alpha 通道。
    painter.setOpacity(m_fadeAlpha);
    // 停靠隐入：球体整体向"屏幕外方向"平移半个球体，超出窗口/屏幕边界的
    // 部分被天然裁剪，实现"一半隐入屏幕外"。不依赖 move()，Wayland 有效。
    painter.translate(m_contentOffset);

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
    painter.setPen(QPen(QColor(255, 255, 255, 120), 1.5));
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
        noteInteraction();
        // 从停靠隐入状态按下：先滑出，让本次点击落在完整球体上。
        if (m_dockState == DockState::Snapped && edgeDockEnabled()) {
            revealBall();
        }
        m_dragging = true;
        m_moved = false;
        m_systemMoveStarted = false;
        m_lastDragActivityMs = monotonicMs();
        m_pressGlobalPos = event->globalPosition().toPoint();
        m_pressFrameTopLeft = frameGeometry().topLeft();
        m_dragOffset = m_pressGlobalPos - m_pressFrameTopLeft;
        // 暂不发起系统拖动：先在移动事件里检测到真实位移再交给合成器，
        // 保证纯单击/双击（截图）不会被系统移动手势吞掉。
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void FloatingBall::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        m_lastDragActivityMs = monotonicMs();
        const QPoint delta = event->globalPosition().toPoint() - m_pressGlobalPos;
        if (!m_moved && delta.manhattanLength() >= kDragThresholdPixels) {
            m_moved = true;
        }
        if (!m_moved) {
            event->accept();
            return;
        }
        if (!m_systemMoveStarted) {
            // 仅对真实用户输入（spontaneous 事件）发起系统拖动：Wayland 下顶层
            // 窗口无法用 move() 定位，交给合成器系统拖动；X11/Windows 也借系统
            // 手势获得原生拖拽体验。合成事件（测试用 QApplication::sendEvent 注入，
            // spontaneous()==false）一律走 move() 回退，保证测试在任意平台可复现
            // （Windows 的 startSystemMove() 无条件返回 true 且不实际移动窗口，
            // 若合成事件也走系统拖动，位置断言类测试会失败）。真实输入行为不变。
            if (event->spontaneous()) {
                if (QWindow *window = windowHandle()) {
                    if (window->startSystemMove()) {
                        m_systemMoveStarted = true;
                        event->accept();
                        return;
                    }
                }
            }
            if (QWidget::mouseGrabber() != this) {
                grabMouse();
            }
            move(event->globalPosition().toPoint() - m_dragOffset);
        }
        event->accept();
        return;
    }
    // 悬停移动（无按键）：也算一次交互，刷新闲置计时。
    noteInteraction();
    QWidget::mouseMoveEvent(event);
}

void FloatingBall::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        event->accept();
        if (!m_moved) {
            // 单击：延迟弹出菜单，等待可能的双击。
            m_pendingClickPos = event->globalPosition().toPoint();
            m_clickTimer->start();
            // 复位拖动状态（点击不进入 finishDrag 的停靠逻辑）。
            m_dragging = false;
            m_systemMoveStarted = false;
            if (QWidget::mouseGrabber() == this) {
                releaseMouse();
            }
            return;
        }
        finishDrag();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void FloatingBall::finishDrag()
{
    // 拖动结束统一收尾（release 事件与拖动卡死自愈共用）。
    const bool wasSystemMove = m_systemMoveStarted;
    m_dragging = false;
    m_systemMoveStarted = false;
    m_lastDragActivityMs = -1;
    if (!wasSystemMove) {
        if (QWidget::mouseGrabber() == this) {
            releaseMouse();
        }
    }
    // 拖动结束：靠近屏幕边缘则停靠并隐入，否则回到自由漂浮。
    // enterDocked 返回 false 表示平台不支持停靠（原生 Wayland）或未贴边。
    if (edgeDockEnabled()) {
        const DockEdge edge = detectDockEdge(frameGeometry().topLeft());
        if (edge != DockEdge::None && enterDocked(edge)) {
            noteInteraction();
            return;
        }
    }
    // 未贴边（或停靠不可用）：退出任何停靠状态，回到自由漂浮。
    exitDockState();
    savePosition();
    noteInteraction();
}

void FloatingBall::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 双击：取消单击菜单，直接快速截图。
        m_clickTimer->stop();
        m_dragging = false;
        event->accept();
        noteInteraction();
        if (m_captureCallback) {
            m_captureCallback();
        }
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void FloatingBall::enterEvent(QEnterEvent *event)
{
    noteInteraction();
    // 悬停滑出：从隐入状态恢复完整显示。
    if (m_dockState == DockState::Snapped && edgeDockEnabled()) {
        revealBall();
    }
    QWidget::enterEvent(event);
}

void FloatingBall::leaveEvent(QEvent *event)
{
    // 注：淡出判定完全由 onFadeTick 依据交互时间戳驱动，这里不依赖
    // leaveEvent。只处理停靠态的延迟隐回。
    if (m_dockState == DockState::Revealed) {
        m_autoHideTimer->start();
    }
    QWidget::leaveEvent(event);
}

void FloatingBall::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setBallOpacity(1.0);
    // 显示即视为一次交互：鼠标在球上则不淡出，否则 tick 会在
    // fadeSeconds 后自然淡出。
    m_lastInteractionMs = monotonicMs();
    if (!m_fadeTickTimer->isActive()) {
        m_fadeTickTimer->start();
    }
    // 恢复显示时若处于停靠状态，回到隐入视觉（重新计算偏移，
    // 窗口位置可能已在隐藏期间变化）。停靠已禁用则退出停靠态。
    if (m_dockState == DockState::Snapped || m_dockState == DockState::Revealed) {
        if (!edgeDockEnabled()) {
            exitDockState();
        } else {
            m_dockState = DockState::Snapped;
            QScreen *screen = QGuiApplication::screenAt(frameGeometry().center());
            if (screen) {
                computeDockOffset(m_dockEdge, screen->availableGeometry(), &m_contentOffset);
            }
            applyDockVisuals();
        }
    }
    markshot::debugLog("floating", "ball shown platform=%s",
                       QGuiApplication::platformName().toUtf8().constData());
}

void FloatingBall::hideEvent(QHideEvent *event)
{
    if (m_fadeTickTimer) {
        m_fadeTickTimer->stop();
    }
    if (m_autoHideTimer) {
        m_autoHideTimer->stop();
    }
    QWidget::hideEvent(event);
}

void FloatingBall::showBallMenu(const QPoint &globalPos)
{
    // 悬浮球菜单每次现建现用（菜单关闭后不持有动作），动作复用 m_menu。
    // 子菜单（延时截图）一并克隆：克隆菜单里的子菜单动作仍绑定原动作，
    // 保证触发走 m_timedCaptureCallback 且语言切换后文案一致。
    m_menuOpen = true;
    QMenu menu;
    for (QAction *action : m_menu->actions()) {
        if (action->isSeparator()) {
            menu.addSeparator();
        } else if (QMenu *submenu = action->menu()) {
            QMenu *clonedSubmenu = menu.addMenu(action->text());
            for (QAction *subAction : submenu->actions()) {
                if (subAction->isSeparator()) {
                    clonedSubmenu->addSeparator();
                    continue;
                }
                QAction *cloned = clonedSubmenu->addAction(subAction->text());
                cloned->setEnabled(subAction->isEnabled());
                QObject::connect(cloned, &QAction::triggered, subAction, [subAction] {
                    if (subAction->isEnabled()) {
                        subAction->trigger();
                    }
                });
            }
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
    m_menuOpen = false;
    noteInteraction();
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
                                 QStringLiteral("DracoPho"),
                                 error.isEmpty() ? MS_TR("Recording failed to start") : error);
        }
    };
    request.selectRegionRecording = [this](recording::RecordingOptions options) {
        if (m_recordingRegionCallback) {
            m_recordingRegionCallback(std::move(options));
            return;
        }
        QMessageBox::warning(nullptr,
                             QStringLiteral("DracoPho"),
                             MS_TR("Failed to start capture session."));
    };
    request.showError = [](const QString &message) {
        QMessageBox::warning(nullptr, QStringLiteral("DracoPho"), message);
    };

    recording::runRecordingStartFlow(request);
}

void FloatingBall::noteInteraction()
{
    m_lastInteractionMs = monotonicMs();
    if (isVisible() && m_fadeAlpha < 1.0 - 0.01 && !m_dragging && !m_menuOpen) {
        fadeTo(1.0);
    }
}

void FloatingBall::onFadeTick()
{
    // 拖动卡死自愈：Wayland 系统移动（startSystemMove）结束时合成器接管手势，
    // 客户端可能永远收不到 mouseReleaseEvent，导致 m_dragging 卡 true（淡出被
    // 禁用）且停靠逻辑（finishDrag → enterDocked）永不执行。若拖动活动已停止
    // 超过 2 秒（无新 move 事件），视为拖动已结束：补一次 finishDrag 完成停靠/
    // 保存位置，再恢复淡出。
    if (m_dragging && m_lastDragActivityMs > 0
        && monotonicMs() - m_lastDragActivityMs > 2000) {
        markshot::debugLog("ball", "drag-stuck-heal: finishDrag after %lld ms idle",
                           monotonicMs() - m_lastDragActivityMs);
        finishDrag();
    }
    if (m_dragging || m_menuOpen || !isVisible() || m_hiddenByUser) {
        return;
    }
    int fadeSeconds = kDefaultFadeSeconds;
    double idleOpacity = kDefaultIdleOpacity;
    idleFadeConfig(&fadeSeconds, &idleOpacity);
    if (fadeSeconds <= 0) {
        return;
    }
    const qint64 now = monotonicMs();
    if (m_lastInteractionMs < 0) {
        m_lastInteractionMs = now;
        return;
    }
    const qint64 idleMs = static_cast<qint64>(fadeSeconds) * 1000;
    const qint64 elapsed = now - m_lastInteractionMs;
    if (elapsed >= idleMs) {
        if (m_fadeAlpha > idleOpacity + 0.01) {
            markshot::debugLog("ball", "tick FADE elapsed=%lld alpha=%.2f -> %.2f",
                               elapsed, double(m_fadeAlpha), idleOpacity);
            fadeTo(idleOpacity);
        }
    } else if (m_fadeAlpha < 1.0 - 0.01) {
        markshot::debugLog("ball", "tick RESTORE elapsed=%lld alpha=%.2f", elapsed, double(m_fadeAlpha));
        fadeTo(1.0);
    }
}

void FloatingBall::fadeTo(qreal target)
{
    if (!m_fadeAnim) {
        setBallOpacity(target);
        return;
    }
    m_fadeAnim->stop();
    m_fadeAnim->setStartValue(m_fadeAlpha);
    m_fadeAnim->setEndValue(qBound<qreal>(0.05, target, 1.0));
    m_fadeAnim->start();
}

void FloatingBall::idleFadeConfig(int *fadeSeconds, double *idleOpacity) const
{
    if (fadeSeconds) {
        *fadeSeconds = kDefaultFadeSeconds;
    }
    if (idleOpacity) {
        *idleOpacity = kDefaultIdleOpacity;
    }

    bool ok = false;
    const QJsonObject root = readAppConfigRoot(&ok);
    if (!ok) {
        return;
    }
    const QJsonObject ball = root.value(QStringLiteral("floatingBall")).toObject();

    const QJsonValue seconds = ball.value(QStringLiteral("idleFadeSeconds"));
    if (seconds.isDouble() && seconds.toInt() >= 0) {
        if (fadeSeconds) {
            *fadeSeconds = seconds.toInt();
        }
    }
    const QJsonValue opacity = ball.value(QStringLiteral("idleOpacity"));
    if (opacity.isDouble() && opacity.toDouble() > 0.0 && opacity.toDouble() <= 1.0) {
        if (idleOpacity) {
            *idleOpacity = opacity.toDouble();
        }
    }
}

bool FloatingBall::positioningEnabled() const
{
    // 原生 Wayland 顶层窗口位置由合成器决定，move()/setGeometry 无效；
    // offscreen 平台 move 记录位置，测试可用。
    const QString platform = QGuiApplication::platformName();
    return platform != QLatin1String("wayland");
}

bool FloatingBall::edgeDockEnabled() const
{
    bool ok = false;
    const QJsonObject root = readAppConfigRoot(&ok);
    if (ok) {
        const QJsonObject ball = root.value(QStringLiteral("floatingBall")).toObject();
        const QJsonValue enabled = ball.value(QStringLiteral("edgeDockEnabled"));
        if (enabled.isBool()) {
            return enabled.toBool();
        }
    }
    return true;
}

int FloatingBall::hiddenExtentPx() const
{
    bool ok = false;
    const QJsonObject root = readAppConfigRoot(&ok);
    if (ok) {
        const QJsonObject ball = root.value(QStringLiteral("floatingBall")).toObject();
        const QJsonValue extent = ball.value(QStringLiteral("hiddenExtentPx"));
        if (extent.isDouble() && extent.toInt() >= 0 && extent.toInt() < kBallSize) {
            return extent.toInt();
        }
    }
    return kDefaultHiddenExtentPx;
}

FloatingBall::DockEdge FloatingBall::detectDockEdge(const QPoint &desired) const
{
    QScreen *screen = QGuiApplication::screenAt(QRect(desired, size()).center());
    if (!screen) {
        return DockEdge::None;
    }
    const QRect available = screen->availableGeometry();
    const int w = width();
    const int h = height();

    // 距离 = 窗口边缘与屏幕边缘的间隙；窗口越过屏幕边缘（重叠/在屏幕外）时
    // 距离为 0（即已贴边）。注意不能对间隙取 abs：球拖到屏幕外时间隙为负，
    // abs 会把"超出屏幕"误判为"距边很远"而不吸附。
    const int dl = std::max(0, desired.x() - available.left());
    const int dr = std::max(0, available.right() - (desired.x() + w - 1));
    const int dt = std::max(0, desired.y() - available.top());
    const int db = std::max(0, available.bottom() - (desired.y() + h - 1));
    const int nearest = std::min({dl, dr, dt, db});
    if (nearest > kSnapMarginPixels) {
        return DockEdge::None;
    }
    if (nearest == dl) {
        return DockEdge::Left;
    }
    if (nearest == dr) {
        return DockEdge::Right;
    }
    if (nearest == dt) {
        return DockEdge::Top;
    }
    return DockEdge::Bottom;
}

/// @brief 计算使球体在指定边缘"隐藏 extent 像素于屏幕外"所需的内容偏移。
///
/// 球体在窗口内的边界为 [kShadowRadius, kShadowRadius + kBallSize]。
/// 以右边缘为例：要求球体右缘（全局）恰好落在 available.right() + extent，
/// 即球体右缘"藏到屏幕外 extent 像素"。偏移量由窗口实际位置推导，
/// 因此不依赖窗口是否贴边（X11/Win 会先 move 贴边；Wayland 用实际位置）。
FloatingBall::DockEdge FloatingBall::computeDockOffset(DockEdge edge,
                                                       const QRect &available,
                                                       QPoint *offset) const
{
    if (!offset) {
        return edge;
    }
    const QPoint tl = frameGeometry().topLeft();
    const int ballLeft = kShadowRadius;
    const int ballRight = kShadowRadius + kBallSize;
    const int extent = hiddenExtentPx();
    switch (edge) {
    case DockEdge::Right:
        offset->setX(available.right() + extent - tl.x() - ballRight);
        offset->setY(0);
        break;
    case DockEdge::Left:
        offset->setX(available.left() - extent - tl.x() - ballLeft);
        offset->setY(0);
        break;
    case DockEdge::Top:
        offset->setX(0);
        offset->setY(available.top() - extent - tl.y() - ballLeft);
        break;
    case DockEdge::Bottom:
        offset->setX(0);
        offset->setY(available.bottom() + extent - tl.y() - ballRight);
        break;
    case DockEdge::None:
        *offset = QPoint();
        break;
    }
    return edge;
}

bool FloatingBall::enterDocked(DockEdge edge)
{
    // 原生 Wayland 客户端无法获取自己的屏幕绝对坐标（frameGeometry()/pos()
    // 返回创建时的陈旧值，通常为 0,0 或初始位置），而停靠依赖该坐标推导
    // 吸附位置与内容偏移；在此平台上尝试停靠会把球体画到窗口之外、叠加成
    // 多层鬼影/重影，还会按陈旧坐标停靠到错误的边缘。因此原生 Wayland 上
    // 一律不进停靠态，球体保持自由漂浮（协议限制，诚实披露）。
    if (!positioningEnabled()) {
        markshot::debugLog("floating", "dock-skip platform=%s (no reliable absolute coords)",
                           QGuiApplication::platformName().toUtf8().constData());
        return false;
    }

    QScreen *screen = QGuiApplication::screenAt(frameGeometry().center());
    if (!screen) {
        return false;
    }
    const QRect available = screen->availableGeometry();

    // 支持程序化定位的平台（X11/Windows/macOS/offscreen）：先把窗口吸附贴边，
    // 这样球体向屏幕外偏移后，隐藏部分真正藏在屏幕外。
    const int w = width();
    const int h = height();
    QPoint target = frameGeometry().topLeft();
    switch (edge) {
    case DockEdge::Left:
        target.setX(available.left());
        break;
    case DockEdge::Right:
        target.setX(available.right() - w + 1);
        break;
    case DockEdge::Top:
        target.setY(available.top());
        break;
    case DockEdge::Bottom:
        target.setY(available.bottom() - h + 1);
        break;
    case DockEdge::None:
        break;
    }
    move(target);

    // 精确计算内容偏移：球体在边缘处隐藏 hiddenExtentPx 于屏幕外。
    computeDockOffset(edge, available, &m_contentOffset);
    m_dockState = DockState::Snapped;
    m_dockEdge = edge;
    applyDockVisuals();
    savePosition();
    markshot::debugLog("floating", "docked edge=%d offset=%d,%d platform=%s",
                       static_cast<int>(edge), m_contentOffset.x(), m_contentOffset.y(),
                       QGuiApplication::platformName().toUtf8().constData());
    return true;
}

void FloatingBall::exitDockState()
{
    if (m_dockState == DockState::Floating) {
        return;
    }
    m_dockState = DockState::Floating;
    m_dockEdge = DockEdge::None;
    m_contentOffset = QPoint();
    clearMask();
    update();
}

void FloatingBall::revealBall()
{
    if (m_dockState != DockState::Snapped) {
        return;
    }
    m_dockState = DockState::Revealed;
    m_contentOffset = QPoint();
    applyDockVisuals();
    noteInteraction();
}

void FloatingBall::autoHideBall()
{
    // 停靠已禁用时无需隐回（球体本来就在自由漂浮态）。
    if (m_dockState != DockState::Revealed || !edgeDockEnabled()) {
        return;
    }
    QScreen *screen = QGuiApplication::screenAt(frameGeometry().center());
    if (!screen) {
        return;
    }
    m_dockState = DockState::Snapped;
    computeDockOffset(m_dockEdge, screen->availableGeometry(), &m_contentOffset);
    applyDockVisuals();
    // 隐回后不立即淡出：先保持可见一段时间。
    noteInteraction();
}

void FloatingBall::applyDockVisuals()
{
    // 输入 mask：停靠隐入时球体向屏幕外方向平移，窗口内可见部分 = 球体
    // 与窗口边界的交集 sliver。mask 精确保留该 sliver 作为输入区域
    // （Wayland 下即 input region），窗口其余透明空白不挡鼠标。
    if (m_contentOffset.isNull()) {
        clearMask();
        update();
        return;
    }
    const int w = width();
    const int h = height();
    const int bl = kShadowRadius;                    // 球体窗口内左缘
    const int br = kShadowRadius + kBallSize;        // 球体窗口内右缘
    const int ox = m_contentOffset.x();
    const int oy = m_contentOffset.y();
    QRect visibleRect;
    switch (m_dockEdge) {
    case DockEdge::Left:
    case DockEdge::Right: {
        const int left = qBound(0, bl + ox, w);
        const int right = qBound(0, br + ox, w);
        visibleRect = QRect(left, 0, std::max(0, right - left), h);
        break;
    }
    case DockEdge::Top:
    case DockEdge::Bottom: {
        const int top = qBound(0, bl + oy, h);
        const int bottom = qBound(0, br + oy, h);
        visibleRect = QRect(0, top, w, std::max(0, bottom - top));
        break;
    }
    case DockEdge::None:
        clearMask();
        update();
        return;
    }
    if (visibleRect.isEmpty()) {
        clearMask();
    } else {
        setMask(QRegion(visibleRect));
    }
    update();
}

void FloatingBall::revalidateDockState()
{
    if (!isVisible() || m_dockState == DockState::Floating) {
        return;
    }
    // 用户运行中关闭了停靠开关：退出停靠态，回到自由漂浮。
    if (!edgeDockEnabled()) {
        exitDockState();
        return;
    }
    // 屏幕变化后重新检测贴边；若不再贴边则回退自由漂浮。
    const DockEdge edge = detectDockEdge(frameGeometry().topLeft());
    if (edge == DockEdge::None || !enterDocked(edge)) {
        exitDockState();
        placeOnScreen();
        return;
    }
}

bool FloatingBall::positionWithinScreenBounds(const QPoint &topLeft) const
{
    const QRect ballRect(topLeft, size());
    QScreen *screen = QGuiApplication::screenAt(ballRect.center());
    if (!screen) {
        return false;
    }
    return screen->geometry().intersects(ballRect);
}

}  // namespace markshot
