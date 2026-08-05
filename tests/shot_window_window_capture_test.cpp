#include "shot_window.h"
#include "window_detection.h"

#include "screen_capture.h"

#include <QApplication>
#include <QImage>
#include <QPointer>
#include <QtTest/QtTest>

/// @brief 交互式"窗口捕获"模式回归测试。
///
/// 窗口捕获模式（StartupTool::WindowCapture，键 W / 托盘 / 悬浮球入口）：
/// 悬停高亮窗口、单击捕获。X11 平台优先读取合成缓冲（遮挡/最小化窗口真实
/// 内容），其他平台回退为从冻结帧裁剪窗口矩形。本测试验证：模式切换、
/// 命中判定、以及单击触发捕获（旧覆盖层关闭、新编辑器窗口打开）。
class ShotWindowWindowCaptureTest : public QObject {
    Q_OBJECT

private slots:
    /// @brief 预选窗口捕获工具后进入窗口捕获模式。
    void preselectEnablesWindowCaptureTool()
    {
        QImage frame(320, 200, QImage::Format_ARGB32_Premultiplied);
        frame.fill(Qt::black);
        auto *window = new ShotWindow(frame, QStringLiteral("test-output"), {}, {}, false);
        QVERIFY(window);
        QVERIFY(!window->windowCaptureToolActive());
        window->preselectWindowCaptureTool();
        QVERIFY(window->windowCaptureToolActive());
        window->close();
        QTest::qWait(20);
    }

    /// @brief 命中判定返回最上层窗口下标（z 序优先，其次最小面积）。
    void windowIndexPrefersTopmost()
    {
        QImage frame(320, 200, QImage::Format_ARGB32_Premultiplied);
        frame.fill(Qt::black);

        QVector<markshot::WindowInfo> infos;
        markshot::WindowInfo bottom;
        bottom.rect = QRect(0, 0, 200, 150);
        bottom.zOrder = 1;
        infos.append(bottom);
        markshot::WindowInfo top;
        top.rect = QRect(50, 40, 120, 90);
        top.zOrder = 5;
        infos.append(top);

        auto *window = new ShotWindow(frame,
                                      QStringLiteral("test-output"),
                                      QRect(0, 0, 320, 200),
                                      infos,
                                      true);
        QVERIFY(window);
        window->show();
        QTest::qWait(20);

        // (100,80) 同时落在两个窗口内，应命中 z 序更高的 top。
        const std::optional<int> hit = window->windowIndexAtImagePoint(QPointF(100, 80));
        QVERIFY(hit.has_value());
        QCOMPARE(hit.value(), 1);

        // (10,10) 只落在 bottom 内。
        const std::optional<int> miss = window->windowIndexAtImagePoint(QPointF(10, 10));
        QVERIFY(miss.has_value());
        QCOMPARE(miss.value(), 0);

        window->close();
        QTest::qWait(20);
    }

    /// @brief 窗口捕获模式下单击命中的窗口：覆盖层关闭、编辑器窗口打开。
    void clickInWindowCaptureModeOpensEditor()
    {
        QImage frame(320, 200, QImage::Format_ARGB32_Premultiplied);
        frame.fill(Qt::darkGray);

        QVector<markshot::WindowInfo> infos;
        markshot::WindowInfo info;
        info.rect = QRect(10, 10, 100, 80);
        info.id = QStringLiteral("0x12345678");
        info.zOrder = 0;
        infos.append(info);

        QPointer<ShotWindow> window =
            new ShotWindow(frame, QStringLiteral("test-output"), QRect(0, 0, 320, 200), infos, true);
        QVERIFY(window);
        window->resize(320, 200);
        window->preselectWindowCaptureTool();
        window->show();
        QTest::qWait(30);

        // 悬停到窗口区域（图像与窗口尺寸一致，(50,50) 位于 info.rect 内）。
        QTest::mouseMove(window, QPoint(50, 50), 20);
        QTest::qWait(10);
        QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
        QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, QPoint(50, 50));
        QTest::qWait(80);

        // 原覆盖层已关闭销毁，新的编辑器窗口打开（顶层 ShotWindow 恰好一个）。
        QVERIFY(!window);
        QList<ShotWindow *> shotWindows;
        for (QWidget *widget : QApplication::topLevelWidgets()) {
            if (auto *shot = qobject_cast<ShotWindow *>(widget)) {
                shotWindows.append(shot);
            }
        }
        QCOMPARE(shotWindows.size(), 1);
        if (shotWindows.size() == 1) {
            shotWindows.first()->close();
            QTest::qWait(30);
        }
    }

    /// @brief 非 X11 id（KWin 风格 uuid）在无对象抓取环境下降级为冻结帧裁剪。
    void clickWithCompositorUuidFallsBackToFrameCrop()
    {
        QImage frame(320, 200, QImage::Format_ARGB32_Premultiplied);
        frame.fill(Qt::lightGray);

        QVector<markshot::WindowInfo> infos;
        markshot::WindowInfo info;
        info.rect = QRect(20, 15, 120, 90);
        // KWin Wayland 检测脚本上报的 internalId（uuid），无 0x 前缀。
        info.id = QStringLiteral("4f7b9c1e-2a3b-4c5d-8e6f-1a2b3c4d5e6f");
        info.zOrder = 0;
        infos.append(info);

        QPointer<ShotWindow> window =
            new ShotWindow(frame, QStringLiteral("test-output"), QRect(0, 0, 320, 200), infos, true);
        QVERIFY(window);
        window->resize(320, 200);
        window->preselectWindowCaptureTool();
        window->show();
        QTest::qWait(30);

        QTest::mouseMove(window, QPoint(80, 60), 20);
        QTest::qWait(10);
        QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, QPoint(80, 60));
        QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, QPoint(80, 60));
        QTest::qWait(80);

        // 无 DISPLAY/KWin 的测试环境必然走冻结帧裁剪回退，编辑器窗口打开。
        QVERIFY(!window);
        QList<ShotWindow *> shotWindows;
        for (QWidget *widget : QApplication::topLevelWidgets()) {
            if (auto *shot = qobject_cast<ShotWindow *>(widget)) {
                shotWindows.append(shot);
            }
        }
        QCOMPARE(shotWindows.size(), 1);
        if (shotWindows.size() == 1) {
            // 裁剪图应为冻结帧的窗口区域（120x90），而非空图。
            const QImage captured = shotWindows.first()->frozenFrameForTest();
            QVERIFY(!captured.isNull());
            QCOMPARE(captured.width(), 120);
            QCOMPARE(captured.height(), 90);
            shotWindows.first()->close();
            QTest::qWait(30);
        }
    }

    /// @brief 无对象抓取平台时 captureWindowObjectContent 干净地返回空图。
    void windowObjectContentReturnsNullWithoutPlatformPath()
    {
        markshot::WindowInfo info;
        info.rect = QRect(0, 0, 100, 80);
        info.id = QStringLiteral("some-uuid-handle");
        QString error;
        const QImage image = captureWindowObjectContent(info, false, &error);
        QVERIFY(image.isNull());
        QVERIFY(error.isEmpty());
    }
};

QTEST_MAIN(ShotWindowWindowCaptureTest)
#include "shot_window_window_capture_test.moc"
