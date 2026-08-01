#include "settings/settings_dialog.h"
#include "ui/i18n.h"
#include "ui/interface_language_config.h"

#include <QCheckBox>
#include <QComboBox>
#include <QListWidget>
#include <QScrollArea>
#include <QStackedWidget>
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
};

QTEST_MAIN(SettingsDialogSmokeTest)
#include "settings_dialog_smoke_test.moc"
