#include "settings/settings_ui_helpers.h"

#include "ui/i18n.h"
#include "ui/theme.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QKeyEvent>
#include <QKeySequence>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextCursor>
#include <QToolTip>
#include <QVBoxLayout>

#include <algorithm>

namespace markshot::settings {

namespace {

void showLineEditContextMenu(QLineEdit *edit, const QPoint &pos)
{
    QMenu menu(edit);
    menu.setStyleSheet(markshot::theme::menuStyleSheet());
    const bool hasSelection = edit->hasSelectedText();
    const bool hasText = !edit->text().isEmpty();
    const bool hasClipboard = !QApplication::clipboard()->text().isEmpty();

    auto addAction = [&menu, edit](const QString &label, const QKeySequence &shortcut, bool enabled, auto callback) {
        QAction *action = menu.addAction(label);
        action->setShortcut(shortcut);
        action->setShortcutVisibleInContextMenu(true);
        action->setEnabled(enabled);
        QObject::connect(action, &QAction::triggered, edit, callback);
    };

    addAction(MS_TR("Undo"), QKeySequence::Undo, edit->isUndoAvailable(), [edit] { edit->undo(); });
    addAction(MS_TR("Redo"), QKeySequence::Redo, edit->isRedoAvailable(), [edit] { edit->redo(); });
    menu.addSeparator();
    addAction(MS_TR("Cut"), QKeySequence::Cut, hasSelection, [edit] { edit->cut(); });
    addAction(MS_TR("Copy"), QKeySequence::Copy, hasSelection, [edit] { edit->copy(); });
    addAction(MS_TR("Paste"), QKeySequence::Paste, hasClipboard, [edit] { edit->paste(); });
    addAction(MS_TR("Delete"), QKeySequence(Qt::Key_Delete), hasSelection, [edit] { edit->del(); });
    menu.addSeparator();
    addAction(MS_TR("Select All"), QKeySequence::SelectAll, hasText, [edit] { edit->selectAll(); });

    menu.exec(edit->mapToGlobal(pos));
}

void showPlainTextEditContextMenu(QPlainTextEdit *edit, const QPoint &pos)
{
    QMenu menu(edit);
    menu.setStyleSheet(markshot::theme::menuStyleSheet());
    const QTextCursor cursor = edit->textCursor();
    const bool hasSelection = cursor.hasSelection();
    const bool hasDocumentText = !edit->document()->isEmpty();
    const bool hasClipboard = !QApplication::clipboard()->text().isEmpty();

    auto addAction = [&menu, edit](const QString &label, const QKeySequence &shortcut, bool enabled, auto callback) {
        QAction *action = menu.addAction(label);
        action->setShortcut(shortcut);
        action->setShortcutVisibleInContextMenu(true);
        action->setEnabled(enabled);
        QObject::connect(action, &QAction::triggered, edit, callback);
    };

    addAction(MS_TR("Undo"), QKeySequence::Undo, edit->document()->isUndoAvailable(), [edit] { edit->undo(); });
    addAction(MS_TR("Redo"), QKeySequence::Redo, edit->document()->isRedoAvailable(), [edit] { edit->redo(); });
    menu.addSeparator();
    addAction(MS_TR("Cut"), QKeySequence::Cut, hasSelection, [edit] { edit->cut(); });
    addAction(MS_TR("Copy"), QKeySequence::Copy, hasSelection, [edit] { edit->copy(); });
    addAction(MS_TR("Paste"), QKeySequence::Paste, hasClipboard, [edit] { edit->paste(); });
    addAction(MS_TR("Delete"), QKeySequence(Qt::Key_Delete), hasSelection, [edit] {
        QTextCursor sel = edit->textCursor();
        sel.removeSelectedText();
        edit->setTextCursor(sel);
    });
    menu.addSeparator();
    addAction(MS_TR("Select All"), QKeySequence::SelectAll, hasDocumentText, [edit] { edit->selectAll(); });

    menu.exec(edit->viewport()->mapToGlobal(pos));
}

} // namespace

/// @brief 判断按键是否属于不应作为快捷键的"危险"键。
/// @param key 按键代码。
/// @return 需要拒绝时返回 true。
bool isForbiddenShortcutKey(int key)
{
    switch (key) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
    case Qt::Key_Escape:
    case Qt::Key_Tab:
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Insert:
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Left:
    case Qt::Key_Up:
    case Qt::Key_Right:
    case Qt::Key_Down:
    case Qt::Key_CapsLock:
    case Qt::Key_NumLock:
    case Qt::Key_ScrollLock:
    case Qt::Key_Pause:
    case Qt::Key_Print:
    case Qt::Key_Menu:
    case Qt::Key_SysReq:
        return true;
    default:
        return false;
    }
}

