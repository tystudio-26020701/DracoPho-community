#include "floating_ball.h"
#include "ui/i18n.h"

#include <QApplication>
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
};

QTEST_MAIN(FloatingBallTest)

#include "floating_ball_test.moc"
