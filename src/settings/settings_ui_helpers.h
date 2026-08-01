#pragma once

#include <QColor>
#include <QKeySequence>
#include <QKeySequenceEdit>
#include <QString>

#include <functional>

class QCheckBox;
class QComboBox;
class QFormLayout;
class QFrame;
class QKeyEvent;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QDoubleSpinBox;
class QSpinBox;
class QVBoxLayout;
class QWidget;

namespace markshot::settings {

/// @brief 提供合法快捷键校验与即时反馈的快捷键输入控件。
///
/// - 仅允许录入单键组合；
/// - 拒绝纯修饰键组合与无修饰键的危险键（Delete/Backspace/Tab/Enter/方向键等）；
/// - 全局快捷键模式额外要求至少一个修饰键或功能键（防呆，避免全局抢键）；
/// - 非法输入即时回退并弹 tooltip 提示；
/// - Delete/Backspace 无修饰键时用于清空快捷键；
/// - 程序化 setKeySequence 跳过校验，保证已保存配置（如默认的 Escape 取消
///   快捷键）能被原样显示。
class ShortcutKeySequenceEdit final : public QKeySequenceEdit {
public:
    /// @brief 创建快捷键输入控件。
    /// @param globalHotkey 是否为全局快捷键（要求修饰键或功能键）。
    /// @param parent 父控件。
    explicit ShortcutKeySequenceEdit(bool globalHotkey = false, QWidget *parent = nullptr);

    /// @brief 程序化设置快捷键，跳过键盘录入校验（用于加载已保存配置）。
    /// @param sequence 需要显示的快捷键序列。
    void setKeySequence(const QKeySequence &sequence);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    /// @brief 返回当前模式下的非法输入提示文案。
    /// @return 提示文本。
    QString invalidToolTip() const;

    /// @brief 显示非法输入提示。
    void showInvalidFeedback();

    bool m_globalHotkey = false;
    QKeySequence m_lastValid;
};

/// @brief 创建设置页根布局。
/// @param parent 父控件。
/// @return 垂直布局。
QVBoxLayout *createSettingsPageLayout(QWidget *parent);

/// @brief 创建设置分组卡片。
/// @param title 分组标题。
/// @param description 分组描述。
/// @param parent 父控件。
/// @return 卡片控件。
QFrame *createSettingsCard(const QString &title, const QString &description, QWidget *parent);

/// @brief 获取卡片中的表单布局。
/// @param card 卡片控件。
/// @return 表单布局。
QFormLayout *settingsCardForm(QFrame *card);

/// @brief 在卡片标题行右侧添加"还原配置"按钮。
/// @param card 卡片控件。
/// @param restore 还原回调，将控件值恢复为最近一次已保存配置。
/// @return 还原按钮。
QPushButton *addCardRestoreButton(QFrame *card, const std::function<void()> &restore);

/// @brief 在设置页底部添加"还原页面"按钮行。
/// @param layout 页面根布局。
/// @param restore 还原回调，将整页控件恢复为最近一次已保存配置。
/// @return 还原按钮。
QPushButton *addPageRestoreButton(QVBoxLayout *layout, const std::function<void()> &restore);

/// @brief 添加开关表单项。
/// @param form 表单布局。
/// @param label 标签文本。
/// @param description 控件说明。
/// @return 复选框控件。
QCheckBox *addSwitchRow(QFormLayout *form, const QString &label, const QString &description = {});

/// @brief 添加文本输入表单项。
/// @param form 表单布局。
/// @param label 标签文本。
/// @param placeholder 占位文本。
/// @return 文本输入控件。
QLineEdit *addTextRow(QFormLayout *form, const QString &label, const QString &placeholder = {});

/// @brief 添加多行文本输入表单项。
/// @param form 表单布局。
/// @param label 标签文本。
/// @param placeholder 占位文本。
/// @return 多行文本输入控件。
QPlainTextEdit *addPlainTextRow(QFormLayout *form, const QString &label, const QString &placeholder = {});

/// @brief 添加数值输入表单项。
/// @param form 表单布局。
/// @param label 标签文本。
/// @param minimum 最小值。
/// @param maximum 最大值。
/// @param suffix 数值后缀。
/// @return 数值输入控件。
QSpinBox *addSpinRow(QFormLayout *form, const QString &label, int minimum, int maximum, const QString &suffix = {});

/// @brief 添加小数输入表单项。
/// @param form 表单布局。
/// @param label 标签文本。
/// @param minimum 最小值。
/// @param maximum 最大值。
/// @param decimals 小数位数。
/// @return 小数输入控件。
QDoubleSpinBox *addDoubleRow(QFormLayout *form, const QString &label, double minimum, double maximum, int decimals);

/// @brief 添加下拉选择表单项。
/// @param form 表单布局。
/// @param label 标签文本。
/// @return 下拉框控件。
QComboBox *addComboRow(QFormLayout *form, const QString &label);

/// @brief 添加快捷键输入表单项。
/// @param form 表单布局。
/// @param label 标签文本。
/// @param globalHotkey 是否为全局快捷键（要求修饰键或功能键，防呆）。
/// @return 快捷键输入控件。
ShortcutKeySequenceEdit *addShortcutRow(QFormLayout *form, const QString &label, bool globalHotkey = false);

/// @brief 创建颜色选择按钮样式。
/// @param color 当前颜色。
/// @return 按钮样式表。
QString colorButtonStyleSheet(const QColor &color);

}  // namespace markshot::settings
