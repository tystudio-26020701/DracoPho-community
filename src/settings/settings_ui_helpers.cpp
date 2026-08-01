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
#include <QHBoxLayout>
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
#include <QWheelEvent>

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

/// @brief 拦截滚轮事件，禁止鼠标悬停/聚焦时通过滚轮篡改控件值。
///
/// Qt 默认行为：QSpinBox/QDoubleSpinBox 即使未获得焦点也会响应滚轮改值；
/// QComboBox 在样式允许滚轮滚动时同样无焦点改选中项（Qt 6 源码
/// QComboBox::wheelEvent 只看 SH_ComboBox_AllowWheelScrolling，不看焦点）。
/// 设置页里滚轮的主要用途是翻页，因此直接吞掉发给这些控件的滚轮事件，
/// 让事件不会被控件消费、也不会误改值。事件被 accept 后不会继续向父级
/// 传播，页面滚动由外层 QScrollArea 自己接收（鼠标在控件上滚动时，
/// 由于本过滤器吞掉了事件，QScrollArea 不会滚动——这与未聚焦保护的目标
/// 一致：宁可停在原地也不误改值）。
class WheelSuppressor final : public QObject {
public:
    explicit WheelSuppressor(QObject *parent)
        : QObject(parent)
    {
    }

    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (event->type() == QEvent::Wheel) {
            event->accept();
            return true;
        }
        return QObject::eventFilter(watched, event);
    }
};

/// @brief 为控件安装滚轮抑制过滤器（控件级，覆盖控件自身及其子控件）。
/// @param widget 需要抑制滚轮的控件。
void suppressWheelOn(QWidget *widget)
{
    if (!widget) {
        return;
    }
    widget->installEventFilter(new WheelSuppressor(widget));
    // 子控件（例如 QSpinBox 内部 QLineEdit）也会收到滚轮，同样拦截。
    const auto children = widget->findChildren<QWidget *>();
    for (QWidget *child : children) {
        if (child->parent() == widget) {
            child->installEventFilter(new WheelSuppressor(child));
        }
    }
}

/// @brief 判断按键是否属于不应作为快捷键的"危险"键。
/// @param key 按键代码。
/// @return 需要拒绝时返回 true。
bool isForbiddenShortcutKey(int key)
{
    switch (key) {
    case Qt::Key_Delete:
    case Qt::Key_Backspace:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
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

/// @brief 判断按键是否为功能键（F1-F35）。
/// @param key 按键代码。
/// @return 是功能键时返回 true。
bool isFunctionKey(int key)
{
    return key >= Qt::Key_F1 && key <= Qt::Key_F35;
}

/// @brief 判断快捷键序列是否仍处于组合键录入中的中间状态。
/// @param sequence 快捷键序列。
/// @return 处于录入中间状态时返回 true。
bool isModifierOnlySequence(const QKeySequence &sequence)
{
    if (sequence.isEmpty() || sequence.count() != 1) {
        return false;
    }
    switch (sequence[0].key()) {
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Alt:
    case Qt::Key_Meta:
    case Qt::Key_AltGr:
        return true;
    default:
        return false;
    }
}

/// @brief 判断一个快捷键序列是否合法。
/// @param sequence 快捷键序列。
/// @param globalHotkey 是否属于全局快捷键（需要修饰键或功能键）。
/// @return 合法时返回 true。
bool isValidShortcutSequence(const QKeySequence &sequence, bool globalHotkey)
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

    // 全局快捷键若为无修饰键的普通键（字母/数字/符号），会在全局抢走该键，
    // 属于明显误操作，要求至少一个修饰键或使用功能键。
    if (globalHotkey && modifiers == Qt::NoModifier && !isFunctionKey(key)) {
        return false;
    }

    // 无修饰键的危险键（Delete/Backspace/Tab/Enter/方向键等）会被拒绝，
    // 避免破坏编辑操作或窗口导航；带修饰键时允许（例如 Ctrl+Delete）。
    if (modifiers == Qt::NoModifier && isForbiddenShortcutKey(key)) {
        return false;
    }

    return true;
}

ShortcutKeySequenceEdit::ShortcutKeySequenceEdit(bool globalHotkey, QWidget *parent)
    : QKeySequenceEdit(parent)
    , m_globalHotkey(globalHotkey)
{
    setMaximumSequenceLength(1);
    setClearButtonEnabled(true);
    connect(this, &QKeySequenceEdit::keySequenceChanged, this, [this](const QKeySequence &sequence) {
        // 组合键录入中的中间状态（例如按住 Ctrl 再按 Shift）不打断录入。
        if (isModifierOnlySequence(sequence)) {
            return;
        }
        if (isValidShortcutSequence(sequence, m_globalHotkey)) {
            m_lastValid = sequence;
            setToolTip({});
            return;
        }
        const QSignalBlocker blocker(this);
        QKeySequenceEdit::setKeySequence(m_lastValid);
        setToolTip(invalidToolTip());
        if (hasFocus()) {
            QToolTip::showText(mapToGlobal(rect().bottomLeft()), invalidToolTip(), this);
        }
    });
}

void ShortcutKeySequenceEdit::setKeySequence(const QKeySequence &sequence)
{
    // 程序化加载：跳过录入校验，保证已保存配置（例如默认的 Escape 取消
    // 快捷键）能原样显示，且不触发冲突检测提示。
    const QSignalBlocker blocker(this);
    QKeySequenceEdit::setKeySequence(sequence);
    m_lastValid = sequence;
    setToolTip({});
}

void ShortcutKeySequenceEdit::keyPressEvent(QKeyEvent *event)
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

    // 无修饰键的其他危险键（Tab/Enter/方向键等）直接拒绝。
    if (modifiers == Qt::NoModifier && isForbiddenShortcutKey(key)) {
        event->accept();
        showInvalidFeedback();
        return;
    }

    // 全局快捷键：无修饰键的普通键直接拒绝（必须带修饰键或功能键）。
    if (m_globalHotkey && modifiers == Qt::NoModifier && !isFunctionKey(key)) {
        event->accept();
        showInvalidFeedback();
        return;
    }

    QKeySequenceEdit::keyPressEvent(event);
}

