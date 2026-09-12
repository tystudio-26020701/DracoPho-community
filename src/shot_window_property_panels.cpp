#include "shot_window_module.h"

namespace cfg = markshot::config;
namespace shortcuts = markshot::shortcut;
using namespace markshot::shot;

void ShotWindow::setSelectedAnnotationOpacity(int opacity)
{
    // 决-A:此滑条是"文本框透明度"——整个标注的合成不透明度,
    // 与文字透明度(颜色对话框的 alpha 滑条)完全独立。
    opacity = std::clamp(opacity, 0, 100);
    const qreal normalized = opacity / 100.0;
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        bool changed = false;
        for (int id : selectedIds) {
            const Annotation *annotation = annotationById(id);
            if (annotation && !qFuzzyCompare(annotation->opacity, normalized)) {
                changed = true;
                break;
            }
        }
        if (!changed) {
            return;
        }
        pushHistorySnapshot();
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id)) {
                annotation->opacity = normalized;
            }
        }
    } else {
        if (qFuzzyCompare(m_defaultAnnotationOpacity, normalized)) {
            return;
        }
        m_defaultAnnotationOpacity = normalized;
    }

    if (m_draft.has_value()) {
        m_draft->opacity = normalized;
    }
    if (m_propertyColorPicker && m_propertyColorDialogPanel && m_propertyColorDialogPanel->isVisible()
        && m_propertyColorDialogMode == ColorModeObject) {
        const QSignalBlocker blocker(m_propertyColorPicker);
        m_propertyColorPicker->setColor(selectedIds.isEmpty() ? m_currentColor : annotationById(selectedIds.first())->color);
    }
    updateColorPalettePreview();
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}

void ShotWindow::setSelectedAnnotationFilled(bool filled)
{
    if (m_selectedAnnotationId.has_value()) {
        Annotation *annotation = annotationById(*m_selectedAnnotationId);
        if (!annotation || annotation->filled == filled) {
            return;
        }
        if (annotation->tool != Tool::Rectangle && annotation->tool != Tool::Ellipse) {
            return;
        }
        pushHistorySnapshot();
        annotation->filled = filled;
    } else {
        if (m_tool != Tool::Rectangle && m_tool != Tool::Ellipse) {
            return;
        }
        m_shapeFilled = filled;
    }
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}

void ShotWindow::setSelectedAnnotationCornerRadius(int radius)
{
    if (m_selectedAnnotationId.has_value()) {
        Annotation *annotation = annotationById(*m_selectedAnnotationId);
        if (!annotation || annotation->tool != Tool::Rectangle || qRound(annotation->cornerRadius) == radius) {
            return;
        }
        pushHistorySnapshot();
        annotation->cornerRadius = radius;
    } else {
        if (m_tool != Tool::Rectangle || qRound(m_rectangleCornerRadius) == radius) {
            return;
        }
        m_rectangleCornerRadius = radius;
    }
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}

void ShotWindow::setSelectedAnnotationArrowStyle(ArrowStyle style)
{
    if (m_selectedAnnotationId.has_value()) {
        Annotation *annotation = annotationById(*m_selectedAnnotationId);
        if (!annotation || annotation->tool != Tool::Arrow || annotation->arrowStyle == style) {
            return;
        }
        pushHistorySnapshot();
        annotation->arrowStyle = style;
    } else {
        if (m_tool != Tool::Arrow || m_arrowStyle == style) {
            return;
        }
        m_arrowStyle = style;
    }
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}

void ShotWindow::setSelectedRectangleStyle(RectangleStyle style)
{
    // 切换矩形风格(描边/高亮/反色)。多选与单选共用同一逻辑,仅作用于
    // Tool::Rectangle 标注。无矩形选中时只更新工具默认值。
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        bool changed = false;
        // 1. 检测是否真的需要修改,避免无意义的历史快照
        for (int id : selectedIds) {
            const Annotation *annotation = annotationById(id);
            if (annotation && annotation->tool == Tool::Rectangle
                && annotation->rectangleStyle != style) {
                changed = true;
                break;
            }
        }
        if (!changed) {
            return;
        }
        // 2. 写入快照后批量改风格
        pushHistorySnapshot();
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id);
                annotation && annotation->tool == Tool::Rectangle) {
                annotation->rectangleStyle = style;
            }
        }
    } else {
        if (m_tool != Tool::Rectangle || m_rectangleStyle == style) {
            return;
        }
        m_rectangleStyle = style;
    }

    if (m_draft.has_value() && m_draft->tool == Tool::Rectangle) {
        m_draft->rectangleStyle = style;
    }
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}

