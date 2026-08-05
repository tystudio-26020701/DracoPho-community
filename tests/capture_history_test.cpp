#include "history/capture_history.h"

#include "app_config_store.h"

#include <QFileInfo>
#include <QImage>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QtTest/QtTest>

/// @brief 截图历史存储模块测试。
///
/// 验证：记录写盘与索引、倒序读取、上限淘汰、单条删除、清空、
/// 以及配置读取（历史开关与上限）。
class CaptureHistoryTest : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_temp;

private slots:
    void initTestCase()
    {
        QVERIFY(m_temp.isValid());
        markshot::capture_history::setStorageDirectoryForTesting(m_temp.path());
        markshot::capture_history::setOverridesForTesting(true, 50);
    }

    void cleanupTestCase()
    {
        markshot::capture_history::setStorageDirectoryForTesting(QString());
        markshot::capture_history::setOverridesForTesting(std::nullopt, std::nullopt);
    }

    void init()
    {
        markshot::capture_history::clearCaptures();
    }

    /// @brief 记录后可按时间倒序读取，且文件真实存在。
    void recordsAndListsNewestFirst()
    {
        QImage image(16, 16, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::red);
        const QString firstId = markshot::capture_history::recordCapture(image, QStringLiteral("first"));
        QVERIFY(!firstId.isEmpty());
        QTest::qWait(5);
        QImage secondImage(16, 16, QImage::Format_ARGB32_Premultiplied);
        secondImage.fill(Qt::green);
        const QString secondId = markshot::capture_history::recordCapture(secondImage, QStringLiteral("second"));
        QVERIFY(!secondId.isEmpty());

        const QVector<markshot::capture_history::HistoryEntry> entries =
            markshot::capture_history::listCaptures();
        QCOMPARE(entries.size(), 2);
        QCOMPARE(entries.at(0).id, secondId);
        QCOMPARE(entries.at(1).id, firstId);
        QVERIFY(QFileInfo::exists(entries.at(0).path));
    }

    /// @brief 禁用后记录不落盘。
    void disabledHistoryDoesNotRecord()
    {
        markshot::capture_history::setOverridesForTesting(false, 50);
        QImage image(16, 16, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::blue);
        const QString id = markshot::capture_history::recordCapture(image, QStringLiteral("none"));
        QVERIFY(id.isEmpty());
        QCOMPARE(markshot::capture_history::listCaptures().size(), 0);
        markshot::capture_history::setOverridesForTesting(true, 50);
    }

    /// @brief 超过上限时淘汰最旧条目及其文件。
    void capsByMaxEntries()
    {
        markshot::capture_history::setOverridesForTesting(true, 2);
        QString oldestId;
        QImage image(16, 16, QImage::Format_ARGB32_Premultiplied);
        for (int i = 0; i < 4; ++i) {
            image.fill(QColor(i * 40, 0, 0));
            const QString id = markshot::capture_history::recordCapture(image, QStringLiteral("cap-%1").arg(i));
            if (i == 0) {
                oldestId = id;
            }
            QTest::qWait(5);
        }
        const QVector<markshot::capture_history::HistoryEntry> entries =
            markshot::capture_history::listCaptures();
        QCOMPARE(entries.size(), 2);
        for (const markshot::capture_history::HistoryEntry &entry : entries) {
            QVERIFY(entry.id != oldestId);
        }
        markshot::capture_history::setOverridesForTesting(true, 50);
    }

    /// @brief 删除单条后文件与索引同步移除。
    void removesSingleEntry()
    {
        QImage image(16, 16, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::cyan);
        const QString id = markshot::capture_history::recordCapture(image, QStringLiteral("doomed"));
        QVERIFY(!id.isEmpty());
        QCOMPARE(markshot::capture_history::listCaptures().size(), 1);

        QVERIFY(markshot::capture_history::removeCapture(id));
        QCOMPARE(markshot::capture_history::listCaptures().size(), 0);
        QVERIFY(!markshot::capture_history::removeCapture(id));
    }

    /// @brief 配置读取：缺省启用且上限 50，可显式覆盖。
    void readsConfigFromRoot()
    {
        QJsonObject root;
        QVERIFY(markshot::capture_history::historyEnabledFromRoot(root));
        QCOMPARE(markshot::capture_history::historyMaxEntriesFromRoot(root), 50);

        QJsonObject history;
        history.insert(QStringLiteral("enabled"), false);
        history.insert(QStringLiteral("maxEntries"), 7);
        root.insert(QStringLiteral("history"), history);
        QVERIFY(!markshot::capture_history::historyEnabledFromRoot(root));
        QCOMPARE(markshot::capture_history::historyMaxEntriesFromRoot(root), 7);

        history.insert(QStringLiteral("maxEntries"), -5);
        root.insert(QStringLiteral("history"), history);
        QCOMPARE(markshot::capture_history::historyMaxEntriesFromRoot(root), 0);
    }
};

QTEST_GUILESS_MAIN(CaptureHistoryTest)
#include "capture_history_test.moc"
