#include "history/capture_history.h"
#include "history/capture_history_dialog.h"

#include <QApplication>
#include <QImage>
#include <QTemporaryDir>
#include <QtTest/QtTest>

/// @brief 截图历史窗口冒烟测试。
class CaptureHistoryDialogTest : public QObject {
    Q_OBJECT

private slots:
    void dialogOpensAndShowsEntries()
    {
        QTemporaryDir temp;
        QVERIFY(temp.isValid());
        markshot::capture_history::setStorageDirectoryForTesting(temp.path());
        markshot::capture_history::setOverridesForTesting(true, 50);
        markshot::capture_history::clearCaptures();

        QImage image(32, 32, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::magenta);
        const QString id = markshot::capture_history::recordCapture(image, QStringLiteral("sample"));
        QVERIFY(!id.isEmpty());

        auto *dialog = new markshot::capture_history::CaptureHistoryDialog();
        QVERIFY(dialog);
        dialog->show();
        QTest::qWait(30);
        QVERIFY(dialog->isVisible());
        dialog->close();
        QTest::qWait(30);

        markshot::capture_history::setStorageDirectoryForTesting(QString());
        markshot::capture_history::setOverridesForTesting(std::nullopt, std::nullopt);
    }
};

QTEST_MAIN(CaptureHistoryDialogTest)
#include "capture_history_dialog_test.moc"
