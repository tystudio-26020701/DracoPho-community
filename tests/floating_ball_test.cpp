#include "floating_ball.h"
#include "ui/i18n.h"

#include <QApplication>
#include <QGraphicsOpacityEffect>
#include <QMenu>
#include <QMouseEvent>
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
        const auto effects = ball.findChildren<QGraphicsOpacityEffect *>();
        QVERIFY(!effects.isEmpty());
        QCOMPARE(effects.first()->opacity(), 1.0);
    }

    void snapsToNearbyScreenEdgeAfterDrag()
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
        // 把球拖到距右边缘 5px（吸附阈值 24px 内）的位置并释放。
        const QPoint startTopLeft = ball.frameGeometry().topLeft();
        const QPoint targetTopLeft(available.right() - w - 5, available.bottom() - h - 100);
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

        QCOMPARE(ball.pos().x(), available.right() - w + 1);
        QCOMPARE(ball.pos().y(), targetTopLeft.y());
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
