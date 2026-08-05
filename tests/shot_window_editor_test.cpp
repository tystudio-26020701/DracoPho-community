#include "shot_window.h"

#include "history/capture_history.h"

#include <QApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QImage>
#include <QMimeData>
#include <QPushButton>
#include <QTemporaryDir>
#include <QUrl>
#include <QtTest/QtTest>

/// @brief 独立图片编辑器（--editor）模式回归测试。
///
/// 独立编辑器以空画布启动：不进入选区/标注状态，显示"打开图片"入口，
/// 支持 Ctrl+O、拖放导入；导入图片后进入常规标注编辑（全幅选区）。
class ShotWindowEditorTest : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_temp;

    static ShotWindow *makeEmptyEditor()
    {
        auto *window = new ShotWindow(QImage(), QStringLiteral("editor"));
        window->setStandaloneEditorMode(true);
        return window;
    }

private slots:
    void initTestCase()
    {
        QVERIFY(m_temp.isValid());
        // 复制/保存会触发截图历史记录：隔离到临时目录，避免污染真实 AppData。
        markshot::capture_history::setStorageDirectoryForTesting(m_temp.path());
        markshot::capture_history::setOverridesForTesting(true, 50);
    }

    void cleanupTestCase()
    {
        markshot::capture_history::setStorageDirectoryForTesting(QString());
        markshot::capture_history::setOverridesForTesting(std::nullopt, std::nullopt);
    }
    /// @brief 空画布：不显示选区/标注工具栏，只有"打开图片"入口可见。
    void emptyCanvasShowsOnlyOpenButton()
    {
        ShotWindow *window = makeEmptyEditor();
        QVERIFY(window);
        QVERIFY(window->isStandaloneEditor());
        QVERIFY(window->hasEmptyEditorCanvas());

        window->show();
        QTest::qWait(30);

        QPushButton *openButton = window->findChild<QPushButton *>(QStringLiteral("editorOpenButton"));
        QVERIFY2(openButton, "editor open button should exist");
        QVERIFY(openButton->isVisible());

        const QStringList allowedVisibleNames = {
            QStringLiteral("editorOpenButton"),
            QStringLiteral("displayCapturePicker"),
        };
        const auto children = window->findChildren<QWidget *>();
        for (QWidget *child : children) {
            if (!allowedVisibleNames.contains(child->objectName())
                && child != window->findChild<QPushButton *>(QStringLiteral("editorOpenButton"))) {
                QVERIFY2(!child->isVisible(), qPrintable(child->objectName()));
            }
        }

        // 空画布上点击不应触发选区（不发出 selectionActivated）。
        QSignalSpy activateSpy(window, &ShotWindow::selectionActivated);
        QTest::mousePress(window, Qt::LeftButton, Qt::NoModifier, QPoint(100, 100));
        QTest::mouseMove(window, QPoint(160, 120), 20);
        QTest::mouseRelease(window, Qt::LeftButton, Qt::NoModifier, QPoint(160, 120));
        QTest::qWait(20);
        QCOMPARE(activateSpy.count(), 0);

        window->close();
        QTest::qWait(20);
    }

    /// @brief 导入图片后进入标注编辑状态，选区覆盖整幅图片。
    void loadingImageEntersEditingMode()
    {
        ShotWindow *window = makeEmptyEditor();
        QVERIFY(window);

        QImage frame(320, 200, QImage::Format_ARGB32_Premultiplied);
        frame.fill(Qt::blue);

        QVERIFY(window->loadEditorImage(frame, QStringLiteral("sample.png")));
        QVERIFY(!window->hasEmptyEditorCanvas());
        window->show();
        QTest::qWait(30);

        QPushButton *openButton = window->findChild<QPushButton *>(QStringLiteral("editorOpenButton"));
        QVERIFY2(openButton, "editor open button should exist");
        QVERIFY(!openButton->isVisible());

        // 进入编辑后工具栏可见，可复制/保存（不应因保存而关闭窗口）。
        QTest::keyClick(window, Qt::Key_C, Qt::ControlModifier);
        QTest::qWait(20);
        QVERIFY(window->isVisible());

        window->close();
        QTest::qWait(20);
    }

    /// @brief 拖放一张图片到窗口即导入并进入编辑状态。
    void droppingImageLoadsIntoEditor()
    {
        ShotWindow *window = makeEmptyEditor();
        QVERIFY(window);
        QVERIFY(window->hasEmptyEditorCanvas());

        QImage frame(64, 48, QImage::Format_ARGB32_Premultiplied);
        frame.fill(Qt::green);

        auto *mime = new QMimeData;
        QTemporaryFile file;
        QVERIFY(file.open());
        file.write("tmp");
        file.flush();
        mime->setUrls({QUrl::fromLocalFile(file.fileName())});

        QDragEnterEvent enter(QPoint(10, 10), Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(window, &enter);
        QVERIFY(enter.isAccepted());

        // 直接构造 imageData 拖放：mimeData->imageData 走 QImage 导入路径。
        auto *imageMime = new QMimeData;
        imageMime->setImageData(frame);
        QDropEvent drop(QPoint(10, 10), Qt::CopyAction, imageMime, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(window, &drop);
        QTest::qWait(20);
        QVERIFY(!window->hasEmptyEditorCanvas());

        window->close();
        QTest::qWait(20);
    }
};

QTEST_MAIN(ShotWindowEditorTest)
#include "shot_window_editor_test.moc"