void ShotWindow::setSelectedHighlighterStyle(HighlighterStyle style)
{
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        bool changed = false;
        for (int id : selectedIds) {
            const Annotation *annotation = annotationById(id);
            if (annotation && annotation->tool == Tool::Highlighter
                && annotation->highlighterStyle != style) {
                changed = true;
                break;
            }
        }
        if (!changed) {
            return;
        }
        pushHistorySnapshot();
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id);
                annotation && annotation->tool == Tool::Highlighter) {
                annotation->highlighterStyle = style;
            }
        }
    } else {
        if (m_tool != Tool::Highlighter || m_highlighterStyle == style) {
            return;
        }
        m_highlighterStyle = style;
    }

    if (m_draft.has_value() && m_draft->tool == Tool::Highlighter) {
        m_draft->highlighterStyle = style;
    }
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}

void ShotWindow::setSelectedNumberStyle(NumberStyle style)
{
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        bool changed = false;
        for (int id : selectedIds) {
            const Annotation *annotation = annotationById(id);
            if (annotation && annotation->tool == Tool::Number
                && annotation->numberStyle != style) {
                changed = true;
                break;
            }
        }
        if (!changed) {
            return;
        }
        pushHistorySnapshot();
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id);
                annotation && annotation->tool == Tool::Number) {
                annotation->numberStyle = style;
            }
        }
    } else {
        if (m_tool != Tool::Number || m_numberStyle == style) {
            return;
        }
        m_numberStyle = style;
    }

    if (m_draft.has_value() && m_draft->tool == Tool::Number) {
        m_draft->numberStyle = style;
    }
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}

void ShotWindow::resetNumberSequence()
{
    if (m_nextNumber == 1) {
        return;
    }

    pushHistorySnapshot();
    m_nextNumber = 1;
    if (m_draft.has_value() && m_draft->tool == Tool::Number) {
        m_draft->number = m_nextNumber;
    }
    updateAnnotationPropertyPanel();
    update();
}

void ShotWindow::setSelectedMagnifierScale(int scaleValue)
{
    const qreal scale = magnifierScaleFromSliderValue(scaleValue);
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        bool changed = false;
        for (int id : selectedIds) {
            const Annotation *annotation = annotationById(id);
            if (annotation && annotation->tool == Tool::Magnifier
                && !qFuzzyCompare(clampedMagnifierScale(annotation->magnifierScale), scale)) {
                changed = true;
                break;
            }
        }
        if (!changed) {
            return;
        }
        pushHistorySnapshot();
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id);
                annotation && annotation->tool == Tool::Magnifier) {
                annotation->magnifierScale = scale;
            }
        }
    } else {
        if (m_tool != Tool::Magnifier || qFuzzyCompare(m_magnifierScale, scale)) {
            return;
        }
        m_magnifierScale = scale;
    }

    if (m_draft.has_value() && m_draft->tool == Tool::Magnifier) {
        m_draft->magnifierScale = scale;
    }
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}

void ShotWindow::setSelectedMagnifierShape(MagnifierShape shape)
{
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        bool changed = false;
        for (int id : selectedIds) {
            const Annotation *annotation = annotationById(id);
            if (annotation && annotation->tool == Tool::Magnifier
                && annotation->magnifierShape != shape) {
                changed = true;
                break;
            }
        }
        if (!changed) {
            return;
        }
        pushHistorySnapshot();
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id);
                annotation && annotation->tool == Tool::Magnifier) {
                annotation->magnifierShape = shape;
            }
        }
    } else {
        if (m_tool != Tool::Magnifier || m_magnifierShape == shape) {
            return;
        }
        m_magnifierShape = shape;
    }

    if (m_draft.has_value() && m_draft->tool == Tool::Magnifier) {
        m_draft->magnifierShape = shape;
    }
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}

