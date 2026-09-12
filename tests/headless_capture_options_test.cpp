#include "cli/headless_capture_options.h"

#include <QCommandLineParser>
#include <QtTest/QtTest>

using namespace markshot::cli;

class HeadlessCaptureOptionsTest : public QObject {
    Q_OBJECT

private slots:
    /**
     * 验证 --delay 解析：合法区间放行，非法值一律拒绝。
     * @return 无返回值。
     */
    void parsesDelaySeconds()
    {
        QCOMPARE(parseHeadlessDelaySeconds(QStringLiteral("5")).value_or(-1), 5);
        QCOMPARE(parseHeadlessDelaySeconds(QStringLiteral("0")).value_or(-1), 0);
        QCOMPARE(parseHeadlessDelaySeconds(QStringLiteral("3600")).value_or(-1), 3600);
        QCOMPARE(parseHeadlessDelaySeconds(QStringLiteral(" 12 ")).value_or(-1), 12);
        QVERIFY(!parseHeadlessDelaySeconds(QStringLiteral("-1")).has_value());
        QVERIFY(!parseHeadlessDelaySeconds(QStringLiteral("3601")).has_value());
        QVERIFY(!parseHeadlessDelaySeconds(QStringLiteral("abc")).has_value());
        QVERIFY(!parseHeadlessDelaySeconds(QString()).has_value());
        QVERIFY(!parseHeadlessDelaySeconds(QStringLiteral("3.5")).has_value());
    }

    /**
     * 验证屏幕截图去向解析：file/inline/stage 放行，clipboard 与未知值拒绝。
     * @return 无返回值。
     */
    void parsesScreenDestination()
    {
        QCOMPARE(parseScreenDestination(QStringLiteral("file")).value_or(ScreenCaptureDestination::Inline),
                 ScreenCaptureDestination::File);
        QCOMPARE(parseScreenDestination(QStringLiteral("Inline")).value_or(ScreenCaptureDestination::File),
                 ScreenCaptureDestination::Inline);
        QCOMPARE(parseScreenDestination(QStringLiteral(" stage ")).value_or(ScreenCaptureDestination::File),
                 ScreenCaptureDestination::Stage);
        QVERIFY(!parseScreenDestination(QStringLiteral("clipboard")).has_value());
        QVERIFY(!parseScreenDestination(QStringLiteral("bogus")).has_value());
        QVERIFY(!parseScreenDestination(QString()).has_value());
    }

    /**
     * 验证暂存目录名约定。
     * @return 无返回值。
     */
    void exposesStageDirectoryName()
    {
        QCOMPARE(screenStageDirectoryName(), QStringLiteral("dracoPho-staging"));
    }

