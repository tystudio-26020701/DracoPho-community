#include "settings/settings_ui_helpers.h"
#include "ui/i18n.h"

#include <QApplication>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <QWheelEvent>
#include <QtTest/QtTest>

namespace {

/// @brief 向控件发送一次滚轮事件。
/// @param target 接收事件的控件。
/// @param deltaY 纵向滚轮增量（每格 120）。
void sendWheelTo(QWidget *target, int deltaY)
{
    const QPointF globalPos = target->mapToGlobal(target->rect().center());
    QWheelEvent event(QPointF(target->rect().center()),
                      globalPos,
                      QPoint(0, 0),
                      QPoint(0, deltaY),
                      Qt::NoButton,
                      Qt::NoModifier,
                      Qt::NoScrollPhase,
                      false);
    QApplication::sendEvent(target, &event);
}

}  // namespace

/// @brief 源头级滚轮防护回归测试：设置控件工厂创建的数值框/下拉框
/// 无论是否聚焦、无论事件目标是控件本身还是内部子控件，
/// 滚轮都不得篡改其值（Qt 默认行为会改值，本测试锁定抑制逻辑）。
class SettingsSourceWheelTest : public QObject {
    Q_OBJECT

private slots:
    void wheelDoesNotTamperFactoryControls()
    {
        QWidget host;
        auto *form = new QFormLayout(&host);
        QSpinBox *spin = markshot::settings::addSpinRow(form, QStringLiteral("spin"), 0, 1000);
        QDoubleSpinBox *dspin =
            markshot::settings::addDoubleRow(form, QStringLiteral("dspin"), 0.0, 100.0, 2);
        QComboBox *combo = markshot::settings::addComboRow(form, QStringLiteral("combo"));
        combo->addItem(QStringLiteral("A"));
        combo->addItem(QStringLiteral("B"));
        host.resize(400, 300);
        host.show();
        QVERIFY(QTest::qWaitForWindowExposed(&host));
        QTest::qWait(50);

        // 1. 未聚焦时滚轮不改值。
        host.setFocus();
        spin->clearFocus();
        dspin->clearFocus();
        combo->clearFocus();
        QTest::qWait(20);
        spin->setValue(22);
        dspin->setValue(3.5);
        combo->setCurrentIndex(0);

        sendWheelTo(spin, 120);
        sendWheelTo(dspin, 120);
        sendWheelTo(combo, 120);
        QCOMPARE(spin->value(), 22);
        QCOMPARE(dspin->value(), 3.5);
        QCOMPARE(combo->currentIndex(), 0);

        // 2. 聚焦时滚轮同样不改值（设置页滚轮用于翻页，不用于调值）。
        spin->setFocus();
        dspin->setFocus();
        combo->setFocus();
        QTest::qWait(20);
        sendWheelTo(spin, 120);
        sendWheelTo(dspin, 120);
        sendWheelTo(combo, 120);
        QCOMPARE(spin->value(), 22);
        QCOMPARE(dspin->value(), 3.5);
        QCOMPARE(combo->currentIndex(), 0);

        // 3. 事件目标是数值框内部 QLineEdit 时同样不改值。
        if (QLineEdit *le = spin->findChild<QLineEdit *>()) {
            sendWheelTo(le, 120);
            QCOMPARE(spin->value(), 22);
        }
        if (QLineEdit *le = combo->findChild<QLineEdit *>()) {
            sendWheelTo(le, 120);
            QCOMPARE(combo->currentIndex(), 0);
        }
    }
};

QTEST_MAIN(SettingsSourceWheelTest)
#include "settings_source_wheel_test.moc"