void ShotWindow::toggleMagnifierShape()
{
    setSelectedMagnifierShape(m_magnifierShape == MagnifierShape::Circle
                                  ? MagnifierShape::Rectangle
                                  : MagnifierShape::Circle);
}

void ShotWindow::deleteSelectedAnnotation()
{
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (selectedIds.isEmpty()) {
        return;
    }
    pushHistorySnapshot();
    for (int i = m_annotations.size() - 1; i >= 0; --i) {
        if (selectedIds.contains(m_annotations.at(i).id)) {
            m_annotations.removeAt(i);
        }
    }
    setSelectedAnnotations({});
    m_annotationDrag = SelectionDrag::None;
    updateAnnotationPropertyPanel();
    updateCursor();
    update();
}

QString ShotWindow::richTextWithHighlight(const QString &html, const QColor &color) const
{
    if (html.isEmpty()) {
        return html;
    }
    // 离线改写整框文字高亮:文档全选合并字符背景,再导出 HTML。
    // alpha 0 视为"清除高亮"。
    QTextDocument document;
    document.setDocumentMargin(0.0);
    document.setHtml(html);
    QTextCursor cursor(&document);
    cursor.select(QTextCursor::Document);
    QTextCharFormat format;
    if (color.isValid() && color.alpha() > 0) {
        format.setBackground(color);
    } else {
        format.clearBackground();
    }
    cursor.mergeCharFormat(format);
    cursor.clearSelection();
    return document.toHtml();
}

void ShotWindow::openSelectedAnnotationColorPalette()
{
    if (!m_propertyColorDialogPanel || !m_propertyColorPicker || !m_annotationPropertyPanel) {
        return;
    }
    const int previousMode = m_propertyColorDialogMode;
    m_propertyColorDialogMode = ColorModeObject;

    if (m_propertyColorDialogPanel->isVisible() && previousMode == ColorModeObject) {
        m_propertyColorDialogPanel->hide();
        return;
    }

    if (m_propertyColorDialogTitle) {
        m_propertyColorDialogTitle->setText(MS_TR("Change selected object color"));
    }
    if (m_colorPalette) {
        m_colorPalette->hide();
    }
    QColor color = m_currentColor;
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        if (const Annotation *annotation = annotationById(selectedIds.first())) {
            color = annotation->color;
        }
    }
    if (m_propertyColorPicker) {
        m_propertyColorPicker->setAlphaToolTip(MS_TR("Text opacity"));
    }
    m_propertyColorEditHistoryCaptured = false;
    {
        const QSignalBlocker blocker(m_propertyColorPicker);
        m_propertyColorPicker->setColor(color);
    }
    updateAnnotationPropertyPanel();
    if (m_annotationPropertyPanel) {
        m_annotationPropertyPanel->show();
        m_annotationPropertyPanel->raise();
        if (QLayout *panelLayout = m_annotationPropertyPanel->layout()) {
            panelLayout->activate();
        }
        updateAnnotationPropertyPanelGeometry();
    }
    if (QLayout *colorLayout = m_propertyColorDialogPanel->layout()) {
        colorLayout->activate();
    }
    updatePropertyColorDialogGeometry();
    m_propertyColorDialogPanel->show();
    updatePropertyColorDialogGeometry();
    m_propertyColorDialogPanel->raise();
    QTimer::singleShot(0, this, [this] {
        if (m_propertyColorDialogPanel && m_propertyColorDialogPanel->isVisible()) {
            updatePropertyColorDialogGeometry();
            m_propertyColorDialogPanel->raise();
        }
    });
}

