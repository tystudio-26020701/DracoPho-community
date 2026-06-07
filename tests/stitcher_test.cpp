#include "scroll/stitcher.h"

#include <QPainter>

#include <QtTest/QtTest>

namespace {

/// @brief Helper function to compute a pseudo-random color for a given row index.
/// @param index The index of the row.
/// @return The calculated QColor object.
QColor rowColor(int index)
{
    return QColor((index * 3 + 17) % 256,
                  (index * 5 + 29) % 256,
                  (index * 7 + 43) % 256);
}

/// @brief Helper function to generate a test frame with vertical color strips.
/// @param firstRow Starting index for row color calculations.
/// @param height Height of the generated frame.
/// @param width Width of the generated frame (defaults to 24).
/// @return The generated test frame QImage.
QImage verticalFrame(int firstRow, int height, int width = 24)
{
    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    for (int y = 0; y < height; ++y) {
        painter.fillRect(QRect(0, y, width, 1), rowColor(firstRow + y));
    }
    painter.end();
    return image;
}

/// @brief Helper function to generate a test frame with horizontal color strips.
/// @param firstColumn Starting index for row color calculations.
/// @param width Width of the generated frame.
/// @param height Height of the generated frame (defaults to 12).
/// @return The generated test frame QImage.
QImage horizontalFrame(int firstColumn, int width, int height = 12)
{
    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    for (int x = 0; x < width; ++x) {
        painter.fillRect(QRect(x, 0, 1, height), rowColor(firstColumn + x));
    }
    painter.end();
    return image;
}

/// @brief Helper function to generate a standard StitchConfig for testing.
/// @return The generated StitchConfig structure.
markshot::scroll::StitchConfig testConfig()
{
    return markshot::scroll::StitchConfig{20, 0.5f, 10, 0.01f};
}

}  // namespace

/// @brief Test suite for testing the Stitcher class.
class StitcherTest : public QObject
{
    /// @brief Qt private signal structure.
    /// @brief Qt meta-object instance.
    Q_OBJECT

private slots:
    void appendsForwardScrollFrames()
    {
        markshot::scroll::Stitcher stitcher(testConfig());

        const markshot::scroll::StitchResult first = stitcher.pushFrame(verticalFrame(0, 80));
        QCOMPARE(first.status, markshot::scroll::StitchStatus::FirstFrame);
        QCOMPARE(first.added, 80);

        const markshot::scroll::StitchResult second = stitcher.pushFrame(verticalFrame(60, 80));
        QCOMPARE(second.status, markshot::scroll::StitchStatus::Appended);
        QCOMPARE(second.edge, markshot::scroll::StitchEdge::End);
        QCOMPARE(second.added, 60);
        QCOMPARE(second.position, 60);

        const QImage full = stitcher.fullImage();
        QCOMPARE(full.size(), QSize(24, 140));
        QCOMPARE(full.pixelColor(0, 0), rowColor(0));
        QCOMPARE(full.pixelColor(0, 139), rowColor(139));
        QCOMPARE(stitcher.stats().frameCount, 2);
        QCOMPARE(stitcher.stats().totalHeight, 140);
    }

    void prependsReverseScrollFrames()
    {
        markshot::scroll::Stitcher stitcher(testConfig());

        stitcher.pushFrame(verticalFrame(60, 80));
        const markshot::scroll::StitchResult second = stitcher.pushFrame(verticalFrame(0, 80));

        QCOMPARE(second.status, markshot::scroll::StitchStatus::Appended);
        QCOMPARE(second.edge, markshot::scroll::StitchEdge::Start);
        QCOMPARE(second.added, 60);

        const QImage full = stitcher.fullImage();
        QCOMPARE(full.size(), QSize(24, 140));
        QCOMPARE(full.pixelColor(0, 0), rowColor(0));
        QCOMPARE(full.pixelColor(0, 139), rowColor(139));
    }

    void stitchesHorizontalAxisByTransposingFrames()
    {
        markshot::scroll::Stitcher stitcher(testConfig());
        stitcher.setAxis(markshot::scroll::ScrollAxis::Horizontal);

        stitcher.pushFrame(horizontalFrame(0, 80));
        const markshot::scroll::StitchResult second = stitcher.pushFrame(horizontalFrame(60, 80));

        QCOMPARE(second.status, markshot::scroll::StitchStatus::Appended);
        QCOMPARE(second.edge, markshot::scroll::StitchEdge::End);
        QCOMPARE(second.added, 60);

        const QImage full = stitcher.fullImage();
        QCOMPARE(full.size(), QSize(140, 12));
        QCOMPARE(full.pixelColor(0, 0), rowColor(0));
        QCOMPARE(full.pixelColor(139, 0), rowColor(139));

        QVERIFY(stitcher.axisLocked());
        stitcher.setAxis(markshot::scroll::ScrollAxis::Vertical);
        QCOMPARE(stitcher.axis(), markshot::scroll::ScrollAxis::Horizontal);
    }

    void rejectsFramesWithDifferentWidths()
    {
        markshot::scroll::Stitcher stitcher(testConfig());

        stitcher.pushFrame(verticalFrame(0, 80, 24));
        const markshot::scroll::StitchResult second = stitcher.pushFrame(verticalFrame(60, 80, 25));

        QCOMPARE(second.status, markshot::scroll::StitchStatus::NoMatch);
        QCOMPARE(stitcher.fullImage().size(), QSize(24, 80));
        QCOMPARE(stitcher.stats().frameCount, 1);
    }
};

/// @brief Main function for the Stitcher test suite.
/// @param argc Argument count.
/// @param argv Argument vector.
/// @return Standard C++ exit code.
QTEST_APPLESS_MAIN(StitcherTest)

#include "stitcher_test.moc"
