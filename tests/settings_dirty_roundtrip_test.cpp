#include "settings/settings_dialog.h"
#include "settings/settings_page_about.h"
#include "settings/settings_page_advanced.h"
#include "settings/settings_page_annotation.h"
#include "settings/settings_page_capture.h"
#include "settings/settings_page_general.h"
#include "settings/settings_page_integrations.h"
#include "settings/settings_page_pinned.h"
#include "settings/settings_page_plugins.h"
#include "settings/settings_page_scroll.h"
#include "settings/settings_page_shortcuts.h"
#include "settings/settings_page_storage.h"

#include <QAbstractSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QKeySequenceEdit>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QSpinBox>
#include <QStackedWidget>
#include <QLabel>
#include <QtTest/QtTest>

#include <functional>

class SettingsDirtyRoundTripTest : public QObject {
    Q_OBJECT

private slots:
    void probeLanguageRoundTrip()
    {
        markshot::settings::SettingsDialog dialog;
        dialog.show();

        auto findLanguage = [&dialog] {
            return dialog.findChild<QComboBox *>(QStringLiteral("settingsLanguageCombo"));
        };

        const int original = findLanguage()->currentIndex();
        const int other = (original + 1) % findLanguage()->count();
        findLanguage()->setCurrentIndex(other);
        QTest::qWait(120);

        QLabel *status = dialog.findChild<QLabel *>(QStringLiteral("settingsStatus"));
        QVERIFY(status != nullptr);
        const bool dirtyAfterChange =
            status->property("state").toString() == QStringLiteral("warning");
        QVERIFY2(dirtyAfterChange, "language change should mark dirty");

        findLanguage()->setCurrentIndex(original);
        QTest::qWait(120);

        status = dialog.findChild<QLabel *>(QStringLiteral("settingsStatus"));
        QVERIFY(status != nullptr);
        const bool dirtyAfterRevert =
            status->property("state").toString() == QStringLiteral("warning");
        QVERIFY2(!dirtyAfterRevert, "language revert should clear dirty marker");
    }

    void probeThemeRoundTrip()
    {
        markshot::settings::SettingsDialog dialog;
        dialog.show();

        auto *stack = dialog.findChild<QStackedWidget *>();
        QVERIFY(stack != nullptr);
        auto *generalArea = qobject_cast<QScrollArea *>(stack->widget(0));
        QVERIFY(generalArea != nullptr);
        QWidget *generalPage = generalArea->widget();

        const auto combos = generalPage->findChildren<QComboBox *>();
        QComboBox *theme = nullptr;
        for (QComboBox *combo : combos) {
            if (combo->objectName() != QStringLiteral("settingsLanguageCombo")) {
                theme = combo;
                break;
            }
        }
        QVERIFY(theme != nullptr && theme->count() >= 2);

        auto *generalPagePtr = dynamic_cast<markshot::settings::SettingsPageGeneral *>(generalPage);
        QVERIFY(generalPagePtr != nullptr);

        const int original = theme->currentIndex();
        theme->setCurrentIndex((original + 1) % theme->count());
        QVERIFY(generalPagePtr->isModified());
        theme->setCurrentIndex(original);
        QVERIFY2(!generalPagePtr->isModified(), "theme revert should clear dirty");
    }

    void probeStatusFollowsComboValue()
    {
        markshot::settings::SettingsDialog dialog;
        dialog.show();
        QVERIFY(QTest::qWaitForWindowExposed(&dialog));
        QTest::qWait(50);

        auto *stack = dialog.findChild<QStackedWidget *>();
        QVERIFY(stack != nullptr);
        auto *generalArea = qobject_cast<QScrollArea *>(stack->widget(0));
        QVERIFY(generalArea != nullptr);
        QWidget *generalPage = generalArea->widget();

        const auto combos = generalPage->findChildren<QComboBox *>();
        QComboBox *theme = nullptr;
        for (QComboBox *combo : combos) {
            if (combo->objectName() != QStringLiteral("settingsLanguageCombo")) {
                theme = combo;
                break;
            }
        }
        QVERIFY(theme != nullptr && theme->count() >= 2);

        QLabel *status = dialog.findChild<QLabel *>(QStringLiteral("settingsStatus"));
        QVERIFY(status != nullptr);
        auto isDirty = [&status] {
            return status->property("state").toString() == QStringLiteral("warning");
        };

        // 改变下拉框值：值变更信号应触发延迟刷新，状态栏立即反映"未保存"。
        const int original = theme->currentIndex();
        theme->setCurrentIndex((original + 1) % theme->count());
        QTest::qWait(50);
        QVERIFY2(isDirty(), "combo change should mark status dirty");

        // 改回已保存值：状态栏必须清除"未保存"标记。
        theme->setCurrentIndex(original);
        QTest::qWait(50);
        QVERIFY2(!isDirty(), "combo revert should clear status dirty");
    }