void ShotWindow::openSelectedTextHighlightPalette()
{
    if (!m_propertyColorDialogPanel || !m_propertyColorPicker || !m_annotationPropertyPanel) {
        return;
    }
    const int previousMode = m_propertyColorDialogMode;
    m_propertyColorDialogMode = ColorModeHighlight;

    if (m_propertyColorDialogPanel->isVisible() && previousMode == ColorModeHighlight) {
        m_propertyColorDialogPanel->hide();
        return;
    }

    if (m_propertyColorDialogTitle) {
        m_propertyColorDialogTitle->setText(MS_TR("Text highlight"));
    }
    if (m_colorPalette) {
        m_colorPalette->hide();
    }
    QColor color = m_textHighlightColor;
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (selectedIds.size() == 1) {
        if (const Annotation *annotation = annotationById(selectedIds.first());
            annotation && annotation->tool == Tool::Text) {
            color = annotation->highlightColor;
        }
    }
    // 高亮未启用时默认透明:按不透明初始化,选色即启用高亮;
    // 透明度仍可用 alpha 滑条显式调
    if (color.isValid() && color.alpha() == 0) {
        color.setAlpha(255);
    }
    if (m_propertyColorPicker) {
        m_propertyColorPicker->setAlphaToolTip(MS_TR("Highlight opacity"));
    }
    m_propertyColorEditHistoryCaptured = false;
    {
        const QSignalBlocker blocker(m_propertyColorPicker);
        m_propertyColorPicker->setColor(color);
    }
    updateAnnotationPropertyPanel();
    if (m_annotationPropertyPanel) {
        m_annotationPropertyPanel->show();
        m_annotationPropertyPanel->raise();
        if (QLayout *panelLayout = m_annotationPropertyPanel->layout()) {
            panelLayout->activate();
        }
        updateAnnotationPropertyPanelGeometry();
    }
    if (QLayout *colorLayout = m_propertyColorDialogPanel->layout()) {
        colorLayout->activate();
    }
    updatePropertyColorDialogGeometry();
    m_propertyColorDialogPanel->show();
    updatePropertyColorDialogGeometry();
    m_propertyColorDialogPanel->raise();
    QTimer::singleShot(0, this, [this] {
        if (m_propertyColorDialogPanel && m_propertyColorDialogPanel->isVisible()) {
            updatePropertyColorDialogGeometry();
            m_propertyColorDialogPanel->raise();
        }
    });
}

void ShotWindow::openSelectedBoxFillPalette()
{
    if (!m_propertyColorDialogPanel || !m_propertyColorPicker || !m_annotationPropertyPanel) {
        return;
    }
    const int previousMode = m_propertyColorDialogMode;
    m_propertyColorDialogMode = ColorModeBoxFill;

    if (m_propertyColorDialogPanel->isVisible() && previousMode == ColorModeBoxFill) {
        m_propertyColorDialogPanel->hide();
        return;
    }

    if (m_propertyColorDialogTitle) {
        m_propertyColorDialogTitle->setText(MS_TR("Text box fill color"));
    }
    if (m_colorPalette) {
        m_colorPalette->hide();
    }
    QColor color = m_textBackgroundColor;
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (selectedIds.size() == 1) {
        if (const Annotation *annotation = annotationById(selectedIds.first());
            annotation && annotation->tool == Tool::Text) {
            color = annotation->backgroundColor;
        }
    }
    // 底色未启用时默认透明:按不透明初始化,选色即启用底色
    if (color.isValid() && color.alpha() == 0) {
        color.setAlpha(255);
    }
    if (m_propertyColorPicker) {
        m_propertyColorPicker->setAlphaToolTip(MS_TR("Fill opacity"));
    }
    m_propertyColorEditHistoryCaptured = false;
    {
        const QSignalBlocker blocker(m_propertyColorPicker);
        m_propertyColorPicker->setColor(color);
    }
    updateAnnotationPropertyPanel();
    if (m_annotationPropertyPanel) {
        m_annotationPropertyPanel->show();
        m_annotationPropertyPanel->raise();
        if (QLayout *panelLayout = m_annotationPropertyPanel->layout()) {
            panelLayout->activate();
        }
        updateAnnotationPropertyPanelGeometry();
    }
    if (QLayout *colorLayout = m_propertyColorDialogPanel->layout()) {
        colorLayout->activate();
    }
    updatePropertyColorDialogGeometry();
    m_propertyColorDialogPanel->show();
    updatePropertyColorDialogGeometry();
    m_propertyColorDialogPanel->raise();
    QTimer::singleShot(0, this, [this] {
        if (m_propertyColorDialogPanel && m_propertyColorDialogPanel->isVisible()) {
            updatePropertyColorDialogGeometry();
            m_propertyColorDialogPanel->raise();
        }
    });
}

