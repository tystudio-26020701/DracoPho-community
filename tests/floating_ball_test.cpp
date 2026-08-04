#include "floating_ball.h"
#include "ui/i18n.h"

#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QScreen>
#include <QtTest/QtTest>

class FloatingBallTest : public QObject {
    Q_OBJECT

private slots:
    void keepsBallFramelessAndAlwaysOnTop()
    {
        markshot::FloatingBall ball;
        const Qt::WindowFlags flags = ball.windowFlags();
        QVERIFY(flags.testFlag(Qt::FramelessWindowHint));
        QVERIFY(flags.testFlag(Qt::WindowStaysOnTopHint));
        QVERIFY(ball.minimumWidth() == ball.maximumWidth());
        QVERIFY(ball.minimumHeight() == ball.maximumHeight());
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
};

QTEST_MAIN(FloatingBallTest)

#include "floating_ball_test.moc"