    void probeAllControls()
    {
        markshot::settings::SettingsDialog dialog;
        dialog.show();

        auto *stack = dialog.findChild<QStackedWidget *>();
        QVERIFY(stack != nullptr);

        struct PageProbe {
            QWidget *page;
            std::function<bool()> modified;
        };
        QList<PageProbe> pages;
        for (int i = 0; i < stack->count(); ++i) {
            auto *area = qobject_cast<QScrollArea *>(stack->widget(i));
            if (!area) {
                continue;
            }
            QWidget *page = area->widget();
            if (!page) {
                continue;
            }
            std::function<bool()> modified;
            if (auto *p = dynamic_cast<markshot::settings::SettingsPageGeneral *>(page)) {
                modified = [p] { return p->isModified(); };
            } else if (auto *p = dynamic_cast<markshot::settings::SettingsPageCapture *>(page)) {
                modified = [p] { return p->isModified(); };
            } else if (auto *p = dynamic_cast<markshot::settings::SettingsPageShortcuts *>(page)) {
                modified = [p] { return p->isModified(); };
            } else if (auto *p = dynamic_cast<markshot::settings::SettingsPageAnnotation *>(page)) {
                modified = [p] { return p->isModified(); };
            } else if (auto *p = dynamic_cast<markshot::settings::SettingsPagePinned *>(page)) {
                modified = [p] { return p->isModified(); };
            } else if (auto *p = dynamic_cast<markshot::settings::SettingsPageIntegrations *>(page)) {
                modified = [p] { return p->isModified(); };
            } else if (auto *p = dynamic_cast<markshot::settings::SettingsPagePlugins *>(page)) {
                modified = [p] { return p->isModified(); };
            } else if (auto *p = dynamic_cast<markshot::settings::SettingsPageScroll *>(page)) {
                modified = [p] { return p->isModified(); };
            } else if (auto *p = dynamic_cast<markshot::settings::SettingsPageStorage *>(page)) {
                modified = [p] { return p->isModified(); };
            } else if (auto *p = dynamic_cast<markshot::settings::SettingsPageAdvanced *>(page)) {
                modified = [p] { return p->isModified(); };
            }
            if (!modified) {
                continue;
            }
            pages.append({page, modified});
        }

        int failures = 0;
        int pageOrdinal = 0;
        for (const PageProbe &probe : pages) {
            QWidget *page = probe.page;
            ++pageOrdinal;
            const QString pageName = page->metaObject()->className();

            if (probe.modified()) {
                qInfo() << "FAIL: page dirty right after load" << pageOrdinal << pageName;
                ++failures;
            }

            const auto checkboxes = page->findChildren<QCheckBox *>();
            for (QCheckBox *box : checkboxes) {
                if (!box->isEnabled()) {
                    continue;
                }
                const bool original = box->isChecked();
                box->setChecked(!original);
                if (!probe.modified()) {
                    qInfo() << "FAIL: checkbox change not detected" << pageOrdinal << pageName << box->objectName();
                    ++failures;
                }
                box->setChecked(original);
                if (probe.modified()) {
                    qInfo() << "FAIL: checkbox revert still dirty" << pageOrdinal << pageName << box->objectName();
                    ++failures;
                }
            }

            const auto combos = page->findChildren<QComboBox *>();
            for (QComboBox *combo : combos) {
                if (!combo->isEnabled() || combo->count() < 2) {
                    continue;
                }
                const int original = combo->currentIndex();
                combo->setCurrentIndex((original + 1) % combo->count());
                if (!probe.modified()) {
                    qInfo() << "FAIL: combo change not detected" << pageOrdinal << pageName << combo->objectName();
                    ++failures;
                }
                combo->setCurrentIndex(original);
                if (probe.modified()) {
                    qInfo() << "FAIL: combo revert still dirty" << pageOrdinal << pageName << combo->objectName();
                    ++failures;
                }
            }

            const auto spins = page->findChildren<QAbstractSpinBox *>();
            for (QAbstractSpinBox *spin : spins) {
                if (!spin->isEnabled()) {
                    continue;
                }
                if (auto *intSpin = qobject_cast<QSpinBox *>(spin)) {
                    const int original = intSpin->value();
                    intSpin->setValue(original + 1 > intSpin->maximum() ? intSpin->minimum() : original + 1);
                    if (!probe.modified()) {
                        qInfo() << "FAIL: spin change not detected" << pageOrdinal << pageName << spin->objectName();
                        ++failures;
                    }
                    intSpin->setValue(original);
                    if (probe.modified()) {
                        qInfo() << "FAIL: spin revert still dirty" << pageOrdinal << pageName << spin->objectName();
                        ++failures;
                    }
                } else if (auto *doubleSpin = qobject_cast<QDoubleSpinBox *>(spin)) {
                    const double original = doubleSpin->value();
                    const double changed = original + 1.0 > doubleSpin->maximum() ? doubleSpin->minimum() : original + 1.0;
                    doubleSpin->setValue(changed);
                    if (!probe.modified()) {
                        qInfo() << "FAIL: spin change not detected" << pageOrdinal << pageName << spin->objectName();
                        ++failures;
                    }
                    doubleSpin->setValue(original);
                    if (probe.modified()) {
                        qInfo() << "FAIL: spin revert still dirty" << pageOrdinal << pageName << spin->objectName();
                        ++failures;
                    }
                }
            }

            const auto lineEdits = page->findChildren<QLineEdit *>();
            for (QLineEdit *edit : lineEdits) {
                if (!edit->isEnabled() || edit->isReadOnly()
                    || qobject_cast<QAbstractSpinBox *>(edit->parent())
                    || qobject_cast<QKeySequenceEdit *>(edit->parent())) {
                    continue;
                }
                const QString original = edit->text();
                edit->setText(QStringLiteral("__probe__"));
                if (!probe.modified()) {
                    qInfo() << "FAIL: lineedit change not detected" << pageOrdinal << pageName << edit->objectName();
                    ++failures;
                }
                edit->setText(original);
                if (probe.modified()) {
                    qInfo() << "FAIL: lineedit revert still dirty" << pageOrdinal << pageName << edit->objectName();
                    ++failures;
                }
            }

            const auto plainEdits = page->findChildren<QPlainTextEdit *>();
            for (QPlainTextEdit *edit : plainEdits) {
                if (!edit->isEnabled() || edit->isReadOnly()) {
                    continue;
                }
                const QString original = edit->toPlainText();
                edit->setPlainText(QStringLiteral("__probe__=1"));
                if (!probe.modified()) {
                    qInfo() << "FAIL: plainedit change not detected" << pageOrdinal << pageName << edit->objectName();
                    ++failures;
                }
                edit->setPlainText(original);
                if (probe.modified()) {
                    qInfo() << "FAIL: plainedit revert still dirty" << pageOrdinal << pageName << edit->objectName();
                    ++failures;
                }
            }

            const auto keyEdits = page->findChildren<QKeySequenceEdit *>();
            for (QKeySequenceEdit *edit : keyEdits) {
                if (!edit->isEnabled()) {
                    continue;
                }
                const QKeySequence original = edit->keySequence();
                QKeySequence other(QStringLiteral("Alt+9"));
                if (other == original) {
                    other = QKeySequence(QStringLiteral("Alt+8"));
                }
                edit->setKeySequence(other);
                if (!probe.modified()) {
                    qInfo() << "FAIL: keyedit change not detected" << pageOrdinal << pageName << edit->objectName();
                    ++failures;
                }
                edit->setKeySequence(original);
                if (probe.modified()) {
                    qInfo() << "FAIL: keyedit revert still dirty" << pageOrdinal << pageName << edit->objectName();
                    ++failures;
                }
            }
        }

        QVERIFY2(failures == 0, qPrintable(QStringLiteral("%1 failures").arg(failures)));
    }
};

QTEST_MAIN(SettingsDirtyRoundTripTest)
#include "settings_dirty_roundtrip_test.moc"