void ShotWindow::toggleSelectedTextFontPanel()
{
    if (!m_propertyFontPanel || !m_propertyFontList || !m_propertyFontButton) {
        return;
    }

    if (m_propertyFontPanel->isVisible()) {
        m_propertyFontPanel->hide();
        return;
    }

    if (m_propertyColorDialogPanel) {
        m_propertyColorDialogPanel->hide();
    }
    updateAnnotationPropertyPanel();
    if (QLayout *fontLayout = m_propertyFontPanel->layout()) {
        fontLayout->activate();
    }
    updatePropertyFontPanelGeometry();
    m_propertyFontPanel->show();
    updatePropertyFontPanelGeometry();
    m_propertyFontPanel->raise();
    // 决-7:记录打开时的字体族与字号,供"取消"按钮还原
    m_fontPanelInitialFamily.clear();
    if (const QListWidgetItem *current = m_propertyFontList->currentItem()) {
        m_fontPanelInitialFamily = current->data(Qt::UserRole).toString();
    }
    m_fontPanelInitialFontSize = 0.0;
    if (m_propertyFontSizeEdit) {
        bool ok = false;
        const qreal parsed = m_propertyFontSizeEdit->text().toDouble(&ok);
        m_fontPanelInitialFontSize = (ok && parsed > 0.0) ? parsed : 0.0;
    }
}

/// @brief 决-6:颜色作用域判定的唯一入口——文本编辑器可见且其中存在
/// 局部文本选区时,前景色只作用于该选区。快速调色板与属性拾色器共用。
/// @return 编辑器内存在局部文本选区时返回 true
bool ShotWindow::editorTextSelectionActive() const
{
    return m_textEditor && m_textEditor->isVisible()
        && m_textEditor->textCursor().hasSelection();
}

void ShotWindow::applyPropertyColor(QColor color)
{
    if (!color.isValid()) {
        return;
    }
    const QVector<int> selectedIds = selectedAnnotationIds();
    // 决-6:三个模式各自的作用域规则唯一确定——
    //   文字颜色:有局部选区→仅选区;无→整框文字/工具默认
    //   文字高亮:有局部选区→选区字符背景;无→整框文字高亮(span 重写)/工具默认
    //   文本框底色:永远整框,与文字选区无关
    const bool editorSelectionActive = editorTextSelectionActive();
    switch (m_propertyColorDialogMode) {
    case ColorModeHighlight:
        if (editorSelectionActive) {
            m_textEditor->setTextBackgroundColor(color);
        } else if (!selectedIds.isEmpty()) {
            if (!m_propertyColorEditHistoryCaptured) {
                pushHistorySnapshot();
                m_propertyColorEditHistoryCaptured = true;
            }
            for (int id : selectedIds) {
                if (Annotation *annotation = annotationById(id);
                    annotation && annotation->tool == Tool::Text) {
                    annotation->richText = richTextWithHighlight(annotation->richText, color);
                    annotation->highlightColor = color;
                }
            }
        } else {
            m_textHighlightColor = color;
        }
        break;
    case ColorModeBoxFill:
        if (!selectedIds.isEmpty()) {
            if (!m_propertyColorEditHistoryCaptured) {
                pushHistorySnapshot();
                m_propertyColorEditHistoryCaptured = true;
            }
            for (int id : selectedIds) {
                if (Annotation *annotation = annotationById(id);
                    annotation && annotation->tool == Tool::Text) {
                    annotation->backgroundColor = color;
                }
            }
        } else if (m_tool == Tool::Text) {
            m_textBackgroundColor = color;
        }
        break;
    default:
        if (editorSelectionActive) {
            m_textEditor->setTextColor(color);
        } else if (!selectedIds.isEmpty()) {
            if (!m_propertyColorEditHistoryCaptured) {
                pushHistorySnapshot();
                m_propertyColorEditHistoryCaptured = true;
            }
            for (int id : selectedIds) {
                if (Annotation *annotation = annotationById(id)) {
                    annotation->color = color;
                }
            }
        } else {
            m_currentColor = color;
        }
        if (m_draft.has_value()) {
            m_draft->color = color;
        }
        break;
    }
    if (m_textEditor && m_textEditor->isVisible()) {
        if (m_propertyColorDialogMode == ColorModeObject && editorSelectionActive) {
            // 文字颜色(局部):字符格式,提交时以富文本 span 承载
            m_textEditor->setTextColor(color);
        } else if (m_propertyColorDialogMode == ColorModeObject
                   || m_propertyColorDialogMode == ColorModeBoxFill) {
            // 整框基色/底色:同步编辑器样式表,保证所见即所得
            const Annotation *editingAnnotation = m_editingTextAnnotationId.has_value()
                ? annotationById(*m_editingTextAnnotationId)
                : nullptr;
            QColor editorColor = editingAnnotation ? editingAnnotation->color : m_currentColor;
            QColor editorBackgroundColor = editingAnnotation ? editingAnnotation->backgroundColor : m_textBackgroundColor;
            const qreal editorBaseWidth = editingAnnotation ? editingAnnotation->width : m_textSize;
            if (m_propertyColorDialogMode == ColorModeBoxFill) {
                editorBackgroundColor = color;
            } else {
                editorColor = color;
            }
            m_textEditor->setStyleSheet(markshot::theme::textEditorStyleSheet(editorColor, editorBackgroundColor, textFontSizeForWidth(editorBaseWidth)));
        }
        // 文字高亮(整框,编辑态):经编辑器全选合并字符背景,实时可见
        if (m_propertyColorDialogMode == ColorModeHighlight && !editorSelectionActive) {
            QTextCursor cursor = m_textEditor->textCursor();
            cursor.select(QTextCursor::Document);
            QTextCharFormat format;
            if (color.isValid() && color.alpha() > 0) {
                format.setBackground(color);
            } else {
                format.clearBackground();
            }
            cursor.mergeCharFormat(format);
            cursor.clearSelection();
            m_textEditor->setTextCursor(cursor);
        }
    }
    updateColorPalettePreview();
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}

