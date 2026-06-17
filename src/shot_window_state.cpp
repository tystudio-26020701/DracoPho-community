#include "shot_window_module.h"

#include "annotation_state_store.h"
#include "debug_log.h"

namespace cfg = markshot::config;
namespace shortcuts = markshot::shortcut;
using namespace markshot::shot;

/// @brief 在构造阶段从磁盘载入工具默认值,覆盖 m_* 内置默认值
///
/// 该函数仅修改"工具默认值"成员,不会触碰 UI 面板;UI 在 setup 末尾按当前
/// 默认值刷新一次,因此调用时机必须早于 initializeToolbar 之后的属性面板刷新,
/// 又必须晚于 initializeShortcuts(避免被快捷键配置回填)。约定在 setup 流程
/// 紧随 m_toolbarAppearance 读取之后调用。
void ShotWindow::loadAnnotationStateFromDisk()
{
    // 1. 从磁盘读取(文件不存在则得到结构默认值)
    const markshot::AnnotationState state = markshot::loadAnnotationState();

    // 2. 颜色与不透明度
    if (state.currentColor.isValid()) {
        m_currentColor = state.currentColor;
    }
    m_textBackgroundColor = state.textBackgroundColor;

    // 3. 各工具笔宽
    m_penWidth = state.penWidth;
    m_shapeWidth = state.shapeWidth;
    m_numberWidth = state.numberWidth;
    m_mosaicBlockSize = state.mosaicBlockSize;
    m_laserWidth = state.laserWidth;

    // 4. 矩形相关
    m_shapeFilled = state.shapeFilled;
    m_rectangleCornerRadius = state.rectangleCornerRadius;
    m_rectangleStyle = state.rectangleStyle;

    // 5. 放大镜相关
    m_magnifierScale = state.magnifierScale;
    m_magnifierShape = state.magnifierShape;

    // 6. 各风格枚举
    m_arrowStyle = state.arrowStyle;
    m_highlighterStyle = state.highlighterStyle;
    m_numberStyle = state.numberStyle;

    // 7. 文本字体(空字符串保留主题默认值)
    if (!state.textFontFamily.isEmpty()) {
        m_textFontFamily = state.textFontFamily;
    }
}

/// @brief 把当前工具默认值快照写回磁盘
///
/// 任意改变 m_currentColor / m_penWidth / m_shapeWidth / m_numberWidth /
/// m_mosaicBlockSize / m_laserWidth / m_shapeFilled / m_rectangleCornerRadius /
/// m_rectangleStyle / m_magnifierScale / m_magnifierShape / m_arrowStyle /
/// m_highlighterStyle / m_numberStyle / m_textFontFamily / m_textBackgroundColor
/// 的入口写入完成后都应调用本函数,以保持磁盘文件与内存状态同步。
void ShotWindow::persistAnnotationState()
{
    markshot::AnnotationState state;
    state.currentColor = m_currentColor;
    state.penWidth = m_penWidth;
    state.shapeWidth = m_shapeWidth;
    state.numberWidth = m_numberWidth;
    state.mosaicBlockSize = m_mosaicBlockSize;
    state.laserWidth = m_laserWidth;
    state.shapeFilled = m_shapeFilled;
    state.rectangleCornerRadius = m_rectangleCornerRadius;
    state.rectangleStyle = m_rectangleStyle;
    state.magnifierScale = m_magnifierScale;
    state.magnifierShape = m_magnifierShape;
    state.arrowStyle = m_arrowStyle;
    state.highlighterStyle = m_highlighterStyle;
    state.numberStyle = m_numberStyle;
    state.textFontFamily = m_textFontFamily;
    state.textBackgroundColor = m_textBackgroundColor;

    // 写入失败仅记录日志,不影响交互;下次任意修改会再次尝试
    if (!markshot::saveAnnotationState(state)) {
        markshot::debugLog("annotation_state",
                           "failed to persist annotation state to %s",
                           markshot::annotationStateFilePath().toUtf8().constData());
    }
}
