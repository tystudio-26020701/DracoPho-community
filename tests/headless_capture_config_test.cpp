#include "headless_capture_config.h"

#include <QJsonObject>
#include <QtTest/QtTest>

class HeadlessCaptureConfigTest : public QObject {
    Q_OBJECT

private slots:
    /**
     * 验证缺省配置：默认去向为 inline，剪贴板写权限关闭。
     * @return 无返回值。
     */
    void defaultConfigKeepsClipboardUntouched()
    {
        const markshot::HeadlessCaptureConfig config =
            markshot::headlessCaptureConfigFromRoot(QJsonObject());

        QCOMPARE(config.defaultDestination, markshot::HeadlessCaptureDestination::Inline);
        QCOMPARE(config.clipboardAllowed, false);
    }

    /**
     * 验证可以读取暂存去向与剪贴板写权限。
     * @return 无返回值。
     */
    void readsStageAndClipboardAllowed()
    {
        QJsonObject headless;
        headless.insert(QStringLiteral("defaultDestination"), QStringLiteral("stage"));
        headless.insert(QStringLiteral("clipboardAllowed"), true);
        QJsonObject root;
        root.insert(QStringLiteral("headless"), headless);

        const markshot::HeadlessCaptureConfig config =
            markshot::headlessCaptureConfigFromRoot(root);

        QCOMPARE(config.defaultDestination, markshot::HeadlessCaptureDestination::Stage);
        QCOMPARE(config.clipboardAllowed, true);
    }

    /**
     * 验证非法去向回退到 inline，剪贴板写权限仍默认关闭。
     * @return 无返回值。
     */
    void invalidDestinationKeepsInline()
    {
        QJsonObject headless;
        headless.insert(QStringLiteral("defaultDestination"), QStringLiteral("clipboard"));
        QJsonObject root;
        root.insert(QStringLiteral("headless"), headless);

        const markshot::HeadlessCaptureConfig config =
            markshot::headlessCaptureConfigFromRoot(root);

        QCOMPARE(config.defaultDestination, markshot::HeadlessCaptureDestination::Inline);
        QCOMPARE(config.clipboardAllowed, false);
    }

    /**
     * 验证写入配置后可以完整读回。
     * @return 无返回值。
     */
    void roundTripsThroughWrite()
    {
        markshot::HeadlessCaptureConfig config;
        config.defaultDestination = markshot::HeadlessCaptureDestination::Stage;
        config.clipboardAllowed = true;

        QJsonObject root;
        markshot::writeHeadlessCaptureConfig(&root, config);

        const markshot::HeadlessCaptureConfig read =
            markshot::headlessCaptureConfigFromRoot(root);

        QCOMPARE(read.defaultDestination, markshot::HeadlessCaptureDestination::Stage);
        QCOMPARE(read.clipboardAllowed, true);
        QCOMPARE(markshot::headlessCaptureDestinationName(read.defaultDestination),
                 QStringLiteral("stage"));
    }
};

QTEST_APPLESS_MAIN(HeadlessCaptureConfigTest)

#include "headless_capture_config_test.moc"