void ShotWindow::clearAnnotations()
{
    commitTextEditor();
    if (m_annotations.isEmpty() && !m_draft.has_value() && m_laserStrokes.isEmpty() && !m_laserDraft.has_value()) {
        return;
    }

    pushHistorySnapshot();
    m_annotations.clear();
    m_draft.reset();
    m_laserStrokes.clear();
    m_laserDraft.reset();
    setSelectedAnnotations({});
    m_annotationDrag = SelectionDrag::None;
    m_annotationSelectionBoxActive = false;
    m_annotationHistoryCaptured = false;
    m_nextNumber = 1;
    m_nextAnnotationId = 1;
    if (m_propertyColorDialogPanel) {
        m_propertyColorDialogPanel->hide();
    }
    if (m_propertyFontPanel) {
        m_propertyFontPanel->hide();
    }
    updateAnnotationPropertyPanel();
    updateCursor();
    update();
}

void ShotWindow::setSelectedTextFontFamily(const QString &fontFamily)
{
    if (fontFamily.isEmpty()) {
        return;
    }

    // 编辑态:作用于编辑器内被选中的局部文本(或光标后的新输入);
    // 决-5:选区操作不写工具默认值,避免污染下一个新建文本框。
    if (m_textEditor && m_textEditor->isVisible()) {
        m_textEditor->setFontFamily(fontFamily);
        updateAnnotationPropertyPanel();
        return;
    }

    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        bool changed = false;
        for (int id : selectedIds) {
            const Annotation *annotation = annotationById(id);
            if (annotation && annotation->tool == Tool::Text && annotation->fontFamily != fontFamily) {
                changed = true;
                break;
            }
        }
        if (!changed) {
            return;
        }
        pushHistorySnapshot();
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id);
                annotation && annotation->tool == Tool::Text) {
                annotation->fontFamily = fontFamily;
            }
        }
    } else {
        if (m_tool != Tool::Text || m_textFontFamily == fontFamily) {
            return;
        }
        m_textFontFamily = fontFamily;
        if (m_textEditor && m_textEditor->isVisible() && !m_editingTextAnnotationId.has_value()) {
            QFont font = m_textEditor->font();
            font.setFamily(m_textFontFamily);
            m_textEditor->setFont(font);
        }
    }
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}