    /**
     * 验证 --capture-window 与无头选项的冲突检测：每个无头选项都要点名，
     * --delay 不算冲突，无冲突时返回空串。
     * @return 无返回值。
     */
    void detectsInteractiveConflicts()
    {
        const QStringList headlessFlags{
            QStringLiteral("capture-to"), QStringLiteral("region"),
            QStringLiteral("display"), QStringLiteral("all-outputs"),
            QStringLiteral("list-displays"), QStringLiteral("window"),
            QStringLiteral("list-windows"), QStringLiteral("window-by"),
            QStringLiteral("capture-destination"), QStringLiteral("output-name"),
            QStringLiteral("include-cursor"),
        };

        for (const QString &flag : headlessFlags) {
            QCommandLineParser parser;
            parser.addOption(QCommandLineOption(QStringLiteral("capture-window")));
            parser.addOption(QCommandLineOption(flag));
            QVERIFY(parser.parse({QStringLiteral("dracoPho"),
                                  QStringLiteral("--capture-window"),
                                  QStringLiteral("--%1").arg(flag)}));
            const QString conflict = headlessInteractiveConflict(parser);
            QVERIFY2(!conflict.isEmpty(), qPrintable(QStringLiteral("flag %1 must conflict").arg(flag)));
            QVERIFY2(conflict.contains(QStringLiteral("--%1").arg(flag)),
                     qPrintable(QStringLiteral("conflict must name --%1").arg(flag)));
        }

        QCommandLineParser delayParser;
        delayParser.addOption(QCommandLineOption(QStringLiteral("capture-window")));
        delayParser.addOption(QCommandLineOption(QStringLiteral("delay")));
        QVERIFY(delayParser.parse({QStringLiteral("dracoPho"),
                                   QStringLiteral("--capture-window"),
                                   QStringLiteral("--delay"), QStringLiteral("5")}));
        QVERIFY(headlessInteractiveConflict(delayParser).isEmpty());

        QCommandLineParser cleanParser;
        cleanParser.addOption(QCommandLineOption(QStringLiteral("capture-window")));
        QVERIFY(cleanParser.parse({QStringLiteral("dracoPho"), QStringLiteral("--capture-window")}));
        QVERIFY(headlessInteractiveConflict(cleanParser).isEmpty());

        QCommandLineParser emptyParser;
        QVERIFY(emptyParser.parse({QStringLiteral("dracoPho")}));
        QVERIFY(headlessInteractiveConflict(emptyParser).isEmpty());
    }

    /**
     * 验证 --doctor 与捕获/录制/窗口/文件参数互斥：每类代表旗标都要点名，
     * 纯自检与调试类旗标不冲突。
     * @return 无返回值。
     */
    void detectsDoctorConflicts()
    {
        const QStringList exclusiveFlags{
            QStringLiteral("capture-to"), QStringLiteral("window"),
            QStringLiteral("capture-window"), QStringLiteral("delay"),
            QStringLiteral("record-display"), QStringLiteral("recording-status"),
            QStringLiteral("stop-recording"), QStringLiteral("list-windows"),
        };

        for (const QString &flag : exclusiveFlags) {
            QCommandLineParser parser;
            parser.addOption(QCommandLineOption(QStringLiteral("doctor")));
            parser.addOption(QCommandLineOption(flag));
            QVERIFY(parser.parse({QStringLiteral("dracoPho"),
                                  QStringLiteral("--doctor"),
                                  QStringLiteral("--%1").arg(flag)}));
            const QString conflict = doctorOptionConflict(parser);
            QVERIFY2(!conflict.isEmpty(), qPrintable(QStringLiteral("flag %1 must conflict with --doctor").arg(flag)));
            QVERIFY2(conflict.contains(QStringLiteral("--%1").arg(flag)),
                     qPrintable(QStringLiteral("conflict must name --%1").arg(flag)));
        }

        QCommandLineParser positionalParser;
        positionalParser.addOption(QCommandLineOption(QStringLiteral("doctor")));
        QVERIFY(positionalParser.parse({QStringLiteral("dracoPho"),
                                        QStringLiteral("--doctor"),
                                        QStringLiteral("image.png")}));
        QVERIFY(!doctorOptionConflict(positionalParser).isEmpty());

        QCommandLineParser cleanParser;
        cleanParser.addOption(QCommandLineOption(QStringLiteral("doctor")));
        QVERIFY(cleanParser.parse({QStringLiteral("dracoPho"), QStringLiteral("--doctor")}));
        QVERIFY(doctorOptionConflict(cleanParser).isEmpty());

        QCommandLineParser debugParser;
        debugParser.addOption(QCommandLineOption(QStringLiteral("doctor")));
        debugParser.addOption(QCommandLineOption(QStringLiteral("debug")));
        QVERIFY(debugParser.parse({QStringLiteral("dracoPho"),
                                   QStringLiteral("--doctor"),
                                   QStringLiteral("--debug")}));
        QVERIFY(doctorOptionConflict(debugParser).isEmpty());
    }
};

QTEST_APPLESS_MAIN(HeadlessCaptureOptionsTest)

#include "headless_capture_options_test.moc"
