#include "shot_window.h"

#include <QApplication>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QShortcut>
#include <QtTest/QtTest>

/// @brief 截图覆盖层"冻结背景"状态回归测试。
///
/// 多屏"冻结全部屏幕"会话中，用户在某台显示器完成选区后，其余显示器的
/// 冻结覆盖层进入冻结背景状态：继续显示冻结画面但拦截全部鼠标/键盘输入，
/// 保持整张虚拟桌面冻结，直到截图会话结束。本测试验证该状态确实生效，
/// 防止回归导致其他屏幕在选区完成后立即变得可操作。
class ShotWindowFrozenBackdropTest : public QObject {
    Q_OBJECT

private:
    /// @brief 构造一个测试覆盖层。
    static ShotWindow *makeWindow()
    {
        QImage frame(320, 200, QImage::Format_ARGB32_Premultiplied);
        frame.fill(Qt::black);
        return new ShotWindow(frame, QStringLiteral("test-output"), {}, {}, false);
    }

private slots:
    /// @brief 进入冻结背景后，鼠标/键盘/滚轮事件被吞掉，窗口状态不变。
    void backdropSwallowsInput()
    {
        ShotWindow *window = makeWindow();
        QVERIFY(window);

        // 直接投递事件（无需显示窗口）：冻结背景必须吞掉全部输入。
        // 用信号计数验证输入确实被拦截：press+move+release 形成有效选区
        // 会触发 selectionActivated，Escape 会触发 sessionCancelRequested，
        // 任一信号出现都说明有输入漏出。
        QSignalSpy activateSpy(window, &ShotWindow::selectionActivated);
        QSignalSpy cancelSpy(window, &ShotWindow::sessionCancelRequested);
        QVERIFY(!window->isFrozenBackdrop());
        window->enterFrozenBackdrop();
        QVERIFY(window->isFrozenBackdrop());

        QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, QPoint(40, 40));
        QTest::mouseMove(window, QPoint(160, 120), 30);
        QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, QPoint(160, 120));
        QTest::mouseDClick(window, Qt::LeftButton, Qt::NoModifier, QPoint(60, 60));
        QWheelEvent wheel(QPointF(50, 50),
                          QPoint(50, 50),
                          QPoint(0, 0),
                          QPoint(0, 120),
                          Qt::NoButton,
                          Qt::NoModifier,
                          Qt::NoScrollPhase,
                          false);
        QApplication::sendEvent(window, &wheel);
        QTest::keyClick(window, Qt::Key_Escape);
        QTest::qWait(20);

        QVERIFY(window->isFrozenBackdrop());
        QCOMPARE(activateSpy.count(), 0);
        QCOMPARE(cancelSpy.count(), 0);

        window->close();
        QTest::qWait(20);
    }

    /// @brief 冻结背景后无任何交互控件显示（防"backdrop 化后控件意外
    /// 显示/恢复"的回归）。Selecting 模式下工具栏本就隐藏，因此本用例
    /// 关注的是 backdrop 后所有子控件保持不可见，而不是前置可见性。
    void backdropHidesToolbars()
    {
        ShotWindow *window = makeWindow();
        QVERIFY(window);

        window->show();
        QTest::qWait(50);

        window->enterFrozenBackdrop();
        QTest::qWait(20);

        const QStringList allowedVisibleNames = {
            QStringLiteral("displayCapturePicker"),
        };
        const auto children = window->findChildren<QWidget *>();
        for (QWidget *child : children) {
            if (!allowedVisibleNames.contains(child->objectName())) {
                QVERIFY2(!child->isVisible(), qPrintable(child->objectName()));
            }
        }

        window->close();
        QTest::qWait(20);
    }

    /// @brief 冻结背景禁用全部 WindowShortcut 快捷键，防止获得焦点后
    /// Escape/复制/保存等快捷键绕过鼠标键盘防护。
    void backdropDisablesShortcuts()
    {
        ShotWindow *window = makeWindow();
        QVERIFY(window);

        const auto shortcutsBefore = window->findChildren<QShortcut *>();
        QVERIFY2(!shortcutsBefore.isEmpty(), "ShotWindow should register shortcut objects");

        window->enterFrozenBackdrop();
        QTest::qWait(20);

        const auto shortcutsAfter = window->findChildren<QShortcut *>();
        for (QShortcut *shortcut : shortcutsAfter) {
            QVERIFY2(!shortcut->isEnabled(),
                     qPrintable(shortcut->key().toString(QKeySequence::PortableText)));
        }

        window->close();
        QTest::qWait(20);
    }
};

QTEST_MAIN(ShotWindowFrozenBackdropTest)
#include "shot_window_frozen_backdrop_test.moc"
