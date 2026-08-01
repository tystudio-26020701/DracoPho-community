#include "settings/settings_page_shortcuts.h"

#include "settings/settings_ui_helpers.h"
#include "ui/i18n.h"

#include <QFormLayout>
#include <QFrame>
#include <QKeySequenceEdit>
#include <QToolTip>
#include <QVBoxLayout>

namespace markshot::settings {
namespace {

/// @brief 返回设置页支持编辑的工具列表。
/// @return 标注工具数组。
std::array<ShotWindow::Tool, static_cast<int>(ShotWindow::Tool::Laser) + 1> shortcutTools()
{
    return {ShotWindow::Tool::Move,
            ShotWindow::Tool::Select,
            ShotWindow::Tool::Pen,
            ShotWindow::Tool::Line,
            ShotWindow::Tool::Highlighter,
            ShotWindow::Tool::Rectangle,
            ShotWindow::Tool::Ellipse,
            ShotWindow::Tool::Arrow,
            ShotWindow::Tool::Text,
            ShotWindow::Tool::Number,
            ShotWindow::Tool::Mosaic,
            ShotWindow::Tool::Magnifier,
            ShotWindow::Tool::Laser};
}

/// @brief 返回设置页支持编辑的动作列表。
/// @return 标注动作数组。
std::array<ShotWindow::Action, 15> shortcutActions()
{
    return {ShotWindow::Action::ToggleCaptureScope,
            ShotWindow::Action::ToggleToolbarLayout,
            ShotWindow::Action::Clear,
            ShotWindow::Action::Undo,
            ShotWindow::Action::Redo,
            ShotWindow::Action::OpenWith,
            ShotWindow::Action::Extensions,
            ShotWindow::Action::Pin,
            ShotWindow::Action::ScrollCapture,
            ShotWindow::Action::OcrCopy,
            ShotWindow::Action::Copy,
            ShotWindow::Action::Save,
            ShotWindow::Action::Upload,
            ShotWindow::Action::Settings,
            ShotWindow::Action::Cancel};
}

/// @brief 返回工具名称的本地化文本。
/// @param tool 工具枚举值。
/// @return 本地化后的工具名称。
QString toolLabel(ShotWindow::Tool tool)
{
    switch (tool) {
    case ShotWindow::Tool::Move: return MS_TR("Move");
    case ShotWindow::Tool::Select: return MS_TR("Select");
    case ShotWindow::Tool::Pen: return MS_TR("Pen");
    case ShotWindow::Tool::Line: return MS_TR("Line");
    case ShotWindow::Tool::Highlighter: return MS_TR("Highlighter");
    case ShotWindow::Tool::Rectangle: return MS_TR("Rect");
    case ShotWindow::Tool::Ellipse: return MS_TR("Ellipse");
    case ShotWindow::Tool::Arrow: return MS_TR("Arrow");
    case ShotWindow::Tool::Text: return MS_TR("Text");
    case ShotWindow::Tool::Number: return MS_TR("Number");
    case ShotWindow::Tool::Mosaic: return MS_TR("Mosaic");
    case ShotWindow::Tool::Magnifier: return MS_TR("Magnifier");
    case ShotWindow::Tool::Laser: return MS_TR("Laser");
    }
    return MS_TR("Pen");
}

/// @brief 返回动作名称的本地化文本。
/// @param action 动作枚举值。
/// @return 本地化后的动作名称。
QString actionLabel(ShotWindow::Action action)
{
    switch (action) {
    case ShotWindow::Action::ToggleCaptureScope: return MS_TR("Toggle Capture Scope");
    case ShotWindow::Action::ToggleToolbarLayout: return MS_TR("Toggle Toolbar Layout");
    case ShotWindow::Action::Clear: return MS_TR("Clear");
    case ShotWindow::Action::Undo: return MS_TR("Undo");
    case ShotWindow::Action::Redo: return MS_TR("Redo");
    case ShotWindow::Action::OpenWith: return MS_TR("Open With");
    case ShotWindow::Action::Extensions: return MS_TR("Extensions");
    case ShotWindow::Action::Pin: return MS_TR("Pin");
    case ShotWindow::Action::ScrollCapture: return MS_TR("Scroll Capture");
    case ShotWindow::Action::OcrCopy: return MS_TR("OCR Copy");
    case ShotWindow::Action::Copy: return MS_TR("Copy");
    case ShotWindow::Action::Save: return MS_TR("Save");
    case ShotWindow::Action::Upload: return MS_TR("Upload");
    case ShotWindow::Action::Settings: return MS_TR("Settings");
    case ShotWindow::Action::Cancel: return MS_TR("Cancel");
    default: break;
    }
    return MS_TR("Copy");
}

}  // namespace

SettingsPageShortcuts::SettingsPageShortcuts(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = createSettingsPageLayout(this);

    QFrame *toolCard = createSettingsCard(MS_TR("Tool Shortcuts"),
                                          MS_TR("Configure shortcuts used while editing a selected screenshot."),
                                          this);
    addToolShortcutRows(settingsCardForm(toolCard));
    addCardRestoreButton(toolCard, [this] {
        for (ShotWindow::Tool tool : shortcutTools()) {
            ShortcutKeySequenceEdit *edit = m_toolEdits.at(shortcut::toolIndex(tool));
            if (edit) {
                edit->setKeySequence(m_saved.shortcuts.tools.at(shortcut::toolIndex(tool)));
            }
        }
        refreshShortcutConflicts();
    });
    layout->addWidget(toolCard);

    QFrame *actionCard = createSettingsCard(MS_TR("Action Shortcuts"),
                                            MS_TR("Configure screenshot action shortcuts."),
                                            this);
    addActionShortcutRows(settingsCardForm(actionCard));
    addCardRestoreButton(actionCard, [this] {
        for (ShotWindow::Action action : shortcutActions()) {
            ShortcutKeySequenceEdit *edit = m_actionEdits.at(shortcut::actionIndex(action));
            if (edit) {
                edit->setKeySequence(m_saved.shortcuts.actions.at(shortcut::actionIndex(action)));
            }
        }
        refreshShortcutConflicts();
    });
    layout->addWidget(actionCard);

    QFrame *startupCard = createSettingsCard(MS_TR("Startup Tool Shortcuts"),
                                             MS_TR("Configure shortcuts available before a region is selected."),
                                             this);
    QFormLayout *startupForm = settingsCardForm(startupCard);
    m_startupColorPicker = addShortcutRow(startupForm, MS_TR("Color Picker"));
    m_startupRuler = addShortcutRow(startupForm, MS_TR("Ruler"));
    m_startupCodeScanner = addShortcutRow(startupForm, MS_TR("Code Scanner"));
    m_startupDisplayCapture = addShortcutRow(startupForm, MS_TR("Display Capture"));
    m_startupGifRecorder = addShortcutRow(startupForm, MS_TR("GIF Recording"));
    m_startupVideoRecorder = addShortcutRow(startupForm, MS_TR("Video Recording"));
    addCardRestoreButton(startupCard, [this] {
        m_startupColorPicker->setKeySequence(m_saved.shortcuts.startupColorPicker);
        m_startupRuler->setKeySequence(m_saved.shortcuts.startupRuler);
        m_startupCodeScanner->setKeySequence(m_saved.shortcuts.startupCodeScanner);
        m_startupDisplayCapture->setKeySequence(m_saved.shortcuts.startupDisplayCapture);
        m_startupGifRecorder->setKeySequence(m_saved.shortcuts.startupGifRecorder);
        m_startupVideoRecorder->setKeySequence(m_saved.shortcuts.startupVideoRecorder);
        refreshShortcutConflicts();
    });
    layout->addWidget(startupCard);
    addPageRestoreButton(layout, [this] { setConfig(m_saved); });
    layout->addStretch();

    connectShortcutConflictChecks();
}

void SettingsPageShortcuts::setConfig(const SettingsConfig &config)
{
    m_saved = config;
    for (ShotWindow::Tool tool : shortcutTools()) {
        ShortcutKeySequenceEdit *edit = m_toolEdits.at(shortcut::toolIndex(tool));
        if (edit) {
            edit->setKeySequence(config.shortcuts.tools.at(shortcut::toolIndex(tool)));
        }
    }
    for (ShotWindow::Action action : shortcutActions()) {
        ShortcutKeySequenceEdit *edit = m_actionEdits.at(shortcut::actionIndex(action));
        if (edit) {
            edit->setKeySequence(config.shortcuts.actions.at(shortcut::actionIndex(action)));
        }
    }
    m_startupColorPicker->setKeySequence(config.shortcuts.startupColorPicker);
    m_startupRuler->setKeySequence(config.shortcuts.startupRuler);
    m_startupCodeScanner->setKeySequence(config.shortcuts.startupCodeScanner);
    m_startupDisplayCapture->setKeySequence(config.shortcuts.startupDisplayCapture);
    m_startupGifRecorder->setKeySequence(config.shortcuts.startupGifRecorder);
    m_startupVideoRecorder->setKeySequence(config.shortcuts.startupVideoRecorder);
    refreshShortcutConflicts();
}

void SettingsPageShortcuts::updateConfig(SettingsConfig *config) const
{
    if (!config) {
        return;
    }

    for (ShotWindow::Tool tool : shortcutTools()) {
        ShortcutKeySequenceEdit *edit = m_toolEdits.at(shortcut::toolIndex(tool));
        if (edit) {
            config->shortcuts.tools[shortcut::toolIndex(tool)] = edit->keySequence();
        }
    }
    for (ShotWindow::Action action : shortcutActions()) {
        ShortcutKeySequenceEdit *edit = m_actionEdits.at(shortcut::actionIndex(action));
        if (edit) {
            config->shortcuts.actions[shortcut::actionIndex(action)] = edit->keySequence();
        }
    }
    config->shortcuts.startupColorPicker = m_startupColorPicker->keySequence();
    config->shortcuts.startupRuler = m_startupRuler->keySequence();
    config->shortcuts.startupCodeScanner = m_startupCodeScanner->keySequence();
    config->shortcuts.startupDisplayCapture = m_startupDisplayCapture->keySequence();
    config->shortcuts.startupGifRecorder = m_startupGifRecorder->keySequence();
    config->shortcuts.startupVideoRecorder = m_startupVideoRecorder->keySequence();
}

void SettingsPageShortcuts::addToolShortcutRows(QFormLayout *form)
{
    if (!form) {
        return;
    }

    for (ShotWindow::Tool tool : shortcutTools()) {
        m_toolEdits[shortcut::toolIndex(tool)] = addShortcutRow(form, toolLabel(tool));
    }
}

void SettingsPageShortcuts::addActionShortcutRows(QFormLayout *form)
{
    if (!form) {
        return;
    }

    for (ShotWindow::Action action : shortcutActions()) {
        m_actionEdits[shortcut::actionIndex(action)] = addShortcutRow(form, actionLabel(action));
    }
}

QList<ShortcutKeySequenceEdit *> SettingsPageShortcuts::allShortcutEdits() const
{
    QList<ShortcutKeySequenceEdit *> edits;
    for (ShortcutKeySequenceEdit *edit : m_toolEdits) {
        if (edit) {
            edits.append(edit);
        }
    }
    for (ShortcutKeySequenceEdit *edit : m_actionEdits) {
        if (edit) {
            edits.append(edit);
        }
    }
    for (ShortcutKeySequenceEdit *edit : {m_startupColorPicker,
                                          m_startupRuler,
                                          m_startupCodeScanner,
                                          m_startupDisplayCapture,
                                          m_startupGifRecorder,
                                          m_startupVideoRecorder}) {
        if (edit) {
            edits.append(edit);
        }
    }
    return edits;
}

void SettingsPageShortcuts::connectShortcutConflictChecks()
{
    for (ShortcutKeySequenceEdit *edit : allShortcutEdits()) {
        connect(edit, &QKeySequenceEdit::keySequenceChanged, this, &SettingsPageShortcuts::refreshShortcutConflicts);
    }
}

void SettingsPageShortcuts::refreshShortcutConflicts()
{
    const QList<ShortcutKeySequenceEdit *> edits = allShortcutEdits();
    const QString conflictMessage = MS_TR("This shortcut is already assigned to another action.");

    // 全量重扫：任何一项变化后，同时刷新所有项，避免改动某项后
    // 其他项残留过期的"已分配"提示。
    QList<ShortcutKeySequenceEdit *> conflicted;
    for (ShortcutKeySequenceEdit *edit : edits) {
        const QKeySequence current = edit->keySequence();
        if (current.isEmpty()) {
            continue;
        }
        for (QKeySequenceEdit *other : edits) {
            if (other == edit || other->keySequence().isEmpty()) {
                continue;
            }
            if (other->keySequence() == current) {
                conflicted.append(edit);
                break;
            }
        }
    }
    for (ShortcutKeySequenceEdit *edit : edits) {
        const bool hasConflict = conflicted.contains(edit);
        edit->setToolTip(hasConflict ? conflictMessage : QString());
        if (hasConflict && edit->hasFocus()) {
            QToolTip::showText(edit->mapToGlobal(edit->rect().bottomLeft()),
                               conflictMessage,
                               edit);
        }
    }
}

}  // namespace markshot::settings
