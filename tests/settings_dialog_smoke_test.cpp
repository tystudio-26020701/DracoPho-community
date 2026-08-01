#include "settings/settings_dialog.h"
#include "ui/i18n.h"
#include "ui/interface_language_config.h"

#include <QAbstractSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QListWidget>
#include <QScrollArea>
#include <QSlider>
#include <QSpinBox>
#include <QStackedWidget>
#include <QWheelEvent>
#include <QtTest/QtTest>

namespace {

/// @brief 在对话框中查找界面语言下拉框。
/// @param dialog 设置对话框。
/// @return 语言下拉框，未找到时返回空指针。
QComboBox *findLanguageCombo(QWidget *dialog)
{
    return dialog->findChild<QComboBox *>(QStringLiteral("settingsLanguageCombo"));
}

/// @brief 查找导航列表。
/// @param dialog 设置对话框。
/// @return 导航列表控件。
QListWidget *findNavigationList(QWidget *dialog)
{
    return dialog->findChild<QListWidget *>(QStringLiteral("settingsNavigation"));
}

/// @brief 向控件发送一次滚轮事件。
/// @param target 接收滚轮事件的控件。
void sendWheelTo(QWidget *target, int angleDeltaY = 120)
{
    const QPointF globalPos = target->mapToGlobal(target->rect().center());
    QWheelEvent event(QPointF(target->rect().center()),
                      globalPos,
                      QPoint(0, 0),
                      QPoint(0, angleDeltaY),
                      Qt::NoButton,
                      Qt::NoModifier,
                      Qt::NoScrollPhase,
                      false);
    QApplication::sendEvent(target, &event);
}

/// @brief 读取数值输入框的当前整数值。
/// @param spin 数值输入框。
/// @return 当前值。
int spinValue(QAbstractSpinBox *spin)
{
    if (auto *intSpin = qobject_cast<QSpinBox *>(spin)) {
        return intSpin->value();
    }
    if (auto *doubleSpin = qobject_cast<QDoubleSpinBox *>(spin)) {
        return static_cast<int>(doubleSpin->value());
    }
    return 0;
}

}  // namespace

class SettingsDialogSmokeTest : public QObject {
    Q_OBJECT

private slots:
    /**
     * 验证设置窗口可构造且包含全部 11 个页面（10 个设置页 + 关于页）。
     */
    void dialogConstructsWithAllPages()
    {
        markshot::settings::SettingsDialog dialog;
        auto *stack = dialog.findChild<QStackedWidget *>();
        QVERIFY(stack != nullptr);
        QCOMPARE(stack->count(), 11);
        QVERIFY(findNavigationList(&dialog) != nullptr);
        QVERIFY(findLanguageCombo(&dialog) != nullptr);
    }

    /**
     * 验证切换界面语言会重建页面并即时翻译界面文本。
     */
    void languageSwitchRebuildsAndTranslates()
    {
        markshot::settings::SettingsDialog dialog;
        auto *stack = dialog.findChild<QStackedWidget *>();
        QVERIFY(stack != nullptr);

        QComboBox *language = findLanguageCombo(&dialog);
        QVERIFY(language != nullptr);
        QCOMPARE(language->count(), 13);

        // 切到日语并等待延迟重建。
        const int japaneseIndex =
            language->findData(static_cast<int>(markshot::ui::UiLanguageMode::Japanese));
        QVERIFY(japaneseIndex >= 0);
        language->setCurrentIndex(japaneseIndex);
        QTest::qWait(100);

        QVERIFY(markshot::i18n::language() == markshot::i18n::Language::Japanese);
        QVERIFY(markshot::i18n::translate(QStringLiteral("Settings")) == QStringLiteral("設定"));
        // 重建后页面数量不变。
        QCOMPARE(stack->count(), 11);

        // 切回英文。
        QComboBox *rebuiltLanguage = findLanguageCombo(&dialog);
        QVERIFY(rebuiltLanguage != nullptr);
        const int englishIndex =
            rebuiltLanguage->findData(static_cast<int>(markshot::ui::UiLanguageMode::English));
        QVERIFY(englishIndex >= 0);
        rebuiltLanguage->setCurrentIndex(englishIndex);
        QTest::qWait(100);
        QVERIFY(markshot::i18n::language() == markshot::i18n::Language::English);
    }

