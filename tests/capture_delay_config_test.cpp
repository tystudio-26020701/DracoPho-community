#include "capture_delay_config.h"
#include "delayed_capture.h"

#include "app_config_store.h"
#include "ui/i18n.h"

#include <QJsonObject>
#include <QSignalSpy>
#include <QString>
#include <QtTest/QtTest>
#include <QTimer>

namespace {

/// @brief 用给定的 capture 段构造一个应用配置根对象。
/// @param delaySeconds capture.delaySeconds 值；负数表示不写入该键。
/// @return 配置根对象。
QJsonObject configRootWithDelay(int delaySeconds)
{
    QJsonObject root;
    QJsonObject capture;
    if (delaySeconds >= 0) {
        capture.insert(QStringLiteral("delaySeconds"), delaySeconds);
    }
    root.insert(QStringLiteral("capture"), capture);
    return root;
}

}  // namespace

class CaptureDelayConfigTest : public QObject {
    Q_OBJECT

private slots:
    void defaultsToImmediateCapture()
    {
        QCOMPARE(markshot::defaultCaptureDelaySeconds(), 0);
    }

    void readsConfiguredDelay()
    {
        const QJsonObject root = configRootWithDelay(5);
        QCOMPARE(markshot::captureDelaySecondsFromConfigRoot(root), 5);
    }

    void missingDelayFallsBackToDefault()
    {
        const QJsonObject root = configRootWithDelay(-1);
        QCOMPARE(markshot::captureDelaySecondsFromConfigRoot(root), 0);
    }

    void negativeDelayFallsBackToDefault()
    {
        const QJsonObject root = configRootWithDelay(-3);
        QCOMPARE(markshot::captureDelaySecondsFromConfigRoot(root), 0);
    }

    void oversizedDelayIsClampedToLimit()
    {
        const QJsonObject root = configRootWithDelay(1000000000);
        QCOMPARE(markshot::captureDelaySecondsFromConfigRoot(root), 3600);
    }

    void zeroDelayIsValid()
    {
        const QJsonObject root = configRootWithDelay(0);
        QCOMPARE(markshot::captureDelaySecondsFromConfigRoot(root), 0);
    }

    void runsCaptureImmediatelyWhenNoDelay()
    {
        int calls = 0;
        markshot::runDelayedCapture(0, [&calls] { ++calls; });
        QCOMPARE(calls, 1);
    }

    void delaysThenFiresCapture()
    {
        int calls = 0;
        bool done = false;
        // 离屏平台下倒计时遮罩依赖事件循环；用短延时驱动事件循环直至回调触发。
        markshot::runDelayedCapture(1, [&calls, &done] {
            ++calls;
            done = true;
        });
        QVERIFY2(!done, "capture should not fire before the countdown completes");
        QElapsedTimer elapsed;
        elapsed.start();
        while (!done && elapsed.elapsed() < 5000) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
            QTest::qWait(50);
        }
        QCOMPARE(calls, 1);
    }

    void escapeCancelsCapture()
    {
        int captured = 0;
        int cancelled = 0;
        markshot::runDelayedCapture(5,
                                    [&captured] { ++captured; },
                                    [&cancelled] { ++cancelled; });
        const auto overlays = QApplication::topLevelWidgets();
        QWidget *overlay = nullptr;
        for (QWidget *widget : overlays) {
            if (widget->objectName() == QLatin1String("delayedCaptureOverlay")) {
                overlay = widget;
                break;
            }
        }
        QVERIFY2(overlay != nullptr, "a countdown overlay should be visible");
        QTest::keyClick(overlay, Qt::Key_Escape);
        QCOMPARE(captured, 0);
        QCOMPARE(cancelled, 1);
        // 遮罩按 WA_DeleteOnClose 自清理，等待其销毁。
        QTest::qWait(100);
    }
};

QTEST_MAIN(CaptureDelayConfigTest)
#include "capture_delay_config_test.moc"
