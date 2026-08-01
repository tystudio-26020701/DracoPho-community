#include "settings/settings_ui_helpers.h"

#include <QApplication>
#include <QKeyEvent>
#include <QtTest/QtTest>

namespace {

/// @brief 向快捷键输入控件派发一次按键。
/// @param edit 快捷键输入控件。
/// @param key 按键代码。
/// @param modifiers 修饰键。
void sendKey(markshot::settings::ShortcutKeySequenceEdit *edit,
             int key,
             Qt::KeyboardModifiers modifiers = Qt::NoModifier)
{
    QKeyEvent press(QEvent::KeyPress, key, modifiers);
    QApplication::sendEvent(edit, &press);
}

}  // namespace

class SettingsShortcutsTest : public QObject {
    Q_OBJECT

private slots:
    /**
     * 验证程序化加载已保存配置不会被校验拦截（默认 Cancel=Escape
     * 必须能原样显示，此前曾因校验被静默清空）。
     */
    void programmaticLoadPreservesSavedSequence()
    {
        markshot::settings::ShortcutKeySequenceEdit edit;
        edit.setKeySequence(QKeySequence(Qt::Key_Escape));
        QCOMPARE(edit.keySequence(), QKeySequence(Qt::Key_Escape));
    }

    /**
     * 验证无修饰键的普通键在全局快捷键模式下被拒绝（防呆：避免
     * 全局抢键破坏日常输入），原有已保存值保持不变。
     */
    void globalHotkeyRejectsPlainKeys()
    {
        markshot::settings::ShortcutKeySequenceEdit edit(true);
        const QKeySequence saved(Qt::CTRL | Qt::SHIFT | Qt::Key_P);
        edit.setKeySequence(saved);

        sendKey(&edit, Qt::Key_A);
        QCOMPARE(edit.keySequence(), saved);
    }

    /**
     * 验证带修饰键的组合在全局快捷键模式下被接受。
     */
    void globalHotkeyAcceptsModifiedCombination()
    {
        markshot::settings::ShortcutKeySequenceEdit edit(true);
        sendKey(&edit, Qt::Key_P, Qt::ControlModifier | Qt::ShiftModifier);
        QCOMPARE(edit.keySequence(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
    }

    /**
     * 验证全局快捷键允许功能键（F1-F35）。
     */
    void globalHotkeyAcceptsFunctionKey()
    {
        markshot::settings::ShortcutKeySequenceEdit edit(true);
        sendKey(&edit, Qt::Key_F5);
        QCOMPARE(edit.keySequence(), QKeySequence(Qt::Key_F5));
    }

    /**
     * 验证普通（非全局）快捷键仍允许无修饰键的普通字母（标注工具
     * 单键习惯，如 Flameshot/Snipaste）。
     */
    void regularShortcutAllowsPlainKeys()
    {
        markshot::settings::ShortcutKeySequenceEdit edit(false);
        sendKey(&edit, Qt::Key_P);
        QCOMPARE(edit.keySequence(), QKeySequence(Qt::Key_P));
    }

    /**
     * 验证无修饰键的危险键（Tab/方向键等）被拒绝，不破坏窗口导航。
     */
    void dangerousKeysAreRejected()
    {
        markshot::settings::ShortcutKeySequenceEdit edit(false);
        const QKeySequence saved(Qt::CTRL | Qt::Key_R);
        edit.setKeySequence(saved);

        sendKey(&edit, Qt::Key_Tab);
        QCOMPARE(edit.keySequence(), saved);
        sendKey(&edit, Qt::Key_Up);
        QCOMPARE(edit.keySequence(), saved);
    }

    /**
     * 验证 Delete/Backspace 无修饰键时清空快捷键。
     */
    void deleteClearsShortcut()
    {
        markshot::settings::ShortcutKeySequenceEdit edit(false);
        edit.setKeySequence(QKeySequence(Qt::CTRL | Qt::Key_C));
        sendKey(&edit, Qt::Key_Delete);
        QVERIFY(edit.keySequence().isEmpty());
    }
};

QTEST_MAIN(SettingsShortcutsTest)
#include "settings_shortcuts_test.moc"
