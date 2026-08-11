#include "floating_ball.h"
#include "ui/i18n.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QMenu>
#include <QMouseEvent>
#include <QScreen>
#include <QTemporaryDir>
#include <QtTest/QtTest>

namespace {

/// @brief 在独立临时目录下隔离配置路径，避免测试拖动画悬浮球时把位置写进
/// 真实用户配置（Linux/macOS 走 XDG_CONFIG_HOME，Windows 覆盖 APPDATA）。
class IsolatedConfigScope {
public:
    bool init()
    {
        if (!m_dir.isValid()) {
            return false;
        }
        const QString configDir = m_dir.path() + QStringLiteral("/dracoPho");
        if (!QDir().mkpath(configDir)) {
            return false;
        }
        QFile file(configDir + QStringLiteral("/config.json"));
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }
        file.write("{}");
        file.close();
#if defined(Q_OS_WIN)
        qputenv("LOCALAPPDATA", m_dir.path().toUtf8());
        qputenv("APPDATA", m_dir.path().toUtf8());
        return qputenv("USERPROFILE", m_dir.path().toUtf8());
#else
        return qputenv("XDG_CONFIG_HOME", m_dir.path().toUtf8());
#endif
    }

private:
    QTemporaryDir m_dir;
};

}  // namespace

class FloatingBallTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // 全部用例共用一份隔离配置：拖动/停靠用例会写悬浮球位置，必须落在
        // 临时目录而不是真实用户配置，否则测试会覆盖用户保存的球位置。
        QVERIFY(m_configScope.init());
    }

    void retranslatesMenuOnLanguageChange()
    {
        // 构造在中文环境下的悬浮球，随后切回英文，验证菜单文案跟随。
        markshot::i18n::setLanguage(markshot::i18n::Language::Chinese);
        markshot::FloatingBall ball;
        const auto menus = ball.findChildren<QMenu *>();
        QVERIFY(!menus.isEmpty());
        const QStringList zhTexts = [&menus] {
            QStringList result;
            for (QAction *action : menus.first()->actions()) {
                if (!action->isSeparator()) {
                    result.append(action->text());
                }
            }
            return result;
        }();
        QVERIFY(zhTexts.contains(markshot::i18n::translate(QStringLiteral("Capture"))));

        markshot::i18n::setLanguage(markshot::i18n::Language::English);
        const QStringList enTexts = [&menus] {
            QStringList result;
            for (QAction *action : menus.first()->actions()) {
                if (!action->isSeparator()) {
                    result.append(action->text());
                }
            }
            return result;
        }();
        QVERIFY(enTexts.contains(QStringLiteral("Capture")));
        // 恢复中文，避免污染其他测试。
        markshot::i18n::setLanguage(markshot::i18n::Language::Chinese);
    }

    void keepsBallFramelessAndAlwaysOnTop()
    {
        markshot::FloatingBall ball;
        const Qt::WindowFlags flags = ball.windowFlags();
        QVERIFY(flags.testFlag(Qt::FramelessWindowHint));
        QVERIFY(flags.testFlag(Qt::WindowStaysOnTopHint));
        QVERIFY(ball.minimumWidth() == ball.maximumWidth());
        QVERIFY(ball.minimumHeight() == ball.maximumHeight());
    }

    void exposesDelayedCaptureMenuAfterSettingCallback()
    {
        // 回归测试：延时截图回调在构造之后设置，子菜单必须惰性构建并插入。
        markshot::FloatingBall ball;
        int triggered = 0;
        ball.setTimedCaptureCallback([&triggered](int seconds) {
            Q_UNUSED(seconds);
            ++triggered;
        });
        const auto menus = ball.findChildren<QMenu *>();
        QMenu *delayed = nullptr;
        for (QMenu *menu : menus) {
            if (menu->objectName() == QLatin1String("floatingDelayedCaptureMenu")) {
                delayed = menu;
                break;
            }
        }
        QVERIFY2(delayed != nullptr, "Delayed Capture submenu should be created lazily");
        const QStringList texts = [delayed] {
            QStringList result;
            for (QAction *action : delayed->actions()) {
                result.append(action->text());
            }
            return result;
        }();
        QCOMPARE(texts.size(), 4);
        // 触发第一个预设项应调用回调。
        QAction *first = delayed->actions().isEmpty() ? nullptr : delayed->actions().first();
        QVERIFY(first != nullptr);
        first->trigger();
        QCOMPARE(triggered, 1);
    }

    void dragMovesBall()
    {
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        const QPoint before = ball.pos();
        const QPoint start = ball.mapToGlobal(ball.rect().center());
        const QPoint moved = start + QPoint(40, 25);

        QMouseEvent press(QEvent::MouseButtonPress,
                          ball.rect().center(),
                          start,
                          Qt::LeftButton,
                          Qt::LeftButton,
                          Qt::NoModifier);
        QApplication::sendEvent(&ball, &press);

        QMouseEvent moveEvent(QEvent::MouseMove,
                              ball.rect().center() + QPoint(40, 25),
                              moved,
                              Qt::NoButton,
                              Qt::LeftButton,
                              Qt::NoModifier);
        QApplication::sendEvent(&ball, &moveEvent);

        QVERIFY2(ball.pos() != before, "drag should move the floating ball");
    }

    void canBeHiddenAndShown()
    {
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(ball.isVisible());
        ball.hide();
        QVERIFY(!ball.isVisible());
        ball.show();
        QVERIFY(ball.isVisible());
    }

    void exposesExpectedMenuActions()
    {
        markshot::FloatingBall ball;
        const auto menus = ball.findChildren<QMenu *>();
        QVERIFY(!menus.isEmpty());
        QMenu *menu = menus.first();
        const QStringList texts = [&menu] {
            QStringList result;
            for (QAction *action : menu->actions()) {
                if (!action->isSeparator()) {
                    result.append(action->text());
                }
            }
            return result;
        }();
        QVERIFY(texts.contains(markshot::i18n::translate(QStringLiteral("Capture"))));
        QVERIFY(texts.contains(markshot::i18n::translate(QStringLiteral("Fullscreen Capture"))));
        QVERIFY(texts.contains(markshot::i18n::translate(QStringLiteral("Settings"))));
        QVERIFY(texts.contains(markshot::i18n::translate(QStringLiteral("Quit"))));
    }

    void staysFullyOpaqueOnShow()
    {
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        QCOMPARE(ball.fadeAlpha(), 1.0);
    }

    void fadesOutAfterIdleTimeout()
    {
        // 闲置判定由交互时间戳 + tick 驱动，不依赖 leaveEvent：
        // 显示后没有任何交互，fadeSeconds（默认 3s）后应淡出。
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        QCOMPARE(ball.fadeAlpha(), 1.0);
        // 等待超过默认 3s 闲置 + 180ms 动画。
        QTest::qWait(3600);
        QVERIFY2(ball.fadeAlpha() < 0.9,
                 qPrintable(QStringLiteral("expected faded alpha, got %1").arg(ball.fadeAlpha())));
    }

    void dragWithoutReleaseStillFadesOutLater()
    {
        // 模拟 Wayland 系统移动卡死场景：press + move 之后没有 release 事件。
        // m_dragging 会卡 true，但 tick 的 2 秒超时自愈必须复位它并恢复淡出。
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        const QPoint start = ball.mapToGlobal(ball.rect().center());
        const QPoint moved = start + QPoint(60, 30);

        QMouseEvent press(QEvent::MouseButtonPress,
                          ball.rect().center(),
                          start,
                          Qt::LeftButton,
                          Qt::LeftButton,
                          Qt::NoModifier);
        QApplication::sendEvent(&ball, &press);
        QMouseEvent moveEvent(QEvent::MouseMove,
                              ball.mapFromGlobal(moved),
                              moved,
                              Qt::NoButton,
                              Qt::LeftButton,
                              Qt::NoModifier);
        QApplication::sendEvent(&ball, &moveEvent);
        // 故意不发 release，模拟 Wayland 系统移动结束事件丢失。
        // m_dragging 应卡 true（tick 会持续 skip），但 2 秒超时自愈必须复位它。

        // 等待拖动卡死自愈（2s）+ 闲置淡出（3s）+ 动画。
        QTest::qWait(6000);
        QVERIFY2(ball.fadeAlpha() < 0.9,
                 qPrintable(QStringLiteral("expected fade after drag-stuck heal, got %1")
                                .arg(ball.fadeAlpha())));
    }

    void dragWithoutReleaseDocksWhenFinishedNearEdge()
    {
        // 关键链路：Wayland 系统移动结束无 release → 拖动卡死自愈 → finishDrag
        // → detectDockEdge → enterDocked（贴边 + 偏移 + mask）。若自愈不补
        // finishDrag，停靠永不发生。本测试直接验证完整链路。
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        QScreen *screen = ball.screen() ? ball.screen() : QGuiApplication::primaryScreen();
        QVERIFY(screen);
        const QRect available = screen->availableGeometry();
        const int w = ball.width();
        const int h = ball.height();

        // 先把球放到左上角附近，再拖向右缘（不释放）。
        ball.move(available.left() + 20, available.top() + 60);
        const QPoint start = ball.mapToGlobal(ball.rect().center());
        const QPoint pressPos = start;
        const QPoint targetGlobal(available.right() - 5, start.y());
        const QPoint moved = targetGlobal;

        QMouseEvent press(QEvent::MouseButtonPress,
                          ball.rect().center(),
                          pressPos,
                          Qt::LeftButton,
                          Qt::LeftButton,
                          Qt::NoModifier);
        QApplication::sendEvent(&ball, &press);
        QMouseEvent moveEvent(QEvent::MouseMove,
                              ball.mapFromGlobal(moved),
                              moved,
                              Qt::NoButton,
                              Qt::LeftButton,
                              Qt::NoModifier);
        QApplication::sendEvent(&ball, &moveEvent);
        // 不发送 release。等自愈（2s）触发 finishDrag → 停靠。
        QTest::qWait(3000);

        const int expectedX = available.right() - w + 1;
        QVERIFY2(ball.pos().x() == expectedX,
                 qPrintable(QStringLiteral("expected docked x=%1 actual=%2")
                                .arg(expectedX).arg(ball.pos().x())));
        QVERIFY(!ball.mask().isEmpty());
        const QRect maskRect = ball.mask().boundingRect();
        QVERIFY2(maskRect.width() >= 15 && maskRect.width() <= 35,
                 qPrintable(QStringLiteral("expected ~half-ball sliver, got %1")
                                .arg(maskRect.width())));
    }

    void docksToRightEdgeAndHidesHalfOnDrag()
    {
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        QScreen *screen = ball.screen() ? ball.screen() : QGuiApplication::primaryScreen();
        QVERIFY(screen);
        const QRect available = screen->availableGeometry();
        const int w = ball.width();
        const int h = ball.height();
        const QPoint start = ball.mapToGlobal(ball.rect().center());
        const QPoint startTopLeft = ball.frameGeometry().topLeft();

        // 拖到距右边缘 5px（吸附阈值 24px 内）并释放：应停靠右边缘。
        const QPoint targetTopLeft(available.right() - w - 5, available.top() + 120);
        const QPoint endGlobal = targetTopLeft + (start - startTopLeft);

        QMouseEvent press(QEvent::MouseButtonPress,
                          ball.rect().center(),
                          start,
                          Qt::LeftButton,
                          Qt::LeftButton,
                          Qt::NoModifier);
        QApplication::sendEvent(&ball, &press);
        QMouseEvent moveEvent(QEvent::MouseMove,
                              ball.mapFromGlobal(endGlobal),
                              endGlobal,
                              Qt::NoButton,
                              Qt::LeftButton,
                              Qt::NoModifier);
        QApplication::sendEvent(&ball, &moveEvent);
        QMouseEvent release(QEvent::MouseButtonRelease,
                            ball.mapFromGlobal(endGlobal),
                            endGlobal,
                            Qt::LeftButton,
                            Qt::NoButton,
                            Qt::NoModifier);
        QApplication::sendEvent(&ball, &release);

        // offscreen 支持 move()：窗口应吸附贴右缘，且隐入（内容偏移 -> mask 非空）。
        const int expectedX = available.right() - w + 1;
        QVERIFY2(ball.pos().x() == expectedX,
                 qPrintable(QStringLiteral("expected docked x=%1 actual=%2")
                                .arg(expectedX).arg(ball.pos().x())));
        // 关键：停靠后可见 sliver = 球体半（默认 hiddenExtentPx=22 藏屏幕外，
        // 球体可见约 kBallSize-22=22px + 阴影余量）。mask 应远小于整窗宽 64，
        // 且位于窗口贴边一侧。
        QVERIFY(!ball.mask().isEmpty());
        const QRect maskRect = ball.mask().boundingRect();
        QVERIFY2(maskRect.width() < w - 20,
                 qPrintable(QStringLiteral("expected slim sliver, mask width=%1 window=%2")
                                .arg(maskRect.width()).arg(w)));
        QVERIFY2(maskRect.width() >= 15 && maskRect.width() <= 35,
                 qPrintable(QStringLiteral("expected ~half-ball sliver, got %1").arg(maskRect.width())));
        QVERIFY(maskRect.right() >= w - 3);  // sliver 贴近窗口右缘（屏幕贴边侧）
    }

    void revealingOnEnterAndHidingOnLeave()
    {
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        QScreen *screen = ball.screen() ? ball.screen() : QGuiApplication::primaryScreen();
        QVERIFY(screen);
        const QRect available = screen->availableGeometry();
        const int w = ball.width();
        const int h = ball.height();
        const QPoint start = ball.mapToGlobal(ball.rect().center());
        const QPoint startTopLeft = ball.frameGeometry().topLeft();

        // 拖到左边缘附近释放，进入停靠隐入状态。
        const QPoint targetTopLeft(available.left() - 5, available.top() + 80);
        const QPoint endGlobal = targetTopLeft + (start - startTopLeft);
        QMouseEvent press(QEvent::MouseButtonPress, ball.rect().center(), start,
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&ball, &press);
        QMouseEvent moveEvent(QEvent::MouseMove, ball.mapFromGlobal(endGlobal), endGlobal,
                              Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&ball, &moveEvent);
        QMouseEvent release(QEvent::MouseButtonRelease, ball.mapFromGlobal(endGlobal), endGlobal,
                            Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(&ball, &release);

        // 窗口吸附贴左缘 + 隐入（mask 非空）。
        QCOMPARE(ball.pos().x(), available.left());
        QVERIFY(!ball.mask().isEmpty());

        // 悬停：滑出（mask 清空）。
        QEnterEvent enter(ball.mapFromGlobal(ball.mapToGlobal(QPoint(0, 0))),
                          ball.mapToGlobal(QPoint(0, 0)),
                          ball.mapToGlobal(QPoint(0, 0)));
        QApplication::sendEvent(&ball, &enter);
        QVERIFY(ball.mask().isEmpty());

        // 移开：延迟后隐回（mask 恢复）。
        QEvent leave(QEvent::Leave);
        QApplication::sendEvent(&ball, &leave);
        QTest::qWait(900);  // 覆盖 kAutoHideDelayMs=600
        QVERIFY(!ball.mask().isEmpty());
    }

    void togglingByUserTracksPersistentHiddenState()
    {
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        QVERIFY(!ball.isHiddenByUser());
        QVERIFY(ball.isVisible());

        ball.toggleByUser();
        QVERIFY(ball.isHiddenByUser());
        QVERIFY(!ball.isVisible());

        // 普通 hide/show 不应清除"用户主动隐藏"状态。
        ball.hide();
        ball.show();
        QVERIFY(ball.isHiddenByUser());

        ball.toggleByUser();
        QVERIFY(!ball.isHiddenByUser());
        QVERIFY(ball.isVisible());
    }

    void captureHideRestoreRespectsUserHiddenState()
    {
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        // 模拟 launchCapture：先临时隐藏（用户未主动隐藏），会话结束恢复。
        ball.hide();
        QVERIFY(!ball.isHiddenByUser());
        ball.show();
        QVERIFY(ball.isVisible());
    }

    void hideShowRestoresPositionAfterDrift()
    {
        // 回归测试：截图会话隐藏窗口后，部分 WM/合成器重新映射时会把自由
        // 漂浮的球挪到别处。重新显示时必须把球移回隐藏前的位置，而不是让
        // 它停在漂移后的新位置（即"截图后悬浮球自己移动"）。
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        QScreen *screen = ball.screen() ? ball.screen() : QGuiApplication::primaryScreen();
        QVERIFY(screen);
        const QRect available = screen->availableGeometry();

        const QPoint original(available.left() + 120, available.top() + 120);
        ball.move(original);
        QVERIFY(ball.pos() == original);

        // 截图会话：隐藏悬浮球。
        ball.hide();

        // 模拟 WM/合成器在隐藏期间把窗口挪到别处。
        ball.move(available.left() + 300, available.top() + 300);
        QVERIFY(ball.pos() != original);

        // 会话结束：恢复显示，事件循环返回后的延迟纠正应把球移回原位。
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        QTest::qWait(100);
        QVERIFY2(ball.pos() == original,
                 qPrintable(QStringLiteral("ball must return to pre-hide position: expected=%1,%2 now=%3,%4")
                                .arg(original.x()).arg(original.y())
                                .arg(ball.pos().x()).arg(ball.pos().y())));
    }

    void captureInterruptedDragMustNotMoveOrDockBall()
    {
        // 回归测试：截图/录制会话在鼠标仍按住或拖动悬浮球时被触发（热键、
        // 托盘、延时倒计时结束），窗口隐藏时拖动状态被遗留。重新显示后若
        // "拖动卡死自愈"把它当成一次真实拖动结束执行 finishDrag，会把球停靠
        // 到屏幕边缘（默认右下角位置就在吸附阈值内）或重定位——即"截图后
        // 悬浮球自己移动到其他地方"。隐藏即拖动终结，恢复显示后球必须留在
        // 原地、不得停靠。
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        QScreen *screen = ball.screen() ? ball.screen() : QGuiApplication::primaryScreen();
        QVERIFY(screen);
        const QRect available = screen->availableGeometry();
        const int w = ball.width();
        const int h = ball.height();

        // 球放到右下角附近（吸附阈值 24px 内，自由漂浮、未停靠）。
        const QPoint freeTopLeft(available.right() - w - 8, available.bottom() - h - 8);
        ball.move(freeTopLeft);

        // 模拟"按住球拖动时截图被触发"：press + move 进入拖动，但不发 release。
        const QPoint start = ball.mapToGlobal(ball.rect().center());
        QMouseEvent press(QEvent::MouseButtonPress, ball.rect().center(), start,
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&ball, &press);
        QMouseEvent moveEvent(QEvent::MouseMove,
                              ball.mapFromGlobal(start + QPoint(20, 20)),
                              start + QPoint(20, 20),
                              Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&ball, &moveEvent);
        // 截图触发时刻球的位置：拖动已生效，球正停在这里等待 release。
        const QPoint before = ball.pos();

        // 截图会话：隐藏悬浮球（不释放鼠标）。
        ball.hide();
        QVERIFY(!ball.isVisible());

        // 会话结束：恢复显示。
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));

        // 等待"拖动卡死自愈"窗口期（>2s），确认自愈不会把球停靠或移动。
        QTest::qWait(2500);
        QVERIFY2(ball.mask().isEmpty(),
                 "capture-interrupted drag must not dock the ball after re-show");
        QVERIFY2(ball.pos() == before,
                 qPrintable(QStringLiteral("capture must not move the ball: before=%1,%2 now=%3,%4")
                                .arg(before.x()).arg(before.y())
                                .arg(ball.pos().x()).arg(ball.pos().y())));
    }

    void userToggleMustNotResurrectStalePreHidePosition()
    {
        // 回归测试：会话隐藏期间记录的"隐藏前位置"只用于会话结束时的漂移纠正，
        // 不得在用户后续主动切换（托盘 toggleByUser → placeOnScreen）时把球拉回
        // 该陈旧位置，覆盖刚按配置/默认放置的结果。
        markshot::FloatingBall ball;
        ball.show();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        QScreen *screen = ball.screen() ? ball.screen() : QGuiApplication::primaryScreen();
        QVERIFY(screen);
        const QRect available = screen->availableGeometry();

        // 模拟一次"拖动中截图"：把球放到屏幕中部，press + move 进入拖动后隐藏，
        // 隐藏前位置被记录为拖动中的中间点 M。
        const QPoint midDrag(available.left() + 200, available.top() + 200);
        ball.move(midDrag);
        const QPoint start = ball.mapToGlobal(ball.rect().center());
        QMouseEvent press(QEvent::MouseButtonPress, ball.rect().center(), start,
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&ball, &press);
        QMouseEvent moveEvent(QEvent::MouseMove,
                              ball.mapFromGlobal(start + QPoint(30, 30)),
                              start + QPoint(30, 30),
                              Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(&ball, &moveEvent);
        ball.hide();

        // 用户经托盘重新打开悬浮球：toggleByUser → placeOnScreen 应把球放到
        // 配置/默认位置，而不是被拉回拖动中的中间点 M。
        ball.toggleByUser();
        QVERIFY(QTest::qWaitForWindowExposed(&ball));
        QTest::qWait(100);  // 让 showEvent 的延迟纠正有机会执行（若未按会话限定）
        const QPoint now = ball.pos();
        const QPoint staleMidDrag = midDrag + QPoint(30, 30);
        QVERIFY2(now != staleMidDrag,
                 qPrintable(QStringLiteral("user toggle must not restore stale mid-drag position %1,%2")
                                .arg(now.x()).arg(now.y())));
        QVERIFY(ball.mask().isEmpty());
        QVERIFY(ball.isVisible());
    }

private:
    IsolatedConfigScope m_configScope;
};

QTEST_MAIN(FloatingBallTest)

#include "floating_ball_test.moc"
