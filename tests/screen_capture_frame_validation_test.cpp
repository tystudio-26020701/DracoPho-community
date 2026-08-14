#include "screen_capture_frame_validation.h"

#include <QColor>
#include <QImage>
#include <QtTest/QtTest>

class ScreenCaptureFrameValidationTest : public QObject {
    Q_OBJECT

private slots:
    void solidColorFrameIsSuspicious()
    {
        // 合成器返回的纯色占位帧（如纯蓝）应判定为可疑。
        QImage frame(320, 240, QImage::Format_ARGB32_Premultiplied);
        frame.fill(QColor(0, 0, 255, 255));
        QVERIFY(markshot::isSuspiciousSolidFrame(frame));
    }

    void nearSolidFrameWithTinyVariationIsSuspicious()
    {
        // 通道差在容差内（<=8）视为纯色。
        QImage frame(100, 80, QImage::Format_RGB32);
        frame.fill(QColor(240, 240, 240));
        frame.setPixelColor(3, 3, QColor(245, 245, 245));
        QVERIFY(markshot::isSuspiciousSolidFrame(frame));
    }

    void detailedFrameIsNotSuspicious()
    {
        // 真实桌面（大面积多色内容，如窗口+任务栏+壁纸）不应被误判。
        QImage frame(400, 300, QImage::Format_RGB32);
        for (int y = 0; y < frame.height(); ++y) {
            for (int x = 0; x < frame.width(); ++x) {
                // 上半：水平渐变（白→蓝）；下半：交替大色块（红/绿/灰）。
                if (y < 150) {
                    frame.setPixelColor(x, y, QColor(255 - x * 255 / 399, 0, 255));
                } else {
                    frame.setPixelColor(x, y, (x / 100) % 2 == 0
                        ? QColor(200, 30, 30)
                        : QColor(30, 200, 30));
                }
            }
        }
        QVERIFY(!markshot::isSuspiciousSolidFrame(frame));
    }

    void nullAndTinyFramesAreNotSuspicious()
    {
        QVERIFY(!markshot::isSuspiciousSolidFrame(QImage()));
        QVERIFY(!markshot::isSuspiciousSolidFrame(QImage(1, 1, QImage::Format_RGB32)));
    }
};

QTEST_MAIN(ScreenCaptureFrameValidationTest)
#include "screen_capture_frame_validation_test.moc"
