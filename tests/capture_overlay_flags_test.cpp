#include "shot_window.h"

#include <QApplication>
#include <QImage>
#include <QtTest/QtTest>

/// @brief 截图覆盖层无闪烁属性回归测试。
///
/// 整屏"花屏闪烁抖动"的三处应用层防护（GNOME Wayland 焦点争夺循环 /
/// 合成器先清后画的双重绘制 / 全屏窗口阴影合成路径）集中体现在覆盖层的
/// 窗口属性上，任何一处被后续改动移除都会重新引入闪烁。本测试在离屏
/// 平台构造真实覆盖层窗口并断言这些属性保持生效。
class CaptureOverlayFlagsTest : public QObject {
    Q_OBJECT

private slots:
    /// @brief 覆盖层窗口具备全部无闪烁属性。
    void overlayWindowIsFlickerSafe()
    {
        QImage frame(320, 200, QImage::Format_ARGB32_Premultiplied);
        frame.fill(Qt::black);
        ShotWindow *window = new ShotWindow(frame,
                                            QStringLiteral("test-output"),
                                            {},
                                            {},
                                            false);
        QVERIFY(window);

        // Wayland 下不会在 show() 时抢占焦点，避免与合成器形成焦点反复
        // 得失循环（mutter #1897 / yukigram#8）。
        QVERIFY(window->testAttribute(Qt::WA_ShowWithoutActivating));
        // 合成器不先清背景再画内容，避免 expose/重排时的双重绘制闪烁。
        QVERIFY(window->testAttribute(Qt::WA_OpaquePaintEvent));
        QVERIFY(window->testAttribute(Qt::WA_NoSystemBackground));
        // 全屏覆盖层不进入阴影合成路径。
        QVERIFY((window->windowFlags() & Qt::NoDropShadowWindowHint) != 0);
        // 覆盖层仍是置顶无边框顶层窗口。
        QVERIFY((window->windowFlags() & Qt::Window) != 0);
        QVERIFY((window->windowFlags() & Qt::FramelessWindowHint) != 0);
        QVERIFY((window->windowFlags() & Qt::WindowStaysOnTopHint) != 0);

        window->close();
        QTest::qWait(20);
    }
};

QTEST_MAIN(CaptureOverlayFlagsTest)
#include "capture_overlay_flags_test.moc"