    /**
     * 验证修改配置后导航出现"未保存修改"标记，还原后标记消失。
     */
    void dirtyMarkerAppearsAndClears()
    {
        markshot::settings::SettingsDialog dialog;
        QListWidget *nav = findNavigationList(&dialog);
        QVERIFY(nav != nullptr);
        const QString initial = nav->item(0)->text();
        QVERIFY(!initial.contains(QStringLiteral("\u25CF")));

        // 展示窗口后模拟真实点击通用页"启动到托盘"开关。
        dialog.show();
        auto *stack = dialog.findChild<QStackedWidget *>();
        QVERIFY(stack != nullptr);
        auto *generalArea = qobject_cast<QScrollArea *>(stack->widget(0));
        QVERIFY(generalArea != nullptr);
        QWidget *generalPage = generalArea->widget();
        const auto generalSwitches = generalPage->findChildren<QCheckBox *>();
        QVERIFY(!generalSwitches.isEmpty());
        QCheckBox *firstSwitch = generalSwitches.first();
        const bool checkedBefore = firstSwitch->isChecked();
        QTest::mouseClick(firstSwitch, Qt::LeftButton);
        QTest::qWait(100);
        QVERIFY2(firstSwitch->isChecked() != checkedBefore,
                 "mouse click should toggle the switch");

        QVERIFY2(nav->item(0)->text().contains(QStringLiteral("\u25CF")),
                 qPrintable(nav->item(0)->text()));

        // 还原该页后标记消失。
        dialog.reloadForDisplay();
        QListWidget *rebuiltNav = findNavigationList(&dialog);
        QVERIFY(rebuiltNav != nullptr);
        const QString after = rebuiltNav->item(0)->text();
        QVERIFY(!after.contains(QStringLiteral("\u25CF")));
    }

    /**
     * 验证滚轮防护在真实设置对话框中生效：悬停在未聚焦的
     * QSpinBox / QDoubleSpinBox / QComboBox / QSlider 上滚动滚轮
     * 不会篡改任何控件内容（Qt 组件自带的滚轮响应必须被拦截）。
     */
    void wheelScrollDoesNotTamperAnyControl()
    {
        markshot::settings::SettingsDialog dialog;
        dialog.show();
        QTest::qWaitForWindowExposed(&dialog);
        QTest::qWait(100);

        // 遍历所有页面里的受防护控件。
        auto *stack = dialog.findChild<QStackedWidget *>();
        QVERIFY(stack != nullptr);
        for (int pageIndex = 0; pageIndex < stack->count(); ++pageIndex) {
            auto *area = qobject_cast<QScrollArea *>(stack->widget(pageIndex));
            if (!area) {
                continue;
            }
            QWidget *page = area->widget();
            if (!page) {
                continue;
            }
            // 数值框：QSpinBox / QDoubleSpinBox
            const auto spins = page->findChildren<QAbstractSpinBox *>();
            for (QAbstractSpinBox *spin : spins) {
                if (!spin->isEnabled()) {
                    continue;
                }
                spin->clearFocus();
                const int before = spinValue(spin);
                sendWheelTo(spin, 120);
                QTest::qWait(20);
                QCOMPARE(spinValue(spin), before);
            }
            // 下拉框：QComboBox（跳过语言下拉框，避免触发语言重建干扰页面）
            const auto combos = page->findChildren<QComboBox *>();
            for (QComboBox *combo : combos) {
                if (!combo->isEnabled()
                    || combo->objectName() == QStringLiteral("settingsLanguageCombo")) {
                    continue;
                }
                combo->clearFocus();
                const int before = combo->currentIndex();
                sendWheelTo(combo, 120);
                QTest::qWait(20);
                QCOMPARE(combo->currentIndex(), before);
            }
            // 滑块：QSlider
            const auto sliders = page->findChildren<QSlider *>();
            for (QSlider *slider : sliders) {
                if (!slider->isEnabled()) {
                    continue;
                }
                slider->clearFocus();
                const int before = slider->value();
                sendWheelTo(slider, 120);
                QTest::qWait(20);
                QCOMPARE(slider->value(), before);
            }
        }
    }
};

QTEST_MAIN(SettingsDialogSmokeTest)
#include "settings_dialog_smoke_test.moc"