/// @brief 判断一个快捷键序列是否合法。
/// @param sequence 快捷键序列。
/// @return 合法时返回 true。
bool isValidShortcutSequence(const QKeySequence &sequence)
{
    if (sequence.isEmpty()) {
        return true;
    }
    if (sequence.count() != 1) {
        return false;
    }

    const QKeyCombination combination = sequence[0];
    const Qt::KeyboardModifiers modifiers = combination.keyboardModifiers();
    const int key = combination.key();

    // 纯修饰键组合（Ctrl/Alt/Shift/Logo 单独或只组合修饰键）不可作为快捷键。
    switch (key) {
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_AltGr:
        return false;
    default:
        break;
    }

    // 无修饰键的危险键（Delete/Backspace/Escape/Tab/Enter/方向键等）会被拒绝，
    // 避免破坏编辑操作或窗口导航；带修饰键时允许（例如 Ctrl+Delete）。
    if (modifiers == Qt::NoModifier && isForbiddenShortcutKey(key)) {
        return false;
    }

    return true;
}

/// @brief 提供合法快捷键校验与即时反馈的 QKeySequenceEdit。
class ShortcutKeySequenceEdit final : public QKeySequenceEdit {
public:
    explicit ShortcutKeySequenceEdit(QWidget *parent = nullptr)
        : QKeySequenceEdit(parent)
    {
        setMaximumSequenceLength(1);
        setClearButtonEnabled(true);
        connect(this, &QKeySequenceEdit::keySequenceChanged, this, [this](const QKeySequence &sequence) {
            if (isValidShortcutSequence(sequence)) {
                m_lastValid = sequence;
                setToolTip({});
                return;
            }
            const QSignalBlocker blocker(this);
            setKeySequence(m_lastValid);
            setToolTip(MS_TR("This key combination is not allowed as a shortcut."));
            if (hasFocus()) {
                QToolTip::showText(mapToGlobal(rect().bottomLeft()),
                                   MS_TR("This key combination is not allowed as a shortcut."),
                                   this);
            }
        });
    }

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        const int key = event->key();
        const Qt::KeyboardModifiers modifiers = event->modifiers();

        // 单独按下修饰键时忽略，避免记录成"只有修饰键"的无效快捷键。
        if (key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt
            || key == Qt::Key_Meta || key == Qt::Key_AltGr) {
            event->accept();
            return;
        }

        // Delete/Backspace 无修饰键时用于清空当前快捷键（而不是把 Delete
        // 本身记录成快捷键），同时给出明确反馈。
        if (modifiers == Qt::NoModifier
            && (key == Qt::Key_Delete || key == Qt::Key_Backspace)) {
            clear();
            event->accept();
            setToolTip(MS_TR("Press a key combination to assign, or leave empty to disable."));
            return;
        }

        // 无修饰键的其他危险键（Escape/Tab/Enter/方向键等）直接拒绝。
        if (modifiers == Qt::NoModifier && isForbiddenShortcutKey(key)) {
            event->accept();
            setToolTip(MS_TR("This key combination is not allowed as a shortcut."));
            QToolTip::showText(mapToGlobal(rect().bottomLeft()),
                               MS_TR("This key combination is not allowed as a shortcut."),
                               this);
            return;
        }

        QKeySequenceEdit::keyPressEvent(event);
    }

private:
    QKeySequence m_lastValid;
};

QVBoxLayout *createSettingsPageLayout(QWidget *parent)
{
    auto *layout = new QVBoxLayout(parent);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(14);
    return layout;
}