QString ShortcutKeySequenceEdit::invalidToolTip() const
{
    if (m_globalHotkey) {
        return MS_TR("Global shortcuts need at least one modifier key or a function key.");
    }
    return MS_TR("This key combination is not allowed as a shortcut.");
}

void ShortcutKeySequenceEdit::showInvalidFeedback()
{
    setToolTip(invalidToolTip());
    QToolTip::showText(mapToGlobal(rect().bottomLeft()), invalidToolTip(), this);
}

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

    // 标题行：标题 + 右侧拉伸区（供"还原配置"按钮等头部操作使用）。
    auto *header = new QWidget(card);
    header->setObjectName(QStringLiteral("settingsCardHeader"));
    auto *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);

    auto *titleLabel = new QLabel(title, header);
    titleLabel->setObjectName(QStringLiteral("settingsCardTitle"));
    headerLayout->addWidget(titleLabel);
    headerLayout->addStretch();
    layout->addWidget(header);

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

QPushButton *addCardRestoreButton(QFrame *card, const std::function<void()> &restore)
{
    if (!card) {
        return nullptr;
    }

    auto *button = new QPushButton(MS_TR("Restore"), card);
    button->setObjectName(QStringLiteral("settingsCardRestore"));
    button->setCursor(Qt::PointingHandCursor);
    button->setFocusPolicy(Qt::NoFocus);
    button->setToolTip(MS_TR("Restore this section to the currently saved configuration."));

    if (QWidget *header = card->findChild<QWidget *>(QStringLiteral("settingsCardHeader"))) {
        if (auto *headerLayout = qobject_cast<QHBoxLayout *>(header->layout())) {
            // 插到标题行末尾的拉伸项之前。
            headerLayout->insertWidget(headerLayout->count() - 1, button);
        }
    }

    if (restore) {
        QObject::connect(button, &QPushButton::clicked, card, [restore] { restore(); });
    }
    return button;
}

QPushButton *addPageRestoreButton(QVBoxLayout *layout, const std::function<void()> &restore)
{
    if (!layout) {
        return nullptr;
    }

    auto *row = new QWidget;
    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(8);
    rowLayout->addStretch();

    auto *button = new QPushButton(MS_TR("Restore Page"), row);
    button->setObjectName(QStringLiteral("settingsCardRestore"));
    button->setCursor(Qt::PointingHandCursor);
    button->setFocusPolicy(Qt::NoFocus);
    button->setToolTip(MS_TR("Restore this page to the currently saved configuration."));
    rowLayout->addWidget(button);

    layout->addWidget(row);
    if (restore) {
        QObject::connect(button, &QPushButton::clicked, row, [restore] { restore(); });
    }
    return button;
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
    // 源头级滚轮防护：无论聚焦与否，滚轮都不再篡改数值。
    suppressWheelOn(spin);
    form->addRow(label, spin);
    return spin;
}

QDoubleSpinBox *addDoubleRow(QFormLayout *form, const QString &label, double minimum, double maximum, int decimals)
{
    auto *spin = new QDoubleSpinBox;
    spin->setRange(minimum, maximum);
    spin->setDecimals(decimals);
    spin->setContextMenuPolicy(Qt::NoContextMenu);
    // 源头级滚轮防护：无论聚焦与否，滚轮都不再篡改数值。
    suppressWheelOn(spin);
    form->addRow(label, spin);
    return spin;
}

QComboBox *addComboRow(QFormLayout *form, const QString &label)
{
    auto *combo = new QComboBox;
    combo->setCursor(Qt::PointingHandCursor);
    combo->setContextMenuPolicy(Qt::NoContextMenu);
    // 源头级滚轮防护：Qt 6 的 QComboBox 未聚焦也会被滚轮改选中项。
    suppressWheelOn(combo);
    form->addRow(label, combo);
    return combo;
}

ShortcutKeySequenceEdit *addShortcutRow(QFormLayout *form, const QString &label, bool globalHotkey)
{
    auto *edit = new ShortcutKeySequenceEdit(globalHotkey);
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