void ShotWindow::applyTextFontSizeFromEdit()
{
    if (!m_propertyFontSizeEdit) {
        return;
    }
    bool ok = false;
    const qreal value = m_propertyFontSizeEdit->text().toDouble(&ok);
    if (!ok || value <= 0.0) {
        const qreal fallback = m_selectedAnnotationId.has_value()
            ? [this]() {
                  const Annotation *annotation = annotationById(*m_selectedAnnotationId);
                  return annotation ? textFontSizeForWidth(annotation->width)
                                    : textFontSizeForWidth(currentToolWidth());
              }()
            : textFontSizeForWidth(currentToolWidth());
        const QSignalBlocker blocker(m_propertyFontSizeEdit);
        m_propertyFontSizeEdit->setText(QString::number(fallback, 'f', 1));
        return;
    }
    setSelectedTextFontSize(value);
}

void ShotWindow::setSelectedTextFontSize(qreal pointSize)
{
    // 渲染字号 = textFontSizeForWidth(annotation.width)，输入框数值直接对应
    // 最终输出大小。宽度下限 1.0 对应 20 pt，更小不可表示。
    pointSize = std::clamp(pointSize, 20.0, 300.0);
    if (m_propertyFontSizeEdit) {
        const QSignalBlocker blocker(m_propertyFontSizeEdit);
        m_propertyFontSizeEdit->setText(QString::number(pointSize, 'f', 1));
    }
    // 编辑态:作用于编辑器内被选中的局部文本(或光标后的新输入),
    // 编辑器与渲染同字号(所见即所得),提交时由富文本 span 承载。
    if (m_textEditor && m_textEditor->isVisible()) {
        m_textEditor->setFontPointSize(pointSize);
        updateAnnotationPropertyPanel();
        return;
    }
    const qreal targetWidth = textWidthForFontSize(pointSize);
    setSelectedAnnotationWidth(qRound(targetWidth));
    update();
}

void ShotWindow::setSelectedTextBold(bool bold)
{
    const QFont::Weight targetWeight = bold ? QFont::DemiBold : QFont::Normal;
    // 编辑态:作用于编辑器内被选中的局部文本(或光标后的新输入);
    // 决-5:不写工具默认值
    if (m_textEditor && m_textEditor->isVisible()) {
        m_textEditor->setFontWeight(static_cast<int>(targetWeight));
        updateAnnotationPropertyPanel();
        return;
    }
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        bool changed = false;
        for (int id : selectedIds) {
            const Annotation *annotation = annotationById(id);
            if (annotation && annotation->tool == Tool::Text && annotation->fontWeight != targetWeight) {
                changed = true;
                break;
            }
        }
        if (!changed) {
            return;
        }
        pushHistorySnapshot();
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id);
                annotation && annotation->tool == Tool::Text) {
                annotation->fontWeight = targetWeight;
            }
        }
    } else {
        if (m_textWeight == targetWeight) {
            return;
        }
        m_textWeight = targetWeight;
    }
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}

void ShotWindow::setSelectedTextItalic(bool italic)
{
    // 编辑态:作用于编辑器内被选中的局部文本(或光标后的新输入);
    // 决-5:不写工具默认值
    if (m_textEditor && m_textEditor->isVisible()) {
        m_textEditor->setFontItalic(italic);
        updateAnnotationPropertyPanel();
        return;
    }
    const QVector<int> selectedIds = selectedAnnotationIds();
    if (!selectedIds.isEmpty()) {
        bool changed = false;
        for (int id : selectedIds) {
            const Annotation *annotation = annotationById(id);
            if (annotation && annotation->tool == Tool::Text && annotation->textItalic != italic) {
                changed = true;
                break;
            }
        }
        if (!changed) {
            return;
        }
        pushHistorySnapshot();
        for (int id : selectedIds) {
            if (Annotation *annotation = annotationById(id);
                annotation && annotation->tool == Tool::Text) {
                annotation->textItalic = italic;
            }
        }
    } else {
        if (m_textItalic == italic) {
            return;
        }
        m_textItalic = italic;
    }
    updateAnnotationPropertyPanel();
    update();
    persistAnnotationState();
}