QFrame *createSettingsCard(const QString &title, const QString &description, QWidget *parent)
{
    auto *card = new QFrame(parent);
    card->setObjectName(QStringLiteral("settingsCard"));
    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 14, 16, 16);
    layout->setSpacing(8);

    auto *titleLabel = new QLabel(title, card);
    titleLabel->setObjectName(QStringLiteral("settingsCardTitle"));
    layout->addWidget(titleLabel);

    if (!description.isEmpty()) {
        auto *descriptionLabel = new QLabel(description, card);
        descriptionLabel->setObjectName(QStringLiteral("settingsCardDescription"));
        descriptionLabel->setWordWrap(true);
        layout->addWidget(descriptionLabel);
    }

    auto *form = new QFormLayout;
    form->setObjectName(QStringLiteral("settingsCardForm"));
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    form->setFormAlignment(Qt::AlignTop);
    form->setHorizontalSpacing(18);
    form->setVerticalSpacing(10);
    layout->addLayout(form);
    return card;
}

QFormLayout *settingsCardForm(QFrame *card)
{
    return card ? card->findChild<QFormLayout *>(QStringLiteral("settingsCardForm")) : nullptr;
}

QCheckBox *addSwitchRow(QFormLayout *form, const QString &label, const QString &description)
{
    auto *box = new QCheckBox(description);
    box->setCursor(Qt::PointingHandCursor);
    form->addRow(label, box);
    return box;
}

QLineEdit *addTextRow(QFormLayout *form, const QString &label, const QString &placeholder)
{
    auto *edit = new QLineEdit;
    edit->setPlaceholderText(placeholder);
    edit->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(edit, &QLineEdit::customContextMenuRequested, edit, [edit](const QPoint &pos) {
        showLineEditContextMenu(edit, pos);
    });
    form->addRow(label, edit);
    return edit;
}

QPlainTextEdit *addPlainTextRow(QFormLayout *form, const QString &label, const QString &placeholder)
{
    auto *edit = new QPlainTextEdit;
    edit->setPlaceholderText(placeholder);
    edit->setMinimumHeight(74);
    edit->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(edit, &QPlainTextEdit::customContextMenuRequested, edit, [edit](const QPoint &pos) {
        showPlainTextEditContextMenu(edit, pos);
    });
    form->addRow(label, edit);
    return edit;
}

QSpinBox *addSpinRow(QFormLayout *form, const QString &label, int minimum, int maximum, const QString &suffix)
{
    auto *spin = new QSpinBox;
    spin->setRange(minimum, maximum);
    spin->setSuffix(suffix);
    spin->setContextMenuPolicy(Qt::NoContextMenu);
    form->addRow(label, spin);
    return spin;
}

QDoubleSpinBox *addDoubleRow(QFormLayout *form, const QString &label, double minimum, double maximum, int decimals)
{
    auto *spin = new QDoubleSpinBox;
    spin->setRange(minimum, maximum);
    spin->setDecimals(decimals);
    spin->setContextMenuPolicy(Qt::NoContextMenu);
    form->addRow(label, spin);
    return spin;
}

QComboBox *addComboRow(QFormLayout *form, const QString &label)
{
    auto *combo = new QComboBox;
    combo->setCursor(Qt::PointingHandCursor);
    combo->setContextMenuPolicy(Qt::NoContextMenu);
    form->addRow(label, combo);
    return combo;
}

QKeySequenceEdit *addShortcutRow(QFormLayout *form, const QString &label)
{
    auto *edit = new ShortcutKeySequenceEdit;
    edit->setContextMenuPolicy(Qt::NoContextMenu);
    if (auto *le = edit->findChild<QLineEdit *>()) {
        le->setContextMenuPolicy(Qt::NoContextMenu);
    }
    form->addRow(label, edit);
    return edit;
}

QString colorButtonStyleSheet(const QColor &color)
{
    const QString name = color.isValid()
        ? color.name(QColor::HexRgb).toUpper()
        : markshot::theme::kDefaultAnnotationColor.name(QColor::HexRgb).toUpper();
    return QStringLiteral(
               "QPushButton {"
               " min-height: 30px;"
               " padding: 2px 8px;"
               " border-radius: 8px;"
               " border: 1px solid #334155;"
               " background: %1;"
               " color: #0F172A;"
               " font-weight: 700;"
               "}"
               "QPushButton:hover { border-color: #5EEAD4; }")
        .arg(name);
}

}  // namespace markshot::settings
